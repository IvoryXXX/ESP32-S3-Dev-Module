#pragma once
#include <stdint.h>

struct EyeFrame;

namespace EyeApi {
  void init();
  void update(uint32_t dtMs);
  void render();
}
