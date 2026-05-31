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

### Hard Rules — these are non-negotiable

1. **Systems do one thing.** A system should be as small and focused as possible. If it needs to do more, split it into multiple systems.
2. **Naming.** See `coding-guidelines.md` for the full naming table covering all system, component, and event types.
3. **Systems have no member variables or logic members.** A system class may only contain a constructor, destructor, and the parent class virtual overrides (`Update` and `DeclareAccess`). All helper functions live in the unnamed namespace of the `.cpp` file.
4. **Components are pure data.** No methods, no logic — plain structs only.
5. **One writer per component.** A component can be read by any number of systems, but only one system may add or modify it.
6. **Automatic execution order.** Systems are sorted automatically based on their `DeclareAccess` declarations. A system that reads a component is guaranteed to run after all systems that write or inframe-add that component. Declare access correctly and the order follows.
7. **One adder per component or event.** Only one system may add a specific component or event type. When two systems need to trigger the same output, they emit separate request components and an intermediate system aggregates them.

## SandboxApp Domain Modules

Systems are grouped into modules. Each module has a dedicated folder under `PigeonLib/` and its own documentation in `Documentation/diagrams/`.

## Module Documentation (Source of Truth)

Each module is documented in `Documentation/diagrams/`:

- `<ModuleName>.info` — text description of the module's purpose, each system's role, and inter-system relationships.

**Always read the `.info` file before implementing or testing anything in a module.** These files are the source of truth. Code must match them. Agents do not modify these files unless explicitly instructed.

## Key Files

| File | Purpose |
|---|---|
| `Code/PigeonLib/Pigeon/ECS/World.h` | ECS World API — registry, system registration, deferred operations |
| `Code/PigeonLib/Pigeon/ECS/CheckedRegistryAccessor.h` | Accessor API — view, get, emplace_deferred, emplace_inframe, EmplaceEvent, etc. |
| `Code/PigeonLib/Pigeon/ECS/System.h` | Base `System` class and `SystemAccessDecl` struct |
| `Code/SandboxApp/Sandbox2D.cpp` | Main game layer — initialization and per-frame orchestration |
