# ADR 0007 — Offscreen 3D render pass

**Status:** Accepted
**Date:** 2026-06-12

## Context

PigeonLib was a 2D engine: `Renderer2DSystem` draws quads, sprites, and text through two
orthographic cameras into the window's back buffer. A `Model` resource (Wavefront `.obj` →
CPU-side triangle geometry) and a `ModelComponent` already existed as a data path, but nothing
uploaded that geometry to the GPU or drew it.

We want to render 3D models (perspective-projected, depth-tested) **without** disturbing the 2D
renderer, and make the 3D image reusable as a texture inside the 2D world (e.g. drawn on a quad).
The two object sets must stay disjoint — a 3D entity carries a `ModelComponent`, a 2D entity carries
a `QuadComponent`/`SpriteComponent`; neither renderer ever touches the other's geometry.

The hard constraints come from the ECS contract: **one writer per component**, and execution order is
derived only from declared component access. The 2D renderer resolves textures **only** through
`ResourceMapSingletonComponent::m_TextureMap` (by UUID), and that component has a single writer
(`ResourceManagerSystem`). So the offscreen 3D image must reach the 2D pass as an ordinary entry in
that texture map.

## Decision

Add a second render pass that draws 3D models into an **offscreen render target**, and expose that
target's colour buffer as a normal texture.

- **Render target is a resource.** A new platform abstraction `pg::RenderTarget` (DirectX11 +
  Testing impls) owns an offscreen colour texture + depth buffer. It is created by
  `ResourceManagerSystem` from a `renderTargets` entry in the engine resource manifest, stored in a
  new `ResourceMapSingletonComponent::m_RenderTargetMap`, and its colour buffer is registered in
  `m_TextureMap` **under the same UUID**. Any 2D draw can then sample the 3D output by referencing
  that UUID like any other texture. `ResourceManagerSystem` stays the sole writer of the resource map;
  both renderers only **read** it.
- **3D camera.** A `pg::PerspectiveCamera` value type (fov / aspect / near-far + look-at view,
  left-handed, zero-to-one depth to match DirectX) carried by a `PerspectiveCameraComponent`. The app
  authors it (places it, points it at the subject), mirroring how the app authors the 2D
  `OrthographicCameraComponent`. The render-target aspect is fixed, so the 3D camera needs no
  resize handling.
- **`Renderer3DSystem`.** Reads the resource map (models, render target, 3D shader), the engine config
  (3D shader + render-target UUIDs), `PerspectiveCameraComponent`, `ModelComponent`, and
  `WorldTransformComponent`. It owns one GPU vertex/index buffer pair + the resolved shader in
  `Renderer3DDataSingletonComponent`. Each frame it binds the render target (clearing colour + depth,
  enabling depth test), and for every model uploads the geometry and the combined
  `viewProjection * world` matrix, then draws.
- **Flat unlit shading.** The model shader outputs a single hardcoded colour (no lighting, no texture
  sampling — `monkey.obj` has no meaningful UVs). Depth testing makes the rotating silhouette read as a
  solid 3D form. Culling stays `CULL_NONE` (consistent with ADR 0006); depth resolves visibility.
- **3D strictly before 2D.** `Renderer3DSystem` writes `Renderer3DDataSingletonComponent`;
  `Renderer2DSystem` declares a **read** of it purely to force the pass order. This guarantees the 2D
  pass samples the current frame's 3D output and that the window back buffer is rebound last (by the
  2D pass's `Begin`) before ImGui draws. There is no automatic ordering edge otherwise — rendering
  into the target is a GPU side effect invisible to the ECS graph.

## Trade-offs

- **Render target as a resource (chosen):** honours one-writer (only `ResourceManagerSystem` writes
  the map; both renderers read), and the 3D output is "just a texture" to the 2D pass — no special
  texture source, no coupling between the two renderers beyond the ordering read. Cost: the resource
  manifest/config gain a render-target concept, and `Renderer3DSystem` renders **into** a const-read
  resource (a GPU side effect, exactly as `Renderer2DSystem` binds textures it only reads).
- **3D renderer owns the target + publishes via a new singleton (rejected):** the 2D pass would need a
  texture source outside the resource map, coupling the two renderers and special-casing the 2D
  sampling path.
- **Ordering via an explicit ECS read-edge (chosen) vs. leaving it unordered (rejected):** without the
  edge the 2D quad could sample the previous frame's 3D output, and the offscreen target could be left
  bound when ImGui draws. The read-edge is a declared dependency, not a hidden one.

## Consequences

- `ResourceMapSingletonComponent` gains `m_RenderTargetMap`; `EngineConfigSingletonComponent` gains the
  3D shader + render-target UUIDs (seeded from `Assets/Engine/Config.json`); the engine resource
  manifest gains a `renderTargets` category and a `Renderer3DModel.shader`.
- `Renderer2DSystem`'s `readSet` gains `Renderer3DDataSingletonComponent` — an ordering-only
  dependency, not consumed in `Update()`.
- The offscreen pass enables depth testing (its own depth buffer + depth-stencil state), restored to
  the 2D no-depth state when the pass ends. The 2D pass is unchanged.
- Like the rest of the renderer, the GPU work (real render target, depth, draw) runs only on DirectX11;
  the Testing backend mocks it, so the offscreen image is verified visually rather than by unit test.
  Unit tests cover the camera math, the system's data flow (geometry upload, singleton lifecycle), and
  the resource wiring.
