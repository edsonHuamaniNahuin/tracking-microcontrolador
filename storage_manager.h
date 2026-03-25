/*
 * ============================================================
 * storage_manager.h — Gestión de configuración persistente (NVS)
 * ============================================================
 * Patrón: Singleton + Repository
 * Almacena y recupera configuración del dispositivo en NVS
 * (Non-Volatile Storage) del ESP32.
 */

#ifndef STORAGE_MANAGER_H
#define STORAGE_MANAGER_H

#include <Arduino.h>
#include <Preferences.h>
#include "config.h"

struct DeviceConfig
{
  String deviceToken;
  String apiUrl;
  unsigned long sendInterval; // ms
  String vesselName;
};

class StorageManager
{
public:
  static StorageManager &getInstance()
  {
    static StorageManager instance;
    return instance;
  }

  void begin()
  {
    _prefs.begin(NVS_NAMESPACE, false);
  }

  // ── Leer configuración completa ──────────────────────────
  DeviceConfig load()
  {
    DeviceConfig cfg;
    _prefs.begin(NVS_NAMESPACE, true);
    cfg.deviceToken = _prefs.getString("token", "");
    cfg.apiUrl = _prefs.getString("api_url", DEFAULT_API_URL);
    cfg.sendInterval = _prefs.getULong("interval", DEFAULT_SEND_INTERVAL);
    cfg.vesselName = _prefs.getString("vessel_name", "");
    _prefs.end();
    return cfg;
  }

  // ── Guardar configuración completa ───────────────────────
  void save(const DeviceConfig &cfg)
  {
    _prefs.begin(NVS_NAMESPACE, false);
    _prefs.putString("token", cfg.deviceToken);
    _prefs.putString("api_url", cfg.apiUrl);
    _prefs.putULong("interval", cfg.sendInterval);
    _prefs.putString("vessel_name", cfg.vesselName);
    _prefs.end();
    Serial.println("[NVS] Configuracion guardada");
  }

  // ── Guardar campos individuales ──────────────────────────
  void saveToken(const String &token)
  {
    _prefs.begin(NVS_NAMESPACE, false);
    _prefs.putString("token", token);
    _prefs.end();
  }

  void saveApiUrl(const String &url)
  {
    _prefs.begin(NVS_NAMESPACE, false);
    _prefs.putString("api_url", url);
    _prefs.end();
  }

  void saveInterval(unsigned long interval)
  {
    _prefs.begin(NVS_NAMESPACE, false);
    _prefs.putULong("interval", interval);
    _prefs.end();
  }

  // ── Borrar toda la configuración ─────────────────────────
  void clear()
  {
    _prefs.begin(NVS_NAMESPACE, false);
    _prefs.clear();
    _prefs.end();
    Serial.println("[NVS] Configuracion borrada");
  }

  bool hasToken()
  {
    _prefs.begin(NVS_NAMESPACE, true);
    bool has = _prefs.getString("token", "").length() > 0;
    _prefs.end();
    return has;
  }

private:
  StorageManager() {}
  StorageManager(const StorageManager &) = delete;
  StorageManager &operator=(const StorageManager &) = delete;
  Preferences _prefs;
};

#endif
