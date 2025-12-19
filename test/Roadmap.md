# ESP32 Eye Engine – Roadmap & API Freeze

Tento dokument je **jediný zdroj pravdy** o stavu projektu.  
Určuje:
- co je považováno za hotové a zabetonované
- jaká pravidla se nesmí porušit
- v jakém pořadí se mají přidávat další funkce

Cíl:
Umožnit další rozšiřování (mrkání, emoce, skiny, senzory)
bez rozbití renderu a základní funkčnosti oka.

---

## 🟢 AKTUÁLNÍ STAV

- Stabilní funkční verze: **Patch 9**
- Základní pohyb oka: funkční
- Render pipeline: stabilní
- EyeApi / RenderApi / EyeFrame: oddělené
- Mrkání / zavřená víčka: **ZÁMĚRNĚ NEIMPLEMENTOVÁNO**

Tato verze slouží jako **baseline**.

---

## 🔒 ZABETONOVANÉ KONTRAKTY (API FREEZE)

### 1) main.cpp
- `main.cpp` nesmí obsahovat žádnou logiku
- Volá výhradně:
  - `EyeApi::init()`
  - `EyeApi::update(dtMs)`
  - `EyeApi::render()`

---

### 2) EyeApi
**Role:** Orchestrátor stavu oka.

- Skládá kompletní stav do `EyeFrame`
- Volá jednotlivé subsystémy (Gaze, Pupil, Lids…)
- Nikdy nekreslí přímo na TFT
- Neobsahuje renderovací detaily

---

### 3) EyeFrame (datový kontrakt)
`EyeFrame` je **jediné rozhraní mezi logikou a rendererem**.

Zabetonovaná pravidla:
- `irisX`, `irisY`
  → pozice irisu v pixelech (definice popsána v kódu)
- `lidTop`, `lidBot`
  → rozsah **0..1000**
    - `0` = plně otevřeno
    - `1000` = plně zavřeno
- `irisDirty`, `lidsDirty`
  → indikují nutnost překreslení vrstvy

Renderer nesmí rozhodovat, *proč* se něco změnilo.

---

### 4) RenderApi
**Role:** Překlad dat → obraz.

- Jediné místo, které renderuje celý frame
- Přijímá `EyeFrame`
- Řídí pořadí vrstev a dirty-rect logiku
- Neobsahuje žádnou logiku mrkání ani stavů

---

### 5) render_eye.cpp
**Role:** Low-level kreslení.

- Obsahuje pouze:
  - kreslení base
  - kreslení irisu
  - kreslení víček
- Neobsahuje žádnou logiku rozhodování
- Nikdy neřeší stav oka

---

### 6) Invariant vrstvení (neměnné pravidlo)

Pořadí vrstev je pevně dané:
1. Base (oční bulva)
2. Iris
3. Otevřená víčka (overlay)

Zakázáno:
- maskovat iris místo dokreslení overlay
- kreslit víčka před irisem

---

## 🧱 STAV BETONOVÁNÍ

### ✅ HOTOVO
- Oddělení `update()` / `render()`
- EyeApi jako jediný vstupní bod
- RenderApi jako jediný renderer
- EyeFrame jako datový kontrakt
- Funkční render pipeline
- Návrat na stabilní Patch 9

---

### 🔧 JEŠTĚ DOBETONOVAT (bez nových funkcí)

1) EyeFrame – dokumentace v kódu  
   - jasně popsat souřadnice irisu  
   - jasně popsat význam dirty flagů  

2) Feature flags v configu  
   - `enableLids`
   - `enableBlink`
   - `enableClosedLidsAssets`
   → defaultně vypnuto, aby nehotové věci neprosakovaly

3) Ownership asset bufferů  
   - kdo alokuje
   - kdo uvolňuje
   - chování při změně skinu

4) Error policy  
   - chybí base / iris → STOP
   - chybí víčka → pokračovat bez nich

---

## ⛔ ZAKÁZANÉ (do odvolání)

- Implementace mrkání v `render_eye.cpp`
- Přímé kreslení mimo RenderApi
- Přímé volání TFT mimo render subsystém
- Zavřená víčka bez explicitního povolení v configu

---

## 🔜 BUDOUCÍ FUNKCE (AŽ PO STABILIZACI)

Následující věci nejsou součástí současného betonu:

- `LidsApi` (mrkání, animace víček)
- Emocionální stavy oka
- Přepínání skinů za běhu
- Senzory / tracking

Pravidlo:
Každá nová funkce musí mít vlastní API
a pracovat výhradně přes `EyeFrame`.

---

## 🧠 ZÁSADA PRO DALŠÍ VÝVOJ

Render je hloupý.  
Logika je chytrá.  
Mezi nimi je EyeFrame.

Pokud by nová změna tuto zásadu porušila,
jde o chybu návrhu, ne detail k doladění.
