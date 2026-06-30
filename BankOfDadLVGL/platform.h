#pragma once

#include <WebServer.h>
#include "model.h"

extern WebServer g_webServer;

void platformInit();
void platformLoop();
void platformStartWifiAp();
