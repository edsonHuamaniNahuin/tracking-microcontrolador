# Guía de Instalación y Configuración

Guía paso a paso para instalar, configurar y desplegar el sistema de tracking GPS en embarcaciones.

---

## 1. Requisitos de hardware

### Por cada barco

| Componente | Modelo recomendado | Precio aprox. | Notas |
|------------|--------------------|---------------|-------|
| Microcontrolador | **ESP32 DevKit V1** (con USB) | $3–5 USD | Debe tener puerto USB para flashear y alimentar |
| Módulo GPS | **GPS6MV2** (HW-248, u-blox NEO-6M) | $3–5 USD | Antena cerámica incluida |
| Cable USB | Micro-USB o USB-C (según el DevKit) | $1 USD | Para flashear y alimentar desde cargador/powerbank |
| Fuente de alimentación | Cargador USB 5V / Powerbank | $5–10 USD | Mínimo 500mA. En barcos con 12V, usar conversor a 5V USB |
| Cables Dupont | 4 cables hembra-hembra | $1 USD | Conexión GPS ↔ ESP32 |

### Herramientas

- PC con **Arduino IDE 2.x** instalado (solo para flashear, se hace una vez)
- Celular o tablet con WiFi (para configurar cada dispositivo)

---

## 2. Conexiones físicas

4 cables entre el ESP32 y el GPS:

```
ESP32 DevKit V1              GPS6MV2 (HW-248)
───────────────              ─────────────────
  3V3  ──────────────────→   VCC
  GND  ──────────────────→   GND
  GPIO 16 (RX2) ─────────→  TX
  GPIO 17 (TX2) ─────────→  RX
```

> **Importante:** Alimentar el GPS desde el pin **3V3** del ESP32 (no 5V). El GPS6MV2 opera a 3.3V.

### Diagrama visual

```
  ┌──────────────────┐         ┌─────────────────┐
  │   ESP32 DevKit   │         │    GPS6MV2      │
  │                  │         │                 │
  │  3V3  ●──────────┼────────→│ VCC             │
  │  GND  ●──────────┼────────→│ GND             │
  │  D16  ●──────────┼────────→│ TX              │
  │  D17  ●──────────┼────────→│ RX              │
  │                  │         │                 │
  │  [USB] ← PC/5V  │         │   [Antena GPS]  │
  └──────────────────┘         └─────────────────┘
```

---

## 3. Preparar el entorno de desarrollo

### 3.1 Instalar Arduino IDE

Descargar desde: https://www.arduino.cc/en/software

### 3.2 Agregar soporte para ESP32

1. Abrir Arduino IDE
2. **Archivo → Preferencias**
3. En "Gestor de URLs adicionales de tarjetas", agregar:
   ```
   https://espressif.github.io/arduino-esp32/package_esp32_index.json
   ```
4. **Herramientas → Placa → Gestor de tarjetas**
5. Buscar **esp32** (por Espressif) → **Instalar**

### 3.3 Instalar librerías

Desde **Herramientas → Gestor de Librerías**, instalar:

| Librería | Buscar como | Versión |
|----------|------------|---------|
| WiFiManager | `WiFiManager` (por tzapu) | 2.x |
| TinyGPSPlus | `TinyGPSPlus` (por Mikal Hart) | 1.x |
| ArduinoJson | `ArduinoJson` (por Benoît Blanchon) | 7.x |

### 3.4 Configurar la placa

En **Herramientas**:

| Opción | Valor |
|--------|-------|
| Placa | ESP32 Dev Module |
| Upload Speed | 115200 |
| CPU Frequency | 240MHz |
| Flash Frequency | 80MHz |
| Flash Size | 4MB (32Mb) |
| Partition Scheme | Default 4MB with spiffs |
| Puerto | El que aparezca al conectar el USB (ej: COM3) |

---

## 4. Configuración pre-flasheo

### 4.1 URL del API (`config.h`)

Antes de flashear, editar `config.h` y ajustar la URL del backend:

