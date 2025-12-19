#pragma once
#include <Arduino.h>

struct RawImage {
  String path;
  String name;

  int w = 0;
  int h = 0;

  // pozice na displeji (z filename L.. T..)
  int x = 0;
  int y = 0;

  uint16_t* pixels = nullptr; // RGB565 v RAM
};

class Assets {
public:
  static bool loadAllRawFromDir(const char* dir);
  static void drawComposedEye();

  static uint32_t count();

  // najdi “nejlepší kandidát” podle substringu v názvu (heuristika)
  static RawImage* findByNameContains(const char* needle);

private:
  static bool loadRawFileToRam(const String& fullPath, RawImage& out);
  static bool parseMetaFromFilename(const String& filename, RawImage& out);
  static void freeAll();
};
