#pragma once
#include <stdint.h>

struct EyeFrame;

namespace LidsApi {

  void init();

  // Patch 9: čistě deterministické – žádné millis() uvnitř
  // dtMs = delta času z main loop
  void update(uint32_t dtMs, EyeFrame& ioFrame);

  // ruční trigger – bez závislosti na čase
  void triggerBlink();

  bool isBlinking();
}
