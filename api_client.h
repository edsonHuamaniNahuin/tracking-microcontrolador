/*
 * ============================================================
 * api_client.h — Cliente HTTP para comunicación con el API
 * ============================================================
 * Patrón: Gateway
 * Maneja toda la comunicación HTTP con tracking-api.
 * Endpoint principal: POST /api/v1/device/ping
 */

#ifndef API_CLIENT_H
#define API_CLIENT_H

#include <Arduino.h>
#include <ETH.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "config.h"
#include "gps_reader.h"
#include "storage_manager.h"

// Respuesta del API al ping
struct ApiResponse
{
  bool success;
  int httpCode;
  String command; // "reboot" | "update_config" | ""
  String errorMessage;
  // Campos para config remota
  long newInterval; // -1 = sin cambio
  String newApiUrl; // "" = sin cambio
};

class ApiClient
{
public:
  /**
   * Envía telemetría GPS al API y recibe comandos pendientes.
   * POST /api/v1/device/ping
   */
  ApiResponse sendPing(const GpsData &gps)
  {
    ApiResponse resp = {false, 0, "", "", -1, ""};

    DeviceConfig cfg = StorageManager::getInstance().load();
    if (cfg.deviceToken.length() == 0 || cfg.apiUrl.length() == 0)
    {
      resp.errorMessage = "Token o URL no configurados";
      Serial.printf("[API] %s\n", resp.errorMessage.c_str());
      return resp;
    }

    if (!ETH.linkUp())
    {
      resp.errorMessage = "Ethernet desconectado";
      return resp;
    }

    HTTPClient http;
    http.begin(cfg.apiUrl);
    http.setTimeout(HTTP_TIMEOUT);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("X-Device-Token", cfg.deviceToken);

    // Construir payload JSON
    JsonDocument doc;
    if (gps.isValid)
    {
      doc["lat"] = serialized(String(gps.latitude, 6));
      doc["lon"] = serialized(String(gps.longitude, 6));
      doc["speed"] = serialized(String(gps.speed / 1.852, 2)); // km/h → knots
      doc["course"] = serialized(String(gps.course, 2));
      doc["altitude"] = serialized(String(gps.altitude, 2));
    }
    // Estado del GPS siempre (diagnóstico remoto, aunque no haya señal)
    doc["satellites"] = gps.satellites;
    if (gps.satellites > 0)
    {
      doc["hdop"] = serialized(String(gps.hdop, 2));
    }
    doc["gps_chars"] = gps.chars;   // bytes NMEA recibidos (0 = GPS no habla)
    doc["gps_fix"] = gps.isValid;
    doc["gps_passed"] = gps.passedChecksums; // sentencias NMEA válidas
    doc["gps_failed"] = gps.failedChecksums; // sentencias corruptas
    doc["firmware"] = FIRMWARE_VERSION;
    doc["device_type"] = DEVICE_TYPE;
    doc["uptime"] = millis() / 1000;

    String payload;
    serializeJson(doc, payload);

    Serial.printf("[API] POST %s\n", cfg.apiUrl.c_str());
    Serial.printf("[API] Payload: %s\n", payload.c_str());

    int httpCode = http.POST(payload);
    resp.httpCode = httpCode;

    if (httpCode == 200 || httpCode == 202)
    {
      resp.success = true;
      String body = http.getString();
      _parseResponse(body, resp);
      Serial.printf("[API] OK (%d): cmd=%s\n", httpCode, resp.command.c_str());
    }
    else if (httpCode > 0)
    {
      resp.errorMessage = "HTTP " + String(httpCode);
      Serial.printf("[API] Error: %s\n", resp.errorMessage.c_str());
    }
    else
    {
      resp.errorMessage = http.errorToString(httpCode);
      Serial.printf("[API] Error: %s\n", resp.errorMessage.c_str());
    }

    http.end();
    return resp;
  }

private:
  void _parseResponse(const String &body, ApiResponse &resp)
  {
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, body);
    if (err)
    {
      Serial.printf("[API] JSON parse error: %s\n", err.c_str());
      return;
    }

    // Comando pendiente del servidor
    if (doc["data"].containsKey("command") && !doc["data"]["command"].isNull())
    {
      resp.command = doc["data"]["command"].as<String>();
    }

    // Configuración remota (si el API la envía)
    if (doc["data"].containsKey("config"))
    {
      JsonObject config = doc["data"]["config"];
      if (config.containsKey("send_interval"))
      {
        resp.newInterval = config["send_interval"].as<long>() * 1000;
      }
      if (config.containsKey("api_url"))
      {
        resp.newApiUrl = config["api_url"].as<String>();
      }
    }
  }
};

#endif
