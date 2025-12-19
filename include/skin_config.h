#pragma once
#include "config.h"

// načte /skins/<skinDir>/settings.txt (pokud existuje)
// vrací true = soubor existoval a aspoň něco přepsal
bool loadSkinConfigIfExists(const char* skinDir, Config& outCfg);
