#pragma once
#include <stdint.h>

// Jak interpretujeme radius z configu
enum IrisCircleMode : uint8_t {
  IRIS_CIRCLE_CENTER, // radius = kam smí STŘED irisu
  IRIS_CIRCLE_EDGE    // radius = kam smí OKRAJ irisu (bezpečný)
};

struct IrisCircleConfig {
  int radiusPx;          // hodnota z configu
  IrisCircleMode mode;   // CENTER / EDGE
  int safetyMarginPx;    // rezerva (jen pro EDGE)
};

struct IrisCircleResult {
  int gridRadiusPx;      // výsledný radius pro EyeGrid
  int debugRadiusPx;     // co se má kreslit v overlay
};

class IrisCircle {
public:
  // init při bootu (jednou)
  static void init(
    int irisW,
    int irisH,
    const IrisCircleConfig& cfg
  );

  // výstupy
  static int gridRadius();   // tohle se dává do EyeGrid::build()
  static int debugRadius();  // tohle kreslí overlay
};
