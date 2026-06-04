# Coding Guidelines

## Language

- C++17. No exceptions in hot paths.

---

## Naming Conventions

| Target | Convention | Example |
|---|---|---|
| Local variable | `lowerCamelCase` | `entityCount`, `inputValue` |
| Function, class, struct | `UpperCamelCase` | `UpdatePosition`, `PhysicsSystem` |
| Member variable | `m_UpperCamelCase` | `m_Position`, `m_EntityCount` |
| Static variable | `s_UpperCamelCase` | `s_Instance`, `s_MaxSize` |
| System class and file | `<Name>System` | `PhysicsSystem`, `UIRenderSystem` |
| Regular component | `<Name>Component` | `PositionComponent`, `VelocityComponent` |
| One-frame component | `<Name>OneFrameComponent` | `CollisionOneFrameComponent` |
| In-frame component | `<Name>InFrameComponent` | `MovementInFrameComponent` |
| Singleton component | `<Name>SingletonComponent` | `CameraConfigSingletonComponent` |
| Event | `<Name>Event` | `JumpEvent`, `SpawnRequestEvent` |
| In-frame event | `<Name>InFrameEvent` | `DrawQuadInFrameEvent` |

---

## Documentation

Each module is documented in `Documentation/diagrams/`:

- `<ModuleName><SystemName>.info` — text description of the module's purpose, each system's role, and inter-system relationships. Code must match them so if a change is done in the code it should be reflected there as well.

General connection between modules and systems needs to be documented in the .claude/docs/architecture.md
It should always match with the code so if code changes this needs to be updated acordingly

## Style Rules

- Avoid `auto` — always write the explicit type. Exception: range-for iterators where the type is immediately obvious from context.
- Opening brace `{` always goes on its own line (Allman style).
- Use `const` and `const&` whenever applicable.
- Always write the full namespace — do not rely on `using namespace`.
- Do not add `TODO` comments or placeholder code.

---

## Include Directives

- Use `""` for all project headers and any header reachable via the configured include paths. Reserve `<>` for system and standard-library headers (e.g., `<vector>`, `<cstdint>`).
- In a `.cpp` file, the **corresponding header** comes first on its own line, followed by all other includes sorted **alphabetically**.
- Project headers are written with their **full path from the project root** (e.g., `"Pigeon/ECS/World.h"`, not `"World.h"`).

---

## CMakeLists.txt

- Source files in `file(GLOB ...)` and explicit source lists are sorted **alphabetically**.
- Do **not** list ECS system header files in CMakeLists sources — only the `.cpp` implementation file.

---

## General Architecture

- Everything should go through ECS when possible. Do not bypass ECS for shared state or cross-system communication.

---

## Components

Plain structs only — no methods, no logic. Constructors are allowed.

**Every component must have at least one data member.** For tag-only components, add `bool m_Dummy = true;` to satisfy the compiler.

Each component lives in its **own file**, named identically to the struct (e.g., `PositionComponent.h`).

### Component Types

There are six component types with distinct lifetime and accessor semantics:

| Type | Naming | Add accessor | Lifetime |
|---|---|---|---|
| Regular | `*Component` | `addSet` / `emplace_deferred` | Permanent; visible from the **next** frame |
| One-frame | `*OneFrameComponent` | `addSet` / `emplace_oneframe` | Visible from next frame; auto-removed at the **end of that next frame** |
| In-frame | `*InFrameComponent` | `inframeAddSet` / `emplace_inframe` | Permanent; visible to later systems **within the same frame** it is added |
| Singleton | `*SingletonComponent` | `addSet` / `emplace_deferred` | Permanent; typically one entity, created at startup, lives until shutdown |
| Event | `*Event` | `addSet` / `EmplaceEvent` | Like one-frame; not attached to a real entity |
| In-frame event | `*InFrameEvent` | `inframeAddSet` / `EmplaceInframeEvent` | Visible to later systems in the same frame; removed at end of that frame |

Component removal via `remove_deferred` is also deferred to the next frame.

---

## Systems

A system reads components and events as input and writes components and events as output.

### Hard Rules

1. **Constructor, destructor, and parent overrides only.** A system class may only contain a `public` constructor, destructor, and the parent class virtual overrides (`Update` and `DeclareAccess`). No additional member functions are allowed on the class.
2. **No member variables.** System classes never hold data. All helper functions live in the **unnamed namespace** of the `.cpp` file.
3. **Single responsibility.** Each system does one thing. If it grows too large, split it into smaller systems.
4. **Communicate via components only.** Systems communicate through ECS components and events — never by calling each other directly.
5. **One writer per component.** Only one system may modify a given component type. Only one system may add a specific component or event type.
6. **Minimal accessor.** `DeclareAccess()` must declare only the components strictly necessary for the system's logic — nothing extra.
7. **Conflicting writers → intermediate system.** When two systems would both need to add the same component or event, neither can do so directly. Each emits a distinct request component/event, and an intermediate system aggregates those into the single canonical output.

