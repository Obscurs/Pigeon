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

The ECS is powered by [EnTT](https://github.com/skypjack/entt). `World` (`PigeonLib/Pigeon/ECS/World.h`) owns the `entt::registry` and `entt::dispatcher`, and calls `Update(Timestep)` on each registered system each frame in order.

### Hard Rules — these are non-negotiable

1. **Systems do one thing.** A system should be as small and focused as possible. If it needs to do more, split it into multiple systems.
2. **Naming.** All systems are named `<Name>System`. All components are named `<Name>Component`. All events are named `<Name>Event`.
3. **Systems have no logic members.** The only member variables allowed on a system class are the event dispatch queues (vectors/lists used to batch dispatched events). No other data members, no helper methods.
4. **Components are pure data.** No methods, no logic — plain structs only.
5. **One writer per component.** A component can be read by any number of systems, but only one system may add or modify it. The system that attaches a component to an entity does not need to be the same system that modifies its values later — but each responsibility belongs to exactly one system.
6. **Execution order determines visibility.** Systems are registered in `SandboxApp/SystemRegister.cpp` in the order they execute each frame. A system that reads a component must be registered **after** the system that writes it.
7. **One dispatcher per event.** Only one system may dispatch a specific event type. Every system that subscribes to an event must process and clear its queue **every frame** — never accumulate across frames.

## SandboxApp Domain Modules

Systems are grouped into modules. Each module has a dedicated folder under `PigeonLib/` and its own documentation in `Documentation/diagrams/`.

## Module Documentation (Source of Truth)

Each module is documented with two files in `Documentation/diagrams/`:

- `<ModuleName>.info` — text description of the module's purpose, each system's role, and inter-system relationships.

**Always read both files before implementing or testing anything in a module.** These files are the source of truth. Code must match them. Agents do not modify these files unless explicitly instructed.

## Key Files

| File | Purpose |
|---|---|
| `SandboxApp/SaveData/DataPaths.h` | Asset file paths for JSON data |
| `PigeonLib/Pigeon/ECS/World.h` | ECS World API (registry, dispatcher, system registration) |
| `SandboxApp/Sandbox2D.cpp` | Main game layer — initialization and per-frame orchestration |
