#pragma once
#include <stdint.h>

namespace EyeApi {
  void init();
  void update(uint32_t dtMs);
  void render();
}
