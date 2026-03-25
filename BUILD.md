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
8. [Configurar el dispositivo (portal cautivo)](#8-configurar-el-dispositivo-portal-cautivo)
9. [Montaje en el barco](#9-montaje-en-el-barco)
10. [Preguntas frecuentes](#10-preguntas-frecuentes)

---

## 1. ¿Qué estamos construyendo?

Un **rastreador GPS** que se instala en un barco. El dispositivo:

- Lee la ubicación del barco con un módulo GPS (como el GPS de tu celular, pero independiente)
- Se conecta a una red WiFi
- Envía las coordenadas al servidor cada pocos segundos
- Se gestiona remotamente desde la aplicación web

El dispositivo completo tiene **solo 2 piezas electrónicas** conectadas entre sí con 4 cables:

```
┌────────────────────┐         ┌────────────────────┐
│                    │  4      │                    │
│   ESP32 DevKit     │ cables  │   Módulo GPS       │
│   (el cerebro)     │────────→│   (el localizador)  │
│                    │         │                    │
│  Ya incluye:       │         │  Ya incluye:       │
│  • WiFi            │         │  • Antena cerámica │
│  • LED indicador   │         │                    │
│  • Botón reset     │         │                    │
│  • Puerto USB      │         │                    │
└────────────────────┘         └────────────────────┘
        │
    Cable USB
        │
   ┌─────────┐
   │ Fuente  │  (cargador de celular, powerbank, o
   │  5V USB │   encendedor del barco con adaptador)
   └─────────┘
```

> **No necesitas comprar LED, botones ni pantallas extra.** El ESP32 DevKit ya trae un LED azul integrado en la placa y un botón marcado "BOOT" que usamos para resetear la configuración. Todo viene incluido.

---

## 2. Lista de compras

### Componentes electrónicos (por cada barco)

| # | Componente | Cantidad | Precio aprox. | Dónde buscar |
|---|-----------|----------|---------------|--------------|
| 1 | **ESP32 DevKit V1** (30 pines, con pines soldados, **USB-C**) | 1 | $3–6 USD | Amazon, AliExpress, MercadoLibre. Buscar: "ESP32 DevKit V1 USB-C" |
| 2 | **Módulo GPS NEO-6M** (GPS6MV2 / HW-248) | 1 | $3–6 USD | Amazon, AliExpress, MercadoLibre. Buscar: "GPS NEO-6M Arduino" |
| 3 | **4 cables cortos** (para que suelden en la tienda) | 4 | $0.50 USD | Cualquier cable fino de cobre. La tienda de electrónica lo tiene. |
| 4 | **Cable USB-C** (para alimentar) | 1 | $1–3 USD | Cable USB-C a USB-A. Si tu fuente de poder tiene USB-C, entonces USB-C a USB-C. |
| 5 | **Servicio de soldadura** (en la tienda) | 1 | $1–3 USD | Pedir en la misma tienda de electrónica que te suelden las 4 conexiones (ver sección 5) |

**Total por barco: ~$9–19 USD**

> **¿Por qué soldar y no usar cables sueltos (dupont)?** En un barco hay vibración, humedad y movimiento constante. Los cables enchufados (dupont) se sueltan fácilmente. Una soldadura es **permanente, confiable y no se desconecta** con el oleaje. Es la diferencia entre un prototipo de escritorio y un dispositivo de producción real.

### Para alimentar en el barco (elegir uno)

El ESP32 se alimenta por su **puerto USB-C** con cualquier fuente de 5V. Solo necesitas enchufar un cable USB-C.

| Opción | Precio | Cuándo usarla |
|--------|--------|---------------|
| **Cargador USB de celular** (5V, 1A mínimo) | $2–5 USD | Si el barco tiene toma de corriente 110V/220V |
| **Puerto USB del Starlink** o de cualquier router | $0 USD | Si el router/Starlink tiene un puerto USB de sobra |
| **Powerbank / batería portátil** (10.000 mAh) | $10–15 USD | Si no hay electricidad. Dura ~2–3 días. |
| **Adaptador encendedor 12V → USB** | $3–5 USD | Si el barco tiene encendedor/cigarrera 12V |

### Para proteger del agua (recomendado)

| Componente | Precio | Notas |
|-----------|--------|-------|
| **Caja estanca IP65** (~10×7×4 cm) | $3–8 USD | Buscar: "caja estanca electrónica IP65". Debe caber el ESP32 + GPS dentro |
| **Prensaestopas PG7** (para pasar el cable USB) | $0.50 USD | Opcional. Buscar: "prensaestopa PG7" |

---

## 3. ¿Cómo se ve cada componente?

### ESP32 DevKit V1

```
    ┌─────────────────────────────────┐
    │  ┌───┐                          │
    │  │USB│  ← Puerto USB-C           │
    │  └───┘    (aquí va el cable)     │
    │                                 │
    │  [BOOT] [EN]  ← Dos botones     │
    │                  pequeños        │
    │  ● ● ● ● ● ● ● ● ● ● ● ● ● ●  │ ← Pines (agujas metálicas)
    │  ● ● ● ● ● ● ● ● ● ● ● ● ● ●  │    a cada lado de la placa
    │                                 │
    │         (°) ← LED azul          │
    │             integrado           │
    └─────────────────────────────────┘
         Tamaño real: ~5 cm × 2.5 cm
```

**¿Cómo identificarlo en la tienda?**
- Placa color negro o azul oscuro
- Tiene 2 filas de pines metálicos (15 a cada lado)
- Puerto USB-C en un extremo
- Tiene impreso "ESP32" o "ESP-WROOM-32"
- Dos botones pequeños marcados "BOOT" y "EN"

> **IMPORTANTE:** Pedir específicamente el modelo con **USB-C** (no Micro-USB). Asegúrate de que tenga los **pines ya soldados** (las agujas metálicas ya puestas). Si los pines no vienen soldados, necesitarás un soldador o pide la versión "con pines soldados" ("pre-soldered headers").

### Módulo GPS (GPS6MV2 / NEO-6M)

```
    ┌─────────────────────────┐
    │  ┌───────────────────┐  │
    │  │  Antena cerámica  │  │ ← Cuadrado plano, color
    │  │   (cuadrado)      │  │   blanco/marrón en la parte
    │  └───────────────────┘  │   superior. Mira al cielo.
    │                         │
    │   [chip NEO-6M]         │
    │                         │
    │  ● ● ● ●               │ ← 4 pines: VCC GND TX RX
    └─────────────────────────┘
        Tamaño real: ~3.5 cm × 2.5 cm
```

**¿Cómo identificarlo en la tienda?**
- Placa pequeña azul o verde
- Tiene una antena cuadrada (cerámica, ~2.5 cm) encima
- Solo 4 pines en un borde: **VCC**, **GND**, **TX**, **RX**
- Puede decir "GY-GPS6MV2" o "HW-248" o "NEO-6M"
- A veces la antena viene conectada con un cable (antena activa), a veces está soldada directamente

### Cables (los lleva la tienda)

No necesitas comprar cables por separado. La tienda de electrónica tiene cable de cobre fino para soldar. Es lo que usarán para conectar el ESP32 al GPS. Pide que usen cables cortos (~5-8 cm) para que quede compacto.

---

## 4. Herramientas necesarias

| Herramienta | ¿La necesitas? | Para qué |
|-------------|----------------|----------|
| **Computador** con USB | **Sí** (solo una vez) | Para cargar el programa en el ESP32 |
| **Celular** con WiFi | **Sí** (cada barco) | Para configurar WiFi, token y **dominio del servidor** desde el portal |
| Destornillador pequeño | Opcional | Si usas caja estanca con tornillos |
| Bridas plásticas (amarras) | Opcional | Para fijar en el barco |

> **NO necesitas:** soldador propio (lo hace la tienda), multímetro, protoboard, resistencias, ni pantalla. Solo compras las 2 piezas, pides que las suelden, y está listo.

---

## 5. Qué pedir que suelden en la tienda

Este es el paso más importante. Vas a la tienda de electrónica con el ESP32 y el GPS, y le pides al técnico que **suelde 4 cables directamente** entre las dos placas.

### Lo que le dices al técnico de la tienda

> "Necesito que me suelde estos 4 cables entre estas dos placas. No quiero usar protoboard ni cables sueltos. Es para un proyecto que va en un barco y necesita aguantar vibración y humedad."

Le muestras esta tabla (o le muestras la pantalla del celular con esta imagen):

```
╔═══════════════════════════════════════════════════════════╗
║  SOLDAR 4 CABLES ENTRE ESP32 Y GPS NEO-6M               ║
║                                                         ║
║  ESP32 pin 3V3  ─── cable rojo ────→  GPS pin VCC      ║
║  ESP32 pin GND  ─── cable negro ───→  GPS pin GND      ║
║  ESP32 pin D16  ─── cable amarillo─→  GPS pin TX       ║
║  ESP32 pin D17  ─── cable verde ──→   GPS pin RX       ║
║                                                         ║
║  ⚠️  IMPORTANTE: Usar 3V3, NO usar 5V ni VIN            ║
╚═══════════════════════════════════════════════════════════╝
```

### Diagrama visual para el técnico

```
   ESP32 DevKit V1                Módulo GPS NEO-6M
   ┌─────────────────┐            ┌────────────────┐
   │  ┌───┐          │            │  ┌──────────┐  │
   │  │USB│          │            │  │ Antena   │  │
   │  └───┘          │            │  │ GPS      │  │
   │                 │            │  └──────────┘  │
   │  [BOOT] [EN]    │            │                │
   │                 │            │                │
   │  3V3 ●════rojo═══════════════●  VCC           │
   │  GND ●═══negro═══════════════●  GND           │
   │  D16 ●══amarillo═════════════●  TX            │
   │  D17 ●═══verde═══════════════●  RX            │
   │                 │            │                │
   └─────────────────┘            └────────────────┘
         │
     Cable USB
     (no se suelda,
      se enchufa)
```

### Explicación de cada cable (para que entiendas qué le estás pidiendo)

| Cable | De (ESP32) | A (GPS) | Qué hace |
|-------|-----------|---------|----------|
| 🔴 Rojo | **3V3** (3.3 voltios) | **VCC** | Le da energía al GPS. Como enchufar un aparato a la corriente. |
| ⚫ Negro | **GND** (tierra) | **GND** | Cierra el circuito eléctrico. Siempre se necesita. |
| 🟡 Amarillo | **D16** (GPIO 16) | **TX** | El GPS envía las coordenadas por este cable al ESP32. |
| 🟢 Verde | **D17** (GPIO 17) | **RX** | El ESP32 puede enviar instrucciones al GPS por aquí. |

> **¿Por qué TX del GPS va a D16 y no a TX del ESP32?**
> Porque TX significa "transmitir" y RX significa "recibir". Lo que transmite uno, lo recibe el otro. Por eso se cruzan: TX del GPS → RX del ESP32 (D16), y viceversa.

### ¿Qué debe quedar al final?

Un bloque único con las dos placas unidas por 4 cables cortos soldados:

```
   ┌──────────────────────────────────────────────────────┐
   │                                                      │
   │   ┌──────────┐    4 cables soldados    ┌─────────┐  │
   │   │  ESP32   │═════════════════════════│   GPS    │  │
   │   │          │  (fijos, no se sueltan) │ (antena↑)│  │
   │   └────┬─────┘                        └──────────┘  │
   │        │ USB-C                                       │
   │        │ (libre para enchufar)                       │
   │                                                      │
   │   LISTO PARA PRODUCCIÓN                              │
   └──────────────────────────────────────────────────────┘
```

**Resultado:** Un dispositivo sólido. Solo necesitas enchufar un cable USB-C para alimentarlo y cargarle el programa. No hay nada que se suelte con el movimiento del barco.

### Errores que el técnico NO debe cometer

| ❌ Error | ✅ Correcto |
|----------|------------|
| Usar el pin **5V** o **VIN** del ESP32 | Usar el pin **3V3** (3.3 voltios) |
| Soldar TX del GPS → TX del ESP32 | Soldar TX del GPS → **D16** del ESP32 |
| Soldar RX del GPS → RX del ESP32 | Soldar RX del GPS → **D17** del ESP32 |
| Cables largos (>10 cm) | Cables cortos (~5-8 cm) para que quede compacto |
| Dejar puntas de estaño expuestas | Cubrir soldaduras con tubo termocontraíble o cinta |

> ⚠️ **Si el técnico conecta VCC del GPS al pin 5V, el GPS se puede quemar.** Insiste en que sea el pin **3V3**.

### ¿Y si no hay tienda de electrónica cerca?

Puedes pedir las piezas por internet y usar **cables dupont hembra-hembra** como alternativa temporal (se enchufan sin soldar). Pero ten en cuenta que en un barco en movimiento, los cables enchufables **se sueltan** con la vibración. Para producción real, la soldadura es la opción correcta.

Si decides usar dupont temporalmente:
- Compra cables **hembra-hembra** (con hueco en ambos extremos)
- Enchufa los mismos 4 pares de la tabla de arriba
- Asegura cada conector con un punto de pegamento caliente para que no se suelte

---

## 6. Ensamblaje paso a paso

Si mandaste a soldar en la tienda (recomendado), el ensamblaje ya está hecho. Solo queda:

### Paso 1 — Verificar visualmente

Revisa que las soldaduras coincidan con esta tabla:

```
✅  3V3 del ESP32 → VCC del GPS  (NO 5V, NO VIN)
✅  GND del ESP32 → GND del GPS
✅  D16 del ESP32 → TX del GPS   (¡cruzado!)
✅  D17 del ESP32 → RX del GPS   (¡cruzado!)
✅  Ningún cable toca otro cable
✅  Las soldaduras están limpias (sin puentes de estaño entre pines)
```

### Paso 2 — Conectar el cable USB

1. Enchufa el cable USB al puerto del ESP32 (esto **no** se suelda, se enchufa y desenchufa)
2. Enchufa el otro extremo al computador
3. Si todo está bien, verás un **LED rojo fijo** en el ESP32 (indica que tiene energía)

```
   Computador
       │
   Cable USB (enchufable)
       │
       ▼
   ┌──────────┐══════════════┌──────────┐
   │  ESP32   │  soldado     │   GPS    │
   │  ┌───┐   │              │ (antena) │
   │  │USB│   │              │          │
   │  └───┘   │              │          │
   └──────────┘              └──────────┘
```

**Ya está.** El dispositivo está ensamblado y listo para cargar el programa.

---

## 7. Verificar que funciona

### 7.1 — Primera vez: cargar el programa

La primera vez que usas un ESP32 nuevo, necesitas cargarle el programa (firmware). Esto se hace **una sola vez por dispositivo**:

1. Conecta el ESP32 al computador por USB
2. Abre Arduino IDE (software gratuito, ver [SETUP.md](SETUP.md) sección 3 para instalación)
3. Abre el archivo `tracking-microcontrolador.ino`
4. Presiona el botón **Upload** (flecha →)
5. Espera 1-2 minutos hasta que diga "Done uploading"

### 7.2 — Ver los mensajes del dispositivo

1. En Arduino IDE, abre **Herramientas → Monitor Serial**
2. Ajusta la velocidad a **115200 baud** (selector abajo a la derecha)
3. Deberías ver algo como esto:

```
╔══════════════════════════════════════════╗
║  Tracking GPS  v1.0.0                   ║
║  ESP32 + GPS6MV2 → WiFi → API          ║
╚══════════════════════════════════════════╝
[GPS] UART2 iniciado (RX=16, TX=17, 9600 baud)
[WIFI] AP: TRACKING-A1B2C3
```

Si ves esto, **el ESP32 y el GPS están funcionando**.

### 7.3 — Entender el LED azul

El ESP32 tiene un LED azul integrado en la placa. **No necesitas conectar ningún LED externo.** Este LED te dice qué está pasando:

| Lo que ves en el LED | Significado | ¿Qué hacer? |
|---------------------|-------------|-------------|
| 🔵 Encendido fijo | Todo funciona bien | Nada, está OK |
| 🔵 Parpadeo rápido (~3 veces/seg) | Buscando WiFi | Esperar, está conectándose |
| 🔵 Parpadeo lento (~1 vez/seg) | Esperando señal GPS | Poner la antena GPS viendo al cielo |
| 🔵 Parpadeo medio (~2 veces/seg) | Portal de configuración activo | Conectarse con el celular para configurar |
| 🔵 2 flashes, pausa, 2 flashes... | Error de WiFi | La contraseña WiFi está mal. Resetear y reconfigurar |
| 🔵 3 flashes, pausa, 3 flashes... | Error del servidor | El servidor no responde. Verificar URL y que el backend esté funcionando |
| 🔵 5 flashes, pausa, 5 flashes... | Falta el token | No se configuró el token del dispositivo |
| 🔵 Parpadeo muy muy rápido | Reseteando | Se está borrando la configuración |

### 7.4 — El botón BOOT (reset)

El ESP32 tiene dos botones pequeños. El que dice **"BOOT"** sirve para resetear la configuración:

```
   ┌──────────────────────┐
   │                      │
   │  [BOOT]  [EN]        │  ← BOOT es el de la izquierda
   │                      │     (puede variar según el modelo)
   └──────────────────────┘
```

- **Presión normal** → No hace nada (es para programar)
- **Mantener presionado 3 segundos** → Borra TODO (WiFi, token, URL) y reabre el portal de configuración

Útil si te equivocaste en la configuración o si el barco cambia de red WiFi.

---

## 8. Configurar el dispositivo (portal cautivo)

Cada dispositivo debe configurarse **individualmente** para el barco donde se va a instalar. Esto se hace desde un celular, sin tocar código.

### 8.1 — Antes de ir al barco: generar el token

Desde la aplicación web (en tu computador o celular):

1. Entrar al sistema con tu usuario y contraseña
2. Ir al módulo de **Embarcaciones**
3. Seleccionar el barco donde va este dispositivo
4. Sección **"Dispositivo IoT"** → presionar **"Generar Token"**
5. **Copiar el token** — es un código largo tipo: `a3f8b2c1-9d4e-4f7a-b5c6-1234567890ab`
6. **Tener a mano el dominio del servidor** — ejemplo: `https://api.tudominio.com`

### 8.2 — En el barco: configurar por WiFi

1. **Encender** el ESP32 (conectar el USB a corriente)
2. El LED azul parpadeará — significa que está esperando configuración
3. En tu **celular**, ir a **Configuración WiFi** (ajustes del celular)
4. Buscar la red que se llama **`TRACKING-XXXXXX`** (las X son letras/números únicos de este dispositivo)
5. Conectarse con la contraseña: **`tracking123`**
6. Se abre automáticamente una **página web en el celular** (portal cautivo)
7. Si no se abre sola, abrir el navegador y escribir: **`192.168.4.1`**

### 8.3 — Llenar los campos del portal

En la página que se abre, llenar estos campos:

```
┌─────────────────────────────────────────────────────────┐
│           TRACKING GPS — Configuración                  │
│                                                         │
│  WiFi SSID:          [Marina-Norte-5G          ]        │
│  WiFi Password:      [clave-del-barco          ]        │
│                                                         │
│  Token dispositivo:  [a3f8b2c1-9d4e-4f7a-b5c6..]       │
│                                                         │
│  URL del servidor:   [https://api.tudominio.com/api/v1/device/ping] │
│                                                         │
│  Intervalo (seg):    [10                       ]        │
│  Nombre embarcación: [Lancha Esperanza         ]        │
│                                                         │
│              [ Guardar ]                                │
└─────────────────────────────────────────────────────────┘
```

**Detalle de cada campo:**

| Campo | Qué poner | Ejemplo |
|-------|-----------|--------|
| **WiFi SSID** | El nombre de la red WiFi **del barco** (o del muelle/marina) | `Marina-Norte-5G` |
| **WiFi Password** | La contraseña de esa red WiFi | `clave-secreta-123` |
| **Token del dispositivo** | El token que generaste en el paso 8.1 (copiarlo completo) | `a3f8b2c1-9d4e-4f7a...` |
| **URL del servidor** | El dominio completo de tu backend + la ruta del endpoint | `https://api.tudominio.com/api/v1/device/ping` |
| **Intervalo de envío** | Cada cuántos segundos envía la ubicación (10 = cada 10 seg) | `10` |
| **Nombre embarcación** | Nombre para identificar este barco (opcional) | `Lancha Esperanza` |

> **IMPORTANTE — URL del servidor:**
> - En **desarrollo local** (tu PC con XAMPP): `http://192.168.1.100:8000/api/v1/device/ping`
> - En **producción** (con dominio): `https://api.tudominio.com/api/v1/device/ping`
> - Siempre debe terminar en `/api/v1/device/ping`
> - Si el dominio cambia después, deberás resetear el dispositivo (botón BOOT 3s) y reconfigurar

### 8.4 — Verificar que se conectó

Después de presionar "Guardar":

1. El celular se desconecta de la red TRACKING-XXXXXX (es normal)
2. El ESP32 se reinicia solo
3. Intenta conectarse al WiFi del barco
4. Si el LED queda **encendido fijo** → todo bien, está funcionando
5. Si parpadea → ver tabla de LEDs en sección 7.3

**Desde la app web** puedes verificar que el barco aparece como "Online" en la sección de dispositivos.

### 8.5 — ¿Cada barco tiene configuración distinta?

**Sí.** Cada barco tiene su propia red WiFi y su propio token. El dominio del servidor es el mismo para todos, pero se configura en cada dispositivo:

| Barco | WiFi | Token | URL del servidor |
|-------|------|-------|------------------|
| Lancha Esperanza | `Marina-Norte-5G` | `abc123...` | `https://api.tudominio.com/api/v1/device/ping` |
| Pesquero San Juan | `Puerto-Sur-WiFi` | `def456...` | `https://api.tudominio.com/api/v1/device/ping` |
| Yate Libertad | `Muelle-Este` | `ghi789...` | `https://api.tudominio.com/api/v1/device/ping` |

El firmware es **idéntico** en los 3. La diferencia es solo la configuración del portal.

---

## 9. Montaje en el barco

### 9.1 — Diagrama de instalación completa

La caja del dispositivo (ESP32 + GPS) va **afuera**, donde la antena GPS puede ver el cielo. El router WiFi (Starlink) queda **dentro de la cabina**. El cable USB-C pasa por la canaleta de cables del Starlink para alimentar el dispositivo.

```
   DENTRO DE CABINA                           AFUERA (ve el cielo)
   ┌──────────────────────────────┐           ┌──────────────────────┐
   │                              │  Cable    │  Caja plástica IP65  │
   │  Fuente 5V USB               │  USB-C    │                     │
   │  (cargador, Starlink,        │  por      │  ┌──────┐══┌─────┐  │
   │   powerbank, etc.)           │  canaleta │  │ESP32 │  │ GPS │  │
   │         │                    │  Starlink │  │      │  │(↑)  │  │
   │         └────────────────────┼───────────┼─→│USB-C │  │cielo│  │
   │                              │           │  └──────┘  └─────┘  │
   │  Router WiFi (Starlink)      │           │                     │
   │                              │           └──────────────────────┘
   └──────────────────────────────┘
```

### 9.2 — Ubicación de la caja (afuera)

**Reglas importantes:**
- La caja va **afuera**, en un lugar donde la antena GPS pueda "ver" el cielo. No ponerla debajo de techos de metal.
- La caja debe ser de **plástico/PVC** (no metálica). El plástico no bloquea ni la señal GPS ni el WiFi.
- Si es posible, ponerla a la **sombra** (bajo un alero plástico, debajo de un asiento en cubierta). Esto evita que se caliente mucho al sol.
- Si tiene que estar al sol, usar caja de **color blanco o claro** (absorbe menos calor).
- La **antena GPS** (cuadrado plano del módulo) debe apuntar **hacia arriba** (hacia la tapa de la caja, que mira al cielo).

### 9.3 — Alimentación por USB-C

El ESP32 se alimenta por su **puerto USB-C** con cualquier fuente de 5V. El cable USB-C sale desde dentro de la cabina hacia la caja afuera, pasando por la canaleta del Starlink.

```
  Fuente 5V (dentro de cabina)
  (cargador, USB del Starlink, powerbank, etc.)
       │
   Cable USB-C
       │ (por canaleta del Starlink hacia afuera)
       │
  ┌────┴──────────────────────────────────┐
  │  Caja plástica IP65 (AFUERA)          │
  │  ┌──────────┐═══════════┌──────────┐  │
  │  │  ESP32   │  soldado  │   GPS    │  │
  │  │  (USB-C) │  (corto)  │ (antena↑)│  │
  │  └──────────┘           └──────────┘  │
  └───────────────────────────────────────┘
```

Así de simple. No se necesita convertidor de voltaje ni fusible porque el ESP32 ya regula internamente lo que recibe por USB-C (5V).

### 9.4 — Armar la caja estanca

**Pasos:**
1. Hacer un agujero pequeño en la caja para pasar el cable USB-C (con prensaestopa PG7 o un agujero sellado con silicona)
2. Meter el ESP32 y GPS dentro, con la antena GPS **apuntando hacia la tapa** (hacia arriba/cielo)
3. Pasar el cable USB-C por el agujero, sellar con silicona o prensaestopa
4. Fijar la caja al barco con bridas plásticas, tornillos de acero inoxidable, o cinta doble cara industrial (VHB)
5. El cable USB-C va por la **canaleta del Starlink** hasta dentro de la cabina, donde se conecta a la fuente de 5V

---

## 10. Preguntas frecuentes

### ¿Necesito comprar un LED aparte?
**No.** El ESP32 DevKit ya tiene un LED azul integrado en la placa (en el pin GPIO 2). El firmware lo usa automáticamente para indicar el estado. No necesitas comprar ni conectar ningún LED externo.

### ¿Necesito comprar un botón aparte?
**No.** El ESP32 DevKit ya tiene un botón marcado "BOOT" que usamos para resetear la configuración (mantener 3 segundos). No necesitas comprar botones adicionales.

### ¿Necesito saber soldar?
**No.** Le pides al técnico de la tienda de electrónica que suelde las 4 conexiones (ver sección 5). Tú solo llevas las piezas y le muestras la tabla de conexiones. Cuesta $1–3 USD en la mayoría de tiendas.

### ¿Puedo usar un ESP32 diferente?
Sí, cualquier placa basada en ESP32 con WiFi funciona. Solo asegúrate de que tenga:
- Puerto USB (para programar y alimentar)
- Pines GPIO 16 y GPIO 17 disponibles (para el GPS)
- Al menos 4MB de Flash

Modelos compatibles: ESP32 DevKit V1, ESP32-WROOM-32, NodeMCU-32S, ESP32-S3 DevKit (ajustar pines).

### ¿Puedo usar otro módulo GPS?
Sí, cualquier GPS que hable protocolo NMEA por UART a 9600 baud. Opciones compatibles:
- **GPS6MV2 / NEO-6M** ← recomendado (barato, probado)
- NEO-7M (mejor precisión)
- NEO-8M (mejor precisión + GLONASS)
- BN-220 / BN-880 (más pequeños, con brújula)

### ¿El GPS funciona dentro de un edificio?
**No confiablemente.** El GPS necesita señal de satélites, que solo llega bien al aire libre. Dentro de un edificio o bajo un techo de concreto/metal, la señal se debilita mucho. En un barco, asegúrate de que la antena esté expuesta al cielo.

### ¿Cuántos dispositivos puedo tener?
**Ilimitados.** Cada barco tiene su propio ESP32 + GPS con un token único. El firmware es idéntico para todos. Solo varía la configuración (WiFi, token, URL) que se pone desde el portal cautivo.

### ¿Qué pasa si se va la luz / se desconecta?
El ESP32 se reinicia automáticamente cuando vuelve la corriente. La configuración (WiFi, token, URL, intervalo) está guardada en memoria permanente (NVS) y **no se pierde** aunque se desconecte. Simplemente vuelve a conectarse y sigue enviando datos.

### ¿Qué pasa si el barco sale de la zona WiFi?
El dispositivo detecta que perdió WiFi, el LED mostrará 2 flashes (error WiFi), e intentará reconectarse cada 30 segundos. Cuando vuelva a estar en rango, se conecta solo.

> **Limitación actual:** Este sistema funciona con WiFi, no con datos móviles. El barco necesita estar en rango de una red WiFi para enviar datos. Para cobertura mar adentro se necesitaría un módulo celular (4G/LTE), que se puede agregar como mejora futura.

### ¿Puedo poner la caja debajo del techo metálico de la cabina?
**No.** El metal bloquea completamente la señal GPS. La caja debe ir **afuera**, en un lugar donde la antena GPS pueda ver el cielo directamente. Una caja de **plástico** no bloquea ni la señal GPS ni el WiFi, así que el plástico no es problema. El problema es solo el metal encima.

### ¿Cuánto consume de electricidad?
Muy poco. El ESP32 consume en promedio ~80mA (en reposo) con picos de ~250mA (al transmitir WiFi). Equivale a menos de 0.5W constantemente. Un powerbank de 10.000mAh dura ~2–3 días. Un cargador de celular normal (5V 1A) sobra de potencia.

### ¿Necesito saber programar?
**No.** El firmware se carga una sola vez usando Arduino IDE (es apretar un botón). La configuración de cada barco se hace desde el celular (portal web). Y la gestión remota se hace desde la aplicación web. No necesitas tocar código.

### ¿Puedo flashear muchos ESP32 con el mismo programa?
**Sí.** De hecho, esa es la idea. Flasheas 10, 50 o 100 ESP32 con el mismo archivo, y luego cada uno se configura individualmente desde su portal cautivo. El programa es genérico; la personalización es por configuración.

---

## Resumen: de la tienda a funcionando

```
Paso 1  │  COMPRAR en tienda de electrónica:
        │    ESP32 DevKit V1 (USB-C) + GPS NEO-6M + cable USB-C
        │
Paso 2  │  PEDIR AL TÉCNICO que suelde 4 cables:
        │    ESP32 3V3  → GPS VCC  (rojo)
        │    ESP32 GND  → GPS GND  (negro)
        │    ESP32 D16  → GPS TX   (amarillo)
        │    ESP32 D17  → GPS RX   (verde)
        │
Paso 3  │  EN TU PC: Conectar USB-C → Abrir Arduino IDE
        │    → Cargar el programa (una sola vez, un botón)
        │
Paso 4  │  EN LA APP WEB: Generar token para el barco
        │
Paso 5  │  EN EL BARCO:
        │    • Instalar caja plástica IP65 AFUERA (GPS ve el cielo)
        │    • Pasar cable USB-C por canaleta del Starlink
        │    • Conectar USB-C a fuente 5V dentro de cabina
        │      (cargador, USB del Starlink, powerbank)
        │    • Configurar con el celular:
        │      WiFi "TRACKING-XXXXXX" (clave: tracking123)
        │      En el portal poner:
        │      • WiFi del barco (nombre y contraseña)
        │      • Token del dispositivo
        │      • Dominio: https://api.tudominio.com/api/v1/device/ping
        │      • Intervalo de envío (10 segundos)
        │
Paso 6  │  LED azul encendido fijo = funcionando ✓
        │
        ▼  LISTO — el barco aparece en la aplicación web
```
