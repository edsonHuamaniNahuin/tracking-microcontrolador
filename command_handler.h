/*
 * ============================================================
 * command_handler.h — Procesamiento de comandos remotos
 * ============================================================
 * Patrón: Command
 * Interpreta y ejecuta comandos recibidos del API.
 * Soporta: reboot, update_config, reset_wifi
 */

#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include <Arduino.h>
#include "storage_manager.h"
#include "led_indicator.h"
#include "api_client.h"

class CommandHandler
{
public:
  CommandHandler(LedIndicator &led) : _led(led) {}

  /**
   * Procesa la respuesta del API y ejecuta comandos.
   * Retorna true si se requiere reinicio.
   */
  bool process(const ApiResponse &response)
  {
    // ── Aplicar configuración remota ─────────────────────
    if (response.newInterval > 0)
    {
      Serial.printf("[CMD] Intervalo remoto: %ld ms\n", response.newInterval);
      StorageManager::getInstance().saveInterval(response.newInterval);
      _configChanged = true;
    }

    if (response.newApiUrl.length() > 0)
    {
      Serial.printf("[CMD] API URL remota: %s\n", response.newApiUrl.c_str());
      StorageManager::getInstance().saveApiUrl(response.newApiUrl);
      _configChanged = true;
    }

    // ── Ejecutar comando ─────────────────────────────────
    if (response.command.length() == 0)
    {
      return false;
    }

    Serial.printf("[CMD] Comando recibido: %s\n", response.command.c_str());

    if (response.command == "reboot")
    {
      return _executeReboot();
    }

    if (response.command == "update_config")
    {
      return _executeUpdateConfig();
    }

    if (response.command == "reset_wifi")
    {
      return _executeResetWifi();
    }

    Serial.printf("[CMD] Comando desconocido: %s\n", response.command.c_str());
    return false;
  }

  /** ¿Se cambió la configuración en el último ciclo? */
  bool configChanged()
  {
    bool changed = _configChanged;
    _configChanged = false;
    return changed;
  }

private:
  LedIndicator &_led;
  bool _configChanged = false;

  bool _executeReboot()
  {
    Serial.println("[CMD] Ejecutando REBOOT...");
    _led.setState(DeviceState::RESETTING);
    _led.update();
    delay(1000);
    ESP.restart();
    return true; // nunca llega aquí
  }

  bool _executeUpdateConfig()
  {
    Serial.println("[CMD] Configuracion actualizada remotamente");
    _configChanged = true;
    return false;
  }

  bool _executeResetWifi()
  {
    Serial.println("[CMD] Reset WiFi — reabriendo portal cautivo...");
    _led.setState(DeviceState::RESETTING);
    _led.update();
    delay(500);
    // Se manejará en el loop principal
    return true;
  }
};

#endif
