#include "sd_manager.h"
#include "config.h"
#include <SPI.h>
#include <SD.h>

static SPIClass sdSPI(HSPI);
static bool sdReady = false;

bool sdInit() {
  sdReady = false;

  Serial.println("[SD] init HSPI (separate SPIClass)...");
  sdSPI.begin(cfg.sdSck, cfg.sdMiso, cfg.sdMosi, cfg.sdCs);

  // very safe init speed first
  const uint32_t speeds[] = { 250000, 400000, 800000, 1000000, 2000000 };
  for (auto f : speeds) {
    Serial.printf("[SD] begin(CS=%d, f=%lu)...\n", cfg.sdCs, (unsigned long)f);
    if (SD.begin(cfg.sdCs, sdSPI, f)) {
      sdReady = true;
      Serial.println("[SD] OK");
      Serial.printf("[SD] cardType=%d (0=none)\n", (int)SD.cardType());
      return true;
    }
    delay(150);
    Serial.println("[SD] failed, retry...");
  }

  Serial.println("[SD] FAILED (all freqs)");
  return false;
}

bool sdIsReady() { return sdReady; }

void sdEnd() {
  // Not strictly necessary with SD lib, but kept for completeness
  // SD.end(); // not always present in older cores; if your core has it, you can enable
}
