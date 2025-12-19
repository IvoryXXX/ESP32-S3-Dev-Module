# ARCHITEKTURA.md — ESP32-S3 DEV MODULE (oko)

> Cíl: udržet projekt čitelný a rozšiřitelný tak, aby přidání nové funkce (pupil, blink, scan, nové skiny)
> **nerozkopalo** existující části (víčka, iris, grid, render).

---

## 0) Rychlá mapa projektu

### Kořen
- `platformio.ini` — build konfigurace (board, framework, lib deps, build flags)
- `.gitignore` — musí ignorovat minimálně `.pio/`, typicky i `.vscode/` podle preference
- `.pio/` — **build output** (nezdrojové soubory, do repa nepatří)
- `.vscode/` — IDE nastavení (volitelně v repu)

### include/ (veřejné API/kontrakty modulů)
- `config.h` — `Config cfg` (runtime nastavení), deklarace globálů/struktur
- `config_defaults.h` — default hodnoty (bez I/O)
- `sd_manager.h` — SD init + “ready” stav
- `TftManager.h` — TFT init + jednotné rozhraní pro kreslení
- `skin_config.h` — načtení `/skins/<skin>/settings.txt` → přepis `cfg`
- `skin_assets.h` — sken skinu na SD + metadata k assetům (rozměry/ořezy…)
- `AssetPath.h` — skládání cest k assetům
- `eye_grid.h` — generování gridu + povolené body
- `eye_gaze.h` — logika pohledu (výběr bodu, krokování)
- `eye_pupil.h` — stav pupil + rasterizace do bufferu
- `render_eye.h` — renderer (jediný modul, který skládá vrstvy a posílá na TFT)
- `debug_grid.h` — debug overlay (grid/recty)
- `iris_circle.h` — pomocná geometrie pro kruh/clip
- `User_Setup.h`, `User_Setup_Select.h` — TFT_eSPI konfigurace (pins/rotace/swapBytes…)

### src/ (implementace)
- `main.cpp` — orchestrátor: init + loop
- `sd_manager.cpp`
- `TftManager.cpp`
- `skin_config.cpp`
- `skin_assets.cpp`
- `eye_grid.cpp`
- `eye_gaze.cpp`
- `eye_pupil.cpp`
- `render_eye.cpp`
- `debug_grid.cpp`
- `iris_circle.cpp`
- `assets.cpp.bak` — záloha (ideálně mimo repo nebo do `archive/`)

### lib/
- `TFT_eSPI_Setups/Setup_Oko_S3_TFT1.h` — historický/alternativní setup (viz pravidla níž)

---

## 1) Vrstevnice (kdo co smí a nesmí)

### 1.1 HAL — Hardware Abstraction Layer
**Soubory:** `TftManager.*`, `sd_manager.*`

**Zodpovědnost:**
- Inicializace HW
- Primitivní operace: pushImage, fillRect, open/read file

**Pravidla (tvrdé):**
- HAL **nesmí** zahrnovat (`#include`) žádné `eye_*`, `render_*`, `skin_*`.
- HAL **nesmí** držet “stav oka” (iris pos, lids stav, pupil stav). Jen zařízení.

---

### 1.2 DATA — Konfigurace a assety (I/O, ale bez kreslení)
**Soubory:** `config*.h/.cpp`, `skin_config.*`, `skin_assets.*`, `AssetPath.h`

**Zodpovědnost:**
- `cfg` defaulty + načtení `settings.txt`
- scan skin složky a nalezení assetů (base/iris/lids/…)
- uložení metadat (rozměry, ořezy, typ assetu)

**Pravidla:**
- DATA vrstva **nesmí** kreslit na TFT.
- DATA vrstva může číst SD (logicky), ale **neposílá pixely** na displej.

---

### 1.3 MODEL — Logika oka (žádné SD/TFT)
**Soubory:** `eye_grid.*`, `eye_gaze.*`, `eye_pupil.*`, `iris_circle.*`

**Zodpovědnost:**
- Grid: vygenerovat body a povolené body
- Gaze: vybrat další bod, interpolovat pohyb
- Pupil: spočítat stav pupil (radius/offset…), případně rasterizovat do bufferu

**Pravidla (tvrdé):**
- MODEL **nesmí** volat SD ani TFT.
- MODEL vrací pouze data (structy/flagy), neprovádí I/O.

---

### 1.4 RENDER — Jediná vrstva, která kreslí
**Soubory:** `render_eye.*`, (debug overlay: `debug_grid.*` může kreslit pouze přes renderer nebo v debug módu)

**Zodpovědnost:**
- Load RAW565 (ze SD do bufferů) — pokud to renderer dělá, je to OK
- Skládání vrstev (base → iris patch → víčka → debug)
- Dirty-rect rozhodnutí
- Jediné místo, které volá `TftManager` push

**Pravidla (tvrdé):**
- Nikdo jiný než renderer **nesmí** dělat `pushImage()` / kreslit pixely na TFT.
- Renderer je jediný “vlastník” pixel bufferů.

---

## 2) Inicializační pořadí (aby config a skiny dávaly smysl)

Doporučené pořadí v `main.cpp`:

1. `Serial.begin(...)`
2. `applyDefaults()` / `loadConfigDefaults()`
3. `TftManager::init()` + (volitelně) boot screen
4. `sdInit()` + `sdIsReady()` (fail fast)
5. `loadSkinConfigIfExists(cfg.skinDir, cfg)`  *(settings.txt přepíše cfg)*
6. `skinScanDir(gSkin, cfg.skinDir)`  *(najde base/iris/lids assets)*
7. `EyeGrid::build(...)`  *(grid závisí na cfg + skin parametrech)*
8. `EyeGaze::init(...)`  *(pokud má init)*
9. `EyePupil::init(...)` *(pokud má init)*
10. `Renderer::init(...)` / `renderInit(...)`

