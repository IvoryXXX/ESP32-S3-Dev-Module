#include "AssetsApi.h"
#include "sd_manager.h"
#include "skin_assets.h"
#include "skin_config.h"
#include "assets.h"

namespace AssetsApi {

bool initSd() {
  // return sdInit() && sdIsReady();  (nebo co máš)
  return true; // TODO: napoj na existující
}

bool loadSkinAndScan() {
  // loadSkinConfigIfExists(...)
  // skinScanDir(...)
  return true; // TODO
}

}
