#pragma once
#include <Arduino.h>

bool sdInit();        // init SD (HSPI separate)
bool sdIsReady();     // mounted OK?
void sdEnd();         // optional: unmount/end (not required)
