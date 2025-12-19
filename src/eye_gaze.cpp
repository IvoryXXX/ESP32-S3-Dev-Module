#include "eye_gaze.h"
#include "eye_grid.h"
#include <math.h>

// ============================================================================
// EyeGaze – řízení pohledu po bodech gridu
//
// Cíl: vybírat cílový bod na gridu, chvíli "koukat", pak jet k cíli v malých
// krocích (travel tick). Při výběru cíle:
// - občas se vrať ke středu (centerReturnChancePct)
// - nevybírej pořád stejný bod (avoidSamePoint)
// - penalizuj okrajové body (edgeWeightPct / edgeSoft)
//
// Pozn.: tady NEřešíme víčka, jen "kde může být iris" podle gridu.
// Masky řeší EyeGrid::setMaskRects() při build/rebuild gridu.
// ============================================================================

// -------------------- static state (definice) --------------------
int  EyeGaze::s_curIdx = -1;
int  EyeGaze::s_tgtIdx = -1;
int  EyeGaze::s_curX   = 0;
int  EyeGaze::s_curY   = 0;
int  EyeGaze::s_tgtX   = 0;
int  EyeGaze::s_tgtY   = 0;

bool     EyeGaze::s_travelling   = false;
uint32_t EyeGaze::s_nextActionMs = 0;

// config
uint32_t EyeGaze::s_dwellMin      = 400;
uint32_t EyeGaze::s_dwellMax      = 1400;
uint32_t EyeGaze::s_travelTickMin = 25;
uint32_t EyeGaze::s_travelTickMax = 80;
uint8_t  EyeGaze::s_centerPct     = 35;
bool     EyeGaze::s_avoidSame     = true;
uint8_t  EyeGaze::s_hopMin        = 1;
uint8_t  EyeGaze::s_hopMax        = 3;
uint8_t  EyeGaze::s_edgeBands     = 2;
uint8_t  EyeGaze::s_edgeWeightPct = 40;
bool     EyeGaze::s_edgeSoft      = true;

int EyeGaze::s_lastTargetIdx = -1;

// -------------------- helpers --------------------
uint32_t EyeGaze::randRange(uint32_t a, uint32_t b) {
  if (a == b) return a;
  if (a > b) { uint32_t t=a; a=b; b=t; }
  // Arduino random(max) je [0,max), random(min,max) je [min,max)
  return (uint32_t)random((long)a, (long)b + 1);
}

int EyeGaze::randInt(int a, int b) {
  if (a == b) return a;
  if (a > b) { int t=a; a=b; b=t; }
  return (int)random((long)a, (long)b + 1);
}

int32_t EyeGaze::dist2(int x1,int y1,int x2,int y2) {
  int32_t dx = (int32_t)x2 - (int32_t)x1;
  int32_t dy = (int32_t)y2 - (int32_t)y1;
  return dx*dx + dy*dy;
}

// -------------------- init / reset --------------------
void EyeGaze::init(
  uint32_t dwellMinMs, uint32_t dwellMaxMs,
  uint32_t travelTickMinMs, uint32_t travelTickMaxMs,
  uint8_t centerReturnChancePct, bool avoidSamePoint,
  uint8_t hopMinCells, uint8_t hopMaxCells,
  uint8_t edgeBands, uint8_t edgeWeightPct, bool edgeSoft
) {
  s_dwellMin      = dwellMinMs;
  s_dwellMax      = dwellMaxMs;
  s_travelTickMin = travelTickMinMs;
  s_travelTickMax = travelTickMaxMs;

  s_centerPct  = centerReturnChancePct;
  s_avoidSame  = avoidSamePoint;
  s_hopMin     = hopMinCells;
  s_hopMax     = hopMaxCells;

  s_edgeBands     = edgeBands;
  s_edgeWeightPct = edgeWeightPct;
  s_edgeSoft      = edgeSoft;

  // bezpečné ohraničení
  if (s_centerPct > 100) s_centerPct = 100;
  if (s_edgeWeightPct > 100) s_edgeWeightPct = 100;
  if (s_hopMin < 1) s_hopMin = 1;
  if (s_hopMax < s_hopMin) s_hopMax = s_hopMin;
  // edgeBands má smysl jen >=1, jinak soft vážení nedává význam
  if (s_edgeBands < 1) s_edgeBands = 1;

  resetToCenter();
}

