#pragma once
#include <stdint.h>

// Oficiální datový model jednoho "frame" oka
struct EyeFrame {
  // pozice duhovky (v pixelech)
  int16_t irisX = 0;
  int16_t irisY = 0;

  // víčka: 0 = plně otevřeno, 1000 = plně zavřeno (fixpoint, bez floatů)
  // (top/bot zvlášť – připraveno na partial blink + mechaniku)
  uint16_t lidTop = 0;
  uint16_t lidBot = 0;

  // dirty flagy – co je potřeba překreslit
  bool irisDirty = false;
  bool lidsDirty = false;
};
