#pragma once
#include <stdint.h>

struct EyeRenderState; // doplníme v EyeApi.cpp (ne tady)

namespace RenderApi {
  void init();                 // init TFT + renderer
  void beginFrame();
  void draw(const EyeRenderState& s);
  void endFrame();
}
