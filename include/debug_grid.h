#pragma once
#include <Arduino.h>

// Debug overlay renderer (grid body, kruh radius, recty masek, target marker).
// Architektura: debug overlay NESMÍ sahat na TFT přímo.
// Renderer mu předá "štětec" (callbacks), a debug jen volá tyto primitivy.

struct DebugGridCallbacks {
  void* ctx = nullptr;

  void (*drawPixel)(void* ctx, int x, int y, uint16_t rgb565) = nullptr;
  void (*drawRect)(void* ctx, int x, int y, int w, int h, uint16_t rgb565) = nullptr;
  void (*drawCircle)(void* ctx, int x, int y, int r, uint16_t rgb565) = nullptr;
  void (*drawLine)(void* ctx, int x0, int y0, int x1, int y1, uint16_t rgb565) = nullptr;
};

// Zavolej z rendereru na konci frame (po složení vrstev).
void debugGridRenderOverlay(const DebugGridCallbacks& cb);
