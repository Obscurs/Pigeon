# Architecture Guidelines

## Engine vs. Game Split

- **PigeonLib** — engine-generic code only: windowing, rendering, ECS primitives, UI, events, input.
- **SandboxApp** — Sandbox app for the engine with sample usages for it


## Application / Layer Stack

`Application` (in `PigeonLib/Pigeon/Core/`) owns a layer stack. Each `Layer` receives `OnUpdate`, `OnRender`, and `OnEvent` callbacks. `Sandbox2D` is the main game layer pushed at startup. `SandboxApp/SandboxApp.cpp` is the entry point (`CreateApplication()`).

## Renderer

`Renderer2D` (`PigeonLib/Pigeon/Renderer/`) uses batched quad rendering (max 1000 quads/batch) with a DirectX 11 backend. Fonts use MSDF (multi-channel signed distance fields). The UI system (`PigeonLib/Pigeon/UI/`) is a separate hierarchy-based system (parent/child entt entities) with its own layout, control, and render systems — distinct from Renderer2D.

## Platform Abstractions

Platform-specific code lives in `PigeonLib/Platform/`. `PG_PLATFORM_WINDOWS` / `PG_PLATFORM_UNIX` preprocessor defines control which implementations are compiled. A `Testing/` platform mock exists for unit test isolation.

## ECS Contract

The ECS is powered by [EnTT](https://github.com/skypjack/entt). `World` (`Code/PigeonLib/Pigeon/ECS/World.h`) owns the `entt::registry`, sorts registered systems by their `DeclareAccess` dependency graph, and calls `Update(Timestep)` on each system in the derived order each frame.

## SandboxApp Domain Modules

Systems are grouped into modules. Each module has a dedicated folder under `PigeonLib/` and its own documentation in `Documentation/diagrams/`.

## Key Files

| File | Purpose |
|---|---|
| `Code/PigeonLib/Pigeon/ECS/World.h` | ECS World API — registry, system registration, deferred operations |
| `Code/PigeonLib/Pigeon/ECS/CheckedRegistryAccessor.h` | Accessor API — view, get, emplace_deferred, emplace_inframe, EmplaceEvent, etc. |
| `Code/PigeonLib/Pigeon/ECS/System.h` | Base `System` class and `SystemAccessDecl` struct |
| `Code/SandboxApp/Sandbox2D.cpp` | Main game layer — initialization and per-frame orchestration |
