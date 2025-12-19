#include "AssetsApi.h"

#include <Arduino.h>

#include "sd_manager.h"
#include "skin_config.h"
#include "skin_assets.h"
#include "config.h"      // cfg

namespace AssetsApi {

bool initSd() {
  if (!sdInit() || !sdIsReady()) return false;
  return true;
}

bool loadSkin(const char* skinDir, SkinAssets& outSkin) {
  // settings.txt je volitelné: když není, jedeme dál
  loadSkinConfigIfExists(skinDir, cfg);

  // scan assets musí projít
  return skinScanDir(outSkin, skinDir);
}

} // namespace AssetsApi
