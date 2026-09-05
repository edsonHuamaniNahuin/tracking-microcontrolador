# Tracking GPS — Firmware Microcontrolador

Sistema de tracking GPS en tiempo real para embarcaciones marítimas, basado en **WT32-ETH01 V1.4** (ESP32 + Ethernet LAN8720) y **GPS NEO-7M/6M**.

Cada dispositivo se instala en un barco, se conecta por cable Ethernet (UTP) a la red local y envía coordenadas GPS periódicamente al backend (Laravel API) vía HTTP.

## Arquitectura

```
                         ┌──────────────────┐
  GPS NEO-7M ─ UART2 ─→ │   WT32-ETH01     │ ── Ethernet/HTTP ──→  tracking-api (Laravel)
                         │   V1.4           │                          ↓
                         │   (LAN8720 ETH)  │                     tracking-front (React)
                         └──────────────────┘
                                │
                           LED integrado
                         (indicador estado)

                         ┌──────────────────┐
                         │  Fuente PoE       │
                         │  Inyector 48V ──→ Splitter 5V/2A
                         │  (alimentación)   │
                         └──────────────────┘
```

> **Nota:** El conversor USB-TTL FT232RL (YP-05 USB-C) se usa **únicamente** para flashear/programar y monitoreo serial en fase de desarrollo. En producción, el dispositivo se alimenta por PoE.

### Módulos del firmware

| Archivo | Patrón de diseño | Responsabilidad |
|---------|-----------------|-----------------|
| `config.h` | Constantes | Pines, timeouts, Ethernet PHY, valores por defecto del sistema |
| `storage_manager.h` | Singleton + Repository | Persistencia en NVS (Non-Volatile Storage) — sobrevive reinicios y cortes de energía |
| `ethernet_manager.h` | Facade | Ethernet LAN8720 + servidor web de configuración + mDNS |
| `gps_reader.h` | Adapter | Lectura del módulo GPS NEO-7M/6M vía TinyGPS++ (UART2: IO5/IO17) |
| `api_client.h` | Gateway | Cliente HTTP → `POST /api/v1/device/ping` con ArduinoJson |
| `command_handler.h` | Command | Ejecuta órdenes remotas: `reboot`, `update_config`, `open_config_server` |
| `led_indicator.h` | State | LED integrado con patrones de parpadeo según el estado |
| `tracking-microcontrolador.ino` | Orquestador | `setup()` + `loop()` — une todos los módulos |

### Flujo de ejecución

```
BOOT
  │
  ├─ Inicializar: LED, NVS, GPS (UART2: IO5/IO17)
  │
  ├─ ¿Hay token configurado en NVS?
  │     SÍ → Modo normal: conectar Ethernet (DHCP)
  │     NO → MODO INSTALACIÓN:
  │            • Conecta Ethernet esperando cable (30s)
  │            • Con DHCP disponible → usa la IP de la red
  │            • Cable directo a laptop → IP fija 169.254.1.1
  │            • Sirve página web en http://tracking.local
  │              └─ El técnico configura: token, API URL, intervalo
  │                 • Botón "Probar conexión" valida antes de guardar
  │                 • Guardar → auto-reinicio
  │
  ├─ Cargar configuración de NVS
  │
  └─ LOOP (cada N segundos)
       │
       ├─ Servir cliente web de configuración si está activo (handleClient)
       ├─ Atender OTA por red (handleOta)
       ├─ Leer GPS (NMEA por UART2)
       ├─ POST /api/v1/device/ping  →  { lat, lon, gps_chars, ... }
       ├─ Recibir respuesta:
       │     • command: "reboot" | "update_config" | "open_config_server" | null
       │     • config:  { send_interval: 10, api_url: "..." }
       ├─ Ejecutar comando si existe
       ├─ Aplicar nueva config si cambió
       └─ Verificar reset de fábrica (IO0 a GND 3s = borra config)
```

### Comunicación con el API

**Endpoint:** `POST /api/v1/device/ping`

**Autenticación:** Header `X-Device-Token` con el token único del barco (generado desde la app web).

**Payload de envío:**
```json
{
  "lat": 10.496200,
  "lon": -66.880000,
  "speed": 4.48,
  "course": 270.50,
  "altitude": 0.00,
  "satellites": 8,
  "hdop": 1.20,
  "firmware": "1.2.0",
  "device_type": "WT32-ETH01-TRACKER",
  "uptime": 3600
}
```

**Respuesta del API:**
```json
{
  "status": 200,
  "data": {
    "command": null,
    "vessel_id": 1,
    "config": {
      "send_interval": 10
    }
  }
}
```

### Comandos remotos

| Comando | Efecto en el ESP32 |
|---------|--------------------|
| `reboot` | Reinicia el microcontrolador |
| `update_config` | Aplica nueva configuración (intervalo, API URL) desde la respuesta |
| `open_config_server` | Abre el servidor web de configuración para reconfigurar manualmente |