```cpp
// ── Desarrollo local (XAMPP) ──────────────────────────────
#define DEFAULT_API_URL  "http://192.168.1.100:8000/api/v1/device/ping"

// ── Producción (con dominio) ──────────────────────────────
// #define DEFAULT_API_URL  "https://api.tudominio.com/api/v1/device/ping"
```

> **Nota:** Esta URL es solo el valor por defecto. Se puede cambiar **sin re-flashear** desde el portal cautivo de cada dispositivo o remotamente desde la app.

### 4.2 Otros valores ajustables en `config.h`

| Constante | Valor por defecto | Descripción |
|-----------|------------------|-------------|
| `DEFAULT_API_URL` | `http://192.168.1.100:8000/api/v1/device/ping` | Endpoint del backend |
| `DEFAULT_SEND_INTERVAL` | `10000` (10s) | Cada cuántos ms envía datos. Configurable remotamente. |
| `GPS_RX_PIN` / `GPS_TX_PIN` | `16` / `17` | Pines UART del GPS. Cambiar solo si usas otros pines. |
| `AP_PASSWORD` | `tracking123` | Contraseña del portal cautivo. Cambiar para seguridad. |
| `AP_TIMEOUT_SECONDS` | `180` (3 min) | Tiempo antes de cerrar el portal si nadie configura. |

---

## 5. Flashear el firmware

1. Conectar el ESP32 al PC por USB
2. Abrir `tracking-microcontrolador.ino` en Arduino IDE
3. Seleccionar la placa y puerto correcto
4. Presionar **Upload** (→)
5. Esperar a que diga "Done uploading"
6. Abrir **Monitor Serial** (115200 baud) para verificar que arranca

Salida esperada al primer arranque:
```
╔══════════════════════════════════════════╗
║  Tracking GPS  v1.0.0                   ║
║  ESP32 + GPS6MV2 → WiFi → API          ║
╚══════════════════════════════════════════╝
[NVS] Configuracion cargada
[GPS] UART2 iniciado (RX=16, TX=17, 9600 baud)
[MAIN] Sin token. El tecnico debe configurar via portal.
[WIFI] AP: TRACKING-A1B2C3
```

> **El firmware se flashea una sola vez.** Los mismos archivos sirven para todos los barcos. La personalización por barco se hace en el paso siguiente.

---

## 6. Configuración por barco (Portal Cautivo)

Este es el paso que se repite **en cada barco** donde se instale un dispositivo.

### 6.1 Generar el token del dispositivo (desde la app web)

Antes de ir al barco, desde la aplicación web:

1. Ir al módulo de embarcaciones
2. Seleccionar el barco
3. Sección "Dispositivo IoT" → **Generar Token**
4. Copiar el token (ej: `a3f8b2c1-9d4e-4f7a-b5c6-1234567890ab`)
5. **Tener a mano el dominio del servidor** (ej: `https://api.tudominio.com`)

### 6.2 Configurar el ESP32 con el celular

1. Encender el ESP32 (conectar a corriente USB)
2. Desde el celular, ir a **Configuración WiFi**
3. Buscar la red `TRACKING-XXXXXX` (las X son los últimos 6 dígitos de la MAC)
4. Conectarse con contraseña: `tracking123`
5. Se abre automáticamente el **portal cautivo** en el navegador
6. Llenar los campos:

| Campo | Qué poner | Ejemplo |
|-------|-----------|---------|
| **WiFi SSID** | Red WiFi del barco | `Marina-Norte-5G` |
| **WiFi Password** | Contraseña de esa red | `clave-del-barco` |
| **Token del dispositivo** | El token generado en paso 6.1 | `a3f8b2c1-9d4e...` |
| **URL del API** | **Dominio completo** del backend + ruta del endpoint | `https://api.tudominio.com/api/v1/device/ping` |
| **Intervalo de envío (seg)** | Cada cuántos segundos enviar datos | `10` |
| **Nombre embarcación** | Nombre identificador (opcional) | `Lancha Esperanza` |

