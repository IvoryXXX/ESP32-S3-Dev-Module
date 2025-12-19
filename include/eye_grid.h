#pragma once
#include <Arduino.h>

// ============================================================================
// EyeGrid – generování bodů pohybu irisu v KRUHU + masky (víčka)
// ----------------------------------------------------------------------------
// Máme 2 seznamy bodů:
//  - ALL      : všechny body v kruhu (debug / overlay / kontrola)
//  - ALLOWED  : body po maskách (tohle používá EyeGaze)
// ============================================================================

struct GridPoint {
  int16_t x;
  int16_t y;
};

struct GridRect {
  int16_t x;
  int16_t y;
  int16_t w;
  int16_t h;
};

namespace EyeGrid {

  // Build grid (kruh → ALL body)
  void build(
    int cx,
    int cy,
    int radiusPx,
    int stepX,
    int stepY,
    int edgeRingPx
  );

  // Nastaví masky (víčka) a přegeneruje ALLOWED body
  void setMaskRects(const GridRect* rects, int count);

  // ==========================================================================
  // NOVÉ API (read-only)
  // ==========================================================================
  const GridPoint* points();   // ALLOWED body
  int pointCount();

  bool isEdge(int index);      // edge flag pro ALLOWED index

  // Debug / mask getters
  int getDebugCircleRadiusPx();
  const GridRect* getMaskRects();
  int getMaskRectCount();

  // ==========================================================================
  // KOMPATIBILNÍ API (pro existující eye_gaze.cpp / debug_grid.cpp)
  // ==========================================================================
  int centerX();
  int centerY();
  int radius();

  // krok mřížky (uložený z build()) – používá EyeGaze pro „mikro-hop“
  int stepX();
  int stepY();

  // ALL body (bez masek)
  int allCount();
  const GridPoint& getAll(int i);

  // ALLOWED body (po maskách)
  int count();                 // alias pro pointCount()
  const GridPoint& get(int i);

  bool isMasked(int x, int y); // vrátí true, pokud bod leží uvnitř libovolné masky
  bool isEdgeIndex(int i);     // alias pro isEdge(i)
}