void EyeGaze::resetToCenter() {
  // když ještě grid není, aspoň nuluj
  if (EyeGrid::count() <= 0) {
    s_curIdx = s_tgtIdx = -1;
    s_curX = s_curY = 0;
    s_tgtX = s_tgtY = 0;
    s_travelling = false;
    s_nextActionMs = 0;
    return;
  }

  int cx = EyeGrid::centerX();
  int cy = EyeGrid::centerY();

  s_curIdx = findNearestIndex(cx, cy);
  if (s_curIdx < 0) s_curIdx = 0;

  const GridPoint &p = EyeGrid::get(s_curIdx);
  s_curX = p.x;
  s_curY = p.y;

  s_tgtIdx = s_curIdx;
  s_tgtX = s_curX;
  s_tgtY = s_curY;

  s_lastTargetIdx = -1;
  s_travelling = false;
  s_nextActionMs = millis() + randRange(s_dwellMin, s_dwellMax);
}

// -------------------- getters --------------------
int EyeGaze::x() { return s_curX; }
int EyeGaze::y() { return s_curY; }

int EyeGaze::targetX() { return s_tgtX; }
int EyeGaze::targetY() { return s_tgtY; }

bool EyeGaze::isTravelling() { return s_travelling; }

// -------------------- index helpers --------------------
int EyeGaze::findNearestIndex(int x, int y) {
  int n = EyeGrid::count();
  if (n <= 0) return -1;

  int best = 0;
  int32_t bestD = dist2(x, y, EyeGrid::get(0).x, EyeGrid::get(0).y);

  for (int i = 1; i < n; i++) {
    const GridPoint &p = EyeGrid::get(i);
    int32_t d = dist2(x, y, p.x, p.y);
    if (d < bestD) { bestD = d; best = i; }
  }
  return best;
}

int EyeGaze::pickWeightedTargetIndex() {
  int n = EyeGrid::count();
  if (n <= 0) return -1;

  // 1) šance návratu do středu
  if (s_centerPct > 0) {
    int r = (int)random(0L, 100L); // 0..99
    if (r < (int)s_centerPct) {
      int ci = findNearestIndex(EyeGrid::centerX(), EyeGrid::centerY());
      if (ci >= 0) {
        if (!s_avoidSame || ci != s_lastTargetIdx || n == 1) return ci;
      }
    }
  }

  // 2) vážený výběr z bodů
  //    normál = 100
  //    okraj  = edgeWeightPct (0..100), případně "soft" přechod v okrajovém prstenci
  int wEdge = (int)s_edgeWeightPct;
  if (wEdge < 0) wEdge = 0;
  if (wEdge > 100) wEdge = 100;

  // sečti total
  long total = 0;
  for (int i = 0; i < n; i++) {
    if (s_avoidSame && i == s_lastTargetIdx && n > 1) continue;
    int w = 100;
    if (EyeGrid::isEdgeIndex(i)) {
      if (!s_edgeSoft) {
        w = wEdge;
      } else {
        // soft váha podle vzdálenosti od středu v okrajovém prstenci
        const GridPoint &p = EyeGrid::get(i);
        const float d = sqrtf((float)dist2(p.x, p.y, EyeGrid::centerX(), EyeGrid::centerY()));
        const float radius = (float)EyeGrid::radius();
        const float band   = (float)s_edgeBands;
        const float th     = radius - band;
        float t = 1.0f;
        if (band > 0.0f) {
          t = (d - th) / band;
          if (t < 0.0f) t = 0.0f;
          if (t > 1.0f) t = 1.0f;
        }
        // uprostřed prstence = blíž 100, na kraji = wEdge
        float wf = 100.0f + (float)(wEdge - 100) * t;
        if (wf < 0.0f) wf = 0.0f;
        if (wf > 100.0f) wf = 100.0f;
        w = (int)(wf + 0.5f);
      }
    }
    total += (long)w;
  }
  if (total <= 0) return randInt(0, n - 1);

  long pick = random(0L, total); // 0..total-1
  long acc = 0;

  for (int i = 0; i < n; i++) {
    if (s_avoidSame && i == s_lastTargetIdx && n > 1) continue;
    int w = 100;
    if (EyeGrid::isEdgeIndex(i)) {
      if (!s_edgeSoft) {
        w = wEdge;
      } else {
        const GridPoint &p = EyeGrid::get(i);
        const float d = sqrtf((float)dist2(p.x, p.y, EyeGrid::centerX(), EyeGrid::centerY()));
        const float radius = (float)EyeGrid::radius();
        const float band   = (float)s_edgeBands;
        const float th     = radius - band;
        float t = 1.0f;
        if (band > 0.0f) {
          t = (d - th) / band;
          if (t < 0.0f) t = 0.0f;
          if (t > 1.0f) t = 1.0f;
        }
        float wf = 100.0f + (float)(wEdge - 100) * t;
        if (wf < 0.0f) wf = 0.0f;
        if (wf > 100.0f) wf = 100.0f;
        w = (int)(wf + 0.5f);
      }
    }
    acc += (long)w;
    if (pick < acc) return i;
  }

  // fallback
  return randInt(0, n - 1);
}

