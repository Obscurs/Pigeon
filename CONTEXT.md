# Pigeon — Domain Glossary

## Namespaces

| Identifier | Meaning |
|---|---|
| `pg` | The primary C++ namespace for all PigeonLib engine code (headers, implementations, platform layers). Previously `pig`. |
| `pg::ui` | Sub-namespace for the UI module within PigeonLib. |
| `pg::ecs` | Sub-namespace for the ECS abstraction layer. Holds the engine's own names for the underlying ECS library's public types (`Entity`, `null`, `Registry`, `Dispatcher`, `exclude`). Engine and game code use these names exclusively; the third-party `entt::` types are named only inside the ECS abstraction header. |
| `sbx` | The C++ namespace for all SandboxApp game code. |
