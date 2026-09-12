# Guía de Instalación y Configuración

Guía técnica paso a paso para instalar, flashear, configurar y desplegar el sistema de tracking GPS en embarcaciones usando WT32-ETH01 V1.4 apuntando a **producción (nautic.run)**.

---

## 1. Requisitos de hardware

### Componentes del dispositivo

| Componente | Modelo | Precio aprox. | Notas |
|------------|--------|---------------|-------|
| Microcontrolador | **WT32-ETH01 V1.4** | $8–12 USD | ESP32 con Ethernet LAN8720 integrado |
| Programador | **FT232RL YP-05** (USB-C) | $3–5 USD | Solo para flashear. Se desconecta en producción. |
| Módulo GPS | **NEO-7M** / NEO-6M | $3–5 USD | Antena cerámica, soldado a UART2 |
| Cable Ethernet | UTP Cat5e/Cat6 | $2–5 USD | Conexión de red en producción |
| Inyector PoE | PoE Activo 48V | $8–15 USD | Alimentación remota por cable de red |
| Splitter PoE | 48V → 5V/2A | $5–10 USD | Convierte PoE a 5V para el WT32-ETH01 |
| Cables Dupont | Hembra-hembra (4 uds) | $1 USD | Conexión FT232RL ↔ WT32-ETH01 (solo flasheo) |

### Infraestructura por barco

- Router/Switch con puerto Ethernet disponible (ej: Starlink, router 4G LTE)
- Cable UTP desde el router hasta la ubicación del dispositivo
- **No se requiere WiFi en el barco** — la conexión es 100% cableada

---

## 2. Conexiones físicas

### A. GPS ↔ WT32-ETH01 (Soldado permanentemente)

```
WT32-ETH01 V1.4              GPS NEO-7M/6M
────────────────────         ─────────────
  5V    ─────────────────→   VCC    (Rojo)
  GND   ─────────────────→   GND    (Negro)
  RXD   ─────────────────→   TX     (Verde)
  TXD   ─────────────────→   RX     (Amarillo)
```

| Pin GPS | Color | Pin WT32-ETH01 | Función |
|---------|-------|---------------|---------|
| VCC | Rojo | 5V | Alimentación del módulo (el módulo trae regulador) |
| GND | Negro | GND | Tierra común |
| TX | Verde | RXD (IO5) | Datos GPS → ESP32 |
| RX | Amarillo | TXD (IO17) | Comandos → GPS |

> **UART2** se usa para el GPS y dejar libre UART0 para programación/monitoreo. La regla de memoria: **VERDE → TX del GPS → RXD del ESP32** y **AMARILLO → RX del GPS → TXD del ESP32**.

### B. FT232RL (YP-05) ↔ WT32-ETH01 (Solo para programar)

El jumper del YP-05 debe estar en **5V**. Orden de pines del YP-05 (izquierda → derecha): **DTR, RX, TX, VCC, CTS, GND**.

```
FT232RL YP-05                 WT32-ETH01 V1.4
─────────────────             ────────────────
  VCC  (plomo)  ──────────→   5V     (Col 2, penúltimo pin — arriba de LINK)
  GND  (verde)  ──────────→   GND    (Col 1, 4to pin — debajo de IO0)
  TX   (morado) ──────────→   RX0    (Col 1, 2do pin — IO3)
  RX   (blanco) ──────────→   TX0    (Col 1, 1er pin — IO1)
  CTS  (azul)   ──────────→   NO CONECTAR
  DTR  (negro)  ──────────→   NO CONECTAR
```

| Pin YP-05 | Posición | Cable | Pin WT32-ETH01 | Ubicación |
|-----------|----------|-------|----------------|-----------|
| VCC | 4 | Plomo (gris) | 5V | Col 2, penúltimo pin (arriba de Link) |
| GND | 6 | Verde | GND | Col 1, 4to pin (debajo de IO0) |
| TX | 3 | Morado | IO3 (RX0) | Col 1, 2do pin |
| RX | 2 | Blanco | IO1 (TX0) | Col 1, 1er pin |
| CTS | 5 | Azul | — | No se usa (dejar sin conectar) |
| DTR | 1 | Negro | — | No se usa (dejar sin conectar) |

