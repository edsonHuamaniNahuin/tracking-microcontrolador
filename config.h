/*
 * ============================================================
 * config.h — Valores por defecto y constantes del sistema
 * ============================================================
 * Estos valores se usan como fallback. La configuración real
 * se almacena en NVS (Non-Volatile Storage) y se gestiona
 * vía WiFiManager (portal cautivo) o comando remoto.
 */

#ifndef CONFIG_H
#define CONFIG_H

// ── Versión del firmware ──────────────────────────────────────
#define FIRMWARE_VERSION "1.0.0"
#define DEVICE_TYPE "ESP32-GPS-TRACKER"

// ── Pines ─────────────────────────────────────────────────────
#define GPS_RX_PIN 16 // UART2 RX ← GPS TX
#define GPS_TX_PIN 17 // UART2 TX → GPS RX
#define GPS_BAUD 9600
#define LED_STATUS_PIN 2   // LED integrado del ESP32
#define RESET_BUTTON_PIN 0 // Botón BOOT = reset configuración

// ── WiFi AP (portal cautivo) ─────────────────────────────────
#define AP_PREFIX "TRACKING-"
#define AP_PASSWORD "tracking123"
#define AP_TIMEOUT_SECONDS 180 // 3 minutos para configurar

// ── Valores por defecto (se sobreescriben desde NVS / remoto) ─
#define DEFAULT_API_URL "http://192.168.1.100:8000/api/v1/device/ping"
#define DEFAULT_SEND_INTERVAL 10000 // ms (10 segundos)
#define DEFAULT_GPS_TIMEOUT 300000  // ms (5 minutos sin GPS = alerta)

// ── Intervalos del sistema ───────────────────────────────────
#define WIFI_RECONNECT_INTERVAL 30000 // ms
#define HEARTBEAT_INTERVAL 60000      // ms (1 minuto)
#define RESET_HOLD_TIME 3000          // ms (mantener botón 3s)
#define LED_BLINK_FAST 150            // ms
#define LED_BLINK_SLOW 1000           // ms

// ── Límites ──────────────────────────────────────────────────
#define MAX_HTTP_RETRIES 3
#define HTTP_TIMEOUT 10000 // ms
#define GPS_MIN_SATELLITES 3
#define NVS_NAMESPACE "tracking"

#endif
