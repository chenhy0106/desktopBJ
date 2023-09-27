#ifndef APP_BIUBIUBIU_H
#define APP_BIUBIUBIU_H

#include "sys/interface.h"

extern APP_OBJ biubiubiu_app;

#define DEBUG 1
#define DPRINTF(fmt, ...) if (DEBUG) Serial.println(fmt, ##__VA_ARGS__)

#endif