> **IMPORTANTE:** Los pines CTS y DTR del YP-05 **no se conectan a nada**. Solo se usan los 4 pines: VCC, GND, TX, RX. El FT232RL se desconecta después de flashear. En producción, el dispositivo se alimenta por PoE y solo tiene conectado el cable Ethernet y el GPS.

---

## 3. Preparar el entorno de desarrollo

### 3.1 Instalar VS Code + PlatformIO

1. Descargar **Visual Studio Code**: https://code.visualstudio.com
2. Instalar la extensión **PlatformIO IDE** desde el marketplace
3. Abrir la carpeta del proyecto (`tracking-microcontrolador`)

### 3.2 El archivo `platformio.ini` ya está configurado

```ini
[platformio]
src_dir = .
default_envs = wt32-eth01

[env:wt32-eth01]        ; flasheo por cable (YP-05)
platform = espressif32
board = wt32-eth01
framework = arduino
upload_speed = 115200
monitor_speed = 115200

[env:wt32-eth01-ota]    ; actualización por RED (sin cables)
upload_protocol = espota
upload_port = <IP-del-dispositivo>
```

Las librerías (`TinyGPSPlus`, `ArduinoJson`, ETH/WebServer/ESPmDNS/ArduinoOTA) se descargan solas la primera vez.

### 3.3 Configurar la placa (botón Upload con entorno `wt32-eth01`)

| Opción | Valor |
|--------|-------|
| Placa | WT32-ETH01 (board `wt32-eth01`) |
| Upload Speed | 115200 |
| Puerto | El que aparezca al conectar el FT232RL (ej: COM3) |

---

## 4. Configuración pre-flasheo (`config.h`)

### 4.1 URL del API (producción)

`config.h` ya trae la URL de **producción** por defecto (no requiere edición):

```cpp
// ── Producción (default — nautic.run) ──────────────────────
#define DEFAULT_API_URL  "https://api.nautic.run/api/v1/device/ping"

// ── Solo desarrollo local (XAMPP) — opcional ───────────────
// #define DEFAULT_API_URL  "http://192.168.1.43:8001/api/v1/device/ping"
```

> **Nota:** Esta URL es solo el valor por defecto. En el flujo normal de instalación el técnico la escribe (o la trae precargada) desde la página `tracking.local` — no hace falta re-flashear.

### 4.2 Parámetros Ethernet y GPS

Ya están configurados para WT32-ETH01 V1.4. Verificar en `config.h`:

```cpp
// ── Ethernet (LAN8720) ─────────────
#define ETH_PHY_ADDR   1
#define ETH_PHY_MDC    23
#define ETH_PHY_MDIO   18
#define ETH_PHY_TYPE   ETH_PHY_LAN8720
#define ETH_PHY_POWER  16   // GPIO16 alimenta el PHY
#define ETH_CLK_MODE   ETH_CLOCK_GPIO0_IN

// ── GPS (UART2) ────────────────────
#define GPS_RX_PIN 5   // IO5 (RXD)
#define GPS_TX_PIN 17  // IO17 (TXD)
#define GPS_BAUD 9600
```

---

## 5. Flashear el firmware (primera vez — con YP-05)

### Protocolo de sincronización (Modo Flash)

El WT32-ETH01 V1.4 **no tiene auto-reset**. Para programar:

1. **Puente de activación:** Conectar **IO0** a **GND** (IO0 es el 3er pin de la columna 1)
2. Enchufar el cable USB-C del FT232RL a la PC
3. En PlatformIO, seleccionar el entorno **`wt32-eth01`** y presionar **Upload**
4. **Durante "Connecting..."**: desconectar y reconectar el USB-C (el chip arranca en modo descarga con el puente puesto)
5. Al ver `Leaving... Hard resetting...`, **retirar el puente IO0→GND**
6. Desconectar y reconectar el USB para que ejecute el programa

