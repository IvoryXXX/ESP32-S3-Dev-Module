#pragma once
#include <stdbool.h>

struct SkinAssets;

namespace AssetsApi {

  // init SD (sdInit + sdIsReady)
  bool initSd();

  // načti settings.txt (pokud existuje) + naskenuj assets ve skinu do SkinAssets
  bool loadSkin(const char* skinDir, SkinAssets& outSkin);

} // namespace AssetsApi
