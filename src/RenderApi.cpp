#include "RenderApi.h"

#include "render_eye.h"
#include "skin_assets.h"   // definuje SkinAssets

static const SkinAssets* gSkin = nullptr;

namespace RenderApi {

void init(const SkinAssets& skin) {
  gSkin = &skin;
}

void drawStatic() {
  if (!gSkin) return;
  eyeRenderDrawStatic(*gSkin);
}

void drawIris(int16_t irisX, int16_t irisY) {
  if (!gSkin) return;
  eyeRenderDrawIris(irisX, irisY, *gSkin);
}

} // namespace RenderApi
