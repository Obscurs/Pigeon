# Architecture Rules

## Engine vs. Game Split

- **PigeonLib** — engine-generic code only: windowing, rendering, ECS primitives, UI, events, input. No application-specific logic.
- **SandboxApp** — application code that uses PigeonLib. All domain-specific systems, components, and assets live here.

## ECS Contract

- `World` owns the single `pg::ecs::Registry`. All entity and component access goes through `CheckedRegistryAccessor`.
- **One writer per component.** Only one system may write or add a given component type. Violations assert at `RegisterSystem` time.
- Systems declare their complete component access via `DeclareAccess()`. Accessing an undeclared component asserts at runtime.
- **Execution order is automatic.** `World` derives system order from `DeclareAccess` declarations using a topological sort. Never set or assume order manually — declare the correct access and the order follows.
- Structural changes (add, remove, entity destroy) are deferred to frame end. Never modify the registry during system iteration.
- In-frame operations (`inframeAddSet`) are visible to later systems within the same frame. Deferred operations (`addSet`) are visible in the next frame.

## Platform Abstractions

- All OS and GPU code lives in `Platform/`. Engine code uses only the abstract interfaces in `Pigeon/Core/` and `Pigeon/Renderer/`.
- `TESTS_ENABLED` switches `Window`, `PlatformInput`, and `RendererAPI` implementations at link time — there are no runtime branches.
- Never reference concrete platform types (`WindowsWindow`, `Dx11Context`, etc.) from engine code outside of `ImGuiLayer`, which is a deliberate exception.

## Communication Rules

- Systems communicate exclusively through ECS components and events — never by calling each other directly.
- Events are short-lived entities tagged with `EventComponent`. `World::ClearEvents` destroys all of them at the end of every frame.
- Platform OS events are bridged into ECS event components by `Application::OnEvent` via `World::EmplaceExternalEvent`. ECS systems never touch `pg::Event` objects.
