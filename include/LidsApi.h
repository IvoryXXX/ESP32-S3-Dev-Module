#pragma once
#include <stdint.h>

struct EyeFrame;

namespace LidsApi {
  void init();

  // dtMs = delta time z main loop
  void update(uint32_t dtMs, EyeFrame& ioFrame);

  // manuální vyvolání bliku
  void triggerBlink();

  bool isBlinking();
}
