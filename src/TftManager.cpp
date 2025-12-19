#include "TftManager.h"

// statics
TFT_eSPI* TftManager::_tft = nullptr;
TFT_eSPI  TftManager::_owned;
bool      TftManager::_swapBytes = false;
bool      TftManager::_ownedActive = false;

void TftManager::begin(TFT_eSPI& tft) {
  _tft = &tft;
  _tft->setSwapBytes(_swapBytes);
}

bool TftManager::isReady() {
  return _tft != nullptr;
}

TFT_eSPI& TftManager::tft() {
  if (!_tft) {
    _tft = &_owned;
    _ownedActive = true;
  }
  return *_tft;
}

void TftManager::init() {
  // Interní instance, aby main.cpp nemusel řešit nic extra
  _ownedActive = true;
  _tft = &_owned;

  _tft->init();
  _tft->setRotation(0);

  // tvoje správná konfigurace pro RAW565 pipeline (režim B)
  // swapBytes = true, swapRB = false (swapRB neřeší TFT_eSPI, to je o datech)
  _swapBytes = true;
  _tft->setSwapBytes(true);

  _tft->fillScreen(TFT_BLACK);
}

void TftManager::showBootScreen() {
  if (!_tft) return;

  _tft->fillScreen(TFT_BLACK);
  _tft->setTextDatum(MC_DATUM);
  _tft->setTextSize(2);
  _tft->drawString("BOOT", _tft->width() / 2, _tft->height() / 2);
}

void TftManager::showAliveTick(uint32_t ms) {
  if (!_tft) return;

  static bool on = false;
  static uint32_t last = 0;

  if (ms - last >= 1000) {
    last = ms;
    on = !on;
    _tft->drawPixel(0, 0, on ? TFT_WHITE : TFT_BLACK);
  }
}

void TftManager::setSwapBytes(bool v) {
  _swapBytes = v;
  if (_tft) _tft->setSwapBytes(v);
}

void TftManager::pushRGB565(int x, int y, int w, int h, const uint16_t* buf, bool useSwap) {
  if (!_tft || !buf || w <= 0 || h <= 0) return;

  if (!useSwap) {
    _tft->startWrite();
    _tft->pushImage(x, y, w, h, (uint16_t*)buf);
    _tft->endWrite();
    return;
  }

  // dočasně vynutit swapBytes=true jen pro tenhle push
  bool prev = _swapBytes;
  if (!prev) _tft->setSwapBytes(true);

  _tft->startWrite();
  _tft->pushImage(x, y, w, h, (uint16_t*)buf);
  _tft->endWrite();

  if (!prev) _tft->setSwapBytes(false);
}
