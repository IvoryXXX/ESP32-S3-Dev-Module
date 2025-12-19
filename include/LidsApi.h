#pragma once
#include <stdint.h>

struct EyeFrame;

namespace LidsApi {

  // Patch 7: jednoduchá autonomie mrkání (bez závislosti na cfg, zatím napevno)
  void init();

  // aktualizuje vnitřní stav víček, zapíše lidTop/lidBot + nastaví lidsDirty když se změnilo
  void update(uint32_t nowMs, EyeFrame& ioFrame);

  // ruční triggery (do budoucna pro interakce / test)
  void triggerBlink(uint32_t nowMs);

  // debug/utility
  bool isBlinking();
}
