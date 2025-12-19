#include "skin_config.h"
#include <SD.h>

static bool parseBool(const String& v) {
  String s = v; s.toLowerCase();
  return (s == "1" || s == "true" || s == "on" || s == "yes");
}

static long parseLongAuto(const String& v) {
  // umí i +6 / -4
  return v.toInt();
}

static uint16_t parseHex565(const String& v, uint16_t fallback) {
  String s = v;
  s.trim();
  if (s.startsWith("0x") || s.startsWith("0X")) s = s.substring(2);
  if (s.length() == 0) return fallback;
  char* endp = nullptr;
  uint32_t val = strtoul(s.c_str(), &endp, 16);
  if (endp == s.c_str()) return fallback;
  return (uint16_t)(val & 0xFFFF);
}

static bool splitKeyVal(String line, String& key, String& val) {
  line.trim();
  if (line.length() == 0) return false;
  if (line.startsWith("#")) return false;
  int eq = line.indexOf('=');
  if (eq < 0) return false;
  key = line.substring(0, eq); key.trim();
  val = line.substring(eq + 1); val.trim();
  key.toLowerCase();
  return (key.length() > 0);
}

bool loadSkinConfigIfExists(const char* skinDir, Config& outCfg) {
  if (!skinDir || !*skinDir) return false;

  String path = String(skinDir) + "/settings.txt";

  if (!SD.exists(path)) {
    Serial.printf("[skin-cfg] no settings.txt (%s)\n", path.c_str());
    return false;
  }

  File f = SD.open(path, FILE_READ);
  if (!f) {
    Serial.printf("[skin-cfg] failed to open (%s)\n", path.c_str());
    return false;
  }

  int applied = 0;

  while (f.available()) {
    String line = f.readStringUntil('\n');
    String key, val;
    if (!splitKeyVal(line, key, val)) continue;

    // --- grid ---
    if (key == "step_x") { outCfg.stepX = (int)parseLongAuto(val); applied++; }
    else if (key == "step_y") { outCfg.stepY = (int)parseLongAuto(val); applied++; }

    // --- iris circle ---
    else if (key == "iris_circle_mode") { outCfg.irisCircleMode = (uint8_t)parseLongAuto(val); applied++; }
    else if (key == "iris_circle_radius_px") { outCfg.irisCircleRadiusPx = (int)parseLongAuto(val); applied++; }
    else if (key == "iris_safety_margin_px") { outCfg.irisSafetyMarginPx = (int)parseLongAuto(val); applied++; }

    // --- lid mask shifts ---
    else if (key == "top_lid_mask_shift_px") { outCfg.topLidMaskShiftPx = (int)parseLongAuto(val); applied++; }
    else if (key == "bot_lid_mask_shift_px") { outCfg.botLidMaskShiftPx = (int)parseLongAuto(val); applied++; }

    // --- pupil ---
    else if (key == "pupil_enabled") { outCfg.pupilEnabled = parseBool(val); applied++; }
    else if (key == "pupil_color565") { outCfg.pupilColor565 = parseHex565(val, outCfg.pupilColor565); applied++; }

    else if (key == "pupil_base_px") { outCfg.pupilBasePx = (int)parseLongAuto(val); applied++; }
    else if (key == "pupil_min_px")  { outCfg.pupilMinPx  = (int)parseLongAuto(val); applied++; }
    else if (key == "pupil_max_px")  { outCfg.pupilMaxPx  = (int)parseLongAuto(val); applied++; }

    else if (key == "pupil_anim_enabled") { outCfg.pupilAnimEnabled = parseBool(val); applied++; }

    else if (key == "pupil_change_ms_min") { outCfg.pupilChangeMsMin = (uint32_t)parseLongAuto(val); applied++; }
    else if (key == "pupil_change_ms_max") { outCfg.pupilChangeMsMax = (uint32_t)parseLongAuto(val); applied++; }

    else if (key == "pupil_hold_ms_min") { outCfg.pupilHoldMsMin = (uint32_t)parseLongAuto(val); applied++; }
    else if (key == "pupil_hold_ms_max") { outCfg.pupilHoldMsMax = (uint32_t)parseLongAuto(val); applied++; }

    else if (key == "pupil_arrive_extra_after_ms") { outCfg.pupilArriveExtraAfterMs = (uint32_t)parseLongAuto(val); applied++; }
    else if (key == "pupil_step_delta_px") { outCfg.pupilStepDeltaPx = (int)parseLongAuto(val); applied++; }

    // --- keying ---
    else if (key == "use_key") { outCfg.useKey = parseBool(val); applied++; }
    else if (key == "key_color565") { outCfg.keyColor565 = parseHex565(val, outCfg.keyColor565); applied++; }
    else if (key == "key_tol_r") { outCfg.keyTolR = (uint8_t)parseLongAuto(val); applied++; }
    else if (key == "key_tol_g") { outCfg.keyTolG = (uint8_t)parseLongAuto(val); applied++; }
    else if (key == "key_tol_b") { outCfg.keyTolB = (uint8_t)parseLongAuto(val); applied++; }
  }

  f.close();

  Serial.printf("[skin-cfg] loaded %s (applied=%d)\n", path.c_str(), applied);
  return (applied > 0);
}
