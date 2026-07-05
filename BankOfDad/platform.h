#pragma once

#include <Arduino.h>

void platformInit();
void platformTick();

bool platformStaConnected();
bool platformApActive();
const char *platformStaIpText();
const char *platformApIpText();
bool platformOtaReady();
void platformWifiReconnect();
