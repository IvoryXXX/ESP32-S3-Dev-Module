#include "RenderApi.h"
#include "render_eye.h"
#include "TftManager.h"

struct EyeRenderState {
  // musí být shodná definice jako v EyeApi.cpp
  // (v Patch 2 to dáme do společného headeru, zatím to držíme jednoduché)
};

namespace RenderApi {

void init() {
  // TftManager::init();
  // render_eye init, boot screen, cokoliv máš dnes
}

void beginFrame() {
  // případně žádná práce, nebo “start DMA”, nebo “set viewport”
}

void draw(const EyeRenderState& s) {
  // zavolej tvoje existující render funkce a předej data ze s
}

void endFrame() {
  // flush / DMA wait / swap
}

}