// -------------------- movement --------------------
bool EyeGaze::stepTowardTarget() {
  if (EyeGrid::count() <= 0) return false;
  if (s_curIdx < 0 || s_tgtIdx < 0) return false;

  // už jsme v cíli?
  if (s_curIdx == s_tgtIdx) return false;

  const GridPoint &pt = EyeGrid::get(s_tgtIdx);

  int hops = randInt((int)s_hopMin, (int)s_hopMax);

  // děláme několik mikro-hopů za jeden travel tick
  bool movedAny = false;

  for (int h = 0; h < hops; h++) {
    const GridPoint &pc = EyeGrid::get(s_curIdx);

    int dx = (int)pt.x - (int)pc.x;
    int dy = (int)pt.y - (int)pc.y;

    // už skoro v cíli – “cvakni” do cílového indexu
    if (abs(dx) <= EyeGrid::stepX() && abs(dy) <= EyeGrid::stepY()) {
      s_curIdx = s_tgtIdx;
      s_curX = pt.x;
      s_curY = pt.y;
      movedAny = true;
      break;
    }

    // posun 1 “grid cell” směrem k cíli (odděleně X/Y)
    int stepX = EyeGrid::stepX();
    int stepY = EyeGrid::stepY();

    int nx = (int)pc.x;
    int ny = (int)pc.y;

    if (dx > 0) nx += stepX;
    else if (dx < 0) nx -= stepX;

    if (dy > 0) ny += stepY;
    else if (dy < 0) ny -= stepY;

    // najdi nejbližší bod k tomuhle “ideálnímu” bodu
    int ni = findNearestIndex(nx, ny);
    if (ni < 0) break;

    // když to nepomáhá, zkus fallback: přímo podle cíle (nejbližší k cíli)
    if (ni == s_curIdx) {
      // zkus posun jen v jedné ose (když diagonála vyjde na masku / díru)
      int ni2 = -1;
      if (dx != 0) ni2 = findNearestIndex(nx, (int)pc.y);
      if (ni2 < 0 || ni2 == s_curIdx) {
        if (dy != 0) ni2 = findNearestIndex((int)pc.x, ny);
      }
      if (ni2 >= 0 && ni2 != s_curIdx) ni = ni2;
    }

    if (ni == s_curIdx) {
      // poslední záchrana: skoč nejblíž k cíli (může být i “větší” krok)
      ni = findNearestIndex(pt.x, pt.y);
      if (ni == s_curIdx) break;
    }

    s_curIdx = ni;
    const GridPoint &pn = EyeGrid::get(s_curIdx);
    s_curX = pn.x;
    s_curY = pn.y;
    movedAny = true;

    if (s_curIdx == s_tgtIdx) break;
  }

  return movedAny;
}

// -------------------- update tick --------------------
bool EyeGaze::update(uint32_t nowMs) {
  if (EyeGrid::count() <= 0) return false;

  if (nowMs < s_nextActionMs) return false;

  // když nejsme na cestě, vyber nový target a začni
  if (!s_travelling) {
    s_tgtIdx = pickWeightedTargetIndex();
    if (s_tgtIdx < 0) {
      s_nextActionMs = nowMs + randRange(s_dwellMin, s_dwellMax);
      return false;
    }

    const GridPoint &pt = EyeGrid::get(s_tgtIdx);
    s_tgtX = pt.x;
    s_tgtY = pt.y;

    s_lastTargetIdx = s_tgtIdx;

    // když je to stejné místo, jen prodluž dwell
    if (s_curIdx == s_tgtIdx) {
      s_travelling = false;
      s_nextActionMs = nowMs + randRange(s_dwellMin, s_dwellMax);
      return false;
    }

    s_travelling = true;
    s_nextActionMs = nowMs + randRange(s_travelTickMin, s_travelTickMax);
    return false;
  }

  // travelling: udělej travel tick (může posunout o víc hopů)
  bool moved = stepTowardTarget();

  // dojel?
  if (s_curIdx == s_tgtIdx) {
    s_travelling = false;
    s_nextActionMs = nowMs + randRange(s_dwellMin, s_dwellMax);
  } else {
    s_nextActionMs = nowMs + randRange(s_travelTickMin, s_travelTickMax);
  }

  return moved;
}