**Invariant:** co ovlivňuje rozměry/masky/griddy se musí vyhodnotit **před** buildem gridu.

---

## 3) Render pipeline (kontrakt vrstev)

### 3.1 Standardní pořadí vrstev
1. **Base** (bulva)
2. **Iris patch** (iris + pupil + volitelně highlight, pokud je “pod víčky”)
3. **Víčka** (top/bot)
4. **Debug overlay** (grid/dirty rect/…)

### 3.2 Kontrakt pro pupil
- Pupil **kreslí pouze** do bufferu, který jí renderer předá (typicky iris patch buffer).
- Pupil neřeší SD ani TFT.

---

## 4) Dirty rectangles (aby nic “nezmizelo”)

### 4.1 Zdroj pravdy
Renderer si pamatuje minimálně:
- `lastIrisRect`
- `lastLidState` (např. openPct / frame index)
- `lastPupilState` (pokud ovlivňuje pixely)
- `lastSkinRevision` / “assets loaded OK”

### 4.2 Minimální korektní pravidlo
Když se změnil kterýkoli pixel v oblasti:
- spočítat `dirty = union(všech změněných rectů)`
- v `dirty` překreslit vrstvy vždy ve stejném pořadí (base → iris → lids)

### 4.3 Zakázané anti-patterny
- „restore jen base“ v oblasti, kde existuje vrstva víček (hrozí zmizení víček).
- kreslit víčka pouze při jejich změně, pokud jiné vrstvy přepisují jejich oblast.
- dělat kreslení mimo renderer (pupil/scan/debug přímo na TFT).

---

## 5) Grid & víčka — povolené body

### 5.1 Dva seznamy bodů
- `allPoints[]` — všechny body v kruhu
- `allowedPoints[]` — body po aplikaci masky víček + edge pravidel

**Pravidlo:** `eye_gaze` pracuje pouze s `allowedPoints[]`.

### 5.2 Nastavitelný ořez víčky
`eye_grid` má mít jasnou funkci, která:
- přijme parametry pro ořez (rect/mask/okraje)
- přepočítá `allowedPoints[]`
- je deterministická (stejný vstup → stejný výstup)

---

## 6) Kontrakty modulů (API hranice)

### 6.1 `eye_grid.*`
- Generuje body a masky.
- Poskytuje `allowedPoints` a `edgePoints` (pokud existuje).

### 6.2 `eye_gaze.*`
- Vstup: `allowedPoints`
- Výstup: `IrisState {x,y,index,movedFlag}` (nebo ekvivalent)
- Neřeší pixely ani SD/TFT.

### 6.3 `eye_pupil.*`
- Vstup: čas + config
- Výstup: `PupilState`
- Rasterizace: jen do poskytnutého bufferu.

### 6.4 `render_eye.*`
- Vstup: aktuální stavy (iris/pupil/lids) + assets
- Výstup: pixely na TFT

---

## 7) Feature flags (bezpečné rozšiřování)

Doporučeno mít v `cfg`:
- `enablePupil`
- `enableLids`
- `enableScan`
- `enableDebugGrid`
- `enableDirtyDebug`

**Pravidlo:** každá nová funkce musí jít vypnout jedním flagem a zbytek musí běžet dál.

---

## 8) TFT_eSPI setup — jedna pravda

Aktuálně existují 2 zdroje setupu:
- `include/User_Setup.h` (+ `User_Setup_Select.h`)
- `lib/TFT_eSPI_Setups/Setup_Oko_S3_TFT1.h`

**Pravidlo:** projekt musí mít **jediný** aktivní setup.
- Pokud používáš build flag `-include include/User_Setup.h`, pak `lib/TFT_eSPI_Setups/*` ber jako archiv/dokumentaci.
- Nebo naopak — ale ne obojí, protože to vede k “funguje to jen někdy” chování.

---

## 9) Repo hygiena (aby se to nemíchalo)

- `.pio/` vždy ignorovat (gitignore).
- `*.bak` držet mimo `src/` (např. `archive/`), aby nebyl “zdvojenej zdroj pravdy”.
- Každý modul má 1 zodpovědnost (viz vrstvy výš).

---

## 10) Jak přidat novou funkci bez rozkopání (checklist)

1. Patří to do MODEL (počítání) nebo do RENDER (pixely)?
2. Má to vlastní `cfg.enableX`?
3. Mění to pixely? → musí to být součást render pipeline a dirty pravidel.
4. Mění to povolené body? → musí to být přes `eye_grid` (rebuild allowed).
5. Přidat debug přepínač, který ukáže, co se děje (dirty rect / lid clip / iris rect).

---

## 11) Debug doporučení
- `debugDrawDirtyRects`
- `debugDrawLidClip`
- `debugDrawIrisRect`
- `debugLogAssetsFound` (base/iris/lids indexy)

Renderer v debug módu kreslí overlay až nakonec (nad víčky), aby byl čitelný.

---

## 12) Co budeme dělat dál (konkrétní kroky refaktoru)
1. Vybrat **jediný** TFT_eSPI setup (a druhý přesunout do `archive/`).
2. Zajistit, že **jen renderer** kreslí na TFT (pupil/debug jen přes renderer).
3. Formalizovat `IrisState`, `LidState`, `PupilState` jako structy (jedno místo pravdy).
4. Zpevnit `allowedPoints[]` a vrátit “lid clip” jako explicitní krok v `eye_grid`.
5. Zavést `lastFrameState` v rendereru a sjednotit dirty-rect pravidla.