> **Importante:** el dispositivo no debe tener otra fuente de energía durante el flasheo (desconectar el PoE/DC jack). La energía debe venir solo del YP-05.

### Salida esperada (Monitor Serial a 115200 baud)

```
╔══════════════════════════════════════════╗
║  Tracking GPS  v1.2.0                   ║
║  WT32-ETH01 + GPS → Ethernet → API      ║
╚══════════════════════════════════════════╝
[GPS] UART2 iniciado (RX=5, TX=17, 9600 baud)
[MAIN] Sin token. Iniciando modo instalacion.
[ETH] MODO INSTALACION (dispositivo sin configuracion)
[ETH] >>> Abre el navegador en: http://tracking.local <<<
```

> **El firmware se flashea una sola vez por dispositivo.** Las actualizaciones posteriores se hacen **por red (OTA)** con el entorno `wt32-eth01-ota`, sin cables.

---

## 6. Configuración del dispositivo (Modo Instalación) — PRODUCCIÓN

Cuando un WT32-ETH01 está **sin configurar** (recién flasheado o tras un reset de fábrica), entra automáticamente en **Modo Instalación**: sirve su página web de configuración en una dirección fácil sin necesidad de conocer IPs.

### 6.0 Antes de empezar — Prueba el flujo tú mismo (demo para el técnico)

> El administrador debe hacer el procedimiento completo **una vez desde su propio equipo** antes de mostrarlo al técnico. Luego el técnico repite los mismos pasos con su laptop.

### 6.1 Generar el token del dispositivo (desde la app de producción)

1. Entrar a **https://nautic.run**
2. Ir al módulo de embarcaciones
3. Seleccionar el barco (o crear uno nuevo)
4. Sección "Dispositivo IoT" → **Generar Token**
5. Copiar el token (ej: `a3f8b2c1-9d4e-4f7a-b5c6-1234567890ab`)
6. La URL del API de producción es: **`https://api.nautic.run/api/v1/device/ping`**

### 6.2 Conectar el dispositivo (2 formas)

**Forma A — Cable directo (recomendado, sin router):**
```
Laptop del técnico ──cable UTP──→ WT32-ETH01 (puerto RJ45)
```
El dispositivo se auto-asigna una IP y la laptop se configura sola.

**Forma B — En la red del barco (con router):**
```
WT32-ETH01 (RJ45) ──→ Inyector PoE ──→ cable ──→ Splitter ──→ Router
```
El dispositivo obtiene IP por DHCP automáticamente.

> **Energía:** durante la instalación el dispositivo se alimenta por el PoE (splitter → jack DC). Si usas cable directo (Forma A), desconecta el RJ45 del splitter y conecta el de la laptop — el splitter sigue entregando 5V.

### 6.3 Abrir la página de configuración

1. Desde cualquier dispositivo en la misma red (o conectado por cable directo), abrir el navegador
2. Escribir: **`http://tracking.local`**
   > Si no resuelve `tracking.local`, el dispositivo también imprime su IP por el Monitor Serial (durante el desarrollo) o aparece en la tabla DHCP del router.
3. La página muestra:
   - **Banner verde "Modo Instalación"**
   - **Dirección de acceso** (`http://tracking.local`)
   - MAC, firmware e IP del dispositivo

### 6.4 Llenar el formulario y probar

| Campo | Qué poner | Ejemplo |
|-------|-----------|---------|
| **Token del dispositivo** | El token generado en paso 6.1 | `a3f8b2c1-9d4e-4f7a-b5c6-1234567890ab` |
| **URL del API** | Dominio del backend + ruta del endpoint | `https://api.nautic.run/api/v1/device/ping` |
| **Intervalo de envío (seg)** | Cada cuántos segundos enviar datos | `10` |
| **Nombre embarcación** | Nombre identificador (opcional) | `Lancha Esperanza` |

