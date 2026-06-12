# ADR 0006 — Y-up world space

**Status:** Accepted
**Date:** 2026-06-11

## Context

World space was effectively **Y-down**: a `PositionComponent.y` of +1.5 rendered *below* the centre
of the screen, and gameplay code had to treat "up on screen" as decreasing world Y (e.g. the
character controller moved up by subtracting from Y). This was surprising for a 2D game world and
leaked the renderer's internal convention into gameplay code.

The Y-down convention was produced by a single mechanism: `Renderer2DSystem` negated every vertex's Y
(`pos.y *= FlipY() ? -1 : 1`, true on DirectX11). That negation did two jobs at once — it mapped the
y-down world to the screen *and* kept textures upright, because the quad UV layout, the font glyph
layout, and the UI canvas are all authored y-down. Removing it naively would render every texture and
every line of text upside-down. The whole stack (textures, MSDF text, UI) was consistently y-down, so
the convention was woven through the renderer, the font layout, the UI camera, and the app.

We want gameplay/world space to be **Y-up** (+Y is up on screen) without changing how any texture or
text reads on screen.

## Decision

Make world space Y-up and move the DirectX texture-orientation compensation out of the vertex
position and into the texture/anchor handling, per draw kind.

- **No vertex-position negation.** The camera projections map world (Y-up) and UI (y-down) space to
  the screen directly. The gameplay `OrthographicCamera` is a standard ortho (already symmetric); the
  UI camera uses an inverted ortho (`bottom = canvas height, top = 0`) so the y-down canvas keeps
  y=0 at the top — backend-independent now, no longer paired with a negation.
- **World quads/sprites flip the texture V** (`QuadData::flipTexV`, gated on `Texture2D::FlipY()`):
  DirectX samples V downward, so a Y-up world quad swaps the rect's top/bottom V to sample upright.
- **World text is reflected about its own anchor** (`AppendString::reflectAnchorY`, gated on
  `Texture2D::FlipY()`): the MSDF font lays glyphs out top-to-bottom in a y-down local space, so a
  world string is reflected about its anchor to read top-to-bottom on the Y-up screen while keeping
  glyph textures upright — no per-glyph V flip.
- **World draw order is descending** by sort key (world-space bottom-edge Y): higher Y (further back)
  draws first, lower Y (nearer the camera bottom) draws in front. The compose sort key is unchanged.
- **UI is untouched** in behaviour: it stays a y-down canvas, drawn with no V flip and no reflection
  through the inverted UI camera. The net on-screen result is identical to before.
- **App authoring is Y-up.** Gameplay uses +Y for up (the character controller adds to Y to move up);
  authored scene positions were re-signed so the showcase renders unchanged (a quad/sprite now grows
  *up* from its position — its bottom-left corner — so corner-anchored positions are
  `-old_y - scale_y`; text anchors are `-old_y`). Spin/orbit signs were negated to preserve their
  on-screen direction.

On the Testing backend `Texture2D::FlipY()` is `false`, so `flipTexV`/`reflectAnchorY` are inactive
and the no-flip path (Y-up, no negation) runs — which is what the unit tests assert.

## Trade-offs

- **Texture-V flip for world draws (chosen):** world space becomes genuinely Y-up everywhere — local
  space, Local Bounds, and the Y-sort are all Y-up — and textures are left semantically untouched
  (only the sampled V is swapped). The cost is that the world and UI draw paths diverge (world flips
  V / reflects text; UI does not), and corner-anchored content grows up rather than down, so authored
  positions carry a per-entity height offset to preserve an existing layout.
- **Negate Y at the transform/compose boundary (rejected):** keeps the renderer y-down and negates Y
  when composing the world matrix. Smaller diff, but it inverts rotation handedness inconsistently and
  hides the convention inside the matrix rather than fixing it.
- **Reflect every world draw about its anchor (considered):** provably pixel-identical to the old
  output, zero texture risk, but a sprite's local +Y still points down-screen, so it is not a truly
  Y-up world internally (Local Bounds / Y-sort stay y-down). Used only for world *text*, where
  top-to-bottom flow makes anchor reflection the natural, robust choice.

## Consequences

- **Back-face culling is disabled** (`Dx11RendererAPI` rasterizer `CULL_NONE`). The old vertex-Y
  negation also flipped triangle winding, which happened to make world geometry front-facing under
  DirectX's default (CW = front). Removing the negation left world triangles wound CCW = back-facing,
  so the entire world was silently culled. The world and UI cameras now wind triangles oppositely (the
  UI camera's inverted ortho flips winding), so winding must never decide visibility — correct for a 2D
  renderer regardless.
- `PositionComponent.y` is Y-up: +Y renders up. Gameplay reads naturally (up = +Y) — e.g. the Sandbox
  camera pans up (W) by increasing the camera's world Y, and the character moves up by increasing its Y.
- The Y-sort rule flips: higher Y draws behind, lower Y draws in front (see CONTEXT.md). The compose
  sort key (world bottom-edge Y) is unchanged; only the renderer's sort direction flipped.
- `GetUICameraOrthoBottomTop` lost its `flipY` parameter — the UI camera is backend-independent.
- The renderer's flip behaviour (texture-V flip, text reflection) only runs on DirectX11 and is not
  exercised by the Testing backend, so it is verified visually rather than by unit test.
