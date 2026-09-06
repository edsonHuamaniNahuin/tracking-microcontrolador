/*
 * ============================================================
 * tracking-microcontrolador.ino
 * ============================================================
 * Sistema de tracking GPS para embarcaciones marítimas.
 *
 * Hardware: WT32-ETH01 V1.4 (ESP32 + LAN8720 Ethernet) + GPS NEO-7M/6M
 * Comunicación: Ethernet → HTTP POST → tracking-api (Laravel)
 *
 * Arquitectura modular:
 *   config.h           → Constantes y valores por defecto
 *   storage_manager.h  → Persistencia en NVS (Preferences)
 *   ethernet_manager.h → Ethernet LAN8720 + Servidor Web Config
 *   gps_reader.h       → Lectura GPS vía TinyGPS++ (UART2: IO5/IO17)
 *   api_client.h       → Cliente HTTP + ArduinoJson
 *   command_handler.h  → Ejecución de comandos remotos
 *   led_indicator.h    → Indicador visual de estado
 *
 * Flujo:
 *   1. Boot → Carga config de NVS
 *   2. Conecta Ethernet (DHCP)
 *   3. Si no hay token → inicia servidor web de configuración
 *   4. Loop: Lee GPS → Envía ping al API → Recibe comandos
 *   5. Comandos remotos: reboot, update_config, open_config_server
 *   6. Botón BOOT (3s): Reset config → Reinicia
 *
 * Librerías requeridas:
 *   - TinyGPSPlus      (Mikal Hart)
 *   - ArduinoJson      (Benoît Blanchon)
 *   - ETH, WebServer, ESPmDNS (ESP32 Arduino Core)
 * ============================================================
 */

#include "config.h"
#include "storage_manager.h"
#include "ethernet_manager.h"
#include "gps_reader.h"
#include "api_client.h"
#include "command_handler.h"
#include "led_indicator.h"

// ── Módulos ──────────────────────────────────────────────────
StorageManager &storage = StorageManager::getInstance();
EthernetManager eth;
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

// Watchdogs de recuperación (para campo — sin acceso físico)
unsigned long linkDownSince = 0;
unsigned long ipMissingSince = 0;

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
  Serial.println("║  WT32-ETH01 + GPS → Ethernet → API      ║");
  Serial.println("╚══════════════════════════════════════════╝");

  // Inicializar módulos
  led.begin();
  led.setState(DeviceState::BOOTING);
  led.update();

  storage.begin();
  currentConfig = storage.load();

  // Inicializar GPS (UART2: RX=IO5, TX=IO17)
  gps.begin();

  // Verificar que hay token configurado
  if (currentConfig.deviceToken.length() == 0)
  {
    Serial.println("[MAIN] Sin token. Iniciando servidor de configuracion web.");
    led.setState(DeviceState::ERROR_NO_TOKEN);
  }

  // Conectar Ethernet
  // Sin token → modo instalación (el técnico configura vía tracking.local)
  led.setState(DeviceState::CONNECTING_NETWORK);
  led.update();

  bool setupMode = currentConfig.deviceToken.length() == 0;

  if (!eth.begin(setupMode))
  {
    Serial.println("[MAIN] Ethernet fallo. Reiniciando en 5s...");
    delay(5000);
    ESP.restart();
  }

  // Servidor web SIEMPRE activo (tracking.local):
  //  - sin token  → formulario de configuración (Modo Instalación)
  //  - con token  → página de estado + posibilidad de reconfigurar
  eth.startConfigServer();
  if (currentConfig.deviceToken.length() == 0)
  {
    led.setState(DeviceState::CONFIG_SERVER);
    Serial.println("[MAIN] MODO INSTALACION. Accede via navegador:");
  }
  else
  {
    led.setState(DeviceState::WAITING_GPS);
    Serial.println("[MAIN] Pagina de estado activa en tracking.local");
  }
  Serial.printf("[MAIN] >>> http://tracking.local  (o http://%s/) <<<\n",
                eth.getIP().c_str());

  // Recargar config (pudo cambiar via web)
  currentConfig = storage.load();

  if (currentConfig.deviceToken.length() == 0)
  {
    led.setState(DeviceState::ERROR_NO_TOKEN);
    Serial.println("[MAIN] ADVERTENCIA: Token no configurado");
  }

  Serial.println("[MAIN] Sistema iniciado correctamente");
  Serial.printf("[MAIN] Token: %s...%s\n",
                currentConfig.deviceToken.substring(0, 4).c_str(),
                currentConfig.deviceToken.substring(
                                             currentConfig.deviceToken.length() - 4)
                    .c_str());
  Serial.printf("[MAIN] API: %s\n", currentConfig.apiUrl.c_str());
  Serial.printf("[MAIN] Intervalo: %lu ms\n", currentConfig.sendInterval);

  // ── Verificar conexión con el servidor (solo si hay token) ──
  if (currentConfig.deviceToken.length() > 0)
  {
    int verifyCode = eth.verifyServerConnection();
    if (verifyCode == 200 || verifyCode == 202)
    {
      Serial.println("[MAIN] >>> SERVIDOR OK - Iniciando tracking <<<");
    }
    else if (verifyCode == 401 || verifyCode == 403)
    {
      Serial.println("[MAIN] >>> TOKEN INVALIDO - Reconfigura via web <<<");
      led.setState(DeviceState::ERROR_NO_TOKEN);
      if (!eth.isConfigServerRunning())
      {
        eth.startConfigServer();
      }
    }
    else
    {
      Serial.println("[MAIN] >>> SIN CONEXION AL SERVIDOR - Continuando igualmente <<<");
      led.setState(DeviceState::ERROR_API);
    }
  }
  else
  {
    Serial.println("[MAIN] >>> Esperando configuracion del tecnico en la pagina web <<<");
  }
}

