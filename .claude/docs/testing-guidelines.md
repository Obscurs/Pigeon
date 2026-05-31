# Testing Guidelines

## Module Documentation (Source of Truth)

Each module is specified with a file in `Documentation/diagrams/`:

- `<ModuleName>.info` — text description of the module's purpose, each system's role, and inter-system relationships.

Read the `.info` file as the primary source for identifying what to test and which edge cases are in scope for each system.

---

## Framework and Configuration

- Tests use **Catch2 v2** (`TEST_CASE`, `REQUIRE`, `CHECK`).
- The `UT` target is only built under the `Testing` CMake configuration.
- Test files are picked up automatically via `file(GLOB ...)` in `UT/CMakeLists.txt` — no manual registration needed when adding files to an existing module folder. When adding a **new** module folder, add a corresponding `file(GLOB ...)` entry in `UT/CMakeLists.txt`.

---

## Test Structure

```
UT/
├── UT_Core/              # Engine primitives (camera, ECS, events, input, renderer, UUID)
└── Utils/                # TestApp.h — test harness helpers
```

Each SandboxApp module has its own folder named `UT_<ModuleName>/`. Within that folder, **each system has its own test file** named `<SystemName>Test.cpp`.

Example: `SampleUISystem` → `UT_Sandbox/SampleUISystemTest.cpp`

A test file covers **only** the specific system it is named after. Do not test multiple systems in one file.

---

## Per-System Independence Rule

Each test file must register **only** the system under test. If the system's behavior requires specific component state that another system would normally provide, set up that state manually in the registry instead of registering the other system. This keeps tests isolated and prevents cascading failures.

---

## Running Without a Window

Use `pig::World::Create()` and `UT/Utils/TestApp.h` together with the `Platform/Testing/` mocks to instantiate systems and run ECS logic without a real window, renderer, or DirectX context.

Each test case must call `pig::World::Create()` to get a fresh, isolated ECS world. Never share world state between test cases.

---

## What to Test per System

For each system, cover:

1. **Happy path** — the system performs its intended transformation given valid input state.
2. **Guard conditions** — the system does nothing (or the correct thing) when preconditions are not met (e.g., required component absent, no relevant entities).
3. **Edge cases** — boundary values, multiple operations in one frame, conflicting state.

Derive the specific cases from the system's entry in `.claude/docs/diagrams/<ModuleName>.info`.

### Input and Output Scope

- Tests set up state by adding or modifying only components declared in the system's **`readSet`**.
- Tests verify the system's output by inspecting components in the **`writeSet`**, **`addSet`**, and **`inframeAddSet`**.
- Checks must cover **all members** of the output components — do not leave fields unverified.

### DeclareAccess Test (required)

Every system test file must include a dedicated test case that instantiates the system, calls `DeclareAccess()`, and asserts that every expected component type appears in the correct set (`readSet`, `writeSet`, `addSet`, `inframeAddSet`).

```cpp
TEST_CASE("Module.MySystem::DeclareAccessIsCorrect")
{
    sbx::MySystem sys;
    pig::SystemAccessDecl decl = sys.DeclareAccess();

    CHECK(decl.readSet.count(std::type_index(typeid(sbx::InputComponent))) > 0);
    CHECK(decl.writeSet.count(std::type_index(typeid(sbx::PositionComponent))) > 0);
    CHECK(decl.inframeAddSet.count(std::type_index(typeid(sbx::DrawQuadInFrameEvent))) > 0);
}
```

---

## Test Case Style

- Test cases should be **small and focused** — one behavior per test case.
- Do **not** use `SECTION` — prefer separate `TEST_CASE` entries instead.
- Tests verify **system behavior** (expected functionality and edge cases), not implementation details of the code.
