#pragma once
#include <stdint.h>

struct SkinAssets;
struct EyeFrame;

struct DirtyRect {
  int16_t x = 0, y = 0, w = 0, h = 0;
};

namespace RenderApi {

  using LidDrawFn = void (*)(const SkinAssets& skin,
                            uint16_t lidTop, uint16_t lidBot,
                            const DirtyRect& dirty);

  void init();
  void drawStatic(const SkinAssets& skin);
  void drawFrame(const EyeFrame& frame);

  // hook pro kreslení víček (Patch 10)
  void setLidDrawFn(LidDrawFn fn);

  // volitelně (debug / budoucí)
  void setSwapBytes(bool v);
  void setKeyColor565(uint16_t key);

}
