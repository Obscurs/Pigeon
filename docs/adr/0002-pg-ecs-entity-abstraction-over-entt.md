# ADR 0002 — Abstract the ECS library behind the `pg::ecs` namespace

**Status:** Accepted  
**Date:** 2026-06-05

## Context

PigeonLib uses [EnTT](https://github.com/skypjack/entt) as its ECS implementation. Until now,
engine and game code referenced EnTT's public types directly: `entt::entity`, `entt::null`,
`entt::registry`, `entt::dispatcher`, and `entt::exclude_t` appeared throughout PigeonLib,
SandboxApp, and the unit tests (~29 files).

This couples every layer to a specific third-party library. There is no single seam at which the
ECS backend could be swapped, audited, or wrapped, and the engine's public vocabulary leaks an
external dependency's names.

## Decision

Introduce a `pg::ecs` sub-namespace (ECS module) that gives the engine its own names for the
ECS library's public surface, and use those names everywhere outside the abstraction header:

| `pg::ecs` name | Backing type |
|---|---|
| `pg::ecs::Entity` | `entt::entity` |
| `pg::ecs::null` | `entt::null` |
| `pg::ecs::Registry` | `entt::registry` |
| `pg::ecs::Dispatcher` | `entt::dispatcher` |
| `pg::ecs::exclude_t` / `pg::ecs::exclude` | `entt::exclude_t` / `entt::exclude` |

These are **type aliases**, not opaque wrappers. `entt::` is named only inside the single
`Pigeon/ECS/Entity.h` header. All other code — PigeonLib, SandboxApp, and UT — uses `pg::ecs::`
names exclusively.

## Trade-offs

- **Type aliases (chosen):** Zero runtime cost; entity comparison, hashing, and the `null`
  sentinel keep working unchanged; the migration is mechanical. The cost is that the abstraction
  is by-name only — `pg::ecs::Entity` *is* `entt::entity`, so code can still rely on EnTT-specific
  behaviour if it goes looking for it.
- **Opaque wrapper struct (rejected):** Would fully hide EnTT and let the backend change without
  touching call sites, but requires reimplementing operators/hashing and converting at every
  registry boundary — a much larger change with real bug risk, unjustified for a single-backend
  2D engine.

## Consequences

- A new header `Code/PigeonLib/Pigeon/ECS/Entity.h` owns the aliases and is the only place
  `entt::` is named (plus the EnTT include in `World`/`CheckedRegistryAccessor`, which still need
  the full library).
- Every `entt::entity` / `entt::null` / `entt::registry` / `entt::dispatcher` / `entt::exclude_t`
  outside that header becomes the corresponding `pg::ecs::` name.
- If EnTT is ever replaced, the aliases (and the registry/dispatcher ownership in `World`) are the
  seam to change; call sites are insulated from the type names.
