#pragma once
#include <Arduino.h>

class EyeGaze {
public:
  static void init(
    uint32_t dwellMinMs, uint32_t dwellMaxMs,
    uint32_t travelTickMinMs, uint32_t travelTickMaxMs,
    uint8_t centerPct, bool avoidSame,
    uint8_t hopMin, uint8_t hopMax,
    uint8_t edgeBands, uint8_t edgeWeightPct, bool edgeSoft
  );

  // update() vrací true, když se musí překreslit iris patch (iris se pohnul nebo se změnila pupil)
  static bool update(uint32_t nowMs);

  static int x();
  static int y();

  static int targetX();
  static int targetY();

  static bool isTravelling();

  // užitečné při (re)build gridu / změně skinu
  static void resetToCenter();

private:
  // helpers
  static uint32_t randRange(uint32_t a, uint32_t b);
  static int      randInt(int a, int b);
  static int32_t  dist2(int x1,int y1,int x2,int y2);

  static int  findNearestIndex(int x, int y);
  static int  pickWeightedTargetIndex();

  static bool stepTowardTarget();

  static int s_curIdx;
  static int s_tgtIdx;

  static int s_curX;
  static int s_curY;

  static int s_tgtX;
  static int s_tgtY;

  static bool     s_travelling;
  static uint32_t s_nextActionMs;

  static uint32_t s_dwellMin;
  static uint32_t s_dwellMax;
  static uint32_t s_travelTickMin;
  static uint32_t s_travelTickMax;

  static uint8_t s_centerPct;
  static bool    s_avoidSame;

  static uint8_t s_hopMin;
  static uint8_t s_hopMax;

  static uint8_t s_edgeBands;
  static uint8_t s_edgeWeightPct;
  static bool    s_edgeSoft;

  static int s_lastTargetIdx;
};
