#pragma once
#include <stdbool.h>
#include <stdint.h>

struct SkinAssets;
struct EyeFrame;

// Jednoduchý "dirty rect" pro budoucí optimalizace
struct DirtyRect {
  int16_t x = 0;
  int16_t y = 0;
  int16_t w = 0;
  int16_t h = 0;
  bool valid = false;
};

namespace RenderApi {

  // callback na vykreslení víček (aby RenderApi nemusel znát tvoje interní funkce)
  // lidTop/lidBot: 0..1000 (0=open, 1000=closed)
  using LidDrawFn = void (*)(const SkinAssets& skin, uint16_t lidTop, uint16_t lidBot, const DirtyRect& dirtyHint);

  void init(const SkinAssets& skin);
  void setupRendererFromConfig();
  bool loadAssets();

  void drawStatic();
  void drawIris(int16_t irisX, int16_t irisY);

  // Patch 8: render passes
  void beginFrame();
  void endFrame();

  // Patch 8: registrace víčkového rendereru
  void setLidDrawFn(LidDrawFn fn);

  // Patch 6+: render podle kontraktu
  void renderFrame(const EyeFrame& frame);

  // Debug: poslední spočítaný dirty rect (zatím hrubý)
  DirtyRect lastDirty();

} // namespace RenderApi
