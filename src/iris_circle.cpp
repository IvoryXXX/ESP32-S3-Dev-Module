#include "iris_circle.h"
#include <algorithm>

static IrisCircleResult s_result;

void IrisCircle::init(
  int irisW,
  int irisH,
  const IrisCircleConfig& cfg
) {
  const int irisHalf = std::max(irisW, irisH) / 2;

  if (cfg.mode == IRIS_CIRCLE_CENTER) {
    // radius = pohyb středu
    s_result.gridRadiusPx  = cfg.radiusPx;
    s_result.debugRadiusPx = cfg.radiusPx;
  }
  else {
    // radius = bezpečný okraj irisu
    s_result.gridRadiusPx =
      cfg.radiusPx - irisHalf - cfg.safetyMarginPx;

    if (s_result.gridRadiusPx < 0)
      s_result.gridRadiusPx = 0;

    // debug kruh ukazuje kam až sahá OKRAJ irisu
    s_result.debugRadiusPx =
      s_result.gridRadiusPx + irisHalf;
  }
}

int IrisCircle::gridRadius() {
  return s_result.gridRadiusPx;
}

int IrisCircle::debugRadius() {
  return s_result.debugRadiusPx;
}
