#include "platform.h"
#include "config.h"
#include "model.h"

#include <ArduinoOTA.h>
#include <WiFi.h>

static bool s_staConnected = false;
static bool s_apActive = false;
static bool s_otaReady = false;
static bool s_wifiStarted = false;
static bool s_reconnectPending = false;
static char s_staIp[16] = "0.0.0.0";
static char s_apIp[16] = "0.0.0.0";
static unsigned long s_lastStaAttemptMs = 0;

static void updateIpStrings() {
  if (WiFi.status() == WL_CONNECTED) {
    strlcpy(s_staIp, WiFi.localIP().toString().c_str(), sizeof(s_staIp));
    s_staConnected = true;
  } else {
    strlcpy(s_staIp, "0.0.0.0", sizeof(s_staIp));
    s_staConnected = false;
  }

  if (WiFi.getMode() & WIFI_MODE_AP) {
    strlcpy(s_apIp, WiFi.softAPIP().toString().c_str(), sizeof(s_apIp));
    s_apActive = true;
  } else {
    strlcpy(s_apIp, "0.0.0.0", sizeof(s_apIp));
    s_apActive = false;
  }
}

static void setupOta() {
  if (s_otaReady) return;
  if (!s_staConnected) return;

  ArduinoOTA.setHostname(OTA_HOSTNAME);
  ArduinoOTA.setPort(OTA_PORT);
  ArduinoOTA.setPassword(OTA_PASSWORD);

  ArduinoOTA.onStart([]() {
    Serial.println("[OTA] Update starting");
  });

  ArduinoOTA.onEnd([]() {
    Serial.println("[OTA] Update complete");
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    static unsigned long lastPrint = 0;
    if (millis() - lastPrint < 500) return;
    lastPrint = millis();
    Serial.printf("[OTA] Progress: %u%%\n", (progress * 100) / total);
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("[OTA] Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive failed");
    else if (error == OTA_END_ERROR) Serial.println("End failed");
    else Serial.println("Unknown");
  });

  ArduinoOTA.begin();
  s_otaReady = true;

  Serial.printf("[OTA] Ready on %s — password required\n", s_staIp);
  Serial.printf("[OTA] Hostname: %s.local (port %u)\n", OTA_HOSTNAME, OTA_PORT);
}

static void stopOta() {
  if (!s_otaReady) return;
  ArduinoOTA.end();
  s_otaReady = false;
}

static void startAp() {
  WiFi.mode(WIFI_AP_STA);
  if (!WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASS)) {
    Serial.println("[WiFi] AP start failed");
    return;
  }

  s_apActive = true;
  updateIpStrings();
  Serial.printf("[WiFi] AP: %s  http://%s\n", WIFI_AP_SSID, s_apIp);
}

static void attemptStaConnect() {
  char ssid[33] = {0};
  char pass[65] = {0};

  if (!modelWifiHasCredentials()) {
    s_staConnected = false;
    updateIpStrings();
    return;
  }

  modelWifiGetCredentials(ssid, sizeof(ssid), pass, sizeof(pass));
  if (ssid[0] == '\0') return;

  stopOta();
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, pass);

  Serial.printf("[WiFi] Connecting to \"%s\"...\n", ssid);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED &&
         millis() - start < WIFI_STA_CONNECT_TIMEOUT_MS) {
    delay(250);
    Serial.print('.');
  }
  Serial.println();

  updateIpStrings();
  s_lastStaAttemptMs = millis();

  if (s_staConnected) {
    Serial.printf("[WiFi] Home network connected: http://%s\n", s_staIp);
    setupOta();
  } else {
    Serial.println("[WiFi] Home network unavailable — AP still active");
  }
}

static void ensureWifiStarted() {
  if (s_wifiStarted) return;
  s_wifiStarted = true;

  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
  startAp();
  attemptStaConnect();
}

void platformInit() {
  ensureWifiStarted();
}

void platformTick() {
  if (!s_wifiStarted) ensureWifiStarted();

  if (s_reconnectPending) {
    s_reconnectPending = false;
    attemptStaConnect();
  } else if (modelWifiHasCredentials() && !s_staConnected &&
             millis() - s_lastStaAttemptMs >= WIFI_STA_RETRY_MS) {
    attemptStaConnect();
  }

  updateIpStrings();

  if (s_staConnected && !s_otaReady) setupOta();
  if (!s_staConnected && s_otaReady) stopOta();

  if (s_otaReady) ArduinoOTA.handle();
}

bool platformStaConnected() { return s_staConnected; }
bool platformApActive() { return s_apActive; }
const char *platformStaIpText() { return s_staIp; }
const char *platformApIpText() { return s_apIp; }
bool platformOtaReady() { return s_otaReady; }

void platformWifiReconnect() {
  s_reconnectPending = true;
}
