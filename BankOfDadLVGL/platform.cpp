#include "platform.h"
#include "storage.h"
#include <WiFi.h>

WebServer g_webServer(80);

static String buildChorePage(bool saved) {
    String html =
        "<!DOCTYPE html><html><head>"
        "<meta name='viewport' content='width=device-width,initial-scale=1'>"
        "<title>Bank of Dad v2.2</title><style>"
        "body{font-family:monospace;max-width:520px;margin:0 auto;padding:16px;"
        "background:#001100;color:#33ff33}"
        "h1{margin:0 0 8px}.chore{background:#002200;padding:10px;margin:8px 0;border:1px solid #1a991a}"
        "input{width:100%;box-sizing:border-box;padding:8px;background:#000;color:#3f3;border:1px solid #393}"
        "button{width:100%;padding:12px;background:#060;color:#3f3;border:1px solid #3f3;margin-top:8px}"
        ".ok{background:#030;padding:8px;margin-bottom:8px}"
        "</style></head><body><h1>BANK OF DAD v2.2</h1>"
        "<p>UK household chore editor</p>";
    if (saved) html += "<div class='ok'>Saved</div>";
    html += "<form method='POST' action='/update'>";
    for (int i = 0; i < (int)g_bank.choreCount; i++) {
        if (!g_bank.chores[i].enabled && g_bank.chores[i].title[0] == '\0') continue;
        char buf[12];
        dtostrf(g_bank.chores[i].rewardPence / 100.0f, 1, 2, buf);
        html += "<div class='chore'><label>" + String(g_bank.chores[i].title) + "</label>"
                "<input type='number' name='r" + String(i) + "' step='0.01' min='0' max='4.50' value='" +
                String(buf) + "'>";
        if (g_bank.chores[i].pendingChild >= 0)
            html += "<small>Pending: " + String(g_bank.accounts[g_bank.chores[i].pendingChild].name) + "</small>";
        html += "</div>";
    }
    html += "<button type='submit'>Save</button></form></body></html>";
    return html;
}

static void setupWebRoutes() {
    g_webServer.on("/", HTTP_GET, []() {
        g_webServer.send(200, "text/html", buildChorePage(g_webServer.hasArg("saved")));
    });
    g_webServer.on("/update", HTTP_POST, []() {
        for (int i = 0; i < BANK_CHORE_SLOTS; i++) {
            String rk = "r" + String(i);
            if (g_webServer.hasArg(rk)) {
                float p = g_webServer.arg(rk).toFloat();
                if (p >= 0.0f && p <= 4.50f) {
                    g_bank.chores[i].rewardPence = (long)(p * 100.0f + 0.5f);
                    if (g_bank.chores[i].rewardPence > BANK_MAX_REWARD_PENCE)
                        g_bank.chores[i].rewardPence = BANK_MAX_REWARD_PENCE;
                }
            }
        }
        modelSaveChores();
        g_webServer.sendHeader("Location", "/?saved=1");
        g_webServer.send(302, "text/plain", "");
    });
    g_webServer.onNotFound([]() {
        g_webServer.sendHeader("Location", "/");
        g_webServer.send(302, "text/plain", "");
    });
}

void platformStartWifiAp() {
    WiFi.softAP(BANK_WIFI_AP_SSID, BANK_WIFI_AP_PASS);
    Serial.printf("AP: %s  http://%s\n", BANK_WIFI_AP_SSID,
                  WiFi.softAPIP().toString().c_str());
}

void platformInit() {
    storageInit();
    platformStartWifiAp();
    setupWebRoutes();
    g_webServer.begin();
}

void platformLoop() {
    g_webServer.handleClient();
}
