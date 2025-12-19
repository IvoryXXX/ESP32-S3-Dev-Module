#include "config.h"
#include "config_defaults.h"

Config cfg;

void applyDefaults() {
  loadConfigDefaults(cfg);
}
