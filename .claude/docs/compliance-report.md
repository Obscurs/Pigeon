# Pigeon Codebase Compliance Report

## Section 1: Violations

### Naming

**N-1. `SandboxApp/SampleUIComponent.h` — orphan file**
The struct `SampleUIComponent` lives at the SandboxApp root rather than under a module folder. This file also appears to be dead code (not in CMakeLists.txt).

**N-2. `UIComponents.h` — wrong tag-member name**
`UIDestroyOneFrameComponent` (line 120) uses `m_DUMMYVAR` for its dummy member. The guideline specifies `m_Dummy`.

---

### Style — `auto` Usage

The guideline forbids `auto` except for range-for iterators. The following lines use `auto accessor = pig::World::GetRegistry();` — must be the explicit type `pig::CheckedRegistryAccessor`:

- `Code/SandboxApp/Sandbox/SampleUISystem.cpp:46`
- `Code/SandboxApp/Sandbox/ConfigLoaderSystem.cpp:40`
- `Code/PigeonLib/Pigeon/Core/CameraSystem.cpp:28`
- `Code/PigeonLib/Pigeon/Core/InputSystem.cpp:100`
- `Code/PigeonLib/Pigeon/Core/ConfigLoaderSystem.cpp:39`
- `Code/PigeonLib/Pigeon/Core/ResourceManagerSystem.cpp:115`
- `Code/PigeonLib/Pigeon/Renderer/Renderer2DSystem.cpp:391`
- `Code/PigeonLib/Pigeon/UI/UIControlSystem.cpp:255`
- `Code/PigeonLib/Pigeon/UI/UIEventSystem.cpp:39`
- `Code/PigeonLib/Pigeon/UI/UIRenderSystem.cpp:87`

Additional stored-view variables (not range-for iterators) in `SampleUISystem.cpp`: lines 48 (`auto resourcesView`), 49 (`auto configView`), 56 (`auto view`).

---

### Style — Brace Style (Allman)

Opening `{` must be on its own line. Same-line braces found at:

- `Code/PigeonLib/Pigeon/Core/ConfigLoaderSystem.cpp:13`
- `Code/PigeonLib/Pigeon/Core/ResourceManagerSystem.cpp:18`
- `Code/SandboxApp/Sandbox/ConfigLoaderSystem.cpp:13`
- `Code/PigeonLib/Pigeon/UI/UIControlSystem.cpp:13`

---

### Style — Namespace Qualification

- `Code/PigeonLib/Pigeon/UI/UIHelpers.cpp:53,57,63,66,69,72,76` — Enum values written without full namespace (e.g., `EHAlignType::eRight` instead of `pig::ui::EHAlignType::eRight`).

---

### Style — What-Comments

Comments that describe what the code does rather than why:

- `Code/SandboxApp/Sandbox/SampleUISystem.cpp:138` — `// Identity matrix`
- `Code/SandboxApp/Sandbox/SampleUISystem.cpp:139` — `// Apply translation`
- `Code/SandboxApp/Sandbox/SampleUISystem.cpp:140` — `// Apply scaling`
- `Code/SandboxApp/Sandbox/SampleUISystem.cpp:167,168,169` — same pattern for clock string transform

---

### Includes

**I-1. Corresponding header not using full project-root path:**

- `Code/SandboxApp/Sandbox/SampleUISystem.cpp:1` — `#include "SampleUISystem.h"` → must be `"Sandbox/SampleUISystem.h"`
- `Code/SandboxApp/Sandbox/ConfigLoaderSystem.cpp:2` — `#include "ConfigLoaderSystem.h"` → must be `"Sandbox/ConfigLoaderSystem.h"`

**I-2. Missing `#pragma once`:**

- `Code/SandboxApp/Sandbox/SampleUISingletonComponent.h`
- `Code/PigeonLib/Pigeon/Core/InputStateSingletonComponent.h`

**I-3. Angle brackets `<>` for project headers in all test files:**

Every test file in `UT/UT_Core/`, `UT/UT_UI/`, `UT/UT_Renderer/`, `UT/UT_Sandbox/` uses angle brackets for project headers (e.g., `<Pigeon/ECS/World.h>`). The guideline requires `""` for all project headers reachable via configured include paths.

---

### CMake

**C-1. System/component header files listed in explicit source lists:**

`Code/PigeonLib/CMakeLists.txt` lists many `.h` files explicitly (e.g., `Pigeon/Core/EngineConfigSingletonComponent.h`, `Pigeon/Core/Clock.h`, `Pigeon/Core/EventComponent.h`, etc.). The guideline says do not list ECS system header files — only `.cpp` implementation files.

---

### Components

**CO-1. Component holds a logic class as a member:**

- `Code/SandboxApp/SampleUIComponent.h:16` — `pig::OrthographicCameraController m_CameraController` is a class with methods and input-processing logic, violating "components are pure data" (ECS Hard Rule 4). This file also appears to be dead code.

**CO-2. Multiple components defined in one file:**

- `Code/PigeonLib/Pigeon/UI/UIComponents.h` bundles 15 distinct component/event types. The guideline requires each component to live in its own file named identically to the struct.

---

### Systems

**S-1. Hidden state via `static` local variable:**

- `Code/SandboxApp/Sandbox/SampleUISystem.cpp:153` — `static pig::Clock clock;` inside `Update()`. A static local persists across calls and is effectively a member variable. The clock state belongs in a singleton component.

**S-2. Orphaned stale file with member variables on system class:**

