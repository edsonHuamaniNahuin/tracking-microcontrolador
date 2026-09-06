/*
 * ============================================================
 * ethernet_manager.h — Ethernet + Servidor Web de Configuración
 * ============================================================
 * Patrón: Facade
 * Encapsula toda la lógica de red Ethernet (LAN8720) para
 * WT32-ETH01 V1.4, incluyendo un servidor web de configuración
 * accesible desde cualquier navegador en la misma red.
 *
 * Reemplaza wifi_portal.h (WiFi + Portal Cautivo).
 * El FT232RL (YP-05 USB-C) se usa únicamente para programar
 * y monitoreo serial.
 */

#ifndef ETHERNET_MANAGER_H
#define ETHERNET_MANAGER_H

#include <Arduino.h>
#include <ETH.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <HTTPClient.h>
#include <ArduinoOTA.h>
#include "config.h"
#include "storage_manager.h"

// ── HTML del servidor de configuración ────────────────────────
static const char CONFIG_HTML[] PROGMEM = R"rawhtml(
<!DOCTYPE html>
<html lang="es">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1.0">
<title>Tracking GPS - WT32-ETH01</title>
<style>
*{box-sizing:border-box;margin:0;padding:0}
body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,sans-serif;background:#0f172a;color:#e2e8f0;min-height:100vh;padding:16px}
.card{background:#1e293b;border-radius:12px;padding:20px;margin-bottom:16px;border:1px solid #334155}
h1{font-size:20px;margin-bottom:4px;color:#f8fafc}
h2{font-size:14px;color:#94a3b8;font-weight:400;margin-bottom:16px}
.info-row{display:flex;justify-content:space-between;padding:6px 0;font-size:13px;border-bottom:1px solid #1e293b}
.info-row:last-child{border-bottom:none}
.info-label{color:#64748b}
.info-value{color:#e2e8f0;font-family:monospace}
label{display:block;font-size:13px;font-weight:600;color:#94a3b8;margin-bottom:4px;margin-top:12px}
input,textarea{width:100%;padding:10px 12px;border:2px solid #334155;border-radius:8px;background:#0f172a;color:#e2e8f0;font-size:14px;font-family:monospace}
input:focus,textarea:focus{outline:none;border-color:#3b82f6}
.btn{display:block;width:100%;padding:12px;margin-top:16px;border:none;border-radius:8px;font-size:15px;font-weight:600;cursor:pointer}
.btn-save{background:#3b82f6;color:#fff}
.btn-save:hover{background:#2563eb}
.btn-test{background:#059669;color:#fff}
.btn-test:hover{background:#047857}
.btn-import{background:#475569;color:#e2e8f0;margin-top:8px}
.access{background:#1e3a8a;border:1px solid #3b82f6;color:#bfdbfe;padding:10px 12px;border-radius:8px;font-size:13px;margin-bottom:12px;text-align:center}
.access b{color:#fff;font-family:monospace}
.imp-box{display:none;margin-top:12px}
.imp-box.show{display:block}
#impTa{height:90px;font-size:12px}
#impMsg{font-size:12px;margin-top:6px;padding:8px;border-radius:6px;display:none}
.imp-ok{display:block!important;background:#064e3b;color:#6ee7b7}
.imp-err{display:block!important;background:#450a0a;color:#fca5a5}
.current-tag{display:inline-block;font-size:10px;padding:2px 6px;border-radius:4px;margin-left:6px}
.tag-ok{background:#064e3b;color:#6ee7b7}
.tag-warn{background:#451a03;color:#fbbf24}
.msg{font-size:13px;margin-top:12px;padding:10px;border-radius:8px;display:none}
.msg-ok{display:block;background:#064e3b;color:#6ee7b7}
.msg-err{display:block;background:#450a0a;color:#fca5a5}
</style>
</head>
<body>
<div class="card">
  <h1>&#128760; Tracking GPS</h1>
  <h2>WT32-ETH01 V1.4 - Configuracion</h2>
  {{MODE_BANNER}}
  <div class="access">&#128279; Direccion de acceso: <b>http://tracking.local</b></div>
  <div class="info-row"><span class="info-label">MAC</span><span class="info-value">{{MAC}}</span></div>
  <div class="info-row"><span class="info-label">Firmware</span><span class="info-value">{{FW}}</span></div>
  <div class="info-row"><span class="info-label">IP</span><span class="info-value">{{IP}}</span></div>
</div>

<div class="card">
  <div style="display:flex;align-items:center;justify-content:space-between">
    <span style="font-weight:600;font-size:14px">&#9881; Configuracion Actual</span>
  </div>
  <div class="info-row"><span class="info-label">Token</span><span class="info-value">{{TOKEN}}</span></div>
  <div class="info-row"><span class="info-label">API URL</span><span class="info-value">{{APIURL}}</span></div>
  <div class="info-row"><span class="info-label">Intervalo</span><span class="info-value">{{INTERVAL}} ms</span></div>
  <div class="info-row"><span class="info-label">Embarcacion</span><span class="info-value">{{VESSEL}}</span></div>
</div>

<div class="card">
  <button class="btn btn-import" onclick="toggleImport()">&#128229; Importar Config JSON</button>
  <div class="imp-box" id="impBox">
    <textarea id="impTa" placeholder='{"token":"...","api_url":"...","interval":10,"vessel_name":"..."}'></textarea>
    <div id="impMsg"></div>
    <button type="button" class="btn btn-save" onclick="applyImport()" style="margin-top:8px">Aplicar</button>
  </div>

  <form action="/save" method="POST" id="cfgForm">
    <label for="token">Token del dispositivo</label>
    <input type="text" id="token" name="token" value="{{TOKEN_VAL}}" maxlength="64" placeholder="Token unico del dispositivo">
    <label for="api_url">URL del API</label>
    <input type="text" id="api_url" name="api_url" value="{{APIURL_VAL}}" maxlength="128" placeholder="http://...">
    <label for="interval">Intervalo de envio (segundos)</label>
    <input type="number" id="interval" name="interval" value="{{INTERVAL_VAL}}" min="1" max="3600">
    <label for="vessel_name">Nombre de la embarcacion</label>
    <input type="text" id="vessel_name" name="vessel_name" value="{{VESSEL_VAL}}" maxlength="40" placeholder="Opcional">
    <button type="button" class="btn btn-test" onclick="testCfg()">&#128269; Probar conexion (token + URL)</button>
    <div id="testMsg" class="msg"></div>
    <button type="submit" class="btn btn-save">&#128190; Guardar configuracion</button>
  </form>
</div>

<script>
function toggleImport(){
  var b=document.getElementById('impBox');
  b.classList.toggle('show');
}
function testCfg(){
  var m=document.getElementById('testMsg');
  m.className='msg';
  m.style.display='block';
  m.innerHTML='&#9203; Probando conexion con el servidor...';
  var tok=document.getElementById('token').value.trim();
  var url=document.getElementById('api_url').value.trim();
  if(!tok||!url){
    m.className='msg-err';
    m.innerHTML='&#10008; Completa el token y la URL del API primero.';
    return;
  }
  fetch('/test',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},
    body:new URLSearchParams({token:tok,api_url:url})})
  .then(function(r){return r.json();})
  .then(function(j){
    m.className=j.ok?'msg-ok':'msg-err';
    m.innerHTML=(j.ok?'&#10004; ':'&#10008; ')+j.message;
    if(j.ok) m.innerHTML+=' <b>Puedes presionar Guardar.</b>';
  })
  .catch(function(){
    m.className='msg-err';
    m.innerHTML='&#10008; No se pudo ejecutar la prueba.';
  });
}
function applyImport(){
  var m=document.getElementById('impMsg');
  m.className='';
  try{
    var c=JSON.parse(document.getElementById('impTa').value);
    if(!c.token||!c.api_url){throw new Error('Faltan token o api_url');}
    var ids=['token','api_url','interval','vessel_name'];
    for(var i=0;i<ids.length;i++){
      var el=document.getElementById(ids[i]);
      if(el&&c[ids[i]]!==undefined)el.value=c[ids[i]];
    }
    m.className='imp-ok';
    m.innerHTML='&#10004; Campos completados. Presiona <b>Guardar</b>.';
  }catch(e){
    m.className='imp-err';
    m.innerHTML='&#10008; '+e.message;
  }
}
</script>
</body>
</html>
)rawhtml";

static const char SAVED_HTML[] PROGMEM = R"rawhtml(
<!DOCTYPE html>
<html lang="es">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1.0">
<title>Tracking GPS - Guardado</title>
<style>
body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,sans-serif;background:#0f172a;color:#e2e8f0;display:flex;align-items:center;justify-content:center;min-height:100vh;margin:0}
.card{background:#1e293b;border-radius:12px;padding:32px;text-align:center;max-width:360px;width:90%}
h1{color:#6ee7b7;margin-bottom:8px}
p{color:#94a3b8;margin-bottom:20px;line-height:1.5}
.spin{display:inline-block;width:28px;height:28px;border:3px solid #334155;border-top-color:#3b82f6;border-radius:50%;animation:girar 0.8s linear infinite}
@keyframes girar{to{transform:rotate(360deg)}}
</style>
</head>
<body>
<div class="card">
  <h1>&#10004; Configuracion guardada</h1>
  <p>El dispositivo se esta reiniciando para aplicar la nueva configuracion...</p>
  <div class="spin"></div>
</div>
</body>
</html>
)rawhtml";


class EthernetManager
{
public:
  /**
   * Inicializa Ethernet.
   * setupMode=true  → dispositivo SIN configurar: prioriza que el técnico
   *                   pueda abrir http://tracking.local (cable directo o router).
   * setupMode=false → modo normal: cliente DHCP que reporta al API.
   */
  bool begin(bool setupMode = false)
  {
    _storage = &StorageManager::getInstance();

    Serial.println("[ETH] Iniciando Ethernet WT32-ETH01 V1.4...");
    Serial.printf("[ETH] PHY: LAN8720, ADDR=%d, MDC=%d, MDIO=%d, CLK=GPIO0_IN\n",
                  ETH_PHY_ADDR, ETH_PHY_MDC, ETH_PHY_MDIO);

    ETH.begin(ETH_PHY_ADDR, ETH_PHY_POWER, ETH_PHY_MDC, ETH_PHY_MDIO,
              ETH_PHY_TYPE, ETH_CLK_MODE);

    if (setupMode)
    {
      return _beginSetupMode();
    }

    if (!_waitForConnection(15000))
    {
      Serial.println("[ETH] Sin cable de red conectado en 15s");
      return false;
    }

    _waitForIp(8000);
    Serial.printf("[ETH] Conectado. IP: %s\n", getIP().c_str());
    Serial.printf("[ETH] MAC: %s\n", getMac().c_str());

    if (MDNS.begin(MDNS_HOSTNAME))
    {
      Serial.printf("[ETH] mDNS: %s.local\n", MDNS_HOSTNAME);
    }
    else
    {
      Serial.println("[ETH] mDNS: fallo al iniciar");
    }

    beginOta();

    return true;
  }

  // ── OTA (actualización de firmware por red, sin YP-05) ────
  void beginOta()
  {
    ArduinoOTA.setHostname(MDNS_HOSTNAME);
    ArduinoOTA.onStart([]()
                       { Serial.println("[OTA] Iniciando actualizacion por red..."); });
    ArduinoOTA.onEnd([]()
                     { Serial.println("\n[OTA] Listo. Reiniciando..."); });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                          { Serial.printf("[OTA] Progreso: %u%%\r", progress / (total / 100)); });
    ArduinoOTA.onError([](ota_error_t error)
                       {
      Serial.printf("[OTA] Error: %u\n", error);
      if (error == OTA_AUTH_ERROR) Serial.println("[OTA] Error de autenticacion");
      else if (error == OTA_BEGIN_ERROR) Serial.println("[OTA] Error al iniciar");
      else if (error == OTA_CONNECT_ERROR) Serial.println("[OTA] Error de conexion");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("[OTA] Error al recibir");
      else if (error == OTA_END_ERROR) Serial.println("[OTA] Error al finalizar"); });
    ArduinoOTA.begin();
    Serial.println("[OTA] Firmware actualizable por red (PlatformIO > espota)");
  }

  /** Llamar en cada iteración del loop para atender OTA */
  void handleOta()
  {
    ArduinoOTA.handle();
  }

  bool isConnected()
  {
    return ETH.linkUp();
  }

  /** ¿Tiene una IP válida asignada? (no 0.0.0.0 ni enlace local) */
  bool hasIp()
  {
    IPAddress ip = ETH.localIP();
    return ip[0] != 0 && !(ip[0] == 169 && ip[1] == 254);
  }

  String getIP()
  {
    return ETH.localIP().toString();
  }

  String getMac()
  {
    uint8_t mac[6];
    ETH.macAddress(mac);
    char buf[18];
    snprintf(buf, sizeof(buf), "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return String(buf);
  }

  void reconnect()
  {
    if (!isConnected())
    {
      Serial.println("[ETH] Link caido. Esperando reconexion...");
      _waitForConnection(5000);
    }
  }

  // ── Servidor web de configuración ───────────────────────────

  void startConfigServer()
  {
    if (_server != nullptr)
    {
      return;
    }

    _server = new WebServer(CONFIG_SERVER_PORT);

    _server->on("/", HTTP_GET, [this]()
                { _handleRoot(); });

    _server->on("/save", HTTP_POST, [this]()
                { _handleSave(); });

    _server->on("/test", HTTP_POST, [this]()
                { _handleTest(); });

    _server->begin();
    Serial.printf("[ETH] Servidor config iniciado en http://%s:%d\n",
                  getIP().c_str(), CONFIG_SERVER_PORT);
  }

  void stopConfigServer()
  {
    if (_server != nullptr)
    {
      _server->stop();
      delete _server;
      _server = nullptr;
      Serial.println("[ETH] Servidor config detenido");
    }
  }

  void handleClient()
  {
    if (_server != nullptr)
    {
      _server->handleClient();
    }
  }

  bool isConfigServerRunning()
  {
    return _server != nullptr;
  }

  // ── Verificación del servidor API ───────────────────────────

  int verifyServerConnection()
  {
    DeviceConfig cfg = _storage->load();

    Serial.println();
    Serial.println("╔══════════════════════════════════════════╗");
    Serial.println("║  VERIFICACION DE CONEXION AL SERVIDOR    ║");
    Serial.println("╚══════════════════════════════════════════╝");
    Serial.printf("  Ethernet: %s\n", isConnected() ? "CONECTADO" : "DESCONECTADO");
    Serial.printf("  IP:       %s\n", isConnected() ? getIP().c_str() : "---");
    Serial.printf("  API:      %s\n", cfg.apiUrl.c_str());
    Serial.printf("  Token:    %s...%s\n",
                  cfg.deviceToken.substring(0, 4).c_str(),
                  cfg.deviceToken.length() > 4
                      ? cfg.deviceToken.substring(cfg.deviceToken.length() - 4).c_str()
                      : "????");

    if (!isConnected())
    {
      Serial.println("  [FAIL] Ethernet no conectado");
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

    String payload = "{\"firmware\":\"" + String(FIRMWARE_VERSION) +
                     "\",\"device_type\":\"" + String(DEVICE_TYPE) +
                     "\",\"uptime\":" + String(millis() / 1000) + "}";

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
  StorageManager *_storage = nullptr;
  WebServer *_server = nullptr;

  bool _waitForConnection(unsigned long timeout)
  {
    unsigned long start = millis();
    while (!ETH.linkUp() && (millis() - start < timeout))
    {
      delay(250);
    }
    return ETH.linkUp();
  }

  bool _waitForIp(unsigned long timeout)
  {
    unsigned long start = millis();
    while (millis() - start < timeout)
    {
      IPAddress ip = ETH.localIP();
      // IP real (no 0.0.0.0 ni 169.254.x.x de enlace local)
      if (ip[0] != 0 && !(ip[0] == 169 && ip[1] == 254))
      {
        return true;
      }
      delay(200);
    }
    return false;
  }

  /**
   * Modo instalación: el dispositivo SIN configurar debe ser alcanzable
   * por el técnico en http://tracking.local
   *   - Si hay un router con DHCP  → usa la IP que le den y sirve ahí.
   *   - Si es cable DIRECTO laptop → cae a IP fija 169.254.1.1
   *     (la laptop se auto-asigna 169.254.x.x y lo alcanza igual).
   */
  bool _beginSetupMode()
  {
    Serial.println();
    Serial.println("╔══════════════════════════════════════════╗");
    Serial.println("║  MODO INSTALACION (dispositivo sin       ║");
    Serial.println("║  configuracion)                           ║");
    Serial.println("╚══════════════════════════════════════════╝");
    Serial.println("[ETH] Conecta un cable de red:");
    Serial.println("[ETH]   A) laptop -> dispositivo (DIRECTO), o");
    Serial.println("[ETH]   B) dispositivo -> router de la red");
    Serial.println("[ETH] Esperando cable...");

    if (!_waitForConnection(30000))
    {
      Serial.println("[ETH] Sin cable en 30s. Reiniciando para reintentar...");
      return false;
    }

    // 1) Intentar DHCP (caso B: red con router que reparte IPs)
    if (_waitForIp(SETUP_DHCP_WAIT_MS))
    {
      Serial.printf("[ETH] Red con DHCP. IP: %s\n", getIP().c_str());
    }
    else
    {
      // 2) Sin DHCP (caso A: cable directo a la laptop)
      IPAddress ip, gw, mask;
      ip.fromString(SETUP_LINK_LOCAL_IP);
      mask.fromString(SETUP_LINK_MASK);
      ETH.config(ip, ip, mask);
      delay(500);
      Serial.printf("[ETH] Sin DHCP -> IP cable directo: %s\n", getIP().c_str());
    }

    Serial.printf("[ETH] MAC: %s\n", getMac().c_str());
    Serial.println();
    Serial.println("  ┌─────────────────────────────────────────┐");
    Serial.println("  │  ABRE EL NAVEGADOR EN:                  │");
    Serial.println("  │     http://tracking.local               │");
    Serial.println("  │  O EN:  http://" + getIP() + "          │");
    Serial.println("  └─────────────────────────────────────────┘");

    if (MDNS.begin(MDNS_HOSTNAME))
    {
      Serial.printf("[ETH] mDNS: %s.local\n", MDNS_HOSTNAME);
    }

    beginOta();
    return true;
  }

  void _handleRoot()
  {
    DeviceConfig cfg = _storage->load();

    String html = FPSTR(CONFIG_HTML);
    html.replace("{{MAC}}", getMac());
    html.replace("{{FW}}", FIRMWARE_VERSION);
    html.replace("{{IP}}", getIP());

    if (cfg.deviceToken.length() > 0)
    {
      html.replace("{{MODE_BANNER}}",
                   "<div style=\"background:#0c4a6e;border:1px solid #38bdf8;color:#bae6fd;"
                   "padding:10px 12px;border-radius:8px;font-size:13px;margin-bottom:12px\">"
                   "&#9989; <b>Dispositivo en funcionamiento</b> — reportando a la API. "
                   "Esta pagina muestra el estado; puedes actualizar la config si es necesario.</div>");
    }
    else
    {
      html.replace("{{MODE_BANNER}}",
                   "<div style=\"background:#065f46;border:1px solid #34d399;color:#a7f3d0;"
                   "padding:10px 12px;border-radius:8px;font-size:13px;margin-bottom:12px\">"
                   "&#128640; <b>Modo Instalacion</b> — completa el formulario y presiona "
                   "<b>Guardar</b>. El dispositivo se reiniciara solo y quedara listo.</div>");
    }

    if (cfg.deviceToken.length() > 0)
    {
      html.replace("{{TOKEN}}", cfg.deviceToken.substring(0, 6) + "...");
    }
    else
    {
      html.replace("{{TOKEN}}", "No configurado");
    }
    html.replace("{{APIURL}}", cfg.apiUrl.length() > 0 ? cfg.apiUrl : "No configurado");
    html.replace("{{INTERVAL}}", String(cfg.sendInterval));
    html.replace("{{VESSEL}}", cfg.vesselName.length() > 0 ? cfg.vesselName : "Sin nombre");

    html.replace("{{TOKEN_VAL}}", cfg.deviceToken);
    html.replace("{{APIURL_VAL}}", cfg.apiUrl);
    html.replace("{{INTERVAL_VAL}}", String(cfg.sendInterval / 1000));
    html.replace("{{VESSEL_VAL}}", cfg.vesselName);

    _server->send(200, "text/html; charset=utf-8", html);
  }

  /**
   * Prueba la conexión con el token y URL escritos en el formulario
   * (aún no guardados). Envía un heartbeat real y reporta el resultado.
   */
  void _handleTest()
  {
    String token = _server->arg("token");
    String url = _server->arg("api_url");

    if (token.length() == 0 || url.length() == 0)
    {
      _server->send(200, "application/json",
                    "{\"ok\":false,\"message\":\"Completa el token y la URL del API.\"}");
      return;
    }

    Serial.println("[TEST] Probando conexion...");
    Serial.printf("[TEST] URL: %s\n", url.c_str());
    Serial.printf("[TEST] Token: %s...%s\n",
                  token.substring(0, 4).c_str(),
                  token.length() > 4 ? token.substring(token.length() - 4).c_str() : "");

    HTTPClient http;
    http.begin(url);
    http.setTimeout(HTTP_TIMEOUT);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("X-Device-Token", token);

    String payload = "{\"firmware\":\"" + String(FIRMWARE_VERSION) +
                     "\",\"device_type\":\"" + String(DEVICE_TYPE) +
                     "\",\"uptime\":" + String(millis() / 1000) + "}";

    int httpCode = http.POST(payload);
    String errBody = (httpCode <= 0) ? http.errorToString(httpCode) : "";
    http.end();

    String message;
    if (httpCode == 200 || httpCode == 202)
    {
      message = "Conexion exitosa y token VALIDO. Puedes guardar.";
    }
    else if (httpCode == 401 || httpCode == 403)
    {
      message = "Servidor alcanzado pero el TOKEN es INVALIDO (HTTP " +
                String(httpCode) + "). Verifica el token.";
    }
    else if (httpCode > 0)
    {
      message = "Servidor respondio HTTP " + String(httpCode) +
                ". Verifica la URL del API.";
    }
    else
    {
      message = "No se pudo conectar: " + errBody + ". Verifica la URL y la red.";
    }
    message.replace("\"", "'");

    String json = "{\"ok\":" +
                  String((httpCode == 200 || httpCode == 202) ? "true" : "false") +
                  ",\"code\":" + String(httpCode) +
                  ",\"message\":\"" + message + "\"}";

    Serial.printf("[TEST] Resultado: %s\n", json.c_str());
    _server->send(200, "application/json", json);
  }

  void _handleSave()
  {
    DeviceConfig cfg;

    if (_server->hasArg("token"))
    {
      cfg.deviceToken = _server->arg("token");
    }
    if (_server->hasArg("api_url"))
    {
      cfg.apiUrl = _server->arg("api_url");
    }
    if (_server->hasArg("interval"))
    {
      long sec = _server->arg("interval").toInt();
      cfg.sendInterval = (sec > 0) ? sec * 1000 : DEFAULT_SEND_INTERVAL;
    }
    if (_server->hasArg("vessel_name"))
    {
      cfg.vesselName = _server->arg("vessel_name");
    }

    _storage->save(cfg);
    Serial.println("[ETH] Configuracion guardada via web. Reiniciando...");

    _server->send(200, "text/html; charset=utf-8", FPSTR(SAVED_HTML));
    delay(2000);
    ESP.restart();
  }
};

#endif
