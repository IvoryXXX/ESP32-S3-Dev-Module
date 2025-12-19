#pragma once
#include <stdint.h>
#include <stdbool.h>

struct SkinAssets;
struct EyeFrame;

// Jednoduchý "dirty rect" (pro optimalizace redraw; zatím hrubé)
struct DirtyRect {
  int16_t x = 0;
  int16_t y = 0;
  int16_t w = 0;
  int16_t h = 0;
  bool valid = false;
};

namespace RenderApi {

  // callback pro dokreslení víček nad aktuální scénu
  typedef void (*LidDrawFn)(const SkinAssets& skin,
                            uint16_t lidTop, uint16_t lidBot,
                            const DirtyRect& dirtyHint);

  // init pouze uloží reference (assets se loadují zvlášť)
  void init(const SkinAssets& skin);

  // nastaví renderer podle globální cfg (rozměry, keying tolerance…)
  void setupRendererFromConfig();

  // načte RAW565 assety do RAM (base/iris/lids)
  bool loadAssets();

  // vykreslí statickou scénu (base)
  void drawStatic();

  // vykreslí iris na pozici (centra) – používá interní patch redraw
  void drawIris(int16_t cx, int16_t cy);

  // Patch 8: frame wrapper (zatím jen reset dirty / no-op)
  void beginFrame();
  void endFrame();

  // Patch 8: registrace víčkového rendereru
  void setLidDrawFn(LidDrawFn fn);

  // Render podle kontraktu (EyeFrame)
  void renderFrame(const EyeFrame& frame);

  // Debug: poslední spočítaný dirty rect (zatím hrubý)
  DirtyRect lastDirty();

} // namespace RenderApi
