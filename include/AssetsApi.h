#pragma once
#include <stdbool.h>

namespace AssetsApi {
  bool initSd();
  bool loadSkinAndScan();      // načti settings + naskenuj assets
}
