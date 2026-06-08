# ADR 0003 — Two-tier transform resolution

**Status:** Accepted
**Date:** 2026-06-08

## Context

The engine had no first-class notion of a world transform. Every renderable component
(`QuadComponent`, `SpriteComponent`, `LabelComponent`) baked its own `glm::mat4 m_Transform`, the
camera carried its own `m_CameraPosition`, and the render layer was packed into the translation's
`z`. Multiple systems (`QuadAnimationSystem`, `SceneSetupSystem`, `QuadSpawnSystem`, `CameraSystem`)
each built matrices directly. There was no shared, queryable placement for an entity, and no single
owner of that placement.

We want all non-UI entities with a world location to carry decomposed transform components
(Position, Rotation, Scale, Local Bounds), a single resolved World Transform that rendering reads,
and a request-driven update path so that placement changes flow through one canonical owner.

Two hard constraints from the ECS contract (`.claude/docs/architecture.md`, enforced at
`RegisterSystem`) shape the design:

1. **One adder/one writer per component type**, enforced globally across all systems regardless of
   entity or module. Two systems may not both add or both write the same component type.
2. **A system can only declare access to types its module can name.** An engine (`pg`) system
   cannot read a SandboxApp (`sbx`) request type.

Together these mean: many app systems want to move entities, but they cannot all write
`PositionComponent` and cannot all emit the same request type — and an engine resolver cannot read
app-specific requests at all.

## Decision

Split transform handling into engine-owned mechanism and a two-tier resolve pipeline.

**Engine (`pg`) owns:**

- The components: `PositionComponent`, `RotationComponent`, `ScaleComponent`,
  `LocalBoundsComponent`, `WorldTransformComponent`.
- `TransformResolveSystem` — the **sole adder and writer** of Position/Rotation/Scale/Local Bounds.
  It reads the single resolved app request (`ResolvedTransformRequestOneFrameComponent`) plus
  engine-origin requests (e.g. the camera's pan request) and applies them, adding the components on
  first request for an entity.
- `TransformComposeSystem` — the **sole adder and writer** of `WorldTransformComponent`. Reads
  Position/Rotation/Scale/Local Bounds, writes the resolved matrix and the render sort key.
- The renderer sorts world draws by the sort key (world-space bottom-edge Y) and always draws UI on
  top.

**App (`sbx`) owns:**

- One request type per app mover (each emitted by exactly one system, satisfying one-adder).
- `TransformResolveSystem` (app tier) — reads every app request type and aggregates them into the
  single engine-typed `ResolvedTransformRequestOneFrameComponent`. It is the **sole adder** of the
  resolved request type.

Requests are **absolute** (they set values, they do not accumulate deltas), so a mover reads the
current value when it needs relative motion.

## Trade-offs

- **Two-tier resolve (chosen):** The app tier collapses an open-ended number of app movers into one
  engine-readable request, so the engine resolver stays generic and unaware of app types. New app
  movers add one app request type and one line in the app aggregator — no engine change. The cost is
  two systems and one extra hop of latency between an app request and the canonical write.
- **Engine resolve with a fixed set of engine request types (rejected):** Keeps everything
  engine-side, but every new kind of app mover needs a new engine request type or careful ownership
  coordination — the engine would have to anticipate app motion sources it cannot know about.
- **Single app "motion driver" system feeding one engine request (rejected):** Simplest engine
  surface, but centralizes unrelated app motion logic into one system, working against the
  small-single-responsibility-system rule.
- **Movers write `PositionComponent` directly (rejected):** Illegal under one-writer — multiple
  systems set positions, so the resolve indirection is mandatory, not optional.

## Consequences

- Renderables no longer carry a matrix or origin. `QuadComponent`/`SpriteComponent`/`LabelComponent`
  keep only their visual data (colour, texture, UVs, text); placement comes from the transform
  components, and geometry extent comes from Local Bounds (the previous always-zero `m_Origin` is
  folded into bounds).
- The render layer is no longer packed into `z`. World draw order is derived from world-space
  bottom-edge Y; UI is drawn in a separate pass guaranteed on top.
- The camera carries a Position like any other entity; `CameraSystem` emits an engine pan request and
  syncs its `OrthographicCamera` from the resolved Position, but still owns zoom/aspect/projection.
- Adding a transform to an entity is done by emitting a "set transform" request, not by emplacing the
  components directly — only the engine `TransformResolveSystem` may add them.