Los comandos se envían desde la app web y el ESP32 los recoge en su siguiente ping.

### Indicador LED

| Patrón | Estado |
|--------|--------|
| Encendido fijo | Funcionando correctamente / Booting |
| Parpadeo rápido | Conectando a Ethernet (DHCP) |
| Parpadeo lento | Esperando señal GPS |
| Parpadeo medio | Servidor web de configuración activo |
| 2 flashes + pausa | Error de red Ethernet |
| 3 flashes + pausa | Error de API |
| 5 flashes + pausa | Token no configurado |

### Configuración y almacenamiento

Toda la configuración se almacena en **NVS** (Non-Volatile Storage) del ESP32, gestionada por `storage_manager.h`. La NVS **persiste tras cortes de energía** — el dispositivo no pierde su configuración al desconectarse.

| Parámetro | Clave NVS | Dónde se configura | Cuándo aplica |
|-----------|-----------|-------------------|---------------|
| Token del dispositivo | `token` | Servidor web config + app web | Al instalar o rotar token |
| URL del API | `api_url` | Servidor web config + comando remoto | Al instalar o cambio de dominio |
| Intervalo de envío | `interval` | Servidor web config + comando remoto | En cualquier momento |
| Nombre del barco | `vessel_name` | Servidor web config | Al instalar |

**Vías de configuración:**
1. **Servidor web (Modo Instalación)** → el dispositivo sin token sirve su página en `http://tracking.local` (o su IP). El técnico completa el formulario y usa el botón **"Probar conexión"** para validar token + URL antes de guardar
2. **Remota** → comandos `update_config` enviados desde la app web a través del API
3. **Import JSON** → pegar configuración completa en formato JSON desde la página web

### Reset de fábrica

El WT32-ETH01 no tiene botón BOOT. Mantener **IO0 conectado a GND durante 3-4 segundos** (con el dispositivo encendido):
- Borra toda la configuración en NVS (token, URL, intervalo, nombre)
- Reinicia el dispositivo
- Al arrancar sin token, entra en **Modo Instalación** automáticamente (página en `tracking.local`)

### Fases del proyecto

| Fase | Alimentación | Programación | Datos |
|------|-------------|-------------|-------|
| **A (Desarrollo)** | USB-C vía FT232RL (YP-05) | IO0→GND + Upload | Serial (debug) + Ethernet |
| **B (Producción)** | PoE: Inyector 48V → Splitter 5V/2A | No aplica (ya flasheado) | 100% Ethernet (RJ45) |

## Tecnologías

- **MCU:** WT32-ETH01 V1.4 (ESP32 + LAN8720)
- **Ethernet PHY:** LAN8720 (RMII, clock GPIO0)
- **GPS:** NEO-7M/6M — UART2 9600 baud (IO5=RX, IO17=TX)
- **Framework:** Arduino (ESP32 Arduino Core)
- **Lenguaje:** C++
- **Persistencia:** NVS (Preferences API del ESP32)
- **Configuración:** Servidor web HTTP + JSON Import

### Librerías

| Librería | Versión mínima | Autor | Uso |
|----------|---------------|-------|-----|
| TinyGPSPlus | 1.x | Mikal Hart | Parsing de datos NMEA del GPS |
| ArduinoJson | 7.x | Benoît Blanchon | Serialización/deserialización JSON |
| ETH, WebServer, ESPmDNS | (ESP32 Core) | Espressif | Ethernet, servidor web, descubrimiento mDNS |

> **Ya no se requiere WiFiManager.** La configuración de red es vía Ethernet (cable UTP), no WiFi.

## Estructura del proyecto

```
tracking-microcontrolador/
├── tracking-microcontrolador.ino   ← Sketch principal (orquestador)
├── config.h                        ← Constantes y valores por defecto
├── storage_manager.h               ← Persistencia NVS (Singleton)
├── ethernet_manager.h              ← Ethernet LAN8720 + servidor web config (Facade)
├── gps_reader.h                    ← Lectura GPS (Adapter)
├── api_client.h                    ← Cliente HTTP (Gateway)
├── command_handler.h               ← Comandos remotos (Command)
├── led_indicator.h                 ← Indicador LED (State)
├── README.md                       ← Este archivo
├── SETUP.md                        ← Guía de instalación y configuración
├── BUILD.md                        ← Guía de compra y ensamblaje
├── .editorconfig
├── .gitignore
└── .vscode/
    └── settings.json
```

## Escalabilidad

El firmware es **idéntico para todos los barcos**. Se flashea una sola vez por dispositivo y la personalización se hace desde el servidor web de configuración o remotamente desde la app web.

```
Mismo firmware (.bin)
  ├─ Barco 1  →  Token ABC  →  Intervalo 10s  →  IP 192.168.1.50
  ├─ Barco 2  →  Token DEF  →  Intervalo 30s  →  IP 192.168.1.51
  └─ Barco N  →  Token GHI  →  Intervalo 15s  →  IP 192.168.1.52
```