> **Sobre la URL del API:** El operador **siempre debe ingresar el dominio completo** del servidor. En desarrollo es algo como `http://192.168.1.100:8000/api/v1/device/ping`. En producción será `https://api.tudominio.com/api/v1/device/ping`. La ruta siempre termina en `/api/v1/device/ping`.
7. Presionar **Save**
8. El ESP32 se reinicia, se conecta al WiFi del barco y empieza a enviar datos

### 6.3 Verificar que funciona

**Opción A — Monitor Serial (si tienes PC conectado):**
```
[WIFI] Conectado. IP: 192.168.1.45
[GPS] OK (8 satelites)
[API] POST https://api.tudominio.com/api/v1/device/ping
[API] OK (200): cmd=
```

**Opción B — LED indicador:**
- LED encendido fijo = todo funcionando
- Parpadeo lento = esperando GPS (normal los primeros 1-2 minutos al aire libre)

**Opción C — Desde la app web:**
- Ir al barco → Sección "Dispositivo" → ver estado "Online" con IP y último ping

---

## 7. Gestión remota (desde la app web)

Una vez instalado, se puede gestionar cada dispositivo sin ir físicamente al barco:

### 7.1 Ver estado del dispositivo

```
GET /api/v1/vessels/{id}/device/status
```

Retorna: online/offline, firmware, IP local, uptime, último contacto.

### 7.2 Cambiar intervalo de envío

```
PUT /api/v1/vessels/{id}/device/config
Content-Type: application/json

{ "send_interval": 30 }
```

El dispositivo aplica el cambio en su siguiente ping (máximo espera = intervalo actual).

Valores válidos: **5** a **3600** segundos.

| Escenario | Intervalo recomendado |
|-----------|----------------------|
| Navegación activa | 5–10 segundos |
| Fondeado / en puerto | 30–60 segundos |
| Ahorro de datos/energía | 120–300 segundos |

### 7.3 Reiniciar el dispositivo

```
POST /api/v1/vessels/{id}/device/command
Content-Type: application/json

{ "command": "reboot" }
```

### 7.4 Reset WiFi remoto

```
POST /api/v1/vessels/{id}/device/command
Content-Type: application/json

{ "command": "reset_wifi" }
```

El dispositivo borra las credenciales WiFi y reabre el portal cautivo. Útil si el barco cambia de red WiFi y alguien está físicamente presente para reconfigurar.

### 7.5 Rotar token de seguridad

```
POST /api/v1/vessels/{id}/device/token/regen
```

Genera un nuevo token. El dispositivo actual dejará de autenticarse hasta que se reconfigure con el nuevo token (vía portal cautivo o reset físico).

---

## 8. Despliegue en producción

### 8.1 Backend con dominio

Cuando el backend esté desplegado con un dominio (ej: `api.tudominio.com`), hay dos maneras de configurar los dispositivos:

**Opción A — Antes de flashear (recomendado para flotas nuevas):**

Cambiar en `config.h`:
```cpp
#define DEFAULT_API_URL  "https://api.tudominio.com/api/v1/device/ping"
```
Así cada nuevo dispositivo tendrá la URL correcta por defecto.

**Opción B — Sin re-flashear (dispositivos ya instalados):**

Cada dispositivo ya en campo puede actualizarse de dos maneras:
1. **Portal cautivo**: Reset físico (botón BOOT 3s) → reconfigurar con la nueva URL
2. **Si aún puede comunicarse con el API anterior**: Enviar comando `update_config` para que recargue la config (la URL en sí se cambia desde el portal, no remotamente)

### 8.2 HTTPS

En producción, el backend debe usar HTTPS. El ESP32 soporta HTTPS nativamente. La URL en el portal cautivo simplemente cambia de `http://` a `https://`.

> **Nota:** Si usas un certificado autofirmado (no recomendado), necesitarías agregar `http.setInsecure()` en `api_client.h`. Con certificados de Let's Encrypt u otra CA reconocida, funciona sin cambios.

### 8.3 Alimentación en el barco

| Fuente | Cómo conectar |
|--------|---------------|
| USB directo (12V-5V) | Conversor DC-DC 12V→5V con salida USB → ESP32 |
| Powerbank | Conectar USB directo. Duración ~24-48h con 10.000mAh |
| Panel solar + powerbank | Para instalaciones permanentes sin acceso a electricidad |
| Cigarrera 12V del barco | Adaptador cigarrera → USB → ESP32 |

