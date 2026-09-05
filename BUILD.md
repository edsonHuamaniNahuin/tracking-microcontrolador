# Guía de Compra y Ensamblaje — Paso a Paso

Guía para alguien que **nunca ha trabajado con microcontroladores**. Explica qué comprar, cómo se ve cada pieza, cómo conectar todo y cómo verificar que funciona.

---

## Índice

1. [¿Qué estamos construyendo?](#1-qué-estamos-construyendo)
2. [Lista de compras](#2-lista-de-compras)
3. [¿Cómo se ve cada componente?](#3-cómo-se-ve-cada-componente)
4. [Herramientas necesarias](#4-herramientas-necesarias)
5. [Qué pedir que suelden en la tienda](#5-qué-pedir-que-suelden-en-la-tienda)
6. [Ensamblaje paso a paso](#6-ensamblaje-paso-a-paso)
7. [Verificar que funciona](#7-verificar-que-funciona)
8. [Configurar el dispositivo](#8-configurar-el-dispositivo)
9. [Montaje en el barco](#9-montaje-en-el-barco)
10. [Preguntas frecuentes](#10-preguntas-frecuentes)

---

## 1. ¿Qué estamos construyendo?

Un **rastreador GPS** que se instala en un barco. El dispositivo:

- Lee la ubicación del barco con un módulo GPS (como el GPS de tu celular, pero independiente)
- Se conecta a la red por **cable Ethernet** (UTP), no por WiFi
- Envía las coordenadas al servidor cada pocos segundos
- Se gestiona remotamente desde la aplicación web (**https://nautic.run**)

El dispositivo completo tiene **solo 2 piezas electrónicas** conectadas entre sí con 4 cables soldados. La conexión a internet es por cable de red (el mismo que usa una computadora de escritorio):

```
┌────────────────────┐         ┌────────────────────┐
│                    │  4      │                    │
│   WT32-ETH01       │ cables  │   Módulo GPS       │
│   (el cerebro)     │────────→│   (el localizador)  │
│                    │         │                    │
│  Ya incluye:       │         │  Ya incluye:       │
│  • Ethernet RJ45   │         │  • Antena cerámica │
│  • LED indicador   │         │                    │
│  • Conector PoE    │         │                    │
└────────────────────┘         └────────────────────┘
        │
    Cable Ethernet
    (datos + energía por PoE)
        │
   ┌─────────┐
   │ Router  │  (Starlink, router 4G LTE, switch del barco)
   └─────────┘
```

> **No necesitas WiFi en el barco.** El WT32-ETH01 se conecta con cable de red físico. Es más confiable en ambientes marítimos que el WiFi. La alimentación se hace por el mismo cable Ethernet usando PoE (Power over Ethernet).

---

## 2. Lista de compras

### Componentes electrónicos (por cada barco)

| # | Componente | Cantidad | Precio aprox. | Dónde buscar |
|---|-----------|----------|---------------|--------------|
| 1 | **WT32-ETH01 V1.4** | 1 | $8–12 USD | Amazon, AliExpress. Buscar: "WT32-ETH01" |
| 2 | **Módulo GPS NEO-7M** (o NEO-6M) | 1 | $3–6 USD | Amazon, AliExpress. Buscar: "GPS NEO-7M Arduino" |
| 3 | **FT232RL YP-05 USB-C** | 1 | $3–5 USD | AliExpress, MercadoLibre. Solo para programar. Buscar: "FT232RL USB-C" |
| 4 | **Cable Ethernet UTP Cat5e/Cat6** | 1 | $3–8 USD | Cable de red estándar. Largo según distancia al router. |
| 5 | **Inyector PoE 48V** | 1 | $8–15 USD | Buscar: "PoE injector 48V" |
| 6 | **Splitter PoE 48V→5V 2A** | 1 | $5–10 USD | Buscar: "PoE splitter 5V 2A" |
| 7 | **Servicio de soldadura** | 1 | $1–3 USD | Pedir en tienda de electrónica que suelden GPS al WT32-ETH01 |

**Total por barco: ~$31–59 USD**

### Para alimentar en el barco (elegir uno)

| Opción | Precio | Cuándo usarla |
|--------|--------|---------------|
| **PoE (inyector + splitter)** | $13–25 USD | Recomendado. Datos y energía por el mismo cable Ethernet. |
| **Cargador USB 5V** + cable USB-C (para FT232RL) | $2–5 USD | Solo para desarrollo/pruebas. No usar en producción. |

> En producción **siempre usar PoE**: un solo cable Ethernet lleva datos y electricidad. El splitter PoE convierte los 48V que vienen por el cable de red a 5V limpios para el WT32-ETH01.

### Para proteger del agua (recomendado)

| Componente | Precio | Notas |
|-----------|--------|-------|
| **Caja estanca IP65** (~10×7×4 cm) | $3–8 USD | Buscar: "caja estanca electrónica IP65" |
| **Prensaestopas PG9** (para cable Ethernet) | $0.50 USD | Opcional. Buscar: "prensaestopa PG9" |

---

## 3. ¿Cómo se ve cada componente?

### WT32-ETH01 V1.4

```
    ┌──────────────────────────────┐
    │  ┌───┐                       │
    │  │RJ45│ ← Puerto Ethernet     │
    │  └───┘   (aquí va el cable   │
    │           de red)             │
    │                              │
    │  ● ● ● ● ● ● ● ● ●          │ ← Pines lado izquierdo
    │  ● ● ● ● ● ● ● ● ●          │ ← Pines lado derecho
    │                              │
    │       (°) ← LED azul         │
    │           integrado          │
    └──────────────────────────────┘
        Tamaño real: ~5.5 cm × 2.5 cm
```

**¿Cómo identificarlo?**
- Placa pequeña color negro
- Tiene un conector RJ45 (puerto de red) en un extremo
- Dos filas de pines (no todos se usan)
- Tiene impreso "WT32-ETH01" en la placa

> El WT32-ETH01 **no tiene botón BOOT** ni puerto USB. Se programa con el adaptador FT232RL (ver sección 6).

### FT232RL YP-05 (Programador USB-TTL)

```
    ┌──────────────────────┐
    │    ┌────┐            │
    │    │USB-C│ ← Conector │
    │    └────┘   USB-C    │
    │                      │
    │  DTR RX TX VCC CTS GND│ ← Pines (de izquierda a derecha)
    │                      │
    │  Jumper: [3V3|5V]    │ ← Debe estar en 5V
    └──────────────────────┘
```

> **Pines del YP-05 (izquierda → derecha):** DTR, RX, TX, VCC, CTS, GND. Solo se usan 4: RX (blanco) → TX0 del ESP32, TX (morado) → RX0 del ESP32, VCC (plomo) → 5V, GND (verde) → GND. **DTR (negro) y CTS (azul) no se conectan a nada.**

> **Importante:** Este dispositivo se usa **solo una vez** (para cargar el programa). Se desconecta después y no va en el barco.

### Módulo GPS (NEO-7M / NEO-6M)

```
    ┌─────────────────────────┐
    │  ┌───────────────────┐  │
    │  │  Antena cerámica  │  │ ← Cuadrado plano, color
    │  │   (cuadrado)      │  │   metálico. La cara de METAL
    │  │                   │  │   debe mirar al cielo.
    │  └───────────────────┘  │
    │                         │
    │   [chip GPS]            │
    │                         │
    │  ● ● ● ●               │ ← 4 pines: VCC GND TX RX
    └─────────────────────────┘
        Tamaño real: ~3.5 cm × 2.5 cm
```

**¿Cómo identificarlo?**
- Placa pequeña azul o verde
- Antena cuadrada cerámica (~2.5 cm) encima
- Solo 4 pines en un borde: **VCC**, **GND**, **TX**, **RX**

### Inyector PoE + Splitter PoE

```
   ┌──────────┐         ┌──────────┐
   │ INYECTOR │         │ SPLITTER │
   │   PoE    │         │   PoE    │
   │          │         │          │
   │ LAN IN ──┤   UTP   ├── POE IN │
   │ POE OUT──┤────────→│          │
   │  ~220V   │         │ 5V OUT ──┤→ WT32-ETH01
   └──────────┘         └──────────┘
```

- **Inyector** (va al enchufe 220V y al router): mete la energía al cable de red
- **Splitter** (va junto al WT32): saca la energía del cable y la baja a 5V

---

## 4. Herramientas necesarias

| Herramienta | ¿La necesitas? | Para qué |
|-------------|----------------|----------|
| **Computador** con USB | **Sí** (solo una vez) | Para cargar el programa con el FT232RL |
| **Laptop con navegador** | **Sí** (cada instalación) | Para configurar el dispositivo en `tracking.local` |
| Destornillador pequeño | Opcional | Si usas caja estanca con tornillos |
| Bridas plásticas (amarras) | Opcional | Para fijar en el barco |

> **NO necesitas:** soldador propio (lo hace la tienda), conocimientos de redes, ni WiFi. La configuración es visual desde el navegador.

---

## 5. Qué pedir que suelden en la tienda

Llevas el WT32-ETH01 y el GPS a la tienda de electrónica. Le pides al técnico que **suelde 4 cables directamente** entre las dos placas.

### Lo que le dices al técnico

> "Necesito que me suelde estos 4 cables entre estas dos placas. Es para un GPS que va en un barco. Usar cables cortos (~5 cm) para que quede compacto."

Le muestras esta tabla:

```
╔═══════════════════════════════════════════════════════════╗
║  SOLDAR 4 CABLES ENTRE WT32-ETH01 Y GPS NEO-7M          ║
║                                                         ║
║  WT32-ETH01 5V    ─── rojo ────→  GPS VCC              ║
║  WT32-ETH01 GND   ─── negro ───→  GPS GND              ║
║  WT32-ETH01 RXD  ─── verde ───→  GPS TX               ║
║  WT32-ETH01 TXD  ─── amarillo─→  GPS RX               ║
║                                                         ║
║  ⚠️  IMPORTANTE: no invertir TX/RX (verde→RXD,          ║
║      amarillo→TXD) ni cruzar 5V con GND                 ║
╚═══════════════════════════════════════════════════════════╝
```

### Diagrama visual para el técnico

```
   WT32-ETH01 V1.4               Módulo GPS NEO-7M
   ┌─────────────────┐           ┌────────────────┐
   │  ┌───┐          │           │  ┌──────────┐  │
   │  │RJ45│         │           │  │ Antena   │  │
   │  └───┘          │           │  │ GPS      │  │
   │                 │           │  └──────────┘  │
   │                 │           │                │
   │  5V   ●══rojo══════════════●  VCC            │
   │  GND  ●══negro═════════════●  GND            │
   │  RXD  ●══verde═════════════●  TX             │
   │  TXD  ●══amarillo══════════●  RX             │
   │                 │           │                │
   └─────────────────┘           └────────────────┘
         │
     Cable Ethernet
     (no se suelda, se enchufa)
```

### Explicación de cada cable

| Cable | De (WT32-ETH01) | A (GPS) | Qué hace |
|-------|----------------|---------|----------|
| Rojo | **5V** | **VCC** | Le da energía al GPS (el módulo trae regulador y acepta 5V). |
| Negro | **GND** (tierra) | **GND** | Cierra el circuito eléctrico. |
| Verde | **RXD** (IO5) | **TX** | El GPS envía coordenadas por este cable. |
| Amarillo | **TXD** (IO17) | **RX** | El ESP32 envía instrucciones al GPS por aquí. |

> **¿Por qué TX del GPS va a RXD?** TX = transmitir, RX = recibir. Se cruzan: lo que transmite uno, lo recibe el otro.

### Errores que el técnico NO debe cometer

| ❌ Error | ✅ Correcto |
|----------|------------|
| Soldar TX del GPS → TXD | Soldar TX del GPS → **RXD** |
| Soldar RX del GPS → RXD | Soldar RX del GPS → **TXD** |
| Cruzar 5V con GND | 5V solo con VCC, GND solo con GND |
| Cables largos (>10 cm) | Cables cortos (~5 cm) compactos |
| Dejar puntas de estaño expuestas | Cubrir con tubo termocontraíble |

---

## 6. Ensamblaje paso a paso

### Paso 1 — Verificar las soldaduras

```
✅  5V del WT32-ETH01  → VCC del GPS
✅  GND del WT32-ETH01 → GND del GPS
✅  RXD del WT32-ETH01 → TX del GPS   (¡cruzado!)
✅  TXD del WT32-ETH01 → RX del GPS   (¡cruzado!)
✅  Ningún cable toca otro cable
✅  Soldaduras limpias, sin puentes de estaño
```

### Paso 2 — Cargar el programa (firmware)

Esto se hace **una sola vez por dispositivo** usando el FT232RL:

1. Conectar FT232RL al WT32-ETH01 con 4 cables dupont.
   Orden de pines del YP-05 (izquierda → derecha): **DTR, RX, TX, VCC, CTS, GND**:

```
YP-05 (pos. → cable)          WT32-ETH01
Pos. 4  VCC (Plomo)  →   5V    (col 2, penúltimo pin, arriba de LINK)
Pos. 6  GND (Verde)  →   GND   (col 1, pin 4, debajo de IO0)
Pos. 3  TX  (Morado) →   RX0   (col 1, pin 2 — IO3)
Pos. 2  RX  (Blanco) →   TX0   (col 1, pin 1 — IO1)
Pos. 1  DTR (Negro)  →   ✗ NO CONECTAR
Pos. 5  CTS (Azul)   →   ✗ NO CONECTAR
```

2. Conectar puente entre **IO0** (col 1, pin 3) y **GND** (solo durante la carga)
3. Enchufar FT232RL al computador por USB-C
4. En VS Code + PlatformIO, abrir el proyecto
5. Seleccionar el entorno **`wt32-eth01`** y presionar **Upload**
6. **Durante "Connecting..."**: desconectar y reconectar el USB-C
7. Cuando aparezca `Leaving... Hard resetting...`, **retirar el puente IO0→GND**
8. Desconectar y reconectar el USB-C

> El FT232RL se retira después. **No va en el barco.** Solo sirvió para cargar el programa. Las próximas actualizaciones se hacen **por red (OTA)**, sin cables.

### Paso 3 — Probar el dispositivo

1. Conectar cable Ethernet del WT32-ETH01 al router
2. Alimentar con 5V (vía FT232RL conectado a USB para pruebas, o vía PoE)
3. Abrir Monitor Serial de PlatformIO (115200 baud)
4. Deberías ver:

```
╔══════════════════════════════════════════╗
║  Tracking GPS  v1.2.0                   ║
║  WT32-ETH01 + GPS → Ethernet → API      ║
╚══════════════════════════════════════════╝
[GPS] UART2 iniciado (RX=5, TX=17, 9600 baud)
[ETH] Conectado. IP: 192.168.1.50
```

---

## 7. Verificar que funciona

### Entender el LED azul

El WT32-ETH01 tiene un LED azul integrado que indica el estado:

| Lo que ves en el LED | Significado | ¿Qué hacer? |
|---------------------|-------------|-------------|
| Encendido fijo | Todo funciona bien | Nada, está OK |
| Parpadeo rápido | Conectando a Ethernet | Esperar, está obteniendo IP del DHCP |
| Parpadeo lento | Esperando señal GPS | Poner antena GPS viendo al cielo |
| Parpadeo medio | Modo Instalación (página web activa) | Acceder a `tracking.local` desde el navegador |
| 2 flashes, pausa, 2 flashes... | Error de red Ethernet | Verificar cable de red y router |
| 3 flashes, pausa, 3 flashes... | Error del servidor | Verificar URL del API y token (botón Probar conexión) |
| 5 flashes, pausa, 5 flashes... | Falta el token | Configurar token desde `tracking.local` |
| Parpadeo muy muy rápido | Reseteando | Se está borrando la configuración |

### El reset de fábrica (sin botón)

El WT32-ETH01 **no tiene botón BOOT**. Para borrar la configuración:

```
IO0 (pin) ──unir con── GND (pin)  durante 4-5 segundos
(con el dispositivo encendido — como un botón invisible)
```

- **Unir IO0 con GND por 4-5 segundos** → Borra TODO (token, URL, intervalo) y reinicia
- Al arrancar de nuevo, entra en **Modo Instalación** (página en `tracking.local`)

---

## 8. Configurar el dispositivo

El dispositivo sin configurar se convierte solo en su propia página web (**Modo Instalación**). No se necesitan conocimientos de redes.

### 8.1 — Antes: generar el token (en nautic.run)

Desde la aplicación web (https://nautic.run):

1. Entrar al sistema con tu usuario y contraseña
2. Ir al módulo de **Embarcaciones**
3. Seleccionar el barco donde va este dispositivo
4. Sección **"Dispositivo IoT"** → presionar **"Generar Token"**
5. **Copiar el token** — es un código largo tipo: `a3f8b2c1-9d4e-4f7a-b5c6-1234567890ab`
6. **URL del servidor (producción):** `https://api.nautic.run/api/v1/device/ping`

### 8.2 — Conectar y abrir la página

**Forma A — Cable directo a la laptop (no necesita router):**
```
Laptop ──cable de red──→ WT32-ETH01 (puerto RJ45)
```
**Forma B — En la red del barco:**
```
WT32-ETH01 ──→ Inyector PoE ──→ Router
```

1. El LED parpadeará medio — significa que la página de configuración está activa
2. Abrir el navegador (Chrome/Edge)
3. Escribir en la barra de direcciones: **`http://tracking.local`**
4. Aparece la página con la dirección de acceso y un aviso verde "Modo Instalación"

### 8.3 — Llenar los campos del formulario

```
┌─────────────────────────────────────────────────────────┐
│           TRACKING GPS — Configuración                  │
│                                                         │
│  Dirección de acceso:  http://tracking.local            │
│                                                         │
│  Token dispositivo:  [a3f8b2c1-9d4e-4f7a-b5c6..]       │
│                                                         │
│  URL del servidor:   [https://api.nautic.run/api/v1/device/ping] │
│                                                         │
│  Intervalo (seg):    [10                       ]        │
│  Nombre embarcación: [Lancha Esperanza         ]        │
│                                                         │
│  [🔍 Probar conexión (token + URL)]                     │
│  ✅ Conexión exitosa y token VÁLIDO. Puedes guardar.    │
│                                                         │
│              [ 💾 Guardar ]                             │
└─────────────────────────────────────────────────────────┘
```

| Campo | Qué poner | Ejemplo |
|-------|-----------|--------|
| **Token del dispositivo** | El token generado en paso 8.1 (copiarlo completo) | `a3f8b2c1-9d4e-4f7a...` |
| **URL del servidor** | `https://api.nautic.run` + ruta del endpoint | `https://api.nautic.run/api/v1/device/ping` |
| **Intervalo de envío** | Cada cuántos segundos envía ubicación (10 = cada 10 seg) | `10` |
| **Nombre embarcación** | Nombre para identificar este barco (opcional) | `Lancha Esperanza` |

> **IMPORTANTE — URL del servidor:**
> - En **producción**: `https://api.nautic.run/api/v1/device/ping`
> - Siempre debe terminar en `/api/v1/device/ping`

### 8.4 — Probar la conexión antes de guardar (botón verde)

Antes de guardar, presionar el botón **"🔍 Probar conexión (token + URL)"**:

- **Mensaje verde** "Conexión exitosa y token VÁLIDO" → presionar **Guardar** ✅
- **Mensaje rojo** → revisar el texto:
  - "TOKEN INVALIDO (401/403)" → el token no corresponde → pedir otro al administrador
  - "No se pudo conectar" → la URL está mal o no hay internet → revisar el campo URL

El dispositivo **se reinicia solo** después de Guardar y queda listo para montar.

### 8.5 — Importar configuración por JSON (alternativa rápida)

1. Hacer clic en **"Importar Config JSON"**
2. Pegar el JSON completo, por ejemplo:
   ```json
   {"token":"a3f8b2c1-9d4e-4f7a-b5c6-1234567890ab","api_url":"https://api.nautic.run/api/v1/device/ping","interval":10,"vessel_name":"Lancha Esperanza"}
   ```
3. Clic en **"Aplicar"** → los campos se llenan solos
4. Clic en **"Probar conexión"** → si sale verde, clic en **"Guardar"**

### 8.6 — ¿Cada barco tiene configuración distinta?

**Sí.** Cada barco tiene su propio token. El dominio del servidor es el mismo para todos:

| Barco | Token | URL del servidor |
|-------|-------|------------------|
| Lancha Esperanza | `abc123...` | `https://api.nautic.run/api/v1/device/ping` |
| Pesquero San Juan | `def456...` | `https://api.nautic.run/api/v1/device/ping` |
| Yate Libertad | `ghi789...` | `https://api.nautic.run/api/v1/device/ping` |

El firmware es **idéntico** en los 3. La diferencia es solo la configuración web.

---

## 9. Montaje en el barco

### 9.1 — Diagrama de instalación completa (PoE)

La caja del dispositivo (WT32-ETH01 + GPS) va **afuera**, donde la antena GPS puede ver el cielo. El cable Ethernet llega desde dentro de la cabina:

```
   DENTRO DE CABINA                           AFUERA (ve el cielo)
   ┌──────────────────────────────┐           ┌──────────────────────┐
   │                              │           │  Caja plástica IP65  │
   │  Router/Switch               │  Cable    │                     │
   │  (Starlink, 4G LTE)          │  Ethernet │  ┌────────┐══┌────┐ │
   │        │                     │  (UTP)    │  │WT32    │  │GPS │ │
   │   ┌────┴────┐    ┌────────┐  │──────────→│  │ETH01   │  │(↑) │ │
   │   │ Inyector│    │Switch  │  │           │  │(RJ45)  │  │cielo│ │
   │   │ PoE 48V │    │/Router │  │           │  └────────┘  └────┘ │
   │   └─────────┘    └────────┘  │           │    ↑               │
   │                              │           │  Splitter PoE       │
   │                              │           │  48V→5V/2A          │
   └──────────────────────────────┘           └──────────────────────┘
```

### 9.2 — Alimentación por PoE (producción)

El mismo cable Ethernet que lleva los datos también lleva la electricidad:

1. Dentro de la cabina: conectar **inyector PoE 48V** a la corriente y al router/switch
2. El cable UTP sale hacia afuera llevando datos + 48V
3. Junto al WT32-ETH01, conectar el **splitter PoE** (convierte 48V → 5V/2A)
4. Conectar los 5V del splitter a los pines **5V** y **GND** del WT32-ETH01
5. Conectar el puerto RJ45 del WT32-ETH01 al puerto LAN del splitter

### 9.3 — Ubicación de la caja (afuera)

**Reglas importantes:**
- Caja de **plástico/PVC** (no metálica). El plástico no bloquea la señal GPS.
- La antena GPS debe apuntar hacia **arriba** (la cara de metal al cielo).
- Si es posible, ponerla a la **sombra** (bajo alero plástico).
- Si tiene que estar al sol, usar caja de color **blanco o claro**.
- Fijar con bridas, tornillos de acero inoxidable o cinta VHB.

---

## 10. Preguntas frecuentes

### ¿Necesito WiFi en el barco?
**No.** Este dispositivo usa cable Ethernet (UTP), el mismo que usa una computadora de escritorio. No necesita WiFi para nada. El barco solo necesita un router/switch con puerto Ethernet disponible.

### ¿Por qué Ethernet y no WiFi?
Ethernet es más confiable en ambientes marítimos: no se desconecta con el oleaje, no tiene interferencia de otros barcos, y el alcance no es problema. Además permite alimentación por PoE (datos + energía en un solo cable).

### ¿Necesito comprar un LED aparte?
**No.** El WT32-ETH01 ya tiene un LED azul integrado que el firmware usa para indicar el estado.

### ¿El FT232RL va en el barco?
**No.** Se usa únicamente para cargar el programa la primera vez (5 minutos). Se desconecta después. En el barco, el dispositivo se alimenta por PoE.

### ¿Necesito saber soldar?
**No.** Le pides al técnico de la tienda de electrónica que suelde las 4 conexiones del GPS al WT32-ETH01.

### ¿Puedo usar otro módulo GPS?
Sí, cualquier GPS que hable protocolo NMEA por UART a 9600 baud:
- **NEO-7M** — recomendado (mejor precisión que 6M)
- NEO-6M
- NEO-8M (GLONASS + GPS)
- BN-220 / BN-880

### ¿El GPS funciona dentro de un edificio?
**No confiablemente.** Necesita vista al cielo. En el barco, asegúrate de que la antena esté al aire libre (cara de metal hacia arriba).

### ¿Qué pasa si se va la luz?
El WT32-ETH01 se reinicia solo cuando vuelve la energía. Toda la configuración (token, URL, intervalo) está guardada en memoria permanente (NVS) y **no se pierde** aunque se desconecte completamente.

### ¿Cuánto consume?
Muy poco. El WT32-ETH01 consume ~100-200mA a 5V (menos de 1W). El splitter PoE entrega hasta 2A (10W), más que suficiente.

### ¿Necesito saber programar?
**No.** El firmware se carga una sola vez (es apretar un botón). La configuración de cada barco se hace desde una página web fácil (`tracking.local`) con un botón que prueba la conexión. Y la gestión diaria se hace desde la aplicación web (nautic.run).

### ¿Puedo flashear muchos dispositivos con el mismo programa?
**Sí.** Flasheas 10, 50 o 100 WT32-ETH01 con el mismo archivo, cada uno se configura individualmente desde su página. El firmware es genérico; la personalización es por configuración.

---

## Resumen: de la tienda a funcionando

```
Paso 1  │  COMPRAR:
        │    WT32-ETH01 + GPS NEO-7M + FT232RL + Inyector PoE + Splitter PoE
        │
Paso 2  │  PEDIR AL TÉCNICO que suelde 4 cables (GPS al WT32-ETH01):
        │    WT32-ETH01 5V   → GPS VCC   (rojo)
        │    WT32-ETH01 GND  → GPS GND   (negro)
        │    WT32-ETH01 RXD  → GPS TX    (verde)
        │    WT32-ETH01 TXD  → GPS RX    (amarillo)
        │
Paso 3  │  EN TU PC: Conectar FT232RL → puente IO0→GND
        │    → Cargar programa (una sola vez, un botón)
        │    (las próximas actualizaciones se hacen por RED, sin cables)
        │
Paso 4  │  EN LA APP WEB (nautic.run): Generar token para el barco
        │
Paso 5  │  CONFIGURAR (lo hace el técnico en 2 minutos):
        │    • Conectar UN cable de red: laptop → WT32-ETH01
        │    • Navegador → http://tracking.local
        │    • En el formulario poner:
        │      • Token del dispositivo
        │      • Dominio: https://api.nautic.run/api/v1/device/ping
        │      • Intervalo de envío (10 segundos)
        │    • Botón "Probar conexión" → mensaje VERDE
        │    • Botón "Guardar" → se reinicia solo
        │
Paso 6  │  EN EL BARCO:
        │    • Instalar caja plástica IP65 AFUERA (GPS ve el cielo)
        │    • Conectar cable Ethernet (UTP)
        │    • Conectar splitter PoE (5V) a WT32-ETH01
        │
Paso 7  │  LED azul encendido fijo = funcionando ✓
        │
        ▼  LISTO — el barco aparece en la aplicación web
```