**Antes de guardar, presionar el botón "🔍 Probar conexión (token + URL)":**

| Respuesta del botón | Significado | Qué hacer |
|---------------------|-------------|-----------|
| ✅ "Conexión exitosa y token VÁLIDO" | Token y URL correctos | Presionar **Guardar** |
| ❌ "TOKEN INVALIDO (HTTP 401/403)" | El servidor responde pero el token no corresponde | Regenerar token en la app y pegar el nuevo |
| ❌ "Servidor respondió HTTP XXX" | La URL no apunta al endpoint correcto | Verificar la URL (debe terminar en `/api/v1/device/ping`) |
| ❌ "No se pudo conectar" | No alcanza el servidor | Verificar red/internet del barco |

### 6.5 Guardar

1. Presionar **"Guardar configuración"**
2. El dispositivo se reinicia **solo** (página con spinner)
3. Sale del Modo Instalación y entra en modo tracking contra producción

### 6.6 Importar configuración por JSON (alternativa)

Presionar "Importar Config JSON" → pegar el JSON → "Aplicar" → "Probar conexión" → "Guardar":

```json
{"token":"a3f8b2c1-9d4e-4f7a-b5c6-1234567890ab","api_url":"https://api.nautic.run/api/v1/device/ping","interval":10,"vessel_name":"Lancha Esperanza"}
```

### 6.7 Verificar que funciona

**Opción A — LED indicador:**
- LED encendido fijo = todo funcionando
- Parpadeo lento = esperando GPS (normal los primeros 1-2 minutos al aire libre)

**Opción B — Desde la app web (nautic.run):**
- Ir al barco → Sección "Dispositivo" → ver estado "Online" con IP, firmware y último ping

**Opción C — Botón "Probar conexión" (repetir si se reconfigura):**
- Si el dispositivo ya tiene configuración y se quiere validar de nuevo, hacer reset de fábrica (mantener IO0 a GND 4-5 segundos) y repetir desde 6.2.

### 6.8 Demostración al técnico (desde otro equipo)

Una vez validado el flujo, para mostrarlo al técnico desde otro equipo en la misma red:

1. El técnico conecta su laptop al dispositivo (cable directo o misma red)
2. Abre `http://tracking.local` en su navegador
3. Completa el formulario con el token y la URL que el administrador le entregó
4. Presiona **"Probar conexión"** → debe verse el mensaje verde
5. Presiona **"Guardar configuración"** → el dispositivo se reinicia solo
6. Listo — el dispositivo queda configurado sin haber tocado ninguna IP

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

### 7.4 Abrir servidor de configuración remoto

```
POST /api/v1/vessels/{id}/device/command
Content-Type: application/json

{ "command": "open_config_server" }
```

El dispositivo inicia el servidor web de configuración. Útil si se necesita reconfigurar manualmente sin acceso físico.

### 7.5 Rotar token de seguridad

```
POST /api/v1/vessels/{id}/device/token/regen
```

Genera un nuevo token. El dispositivo actual dejará de autenticarse hasta que se reconfigure con el nuevo token (vía servidor web de configuración o reset físico).

---

## 8. Despliegue en producción (Fase B — PoE)

### 8.1 Alimentación vía PoE

En producción, el dispositivo se alimenta por el mismo cable Ethernet que usa para datos:

```
Router/Switch ──UTP──→ Inyector PoE 48V ──UTP──→ Splitter PoE ──5V──→ WT32-ETH01
                                         │
                                    220V/110V
```

El splitter PoE entrega **5V/2A** a los pines 5V y GND del WT32-ETH01 (los mismos donde se conectaba el FT232RL durante el desarrollo).

### 8.2 Dominio de producción