// =============================================================
// LOOP
// =============================================================
void loop()
{
  unsigned long now = millis();

  // ── 1. Servir cliente web de configuración ──────────────
  eth.handleClient();

  // ── 1b. Atender actualizaciones OTA por red ─────────────
  eth.handleOta();

  // ── 2. Actualizar LED ────────────────────────────────────
  led.update();

  // ── 3. Leer GPS ──────────────────────────────────────────
  gps.update();

  // ── 4. Verificar botón reset (BOOT presionado 3s) ────────
  checkResetButton();

  // ── 5. Reconexión Ethernet automática ────────────────────
  if (!eth.isConnected() && (now - lastReconnectTime >= ETH_RECONNECT_INTERVAL))
  {
    lastReconnectTime = now;
    led.setState(DeviceState::ERROR_NETWORK);
    eth.reconnect();
    return;
  }

  // ── 5b. Watchdogs de auto-recuperación (campo) ───────────
  // Link caído por mucho tiempo → reinicio limpio (re-negocia DHCP)
  if (!eth.isConnected())
  {
    if (linkDownSince == 0)
    {
      linkDownSince = now;
    }
    else if (now - linkDownSince >= ETH_LINK_DOWN_RESTART_MS)
    {
      Serial.printf("[MAIN] Link caido %lu ms. Reiniciando para reconexion limpia...\n",
                    now - linkDownSince);
      ESP.restart();
    }
  }
  else
  {
    linkDownSince = 0;

    // Link activo pero sin IP (DHCP atascado) → reinicio
    if (!eth.hasIp())
    {
      if (ipMissingSince == 0)
      {
        ipMissingSince = now;
      }
      else if (now - ipMissingSince >= ETH_NO_IP_RESTART_MS)
      {
        Serial.println("[MAIN] Sin IP con link activo. Reiniciando...");
        ESP.restart();
      }
    }
    else
    {
      ipMissingSince = 0;
    }
  }

  // Muchos errores de API seguidos (DNS/red bloqueada) → reinicio
  if (consecutiveApiErrors >= API_ERROR_RESTART_THRESHOLD)
  {
    Serial.printf("[MAIN] %d errores API consecutivos. Reiniciando...\n",
                  consecutiveApiErrors);
    consecutiveApiErrors = 0;
    ESP.restart();
  }

  // ── 6. Enviar telemetría al API ──────────────────────────
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
    if (eth.isConnected() && currentConfig.deviceToken.length() > 0)
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

        // Abrir servidor config si se solicitó remotamente
        if (commands.configServerRequested())
        {
          if (!eth.isConfigServerRunning())
          {
            eth.startConfigServer();
            led.setState(DeviceState::CONFIG_SERVER);
          }
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

  // ── 7. Heartbeat serial (debug) ──────────────────────────
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
        eth.stopConfigServer();
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
  Serial.printf("  Ethernet:   %s (%s)\n",
                eth.isConnected() ? "OK" : "DESCONECTADO",
                eth.isConnected() ? eth.getIP().c_str() : "---");
  if (eth.isConfigServerRunning())
  {
    Serial.printf("  Web Config: ACTIVO (%s)\n", eth.getIP().c_str());
  }
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
