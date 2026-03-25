/*
 * ============================================================
 * tracking-microcontrolador.ino
 * ============================================================
 * Sistema de tracking GPS para embarcaciones marítimas.
 *
 * Hardware: ESP32 DevKit V1 + GPS6MV2 (HW-248)
 * Comunicación: WiFi → HTTP POST → tracking-api (Laravel)
 *
 * Arquitectura modular:
 *   config.h          → Constantes y valores por defecto
 *   storage_manager.h → Persistencia en NVS (Preferences)
 *   wifi_portal.h     → WiFiManager + Portal Cautivo
 *   gps_reader.h      → Lectura GPS vía TinyGPS++
 *   api_client.h      → Cliente HTTP + ArduinoJson
 *   command_handler.h → Ejecución de comandos remotos
 *   led_indicator.h   → Indicador visual de estado
 *
 * Flujo:
 *   1. Boot → Carga config de NVS
 *   2. Conecta WiFi (o abre portal cautivo para configurar)
 *   3. Loop: Lee GPS → Envía ping al API → Recibe comandos
 *   4. Comandos remotos: reboot, update_config, reset_wifi
 *   5. Botón BOOT (3s): Reset config → Reabre portal
 *
 * Librerías requeridas:
 *   - WiFiManager      (tzapu)
 *   - TinyGPSPlus      (Mikal Hart)
 *   - ArduinoJson      (Benoît Blanchon)
 * ============================================================
 */

#include "config.h"
#include "storage_manager.h"
#include "wifi_portal.h"
#include "gps_reader.h"
#include "api_client.h"
#include "command_handler.h"
#include "led_indicator.h"

// ── Módulos ──────────────────────────────────────────────────
StorageManager &storage = StorageManager::getInstance();
WifiPortal wifi;
GpsReader gps;
ApiClient api;
LedIndicator led;
CommandHandler commands(led);

// ── Estado ───────────────────────────────────────────────────
DeviceConfig currentConfig;
unsigned long lastSendTime = 0;
unsigned long lastReconnectTime = 0;
unsigned long lastHeartbeat = 0;
int consecutiveApiErrors = 0;

// =============================================================
// SETUP
// =============================================================
void setup()
{
  Serial.begin(115200);
  delay(500);

  Serial.println();
  Serial.println("╔══════════════════════════════════════════╗");
  Serial.printf("║  Tracking GPS  v%s                 ║\n", FIRMWARE_VERSION);
  Serial.println("║  ESP32 + GPS6MV2 → WiFi → API           ║");
  Serial.println("╚══════════════════════════════════════════╝");

  // Inicializar módulos
  led.begin();
  led.setState(DeviceState::BOOTING);
  led.update();

  storage.begin();
  currentConfig = storage.load();

  // Inicializar GPS
  gps.begin();

  // Verificar que hay token configurado
  if (currentConfig.deviceToken.length() == 0)
  {
    Serial.println("[MAIN] Sin token. El tecnico debe configurar via portal.");
    led.setState(DeviceState::ERROR_NO_TOKEN);
  }

  // Conectar WiFi (o abrir portal cautivo)
  led.setState(DeviceState::CONNECTING_WIFI);
  led.update();

  if (!wifi.begin())
  {
    Serial.println("[MAIN] WiFi fallo. Reiniciando en 5s...");
    delay(5000);
    ESP.restart();
  }

  // Recargar config (pudo cambiar en el portal)
  currentConfig = storage.load();

  if (currentConfig.deviceToken.length() == 0)
  {
    led.setState(DeviceState::ERROR_NO_TOKEN);
    Serial.println("[MAIN] ADVERTENCIA: Token no configurado");
  }
  else
  {
    led.setState(DeviceState::WAITING_GPS);
  }

  Serial.println("[MAIN] Sistema iniciado correctamente");
  Serial.printf("[MAIN] Token: %s...%s\n",
                currentConfig.deviceToken.substring(0, 4).c_str(),
                currentConfig.deviceToken.substring(
                                             currentConfig.deviceToken.length() - 4)
                    .c_str());
  Serial.printf("[MAIN] API: %s\n", currentConfig.apiUrl.c_str());
  Serial.printf("[MAIN] Intervalo: %lu ms\n", currentConfig.sendInterval);

  // ── Verificar conexión con el servidor ─────────────────
  int verifyCode = wifi.verifyServerConnection();
  if (verifyCode == 200 || verifyCode == 202)
  {
    Serial.println("[MAIN] >>> SERVIDOR OK - Iniciando tracking <<<");
  }
  else if (verifyCode == 401 || verifyCode == 403)
  {
    Serial.println("[MAIN] >>> TOKEN INVALIDO - Reconfigura via portal <<<");
    led.setState(DeviceState::ERROR_NO_TOKEN);
  }
  else
  {
    Serial.println("[MAIN] >>> SIN CONEXION AL SERVIDOR - Continuando igualmente <<<");
    led.setState(DeviceState::ERROR_API);
  }
}

