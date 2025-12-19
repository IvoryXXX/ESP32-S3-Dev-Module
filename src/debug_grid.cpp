#include "debug_grid.h"

#include "eye_grid.h"
#include "config.h"

// lokální RGB565 konstanty (bez TFT_eSPI)
static constexpr uint16_t RGB565(uint8_t r, uint8_t g, uint8_t b) {
  return (uint16_t)(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
}
static constexpr uint16_t C_RED   = RGB565(255, 0, 0);
static constexpr uint16_t C_GREEN = RGB565(0, 255, 0);
static constexpr uint16_t C_BLUE  = RGB565(0, 0, 255);
static constexpr uint16_t C_WHITE = RGB565(255, 255, 255);
static constexpr uint16_t C_YELLOW= RGB565(255, 255, 0);

static inline bool cbOk(const DebugGridCallbacks& cb) {
  return cb.drawPixel && cb.drawRect && cb.drawCircle && cb.drawLine;
}

static void drawMaskRects(const DebugGridCallbacks& cb) {
  if (!cfg.dbgOverlay || !cfg.dbgGrid) return;

  const GridRect* rects = EyeGrid::getMaskRects();
  int count = EyeGrid::getMaskRectCount();
  if (!rects || count <= 0) return;

  for (int i = 0; i < count; i++) {
    const GridRect &r = rects[i];
    cb.drawRect(cb.ctx, r.x, r.y, r.w, r.h, C_RED);
  }
}

static void drawAllCirclePoints(const DebugGridCallbacks& cb) {
  if (!cfg.dbgOverlay || !cfg.dbgGrid) return;

  const int n = EyeGrid::allCount();
  for (int i = 0; i < n; i++) {
    const GridPoint& p = EyeGrid::getAll(i);
    // masked = červeně, jinak bíle
    const bool masked = EyeGrid::isMasked(p.x, p.y);
    cb.drawPixel(cb.ctx, p.x, p.y, masked ? C_RED : C_WHITE);
  }
}

static void drawAllowedPoints(const DebugGridCallbacks& cb) {
  if (!cfg.dbgOverlay || !cfg.dbgGrid) return;

  const int n = EyeGrid::count();
  for (int i = 0; i < n; i++) {
    const GridPoint& p = EyeGrid::get(i);
    const bool edge = EyeGrid::isEdgeIndex(i);
    cb.drawPixel(cb.ctx, p.x, p.y, edge ? C_GREEN : C_BLUE);
  }
}

static void drawDebugCircle(const DebugGridCallbacks& cb) {
  if (!cfg.dbgOverlay || !cfg.dbgGrid) return;

  const int cx = EyeGrid::centerX();
  const int cy = EyeGrid::centerY();
  const int r  = EyeGrid::getDebugCircleRadiusPx();
  if (r > 0) cb.drawCircle(cb.ctx, cx, cy, r, C_YELLOW);
}

void debugGridRenderOverlay(const DebugGridCallbacks& cb) {
  if (!cfg.dbgOverlay) return;
  if (!cbOk(cb)) return;

  // pořadí overlay prvků: circle -> mask rects -> all pts -> allowed pts
  drawDebugCircle(cb);
  drawMaskRects(cb);

  if (cfg.dbgAllPoints) {
    drawAllCirclePoints(cb);
  } else {
    drawAllowedPoints(cb);
  }
}
