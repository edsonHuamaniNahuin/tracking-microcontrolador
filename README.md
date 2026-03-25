# Tracking GPS — Firmware Microcontrolador

Sistema de tracking GPS en tiempo real para embarcaciones marítimas, basado en **ESP32** y **GPS6MV2**.

Cada dispositivo se instala en un barco, se conecta de forma autónoma a la red WiFi local y envía coordenadas GPS periódicamente al backend (Laravel API) vía HTTP.

## Arquitectura

```
                         ┌──────────────┐
  GPS6MV2 ─── UART ───→ │   ESP32      │ ─── WiFi/HTTP ───→  tracking-api (Laravel)
                         │  DevKit V1   │                         ↓
                         └──────────────┘                    tracking-front (React)
                               │
                          LED integrado
                        (indicador estado)
```

### Módulos del firmware

| Archivo | Patrón de diseño | Responsabilidad |
|---------|-----------------|-----------------|
| `config.h` | Constantes | Pines, timeouts, valores por defecto del sistema |
| `storage_manager.h` | Singleton | Persistencia en NVS (Non-Volatile Storage) — sobrevive reinicios |
| `wifi_portal.h` | Facade | WiFiManager: conexión WiFi + portal cautivo para técnicos |
| `gps_reader.h` | Adapter | Lectura del módulo GPS6MV2 vía TinyGPS++ |
| `api_client.h` | Gateway | Cliente HTTP → `POST /api/v1/device/ping` con ArduinoJson |
| `command_handler.h` | Command | Ejecuta órdenes remotas: `reboot`, `update_config`, `reset_wifi` |
| `led_indicator.h` | State | LED integrado con patrones de parpadeo según el estado |
| `tracking-microcontrolador.ino` | Orquestador | `setup()` + `loop()` — une todos los módulos |

### Flujo de ejecución

```
BOOT
  │
  ├─ Inicializar: LED, NVS, GPS (UART2)
  │
  ├─ ¿Hay WiFi guardado?
  │     SÍ → Conectar automáticamente
  │     NO → Abrir portal cautivo (AP: TRACKING-XXXXXX)
  │            └─ Técnico configura desde su celular
  │
  ├─ Cargar configuración de NVS (token, URL, intervalo)
  │
  └─ LOOP (cada N segundos)
       │
       ├─ Leer GPS
       ├─ POST /api/v1/device/ping  →  { lat, lon, speed, ... }
       ├─ Recibir respuesta:
       │     • command: "reboot" | "update_config" | "reset_wifi" | null
       │     • config:  { send_interval: 10 }
       ├─ Ejecutar comando si existe
       ├─ Aplicar nueva config si cambió
       └─ Verificar botón reset (BOOT 3s = reinicio total)
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
  "firmware": "1.0.0",
  "device_type": "ESP32-GPS-TRACKER",
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
| `update_config` | Recarga el intervalo de envío desde la respuesta del API |
| `reset_wifi` | Borra credenciales WiFi y reabre el portal cautivo |

Los comandos se envían desde la app web y el ESP32 los recoge en su siguiente ping.

### Indicador LED

| Patrón | Estado |
|--------|--------|
| Encendido fijo | Funcionando correctamente / Booting |
| Parpadeo rápido | Conectando a WiFi |
| Parpadeo lento | Esperando señal GPS |
| Parpadeo medio | Portal cautivo activo |
| 2 flashes + pausa | Error de WiFi |
| 3 flashes + pausa | Error de API |
| 5 flashes + pausa | Token no configurado |

### Configuración dinámica

Toda la configuración es **dinámica por barco** — no se necesita re-flashear el firmware para cambiar la red WiFi, el token o el intervalo de envío:

| Parámetro | Dónde se configura | Cuándo aplica |
|-----------|-------------------|---------------|
| Red WiFi (SSID/contraseña) | Portal cautivo | Al instalar en cada barco |
| Token del dispositivo | Portal cautivo + app web | Al instalar o rotar token |
| URL del API | Portal cautivo | Al instalar (o cambio de dominio) |
| Intervalo de envío | Portal cautivo + app web (remoto) | En cualquier momento |
| Nombre del barco | Portal cautivo | Al instalar |

### Reset de fábrica

Mantener presionado el **botón BOOT** del ESP32 durante **3 segundos**:
- Borra toda la configuración (WiFi + NVS)
- Reinicia el dispositivo
- Reabre el portal cautivo para configurar de nuevo

## Tecnologías

- **MCU:** ESP32 (Espressif) — WiFi integrado
- **GPS:** GPS6MV2 (u-blox NEO-6M) — UART 9600 baud
- **Framework:** Arduino
- **Lenguaje:** C++
- **Persistencia:** NVS (Preferences API del ESP32)

### Librerías

| Librería | Versión mínima | Autor | Uso |
|----------|---------------|-------|-----|
| WiFiManager | 2.x | tzapu | Portal cautivo para configuración WiFi |
| TinyGPSPlus | 1.x | Mikal Hart | Parsing de datos NMEA del GPS |
| ArduinoJson | 7.x | Benoît Blanchon | Serialización/deserialización JSON |

## Estructura del proyecto

```
tracking-microcontrolador/
├── tracking-microcontrolador.ino   ← Sketch principal (orquestador)
├── config.h                        ← Constantes y valores por defecto
├── storage_manager.h               ← Persistencia NVS (Singleton)
├── wifi_portal.h                   ← WiFiManager + portal cautivo (Facade)
├── gps_reader.h                    ← Lectura GPS (Adapter)
├── api_client.h                    ← Cliente HTTP (Gateway)
├── command_handler.h               ← Comandos remotos (Command)
├── led_indicator.h                 ← Indicador LED (State)
├── README.md                       ← Este archivo
├── SETUP.md                        ← Guía de instalación y configuración
├── .editorconfig
├── .gitignore
└── .vscode/
    └── settings.json
```

## Escalabilidad

El firmware es **idéntico para todos los barcos**. Se flashea una sola vez y cada dispositivo se personaliza exclusivamente desde el portal cautivo al momento de la instalación. El backend gestiona las diferencias (token, nombre, intervalo) por barco.

```
Mismo firmware (.bin)
  ├─ Barco 1  →  WiFi "Marina-Norte"   →  Token ABC  →  Intervalo 10s
  ├─ Barco 2  →  WiFi "Puerto-Sur"     →  Token DEF  →  Intervalo 30s
  └─ Barco N  →  WiFi "Embarcación-X"  →  Token GHI  →  Intervalo 15s
```
