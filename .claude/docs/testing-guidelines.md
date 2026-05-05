# Testing Guidelines

## Module Documentation (Source of Truth)

Each module is specified with two files in `Documentation/diagrams/`:

- `<ModuleName>.info` — text description of the module's purpose, each system's role, and inter-system relationships.

Read the `.info` file as the primary source for identifying what to test and which edge cases are in scope for each system.

---

## Framework and Configuration

- Tests use **Catch2 v2** (`TEST_CASE`, `SECTION`, `REQUIRE`, `CHECK`).
- The `UT` target is only built under the `Testing` CMake configuration.
- Test files are picked up automatically via `file(GLOB ...)` in `UT/CMakeLists.txt` — no manual registration needed when adding files to an existing module folder. When adding a **new** module folder, add a corresponding `file(GLOB ...)` entry in `UT/CMakeLists.txt`.

---

## Test Structure

```
UT/
├── UT_Core/              # Engine primitives (camera, ECS, events, input, renderer, UUID)
└── Utils/                # TestApp.h — test harness helpers
```

Each SandboxApp module has its own folder named `UT_<ModuleName>/`. Within that folder, **each system has its own test file** named `<SystemFileName>Test.cpp`.

Example: `GameStateChangeSystem.cpp` → `UT_GameState/GameStateChangeSystemTest.cpp`

A test file covers **only** the specific system it is named after. Do not test multiple systems in one file.

---

## Running Without a Window

Use `pig::World::Create()` and `UT/Utils/TestApp.h` together with the `Platform/Testing/` mocks to instantiate systems and run ECS logic without a real window, renderer, or DirectX context.

Each test case must call `pig::World::Create()` to get a fresh, isolated ECS world. Never share world state between test cases.

---

## What to Test per System

For each system, cover:

1. **Happy path** — the system performs its intended transformation given valid input state.
2. **Guard conditions** — the system does nothing (or does the correct thing) when preconditions are not met (e.g., required component absent, empty event queue).
3. **Event handling** — dispatching an event causes the expected state change; the queue is empty after `Update()`.
4. **Edge cases** — boundary values, multiple events in one frame, conflicting state.

Derive the specific cases from the system's entry in `.claude/docs/diagrams/<ModuleName>.info`.

---

## Per-System Independence Rule

Each test file must register only the system it is testing. If the behavior under test genuinely requires another system to run first, set up the required component state manually in the registry instead of registering the other system. This keeps tests isolated and prevents cascading failures.
