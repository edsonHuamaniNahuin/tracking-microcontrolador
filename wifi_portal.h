/*
 * ============================================================
 * wifi_portal.h — WiFiManager + Portal Cautivo
 * ============================================================
 * Patrón: Facade
 * Encapsula toda la lógica de WiFi: conexión, portal cautivo
 * para configuración por técnico, y reconexión automática.
 *
 * Incluye página custom "Importar Config" que permite
 * pegar un JSON copiado desde la vista web del vessel.
 */

#ifndef WIFI_PORTAL_H
#define WIFI_PORTAL_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <HTTPClient.h>
#include "config.h"
#include "storage_manager.h"

// ── HTML inyectado como parámetro custom en el formulario WiFi ───
// Se inserta directamente en el body del form via WiFiManagerParameter(html)
static const char IMPORT_HTML[] PROGMEM = R"rawhtml(
<style>
#impBox{margin:10px 0;padding:12px;background:#eff6ff;border:2px solid #93c5fd;border-radius:10px;display:none}
#impBtn{display:block;width:100%;padding:8px;margin:8px 0 4px;background:#3b82f6;color:#fff;border:none;border-radius:8px;font-weight:600;font-size:14px;cursor:pointer}
#impTa{width:100%;height:80px;border:1px solid #93c5fd;border-radius:6px;padding:8px;font-family:monospace;font-size:11px;resize:vertical;box-sizing:border-box}
#impMsg{font-size:12px;margin-top:6px;padding:6px 8px;border-radius:6px;display:none}
.impOk{display:block!important;background:#dcfce7;color:#166534}
.impErr{display:block!important;background:#fee2e2;color:#991b1b}
</style>
<br/>
<div style="text-align:center;padding:8px;background:#3b82f6;color:#fff;border-radius:8px;font-weight:600;margin:8px 0;cursor:pointer" onclick="var b=document.getElementById('impBox');b.style.display=b.style.display==='block'?'none':'block'">
&#128229; Importar Config JSON
</div>
<div id="impBox">
<label style="font-size:12px;font-weight:600;color:#1e40af">Pega el JSON copiado desde la web:</label><br/>
<textarea id="impTa" placeholder='{"token":"...","api_url":"...","wifi_ssid":"...","wifi_pass":"..."}'></textarea>
<div id="impMsg"></div>
<button type="button" id="impBtn" onclick="applyImp()">Aplicar configuraci&oacute;n</button>
</div>
<script>
function applyImp(){
var m=document.getElementById('impMsg');
try{
var c=JSON.parse(document.getElementById('impTa').value);
if(!c.token||!c.api_url){throw new Error('Faltan token o api_url');}
var s=document.getElementById('s');if(s&&c.wifi_ssid)s.value=c.wifi_ssid;
var p=document.getElementById('p');if(p&&c.wifi_pass)p.value=c.wifi_pass;
var ids=['token','api_url','interval','vessel_name'];
for(var i=0;i<ids.length;i++){var el=document.getElementById(ids[i]);if(el&&c[ids[i]]!==undefined)el.value=c[ids[i]];}
m.className='impOk';
m.innerHTML='&#10004; Campos completados. Presiona <b>Save</b>.';
}catch(e){
m.className='impErr';
m.innerHTML='&#10008; '+e.message;
}
}
</script>
)rawhtml";

class WifiPortal
{
public:
  // Parámetros custom del portal
  WiFiManagerParameter *paramImportHtml;
  WiFiManagerParameter *paramToken;
  WiFiManagerParameter *paramApiUrl;
  WiFiManagerParameter *paramInterval;
  WiFiManagerParameter *paramVesselName;

  WifiPortal()
  {
    paramImportHtml = new WiFiManagerParameter(IMPORT_HTML);
    paramToken = new WiFiManagerParameter("token", "Token del dispositivo", "", 64);
    paramApiUrl = new WiFiManagerParameter("api_url", "URL del API", DEFAULT_API_URL, 128);
    paramInterval = new WiFiManagerParameter("interval", "Intervalo envio (seg)", "10", 5);
    paramVesselName = new WiFiManagerParameter("vessel_name", "Nombre embarcacion", "", 40);
  }

  ~WifiPortal()
  {
    delete paramImportHtml;
    delete paramToken;
    delete paramApiUrl;
    delete paramInterval;
    delete paramVesselName;
  }

  /**
   * Inicializa WiFi. Si no hay credenciales guardadas,
   * abre portal cautivo para que el técnico configure.
   * Retorna true si se conectó exitosamente.
   */
  bool begin()
  {
    // Pre-llenar campos con valores guardados
    DeviceConfig cfg = StorageManager::getInstance().load();
    if (cfg.deviceToken.length() > 0)
    {
      paramToken->setValue(cfg.deviceToken.c_str(), 64);
    }
    if (cfg.apiUrl.length() > 0)
    {
      paramApiUrl->setValue(cfg.apiUrl.c_str(), 128);
    }
    if (cfg.vesselName.length() > 0)
    {
      paramVesselName->setValue(cfg.vesselName.c_str(), 40);
    }

    // Agregar campos al portal (importHTML primero para que aparezca arriba)
    _wm.addParameter(paramImportHtml);
    _wm.addParameter(paramToken);
    _wm.addParameter(paramApiUrl);
    _wm.addParameter(paramInterval);
    _wm.addParameter(paramVesselName);

    // Callback al guardar
    _wm.setSaveParamsCallback([this]()
                              { _onSaveParams(); });

    // Timeout del portal
    _wm.setConfigPortalTimeout(AP_TIMEOUT_SECONDS);

    // Nombre AP único basado en MAC
    _apName = String(AP_PREFIX) + _getShortMac();
    Serial.printf("[WIFI] AP: %s\n", _apName.c_str());

    // Intentar conectar o abrir portal
    if (!_wm.autoConnect(_apName.c_str(), AP_PASSWORD))
    {
      Serial.println("[WIFI] Fallo de conexion");
      return false;
    }

    Serial.printf("[WIFI] Conectado. IP: %s\n", WiFi.localIP().toString().c_str());
    return true;
  }

  /** Fuerza apertura del portal (para reconfigurar) */
  void openPortal()
  {
    Serial.println("[WIFI] Abriendo portal de configuracion...");
    _wm.startConfigPortal(_apName.c_str(), AP_PASSWORD);
  }

  /** Borra credenciales WiFi guardadas */
  void resetWifi()
  {
    _wm.resetSettings();
    Serial.println("[WIFI] Credenciales WiFi borradas");
  }

  /** Verifica estado de conexión */
  bool isConnected()
  {
    return WiFi.status() == WL_CONNECTED;
  }

  /** Reintenta conexión */
  void reconnect()
  {
    if (!isConnected())
    {
      Serial.println("[WIFI] Reconectando...");
      WiFi.reconnect();
    }
  }

  String getIP()
  {
    return WiFi.localIP().toString();
  }

  String getApName()
  {
    return _apName;
  }

  /**
   * Verifica la conexión con el servidor API.
   * Hace un POST /device/ping sin GPS (heartbeat) y evalúa la respuesta.
   * Retorna el HTTP code (200/202 = OK, 401 = token inválido, -1 = sin conexión).
   */
  int verifyServerConnection()
  {
    DeviceConfig cfg = StorageManager::getInstance().load();

    Serial.println();
    Serial.println("╔══════════════════════════════════════════╗");
    Serial.println("║  VERIFICACION DE CONEXION AL SERVIDOR    ║");
    Serial.println("╚══════════════════════════════════════════╝");
    Serial.printf("  WiFi:   %s\n", isConnected() ? "CONECTADO" : "DESCONECTADO");
    Serial.printf("  IP:     %s\n", isConnected() ? getIP().c_str() : "---");
    Serial.printf("  API:    %s\n", cfg.apiUrl.c_str());
    Serial.printf("  Token:  %s...%s\n",
                  cfg.deviceToken.substring(0, 4).c_str(),
                  cfg.deviceToken.length() > 4
                      ? cfg.deviceToken.substring(cfg.deviceToken.length() - 4).c_str()
                      : "????");

    if (!isConnected())
    {
      Serial.println("  [FAIL] WiFi no conectado");
      return -1;
    }

    if (cfg.deviceToken.length() == 0)
    {
      Serial.println("  [FAIL] Token no configurado");
      return -1;
    }

    if (cfg.apiUrl.length() == 0)
    {
      Serial.println("  [FAIL] URL del API no configurada");
      return -1;
    }

    Serial.println("  Enviando ping de prueba...");

    HTTPClient http;
    http.begin(cfg.apiUrl);
    http.setTimeout(HTTP_TIMEOUT);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("X-Device-Token", cfg.deviceToken);

    // Heartbeat mínimo (sin GPS)
    String payload = "{\"firmware\":\"" + String(FIRMWARE_VERSION) + "\",\"device_type\":\"" + String(DEVICE_TYPE) + "\",\"uptime\":" + String(millis() / 1000) + "}";

    int httpCode = http.POST(payload);
    String body = (httpCode > 0) ? http.getString() : http.errorToString(httpCode);
    http.end();

    Serial.println("  ────────────────────────────────────");
    if (httpCode == 200 || httpCode == 202)
    {
      Serial.printf("  [OK] Servidor respondio HTTP %d\n", httpCode);
      Serial.println("  [OK] Conexion verificada exitosamente");
    }
    else if (httpCode == 401 || httpCode == 403)
    {
      Serial.printf("  [FAIL] HTTP %d - TOKEN INVALIDO\n", httpCode);
      Serial.println("  El token no coincide con ningun dispositivo.");
      Serial.println("  Regenera el token desde la web y reconfigura.");
    }
    else if (httpCode > 0)
    {
      Serial.printf("  [FAIL] HTTP %d\n", httpCode);
      Serial.printf("  Respuesta: %s\n", body.substring(0, 200).c_str());
    }
    else
    {
      Serial.printf("  [FAIL] Error de conexion: %s\n", body.c_str());
      Serial.println("  No se pudo alcanzar el servidor.");
      Serial.println("  Verifica la URL y que el servidor este corriendo.");
    }
    Serial.println("  ────────────────────────────────────");

    return httpCode;
  }

private:
  WiFiManager _wm;
  String _apName;

  void _onSaveParams()
  {
    DeviceConfig cfg;
    cfg.deviceToken = String(paramToken->getValue());
    cfg.apiUrl = String(paramApiUrl->getValue());
    cfg.vesselName = String(paramVesselName->getValue());

    String intervalStr = String(paramInterval->getValue());
    unsigned long sec = intervalStr.toInt();
    cfg.sendInterval = (sec > 0) ? sec * 1000 : DEFAULT_SEND_INTERVAL;

    StorageManager::getInstance().save(cfg);
    Serial.println("[WIFI] Configuracion del portal guardada en NVS");
  }

  String _getShortMac()
  {
    uint8_t mac[6];
    WiFi.macAddress(mac);
    char buf[7];
    snprintf(buf, sizeof(buf), "%02X%02X%02X", mac[3], mac[4], mac[5]);
    return String(buf);
  }
};

#endif
