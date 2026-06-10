# ADR 0005 — Screen-space UI canvas with reference-resolution anchor layout

**Status:** Proposed
**Date:** 2026-06-09

## Context

The UI module did not adapt to window resolution, and three defects made it unfit for game UI:

1. **The UI's own camera was dead code.** `RendererConfigSingletonComponent.m_Camera` was never read.
   `Renderer2DSystem` merged UI draw items into the world draw list and rendered them through the
   **gameplay** `OrthographicCameraComponent` — so the UI inherited the camera's WASD pan, scroll zoom,
   and world units, and did not map predictably to screen pixels.
2. **Hit-testing lived in a different space than rendering.** `UIEventSystem` compared the raw window-pixel
   mouse position against element bounds computed in a fixed 1920×1080 virtual space, with no window→canvas
   transform. Clicks aligned only by accident, and never once the window was not exactly the canvas size.
3. **No resolution/aspect policy.** The virtual canvas was a fixed size; nothing scaled it to the live
   window or handled aspect mismatch.

The layout model was also too thin for game UI: absolute-pixel sizing only (no fill-parent / percentage),
a discrete 3×3 align enum, no layout containers, no input capture (a click hit *every* overlapping element),
no clipping, no nine-slice.

## Decision

UI is authored against a fixed **Reference Resolution** (the **UI Canvas**, standardized at 1920×1080,
config-driven) and rendered in a **dedicated screen-space UI Pass** drawn after the world through a UI-only
orthographic projection spanning the canvas — never through the gameplay camera. The `RendererConfig`
camera is removed.

The canvas maps to the live window by a single uniform **UI Scale Factor** derived from window-vs-reference
by a **Match Factor** (0 = match width, 1 = match height, blended between; Unity-style, default 0.5,
config-driven), recomputed on resize. **No letterboxing**: elements anchor to the real screen edges and the
canvas scales, rather than being scaled as one fixed-aspect image with bars.

Layout uses a **RectTransform-style anchor model**: each element carries `anchorMin`/`anchorMax` (normalized
points in the parent rect), a `pivot`, and pixel **Anchored Offsets**. `min == max` is a point anchor with
absolute pixel size; `min != max` stretches the element with its parent, unifying corner-pinning, centering,
and relative/fill-with-margins sizing. This replaces the discrete 3×3 align enum and absolute-only size.

**Hit-testing transforms the window-pixel mouse into canvas units** (the inverse of the scale factor /
viewport) before testing bounds, so interaction matches rendering at any resolution. Pointer resolution is
**front-most-only with input capture**: the single nearest element under the cursor consumes the click and
raises an input-consumed flag so it does not leak to elements or gameplay behind it.

Interaction is **pointer-only** — no keyboard/gamepad focus navigation — though the event system is shaped so
focus state could be added later without reworking the component model.

On top of this core, **Layout Containers** (vertical/horizontal stack, grid), **Clip Rects** (scissor-based
masking for scroll views), and **Nine-Slice** image scaling are added as independent, additive capabilities.

## Considered options

- **Resolution policy — reference + anchors (chosen)** over **uniform-scale + letterbox** and
  **stretch-to-fill**. Letterbox is the simplest correct option but wastes screen space as bars and cannot
  pin HUD to true screen edges; stretch distorts on any aspect mismatch. Reference + anchors is the most work
  but the only option that gives edge-pinned HUD, centered menus, and aspect-correct elements together.
- **Layout model — anchor-rect (min/max) + pivot (chosen)** over a single-anchor-point + relative-size model
  and over keeping the discrete 3×3 enum plus a fill flag. The rect model is the only one expressing
  "stretch between two edges with independent margins," and it unifies anchoring and relative sizing in one
  system, at the cost of a richer authoring schema and bounds math.

## Consequences

- `BaseComponent`, the layout JSON schema + loader, the `UIUpdate*OneFrameComponent` mutation types, the
  SandboxApp layout JSON, and the app's UI systems all migrate to the anchor-rect model.
- `Renderer2DSystem` gains a second draw pass with the UI projection; `RendererConfigSingletonComponent`
  loses its camera and carries reference size + match factor, seeded from `EngineConfig`/`Config.json` and
  recomputed on resize.
- `UIEventSystem` becomes window→canvas-aware and front-most-with-capture; a UI-input-consumed signal is
  exposed for gameplay to honor.
- New additive features land independently: stack/grid containers, scissor clipping (the DX11 backend
  sets/clears a scissor per sub-tree, flushing the batch on change, like a texture change), and nine-slice
  image draws.
- The dead `RendererConfig.m_Camera` and the 1280×720/1920×1080 inconsistency are removed.
