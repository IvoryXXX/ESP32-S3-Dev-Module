#include "EyeApi.h"
#include "AssetsApi.h"
#include "RenderApi.h"

// include tvých stávajících modulů:
#include "config.h"
#include "eye_grid.h"
#include "eye_gaze.h"
#include "eye_pupil.h"
#include "iris_circle.h"
#include "debug_grid.h"

struct EyeRenderState {
  // sem dáš jen data nutná pro kreslení (pozice iris/pupil, indexy assetů, víčka…)
  // nic “živého”, jen hodnoty
};

static EyeRenderState gState;

namespace EyeApi {

void init() {
  // 1) config init / defaults
  // 2) RenderApi::init()
  // 3) AssetsApi::initSd() + AssetsApi::loadSkinAndScan()
  // 4) grid init/build, gaze init, pupil init, iris_circle init...
}

void update(uint32_t dtMs) {
  // 1) gaze update => cílová pozice
  // 2) pupil/iris update => aktuální pozice
  // 3) naplň gState hodnotami pro render
}

void render() {
  RenderApi::beginFrame();
  RenderApi::draw(gState);
  RenderApi::endFrame();
}

} // namespace EyeApi
