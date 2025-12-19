# BETON API FREEZE – ESP32 Eye (v0.3)

## Cíl
Zastavit rozbíjení projektu při změnách. Zavést stabilní hranice (API) mezi moduly.

## Pravidla (beton)
1) main.cpp volá pouze EyeApi::{init,update,render}
2) Logika (eye_*.cpp, iris_circle, debug_grid) NESMÍ sahat na TFT ani SD ani skin soubory přímo.
3) Render (render_eye.cpp, TftManager.cpp) NESMÍ sahat do grid/gaze/pupil internals.
4) Assets/SD (sd_manager, skin_assets, skin_config) NESMÍ volat render.
5) _render_internal.h je PRIVÁTNÍ: includovat jen render_eye.cpp (+ případně TftManager.cpp).

## Betonové API (stabilní povrch)
- EyeApi.h: init(), update(dtMs), render()
- RenderApi.h: init(), beginFrame(), draw(state), endFrame()
- AssetsApi.h: initSd(), loadSkinAndScan()

## Patch 1 (bez změny chování)
- Přidat EyeApi/RenderApi/AssetsApi jako adapter layer.
- Přesunout stávající setup/loop logiku do EyeApi (1:1).
- main.cpp zjednodušit na orchestrátor.
- Commit + tag: v0.3-api-frozen

## Stav / TODO
- [1 ] EyeApi::init napojit na původní init sekvenci (applyDefaults, TftManager init, SD init, skin load+scan, grid build)
- [1 ] EyeApi::update napojit na gaze update + příprava state
- [ 1] EyeApi::render volá RenderApi::draw
- [1 ] RenderApi::draw volá stávající render funkce
- [ 1] AssetsApi funkce volají sd/skin funkce