// =============================================================
// LOOP
// =============================================================
void loop()
{
  unsigned long now = millis();

  // ── 1. Actualizar LED ────────────────────────────────────
  led.update();

  // ── 2. Leer GPS ──────────────────────────────────────────
  gps.update();

  // ── 3. Verificar botón reset (BOOT presionado 3s) ────────
  checkResetButton();

  // ── 4. Reconexión WiFi automática ────────────────────────
  if (!wifi.isConnected() && (now - lastReconnectTime >= WIFI_RECONNECT_INTERVAL))
  {
    lastReconnectTime = now;
    led.setState(DeviceState::ERROR_WIFI);
    wifi.reconnect();
    return; // skip este ciclo
  }

  // ── 5. Enviar telemetría al API ──────────────────────────
  if (now - lastSendTime >= currentConfig.sendInterval)
  {
    lastSendTime = now;

    GpsData gpsData = gps.read();

    if (!gpsData.isValid)
    {
      led.setState(DeviceState::WAITING_GPS);
      Serial.printf("[MAIN] GPS sin señal (%d satelites, %lu chars)\n",
                    gpsData.satellites, gps.charsProcessed());
    }

    // Enviar ping siempre (con o sin GPS válido)
    // El API acepta pings sin coordenadas como heartbeat
    if (wifi.isConnected() && currentConfig.deviceToken.length() > 0)
    {
      ApiResponse resp = api.sendPing(gpsData);

      if (resp.success)
      {
        consecutiveApiErrors = 0;
        if (gpsData.isValid)
        {
          led.setState(DeviceState::RUNNING);
        }

        // Procesar comandos remotos
        bool needsRestart = commands.process(resp);
        if (needsRestart)
        {
          delay(1000);
          ESP.restart();
        }

        // Recargar config si cambió remotamente
        if (commands.configChanged())
        {
          currentConfig = storage.load();
          Serial.printf("[MAIN] Config actualizada: intervalo=%lu ms\n",
                        currentConfig.sendInterval);
        }
      }
      else
      {
        consecutiveApiErrors++;
        led.setState(DeviceState::ERROR_API);

        if (consecutiveApiErrors >= MAX_HTTP_RETRIES)
        {
          Serial.printf("[MAIN] %d errores consecutivos del API\n",
                        consecutiveApiErrors);
        }
      }
    }
  }

  // ── 6. Heartbeat serial (debug) ──────────────────────────
  if (now - lastHeartbeat >= HEARTBEAT_INTERVAL)
  {
    lastHeartbeat = now;
    printStatus();
  }
}

// =============================================================
// FUNCIONES AUXILIARES
// =============================================================

/**
 * Verifica si el botón BOOT se mantiene presionado 3 segundos.
 * Si es así, borra toda la configuración y reinicia.
 */
void checkResetButton()
{
  if (digitalRead(RESET_BUTTON_PIN) == LOW)
  {
    unsigned long pressStart = millis();

    while (digitalRead(RESET_BUTTON_PIN) == LOW)
    {
      led.setState(DeviceState::RESETTING);
      led.update();
      delay(50);

      if (millis() - pressStart > RESET_HOLD_TIME)
      {
        Serial.println("[MAIN] === RESET COMPLETO ===");
        wifi.resetWifi();
        storage.clear();
        delay(500);
        ESP.restart();
      }
    }
  }
}

/** Imprime estado actual por serial (debug) */
void printStatus()
{
  GpsData gpsData = gps.read();
  Serial.println("────────────────────────────────────────");
  Serial.printf("  Uptime:     %lu s\n", millis() / 1000);
  Serial.printf("  WiFi:       %s (%s)\n",
                wifi.isConnected() ? "OK" : "DESCONECTADO",
                wifi.isConnected() ? wifi.getIP().c_str() : "---");
  Serial.printf("  GPS:        %s (%d sat, HDOP=%.1f)\n",
                gpsData.isValid ? "OK" : "SIN SEÑAL",
                gpsData.satellites, gpsData.hdop);
  if (gpsData.isValid)
  {
    Serial.printf("  Posicion:   %.6f, %.6f\n",
                  gpsData.latitude, gpsData.longitude);
    Serial.printf("  Velocidad:  %.1f km/h (%.1f nudos)\n",
                  gpsData.speed, gpsData.speed / 1.852);
  }
  Serial.printf("  API Errors: %d\n", consecutiveApiErrors);
  Serial.printf("  Intervalo:  %lu ms\n", currentConfig.sendInterval);
  Serial.println("────────────────────────────────────────");
}