El firmware apunta por defecto a **`https://api.nautic.run`**. Si algún día cambiara el dominio, hay dos maneras de actualizar los dispositivos:

**Opción A — Antes de flashear (recomendado para flotas nuevas):**

Cambiar en `config.h`:
```cpp
#define DEFAULT_API_URL  "https://api.nautic.run/api/v1/device/ping"
```

**Opción B — Sin re-flashear (dispositivos ya instalados):**

Enviar comando remoto `update_config` con la nueva URL. El dispositivo la aplica en su siguiente ping.

### 8.3 HTTPS

El backend de producción usa HTTPS. El ESP32 soporta HTTPS nativamente, por lo que la URL `https://api.nautic.run/...` funciona sin cambios.

### 8.4 Protección física

Para instalación en barcos se recomienda:
- Caja estanca IP65 o IP67 (protección contra agua y sal)
- Antena GPS visible al cielo (no encerrada en metal)
- El WT32-ETH01 dentro de la caja, con entrada para cable Ethernet
- Fijar con bridas o cinta VHB para que no se mueva con el oleaje

---

## 9. Instalar en un nuevo barco (checklist rápido)

```
□  En nautic.run: generar token para el barco
□  Conectar GPS al WT32-ETH01 (4 cables: 5V, GND, RXD, TXD)
□  Flashear firmware (una sola vez, requiere FT232RL + puente IO0→GND)
□  Conectar UN cable de red: laptop → WT32-ETH01 (o al router)
□  Navegador → http://tracking.local
□  En el formulario, llenar:
     • Token del dispositivo
     • URL del API (https://api.nautic.run/api/v1/device/ping)
     • Intervalo de envío (10 seg default)
□  Botón "Probar conexión" → mensaje VERDE
□  Guardar → el dispositivo se reinicia solo y empieza a enviar
□  Verificar en nautic.run → barco aparece "Online"
□  Montar en el barco: splitter PoE (5V) a pines 5V/GND + RJ45 al router
□  LISTO
```

---

## 10. Solución de problemas

| Problema | LED | Causa probable | Solución |
|----------|-----|----------------|----------|
| No conecta Ethernet | 2 flashes + pausa | Cable UTP suelto, DHCP no asigna IP | Verificar cable, router, servidor DHCP |
| No envía datos al API | 3 flashes + pausa | URL incorrecta o token inválido | Verificar URL y token en la página config (botón Probar conexión) |
| Sin señal GPS | Parpadeo lento | GPS sin vista al cielo, o recién encendido | Mover antena a cielo abierto, esperar 1-2 min |
| GPS transmite pero 0 satélites | — | **Antena/receptor dañado** (calor excesivo al soldar) o sin cielo | Verificar en producción: `chars` sube pero `sats=0` → cambiar módulo. Soldar máximo 2-3 seg por punto |
| Token no configurado | 5 flashes + pausa | Faltó poner el token | Acceder a http://tracking.local y configurar token |
| WT32-ETH01 no enciende | Nada | Alimentación incorrecta | Verificar 5V en pines de alimentación |
| No se puede flashear | Error upload | Puente IO0→GND no conectado o PoE alimentando | Revisar puente, quitar PoE, reconectar USB |
| Servidor config no accesible | Parpadeo medio | mDNS no resuelve | Probar con la IP del dispositivo en la tabla DHCP del router |
| GPS demora mucho en fijar | Parpadeo lento >5 min | Primera vez en ubicación nueva (cold start) | Normal; el GPS tarda 1-5 min en cold start |

### Reiniciar todo desde cero (sin botón BOOT)

El WT32-ETH01 no tiene botón BOOT. Para borrar la configuración:

1. Con el dispositivo encendido, **unir IO0 con GND** (cables juntos) **4-5 segundos**
2. Soltar — el dispositivo borra la configuración NVS y se reinicia
3. Al no tener token, entra en **Modo Instalación** (página en `tracking.local`)
4. Reconfigurar desde el navegador

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