- `Code/SandboxApp/SampleUISystem.cpp` — This root-level file references `m_Helper` (a member variable) and uses non-existent type names (e.g., `RendererConfig` instead of `RendererConfigSingletonComponent`). It appears to be dead code not included in CMakeLists.txt and should be deleted.

**S-3. Inaccurate `DeclareAccess` — one-frame components in `writeSet`:**

- `Code/PigeonLib/Pigeon/UI/UIControlSystem.cpp:238` — `UIUpdateTransformOneFrameComponent` declared in `writeSet` but the system only reads these components; they should be in `readSet`.

---

### Testing

**T-1. `#pragma once` on `.cpp` test files:**

All 14 test files have `#pragma once` as the first line. This has no effect in a translation unit. Affects:
`CameraSystemTest.cpp`, `ConfigLoaderSystemTest.cpp` (Core and Sandbox), `InputSystemTest.cpp`, `OrthographicCameraTest.cpp`, `ResourceManagerSystemTest.cpp`, `UUIDTest.cpp`, `WorldTest.cpp`, `Dx11RendererAPITest.cpp`, `Renderer2DSystemTest.cpp`, `SampleUISystemTest.cpp`, `UIControlSystemTest.cpp`, `UIEventSystemTest.cpp`, `UIRenderSystemTest.cpp`.

**T-2. `SECTION` used in test files:**

- `Code/UT/UT_Core/UUIDTest.cpp:23,32,40,54` — four `SECTION` blocks inside one `TEST_CASE`. The guideline explicitly forbids `SECTION`; use separate `TEST_CASE` entries.

**T-3. Test file with zero active test content:**

- `Code/UT/UT_Interface/Dx11RendererAPITest.cpp` — All test case bodies are entirely commented out.

**T-4. Missing DeclareAccess assertions:**

- `Code/UT/UT_UI/UIRenderSystemTest.cpp` — The `DeclareAccessIsCorrect` test does not assert `inframeAddSet` contains `DrawUIQuadInFrameEvent` or `DrawUIStringInFrameEvent`, even though `UIRenderSystem::DeclareAccess()` declares them there.

**T-5. Test-local system class with member variable:**

- `Code/UT/UT_Core/WorldTest.cpp:103` — `AdderSystem` has `bool m_Added = false;` as a class member. Though a test helper, it encodes the wrong pattern in the ECS test file itself.

**T-6. Test name format inconsistency:**

- `Code/UT/UT_Core/WorldTest.cpp:162,176,199,221` — Names like `"Core.ECS::World"` and `"Core.ECS.SystemOrdering::WriterRunsBeforeReader"` use extra dot segments. The expected format is `<ModuleName>.<SystemName>::<Description>`.

**T-7. `pig::CreateApplication()` in a World test:**

- `Code/UT/UT_Core/OrthographicCameraTest.cpp:24` — Uses `pig::CreateApplication()` instead of `pig::World::Create()`. Every test case must create a fresh ECS world via `pig::World::Create()`.

---

## Section 2: Suggested Rules

Patterns observed consistently across the codebase that appear intentional but are not documented in the guidelines.

**SR-1. Guard pattern: return early if singleton absent**
Every system that depends on a singleton component checks for its presence (via `view.empty()`) at the top of `Update()` and returns early if absent. Proposed rule: *"Systems that depend on singleton components must check for the singleton's presence at the top of `Update()` and return early if absent."*

**SR-2. Bootstrap pattern: first-frame creation of singletons**
Systems responsible for a singleton component detect its absence on the first frame and create it via `emplace_deferred`. Proposed rule: *"A system responsible for creating a singleton component must guard with `view.empty()` and use `emplace_deferred` to create it on the first frame."*

**SR-3. Test namespace: all tests wrapped in `namespace CatchTestsetFail`**
Every test file wraps its `TEST_CASE` entries in `namespace CatchTestsetFail { ... }`. Proposed rule: *"All test cases must be wrapped in `namespace CatchTestsetFail` to avoid linker collisions."*

**SR-4. Test state setup via `GetRegistryDirect()`**
Tests seed pre-conditions using `pig::World::GetRegistryDirect().create()` and `.emplace<>()`, bypassing the checked accessor. Proposed rule: *"Test cases seed component state using `pig::World::GetRegistryDirect()` directly, bypassing the access-check layer."*

**SR-5. Components reference resources by UUID only**
All resource references in components are `pig::UUID` handles; actual resources live in `ResourceMapSingletonComponent`. No raw or shared pointers appear in component definitions. Proposed rule: *"Components reference resources by UUID only; direct resource pointers (`shared_ptr`, raw ptr) must not appear in component definitions."*

**SR-6. Views stored in named variables before iteration**
In all systems, `accessor.view<...>()` is called once and stored in a named variable before being iterated — never created inline inside a range-for. Proposed rule: *"Store view results in named variables before iterating; do not create views inline in range-for expressions."*

**SR-7. Components explicitly declare both default and copy constructors**
Nearly all components declare `ComponentName() = default;` and `ComponentName(const ComponentName&) = default;` explicitly. Proposed rule: *"All components must explicitly declare both the default constructor and the copy constructor as `= default;`."*

**SR-8. One-frame components as request/command signals**
`UIUpdateXxx...OneFrameComponent` types are consistently used as command objects: an external caller adds one to request a mutation; the owning system reads and applies it in the same frame. Proposed rule: *"To request a mutation on a component owned by another system, add a dedicated `*OneFrameComponent` carrying the new values; the owning system reads and applies it within the same frame."*
