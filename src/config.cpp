#include "config.h"
#include "config_defaults.h"

Config cfg;

void applyDefaults() {
cfg.lidsEnabled = false; // Patch 10 – mrkání zatím vypnuto  
  loadConfigDefaults(cfg);
}
