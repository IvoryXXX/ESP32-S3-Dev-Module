#pragma once
#include <Arduino.h>

// Složí absolutní cestu: skinDir + "/" + filename
// - hlídá, aby výsledek vždy začínal '/'
// - hlídá, aby mezi nimi nebyly "//"
namespace AssetPath {

inline String normalizeDir(const String& dir) {
  if (dir.length() == 0) return String("/");
  String d = dir;

  // vždy začni lomítkem
  if (d[0] != '/') d = "/" + d;

  // odeber trailing '/'
  while (d.length() > 1 && d.endsWith("/")) {
    d.remove(d.length() - 1);
  }
  return d;
}

inline String normalizeFile(const String& filename) {
  if (filename.length() == 0) return String("");
  String f = filename;

  // odeber leading '/'
  while (f.startsWith("/")) {
    f.remove(0, 1);
  }
  return f;
}

inline String makeFullPath(const String& skinDir, const String& filename) {
  String d = normalizeDir(skinDir);
  String f = normalizeFile(filename);

  if (f.length() == 0) return d; // fallback

  return d + "/" + f; // vždy absolutní
}

} // namespace AssetPath
