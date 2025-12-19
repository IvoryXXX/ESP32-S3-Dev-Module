#pragma once
#include <Arduino.h>
#include <TFT_eSPI.h>

// Poznámka k architektuře:
// - Inicializace a drobný "boot/alive" vizuál může volat main.
// - Všechny ostatní kreslící operace (push image, přístup na TFT instance) jsou dostupné jen v rendereru.
//   Renderer musí před include definovat:  #define RENDERER_CAN_DRAW

class TftManager {
public:
  // ===== Bezpečné API (může volat i main / ostatní moduly) =====
  static void init();                       // init + rotation + swapBytes + clear
  static void showBootScreen();             // jednoduchý text "BOOT"
  static void showAliveTick(uint32_t ms);   // heartbeat pixel (1x za sekundu)
  static bool isReady();

#if defined(RENDERER_CAN_DRAW)
public:
  // ===== Kreslící API (jen renderer) =====
  static void begin(TFT_eSPI& tft);         // pokud chceš externí TFT instanci
  static void setSwapBytes(bool v);         // nastaví TFT_eSPI swapBytes
  static void pushRGB565(int x, int y, int w, int h, const uint16_t* buf, bool useSwap);
  static TFT_eSPI& tft();                   // přístup na interní TFT
#else
private:
  // schválně private: kreslení jen v render vrstvě
  static void begin(TFT_eSPI& tft);
  static void setSwapBytes(bool v);
  static void pushRGB565(int x, int y, int w, int h, const uint16_t* buf, bool useSwap);
  static TFT_eSPI& tft();
#endif

private:
  static TFT_eSPI* _tft;
  static TFT_eSPI  _owned;                  // interní instance pro init()
  static bool _swapBytes;
  static bool _ownedActive;
};
