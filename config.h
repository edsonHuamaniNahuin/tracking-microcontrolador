/*
 * ============================================================
 * config.h — Valores por defecto y constantes del sistema
 * ============================================================
 * Hardware: WT32-ETH01 V1.4 (ESP32 + LAN8720 Ethernet)
 * Estos valores se usan como fallback. La configuración real
 * se almacena en NVS (Non-Volatile Storage) y se gestiona
 * vía servidor web de configuración o comando remoto.
 */

#ifndef CONFIG_H
#define CONFIG_H

// ── Versión del firmware ──────────────────────────────────────
#define FIRMWARE_VERSION "1.2.0"
#define DEVICE_TYPE "WT32-ETH01-TRACKER"

// ── Pines ─────────────────────────────────────────────────────
#define GPS_RX_PIN 5  // UART2 RX ← GPS TX (IO5)
#define GPS_TX_PIN 17 // UART2 TX → GPS RX (IO17)
#define GPS_BAUD 9600
#define LED_STATUS_PIN 2   // LED integrado del ESP32
#define RESET_BUTTON_PIN 0 // Botón BOOT = reset configuración

// ── Ethernet (LAN8720) ───────────────────────────────────────
#define ETH_PHY_ADDR   1
#define ETH_PHY_MDC    23
#define ETH_PHY_MDIO   18
#define ETH_PHY_TYPE   ETH_PHY_LAN8720
#define ETH_PHY_POWER  16 // GPIO16 alimenta el PHY LAN8720 en WT32-ETH01
#define ETH_CLK_MODE   ETH_CLOCK_GPIO0_IN

// ── Servidor web de configuración ────────────────────────────
#define CONFIG_SERVER_PORT 80
#define MDNS_HOSTNAME "tracking"

// ── Modo instalación (dispositivo SIN configurar) ────────────
// IP fija de enlace local: sirve para conectar la laptop DIRECTO
// al dispositivo con un cable (sin router). El técnico abre
// http://tracking.local en el navegador.
#define SETUP_LINK_LOCAL_IP "169.254.1.1"
#define SETUP_LINK_MASK "255.255.0.0"
#define SETUP_DHCP_WAIT_MS 8000 // espera DHCP antes del fallback a cable directo

// ── Valores por defecto (se sobreescriben desde NVS / remoto) ─
#define DEFAULT_API_URL "https://api.nautic.run/api/v1/device/ping"
#define DEFAULT_SEND_INTERVAL 10000 // ms (10 segundos)
#define DEFAULT_GPS_TIMEOUT 300000  // ms (5 minutos sin GPS = alerta)

// ── Intervalos del sistema ───────────────────────────────────
#define ETH_RECONNECT_INTERVAL 10000 // ms
#define HEARTBEAT_INTERVAL 60000     // ms (1 minuto)
#define RESET_HOLD_TIME 3000         // ms (mantener botón 3s)
#define LED_BLINK_FAST 150           // ms
#define LED_BLINK_SLOW 1000          // ms

// ── Límites ──────────────────────────────────────────────────
#define MAX_HTTP_RETRIES 3
#define HTTP_TIMEOUT 10000 // ms
#define GPS_MIN_SATELLITES 3
#define NVS_NAMESPACE "tracking"

#endif
