#pragma once
#include <Arduino.h>

struct AssetInfo {
  bool found = false;

  String path;
  int w = 0, h = 0;

  // from filename: L/T/R/B (margins)
  int L = 0, T = 0, R = 0, B = 0;

  // draw origin (for our usage)
  int x() const { return L; }
  int y() const { return T; }
};

struct SkinAssets {
  String dir;

  AssetInfo base;
  AssetInfo iris;

  AssetInfo topOpen;
  AssetInfo topClosed;
  AssetInfo botOpen;
  AssetInfo botClosed;

  bool hasCore() const { return base.found && iris.found; }
};

bool skinScanDir(SkinAssets &out, const char* skinDir);
