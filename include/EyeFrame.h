/*
EYEFRAME – ZABETONOVANÝ DATOVÝ KONTRAKT

EyeFrame je JEDINÉ rozhraní mezi logikou oka (EyeApi, Gaze, Lids)
a renderem (RenderApi, render_eye).

PRAVIDLA (NESMÍ SE PORUŠIT):

1) Souřadnice
- irisX, irisY = pozice IRISU V PIXELECH
- Souřadnice odpovídají REFERENČNÍMU BODU IRISU
  (definice: viz render_eye.cpp, MUSÍ být konzistentní)
- Renderer nepočítá logiku, pouze kreslí podle těchto hodnot

2) Víčka
- lidTop, lidBot mají rozsah 0..1000
  - 0     = plně otevřeno
  - 1000  = plně zavřeno
- EyeFrame NEŘEŠÍ, PROČ jsou víčka v dané poloze
- Pouze říká rendereru, CO má vykreslit

3) Dirty flagy
- irisDirty = změnila se poloha nebo tvar irisu
- lidsDirty = změnila se poloha víček
- Renderer může použít dirty flagy k optimalizaci,
  ale NESMÍ změnit vizuální výsledek

4) Odpovědnosti
- EyeApi: skládá EyeFrame
- RenderApi: vykresluje EyeFrame
- render_eye: kreslí jednotlivé vrstvy

Jakákoli logika, která poruší tyto body,
je CHYBA NÁVRHU, ne implementace.
*/

#pragma once
#include <stdint.h>

// Oficiální datový model jednoho "frame" oka
struct EyeFrame {
  // pozice duhovky (v pixelech)
  int16_t irisX = 0;
  int16_t irisY = 0;

  // víčka: 0 = plně otevřeno, 1000 = plně zavřeno (fixpoint, bez floatů)
  // (top/bot zvlášť – připraveno na partial blink + mechaniku)
  uint16_t lidTop = 0;
  uint16_t lidBot = 0;

  // dirty flagy – co je potřeba překreslit
  bool irisDirty = false;
  bool lidsDirty = false;
};