El ESP32 consume ~80mA promedio (picos de ~250mA al transmitir WiFi).

### 8.4 Protección física

Para instalación en barcos se recomienda:
- Caja estanca IP65 o IP67 (protección contra agua y sal)
- Antena GPS visible al cielo (no encerrada en metal)
- Separar GPS de motores eléctricos (interferencia magnética)
- Fijar con bridas o cinta VHB para que no se mueva con el oleaje

---

## 9. Instalar en un nuevo barco (checklist rápido)

```
□  Generar token desde la app web para el barco
□  Conectar GPS al ESP32 (4 cables: 3V3, GND, GPIO16, GPIO17)
□  Alimentar ESP32 por USB
□  Desde celular → WiFi → conectar a "TRACKING-XXXXXX"
□  En el portal cautivo, llenar:
     • WiFi del barco (SSID + contraseña)
     • Token del dispositivo
     • URL del API (https://api.tudominio.com/api/v1/device/ping)
     • Intervalo de envío (10 seg default)
□  Guardar → ESP32 se conecta
□  Verificar en la app web → barco aparece "Online"
□  Montar en caja estanca con antena GPS al cielo
□  LISTO
```

---

## 10. Solución de problemas

| Problema | LED | Causa probable | Solución |
|----------|-----|----------------|----------|
| No se conecta a WiFi | 2 flashes + pausa | SSID o contraseña incorrecta | Reset (BOOT 3s) → reconfigurar |
| No envía datos al API | 3 flashes + pausa | URL incorrecta, backend caído, o token inválido | Verificar URL y token en portal |
| Sin señal GPS | Parpadeo lento | GPS sin vista al cielo, o recién encendido | Mover antena a cielo abierto, esperar 1-2 min |
| Token no configurado | 5 flashes + pausa | Faltó poner el token en el portal | Reset → reconfigurar con el token |
| ESP32 no enciende | Nada | Cable USB malo, fuente sin corriente | Probar otro cable/fuente |
| Portal cautivo no aparece | Parpadeo medio | El celular no redirige automáticamente | Abrir `192.168.4.1` manualmente en el navegador |
| GPS demora mucho en fijar | Parpadeo lento >5 min | Primera vez en una ubicación nueva (cold start) | Normal; el GPS6MV2 tarda 1-5 min en cold start |
| Errores consecutivos del API | 3 flashes | Backend no responde | Verificar que el backend está levantado y accesible |

### Reiniciar todo desde cero

1. Mantener botón **BOOT** presionado 3 segundos
2. El LED parpadea muy rápido → suelta el botón
3. Se borra: WiFi + Token + URL + Intervalo
4. Se abre el portal cautivo de nuevo
5. Reconfigurar con el celular

---

## 11. Referencia: API endpoints para dispositivos

### Endpoints IoT (los usa el ESP32)

| Método | Ruta | Auth | Descripción |
|--------|------|------|-------------|
| `POST` | `/api/v1/device/ping` | `X-Device-Token` | Enviar telemetría + recibir comandos |

### Endpoints de gestión (los usa la app web)

| Método | Ruta | Auth | Descripción |
|--------|------|------|-------------|
| `GET` | `/api/v1/vessels/{id}/device/token` | JWT | Ver token del dispositivo |
| `POST` | `/api/v1/vessels/{id}/device/token/regen` | JWT | Rotar token |
| `POST` | `/api/v1/vessels/{id}/device/reboot` | JWT | Encolar reinicio |
| `POST` | `/api/v1/vessels/{id}/device/command` | JWT | Enviar comando genérico |
| `GET` | `/api/v1/vessels/{id}/device/config` | JWT | Ver configuración |
| `PUT` | `/api/v1/vessels/{id}/device/config` | JWT | Actualizar configuración |
| `GET` | `/api/v1/vessels/{id}/device/status` | JWT | Estado del dispositivo |
