#pragma once
#include <stdint.h>

// Oficiální datový model jednoho "frame" oka
struct EyeFrame {
  // pozice duhovky (v pixelech)
  int16_t irisX = 0;
  int16_t irisY = 0;

  // dirty flagy – co je potřeba překreslit
  bool irisDirty = false;
  bool lidsDirty = false;   // připraveno na Patch 7
};
