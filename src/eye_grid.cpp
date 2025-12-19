#include "eye_grid.h"
#include <math.h>

// ============================================================================
// Nastavení kapacit
// (můžeš zvednout, když budeš mít jemnější step)
// ============================================================================
static constexpr int MAX_ALL     = 512;
static constexpr int MAX_ALLOWED = 512;
static constexpr int MAX_MASKS   = 4;

// ============================================================================
// Interní data
// ============================================================================
static GridPoint gAllPoints[MAX_ALL];
static int       gAllCount = 0;

static GridPoint gAllowedPoints[MAX_ALLOWED];
static bool      gAllowedEdge[MAX_ALLOWED];
static int       gAllowedCount = 0;

static GridRect  gMasks[MAX_MASKS];
static int       gMaskCount = 0;

static int gCx = 0;
static int gCy = 0;
static int gRadiusPx = 0;
static int gEdgeRingPx = 0;
static int gStepX = 0;
static int gStepY = 0;

// ============================================================================
// Helpers
// ============================================================================
static bool pointInRect(int x, int y, const GridRect& r) {
  return (x >= r.x &&
          y >= r.y &&
          x <  r.x + r.w &&
          y <  r.y + r.h);
}

static float distf(int x1, int y1, int x2, int y2) {
  float dx = (float)(x1 - x2);
  float dy = (float)(y1 - y2);
  return sqrtf(dx*dx + dy*dy);
}

static bool isMaskedInternal(int x, int y) {
  for (int i = 0; i < gMaskCount; i++) {
    if (pointInRect(x, y, gMasks[i])) return true;
  }
  return false;
}

static void rebuildAllowedFromAll() {
  gAllowedCount = 0;

  // hranice “edge prstence” – počítáme podle radiusPx
  // edgeRingPx je v pixelech logiky (většinou 2..8), ale my to přepočteme na práh
  float edgeThreshold = (float)gRadiusPx - (float)gEdgeRingPx;
  if (edgeThreshold < 0.0f) edgeThreshold = 0.0f;

  for (int i = 0; i < gAllCount; i++) {
    const GridPoint& p = gAllPoints[i];

    if (isMaskedInternal(p.x, p.y)) continue;

    if (gAllowedCount >= MAX_ALLOWED) break;

    gAllowedPoints[gAllowedCount] = p;

    float d = distf(p.x, p.y, gCx, gCy);
    gAllowedEdge[gAllowedCount] = (d >= edgeThreshold);

    gAllowedCount++;
  }
}

// ============================================================================
// API
// ============================================================================
namespace EyeGrid {

void build(int cx, int cy, int radiusPx, int stepX, int stepY, int edgeRingPx) {
  gCx = cx;
  gCy = cy;
  gRadiusPx = radiusPx;
  gEdgeRingPx = edgeRingPx;
  gStepX = stepX;
  gStepY = stepY;

  gAllCount = 0;

  if (stepX <= 0 || stepY <= 0 || radiusPx <= 0) {
    gAllowedCount = 0;
    return;
  }

  // kolik kroků v mřížce je potřeba, aby se pokryl kruh
  int maxI = radiusPx / stepX + 1;
  int maxJ = radiusPx / stepY + 1;

  for (int j = -maxJ; j <= maxJ; j++) {
    for (int i = -maxI; i <= maxI; i++) {
      int x = cx + i * stepX;
      int y = cy + j * stepY;

      float d = distf(x, y, cx, cy);
      if (d > (float)radiusPx) continue;

      if (gAllCount >= MAX_ALL) {
        // už se nevejde – radši stop než přepisovat paměť
        rebuildAllowedFromAll();
        return;
      }

      gAllPoints[gAllCount++] = { (int16_t)x, (int16_t)y };
    }
  }

  // po buildnutí all bodů hned vytvoř allowed
  rebuildAllowedFromAll();
}

void setMaskRects(const GridRect* rects, int count) {
  gMaskCount = 0;

  if (rects && count > 0) {
    if (count > MAX_MASKS) count = MAX_MASKS;
    for (int i = 0; i < count; i++) gMasks[gMaskCount++] = rects[i];
  }

  // jakmile se změní masky, přepočítáme allowed list
  rebuildAllowedFromAll();
}

// --------------------
// nové read-only
// --------------------
const GridPoint* points() { return gAllowedPoints; }
int pointCount() { return gAllowedCount; }

bool isEdge(int index) {
  if (index < 0 || index >= gAllowedCount) return false;
  return gAllowedEdge[index];
}

int getDebugCircleRadiusPx() { return gRadiusPx; }
const GridRect* getMaskRects() { return (gMaskCount > 0) ? gMasks : nullptr; }
int getMaskRectCount() { return gMaskCount; }

// --------------------
// kompatibilní API
// --------------------
int centerX() { return gCx; }
int centerY() { return gCy; }
int radius()  { return gRadiusPx; }

int stepX() { return gStepX; }
int stepY() { return gStepY; }

int allCount() { return gAllCount; }
const GridPoint& getAll(int i) {
  if (i < 0) i = 0;
  if (i >= gAllCount) i = gAllCount - 1;
  return gAllPoints[i];
}

int count() { return gAllowedCount; }
const GridPoint& get(int i) {
  if (i < 0) i = 0;
  if (i >= gAllowedCount) i = gAllowedCount - 1;
  return gAllowedPoints[i];
}

bool isMasked(int x, int y) { return isMaskedInternal(x, y); }
bool isEdgeIndex(int i) { return isEdge(i); }

} // namespace EyeGrid
