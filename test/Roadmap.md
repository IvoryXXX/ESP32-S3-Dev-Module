# Eye Engine – Roadmap (betonová verze)

> Tento dokument je **zdroj pravdy** projektu.
> Když se ztratí kontext (člověk, AI, únava), **vracíme se sem**.
> Pokud je něco v rozporu s tímto dokumentem, **platí Roadmapa**.

---

## 🧱 Základní filozofie

* Projekt se **betonuje po patchech** – malé, izolované kroky
* Každý patch:

  * má jasný cíl
  * **nemění chování**, pokud to není výslovně uvedeno
  * končí funkčním stavem + commitem
* Architektura > optimalizace

---

## ✅ Hotové patche (stav: dokončeno)

### Patch 1 – API freeze

**Cíl:** Stabilní vstupní bod aplikace

* `main.cpp` volá pouze `EyeApi::init/update/render`
* Přesun původního setup/loop do EyeApi

**Stav:** hotovo

---

### Patch 2 – Oddělení update vs render

**Cíl:** Čisté rozdělení výpočtu a kreslení

* `update()` = logika
* `render()` = kreslení

**Stav:** hotovo

---

### Patch 3 – RenderApi (draw)

**Cíl:** EyeApi už nekreslí přímo

* Wrapper nad `eyeRenderDraw*`

**Stav:** hotovo

---

### Patch 4 – RenderApi (init + load)

**Cíl:** EyeApi nezná renderer interně

* `eyeRenderInit / LoadAssets` přes RenderApi

**Stav:** hotovo

---

### Patch 5 – AssetsApi

**Cíl:** EyeApi neřeší SD ani skiny

* SD init
* skin config
* scan assets

**Stav:** hotovo

---

## 🔜 Plánované patche (pořadí je závazné)

### Patch 6 – FrameState jako kontrakt

**Cíl:** Oficiální datový model frame

```cpp
struct EyeFrame {
  int16_t irisX, irisY;
  bool irisDirty;
  bool lidsDirty;
};
```

* EyeApi frame **vytváří**
* RenderApi frame **spotřebovává**

**Odemkne:**

* debug overlay
* replay pohybu
* synchronizaci víček

---

### Patch 7 – LidsApi (víčka)

**Cíl:** Víčka jako samostatná logická entita

* `LidsApi::update(dt, gaze)`
* `LidsApi::applyToFrame(frame)`

**Odemkne:**

* realistické blikání
* reakci víček na směr pohledu
* přípravu na servo/mechaniku

---

### Patch 8 – Render passes + dirty rects

**Cíl:** Optimalizace bez chaosu

```cpp
beginFrame();
drawBase();
drawIris();
drawLids();
endFrame();
```

**Odemkne:**

* menší překreslování
* DMA optimalizace
* vyšší FPS

---

### Patch 9 – Deterministický timing

**Cíl:** Odstranit implicitní časování

* žádné `delay()` v logice
* žádné skryté `millis()`
* vše přes `dtMs`

**Odemkne:**

* simulaci bez HW
* testy
* stabilní chování

---

### Patch 10 – Hot reload / fallback skin

**Cíl:** Robustnost

* přepnutí skinu za běhu
* fallback při chybě

---

## 🧠 Zásady další práce

* Když se projekt začne chovat divně:

  1. zkontroluj Roadmapu
  2. zkontroluj poslední patch
  3. **nepřeskakuj patche**

* Když AI „ztratí paměť“:

  * pošli jí tento soubor

---

## 🧱 Motto projektu

> Nejdřív architektura.
> Optimalizace až když víme, co optimalizujeme.
