# Plan

This file is the active work plan. All agents read it before starting. Update it as steps are completed.


## Steps

| # | Task | Status |
|---|---|---|
| 1 | The already existing systems are not covered by unit tests write and run them | DONE |
| 2 | Review changes from task 1 | DONE |
| 3 | Transform management — see "Transform feature" below | DONE |
| 4 | 3D model resource (.obj) — engine loads `pg::Model` into the resource map; app declares + requests it via `pg::ModelComponent` (foundation for 3D scenes) | DONE |

## Transform feature

Add proper transform management (ADR 0003, two-tier resolve). All non-UI entities carry decomposed
transform components (Position/Rotation/Scale/LocalBounds); a resolved WorldTransform is the single
value rendering reads; world draw order comes from world-Y + vertical bounds; UI always on top.

| Stage | Work | Status |
|---|---|---|
| 1 | Engine: P/R/S/Bounds/WorldTransform components, `pg::TransformResolveSystem`, `pg::TransformComposeSystem`, resolved + camera request types | DONE |
| 2 | App: `sbx::TransformResolveSystem` + per-mover request types (Scene/QuadSpawn/Animation) | DONE |
| 3 | Renderer: order world draws by WorldTransform sort key (lower Y behind); UI drawn on top | DONE |
| 4 | Migrate quads (QuadComponent visual-only; QuadSpawn/QuadAnimation emit requests; QuadRenderSystem reads WorldTransform) | DONE |
| 5 | Migrate sprites + text to transform components | DONE |
| 6 | Migrate camera (Position via transform; CameraSystem emits pan request, syncs OrthographicCamera) | DONE |
| 7 | Remove dead code (m_Transform/m_Origin fields, z-as-layer), update all .info + architecture.md | DONE |


