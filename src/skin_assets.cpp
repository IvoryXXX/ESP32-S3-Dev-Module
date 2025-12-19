#include "skin_assets.h"
#include <SD.h>

static bool parseMetaFromName(const String &name, int &w, int &h, int &L, int &T, int &R, int &B) {
  // expects "..._240x240_L0_T0_R0_B0.raw"
  // (žádný side-effect; jen parser názvu)

  auto getNumAfter = [&](const char* key, int &val) -> bool {
    int p = name.indexOf(key);
    if (p < 0) return false;
    p += strlen(key);
    int end = p;
    while (end < (int)name.length() && isDigit(name[end])) end++;
    if (end == p) return false;
    val = name.substring(p, end).toInt();
    return true;
  };

  // WxH
  // find pattern like "240x240"
  int xPos = name.indexOf('x');
  if (xPos < 0) return false;

  // scan backwards to find start of W
  int startW = xPos - 1;
  while (startW >= 0 && isDigit(name[startW])) startW--;
  startW++;

  int endH = xPos + 1;
  while (endH < (int)name.length() && isDigit(name[endH])) endH++;

  if (startW >= xPos) return false;
  String sw = name.substring(startW, xPos);
  String sh = name.substring(xPos + 1, endH);
  w = sw.toInt();
  h = sh.toInt();

  if (!getNumAfter("_L", L)) return false;
  if (!getNumAfter("_T", T)) return false;
  if (!getNumAfter("_R", R)) return false;
  if (!getNumAfter("_B", B)) return false;

  return (w > 0 && h > 0);
}

static void tryAssign(AssetInfo &slot, const String &fullPath, const String &fileName) {
  int w,h,L,T,R,B;
  if (!parseMetaFromName(fileName, w,h,L,T,R,B)) return;

  slot.found = true;
  slot.path = fullPath;
  slot.w = w; slot.h = h;
  slot.L = L; slot.T = T; slot.R = R; slot.B = B;
}

bool skinScanDir(SkinAssets &out, const char* skinDir) {
  out = SkinAssets();
  out.dir = String(skinDir);

  Serial.printf("[skin] scan dir: %s\n", skinDir);

  File dir = SD.open(skinDir);
  if (!dir) {
    Serial.printf("[skin] SD.open DIR FAILED: %s\n", skinDir);
    return false;
  }
  if (!dir.isDirectory()) {
    Serial.printf("[skin] NOT a dir: %s\n", skinDir);
    dir.close();
    return false;
  }

  while (true) {
    File f = dir.openNextFile();
    if (!f) break;
    if (f.isDirectory()) { f.close(); continue; }

    String name = String(f.name()); // usually "/skins/human/xxx.raw" or "xxx.raw" depending on SD lib
    f.close();

    // normalize: get basename
    int slash = name.lastIndexOf('/');
    String baseName = (slash >= 0) ? name.substring(slash+1) : name;

    String fullPath = String(skinDir);
    if (!fullPath.endsWith("/")) fullPath += "/";
    fullPath += baseName;

    if (baseName.startsWith("file_eye_base_")) {
      tryAssign(out.base, fullPath, baseName);
      Serial.printf("[skin] found file_eye_base_  -> %s  (%dx%d L%d T%d R%d B%d)\n",
                    fullPath.c_str(), out.base.w, out.base.h, out.base.L, out.base.T, out.base.R, out.base.B);
    } else if (baseName.startsWith("iris_")) {
      tryAssign(out.iris, fullPath, baseName);
      Serial.printf("[skin] found iris_           -> %s  (%dx%d L%d T%d R%d B%d)\n",
                    fullPath.c_str(), out.iris.w, out.iris.h, out.iris.L, out.iris.T, out.iris.R, out.iris.B);
    } else if (baseName.startsWith("lid_top_open_")) {
      tryAssign(out.topOpen, fullPath, baseName);
      Serial.printf("[skin] found lid_top_open_   -> %s  (%dx%d L%d T%d R%d B%d)\n",
                    fullPath.c_str(), out.topOpen.w, out.topOpen.h, out.topOpen.L, out.topOpen.T, out.topOpen.R, out.topOpen.B);
    } else if (baseName.startsWith("lid_top_closed_")) {
      tryAssign(out.topClosed, fullPath, baseName);
      Serial.printf("[skin] found lid_top_closed_ -> %s  (%dx%d L%d T%d R%d B%d)\n",
                    fullPath.c_str(), out.topClosed.w, out.topClosed.h, out.topClosed.L, out.topClosed.T, out.topClosed.R, out.topClosed.B);
    } else if (baseName.startsWith("bot_lid_open_")) {
      tryAssign(out.botOpen, fullPath, baseName);
      Serial.printf("[skin] found bot_lid_open_   -> %s  (%dx%d L%d T%d R%d B%d)\n",
                    fullPath.c_str(), out.botOpen.w, out.botOpen.h, out.botOpen.L, out.botOpen.T, out.botOpen.R, out.botOpen.B);
    } else if (baseName.startsWith("bot_lid_closed_")) {
      tryAssign(out.botClosed, fullPath, baseName);
      Serial.printf("[skin] found bot_lid_closed_ -> %s  (%dx%d L%d T%d R%d B%d)\n",
                    fullPath.c_str(), out.botClosed.w, out.botClosed.h, out.botClosed.L, out.botClosed.T, out.botClosed.R, out.botClosed.B);
    }
  }

  dir.close();

  Serial.printf("[skin] core: base=%d iris=%d\n", out.base.found, out.iris.found);
  Serial.printf("[skin] lids: topOpen=%d topClosed=%d botOpen=%d botClosed=%d\n",
                out.topOpen.found, out.topClosed.found, out.botOpen.found, out.botClosed.found);

  if (!out.hasCore()) {
    Serial.println("[skin] ERROR: missing base or iris in skinDir");
    return false;
  }
  return true;
}