### Hard Rules — these are non-negotiable

1. **Systems do one thing.** A system should be as small and focused as possible. If it needs to do more, split it into multiple systems.
2. **Naming.** See `coding-guidelines.md` for the full naming table covering all system, component, and event types.
3. **Systems have no member variables or logic members.** A system class may only contain a constructor, destructor, and the parent class virtual overrides (`Update` and `DeclareAccess`). All helper functions live in the unnamed namespace of the `.cpp` file.
4. **Components are pure data.** No methods, no logic — plain structs only.
5. **One writer per component.** A component can be read by any number of systems, but only one system may add or modify it.
6. **Automatic execution order.** Systems are sorted automatically based on their `DeclareAccess` declarations. A system that reads a component is guaranteed to run after all systems that write or inframe-add that component. Declare access correctly and the order follows.
7. **One adder per component or event.** Only one system may add a specific component or event type. When two systems need to trigger the same output, they emit separate request components and an intermediate system aggregates them.


### System Class Shape

```cpp
// MySystem.h
#include "Pigeon/ECS/System.h"

namespace sbx
{
    class MySystem : public pg::System
    {
    public:
        MySystem() = default;
        ~MySystem() = default;
        void Update(const pg::Timestep& ts) override;
        pg::SystemAccessDecl DeclareAccess() const override;
    };
}
```

```cpp
// MySystem.cpp
#include "Sandbox/MySystem.h"

#include "Pigeon/ECS/World.h"
#include "Sandbox/InputComponent.h"
#include "Sandbox/PositionComponent.h"

namespace
{
    void MoveEntity(pg::CheckedRegistryAccessor& accessor, entt::entity e) { ... }
}

pg::SystemAccessDecl sbx::MySystem::DeclareAccess() const
{
    pg::SystemAccessDecl decl;
    decl.readSet       = { std::type_index(typeid(sbx::InputComponent)) };
    decl.writeSet      = { std::type_index(typeid(sbx::PositionComponent)) };
    decl.inframeAddSet = { std::type_index(typeid(sbx::DrawQuadInFrameEvent)) };
    return decl;
}

void sbx::MySystem::Update(const pg::Timestep& ts)
{
    pg::CheckedRegistryAccessor accessor = pg::World::GetRegistry();
    for (entt::entity e : accessor.view<const sbx::InputComponent, sbx::PositionComponent>())
    {
        // ...
    }
}
```

### Access Declarations (`DeclareAccess`)

| Field | Purpose | Accessor methods |
|---|---|---|
| `readSet` | Read components via `view<>` / `get<>` | — |
| `writeSet` | Modify existing component values in-place | — |
| `addSet` | Add components/events deferred to the next frame | `emplace_deferred`, `emplace_oneframe`, `EmplaceEvent` |
| `inframeAddSet` | Add in-frame components/events visible within this frame | `emplace_inframe`, `EmplaceInframeEvent` |

A type may appear in both `writeSet` and `addSet` when one system both creates new instances (deferred, via `addSet`) and modifies existing instances in-place (via `writeSet`).

### Automatic Execution Order

Systems are sorted automatically based on their `DeclareAccess` declarations. A system that has a component in its `readSet` is guaranteed to run **after** all systems that have that component in their `writeSet` or `inframeAddSet`. Declare access correctly and the correct order follows automatically.

---

## New System Checklist

Before implementing a new system:

1. Read `Documentation/<ModuleName>.info` — understand the system's role and its relationships.
2. Inherit from `pg::System`; implement `Update(const pg::Timestep& ts)` and `DeclareAccess() const`.
3. Implement required components as pure-data structs in dedicated header files.
4. Declare the minimal `readSet`, `writeSet`, `addSet`, `inframeAddSet` in `DeclareAccess()`.
5. Place all helper functions in the unnamed namespace of the `.cpp` — not as class methods.
6. Use `pg::World::GetRegistry()` inside `Update()`.

---

## Comments

Only add a comment when the **why** is non-obvious — a hidden constraint, a subtle invariant, a workaround for a specific bug. Do not describe what the code does; well-named identifiers already do that.
