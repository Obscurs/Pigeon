# ADR 0004 — Window resolution & display mode as configurable engine state

**Status:** Accepted
**Date:** 2026-06-09

## Context

The window was always created at a hard-coded `1280x720` (the `WindowProps` default) and there was no
way to change its resolution or switch to fullscreen at runtime, nor to persist a chosen resolution
across launches. `Config.json` already carried engine defaults (shaders, font, audio volumes) with a
savedata override file layered on top by `ConfigLoaderSystem`, but window geometry was not part of it.

We want:

1. A **default window resolution and display mode** declared in `Config.json` (engine) and overridable
   in the savedata `Config.json`, loaded into the engine config like every other config value.
2. **Windowed / fullscreen** options.
3. A runtime path so the game (SandboxApp) can change the resolution and have the change **applied to
   the live window** and **persisted** back to the savedata `Config.json`.

Two forces shape the design:

- The live OS `Window` is a platform abstraction owned by `Application`, created in `Init()` *before*
  the ECS world runs its first frame. ECS systems cannot name concrete platform types.
- The ECS contract (`.claude/docs/architecture.md`): one adder/writer per component; engine (`pg`)
  systems cannot read app (`sbx`) request types; systems talk only through components/events.

## Decision

Model window geometry exactly like audio volume (the established precedent):

**Config → engine config:** `windowWidth`, `windowHeight`, `windowMode` keys are read by
`ConfigLoaderSystem` into `EngineConfigSingletonComponent` (`m_WindowWidth`, `m_WindowHeight`,
`m_WindowMode`). They are optional with sensible defaults (`1280x720`, windowed), and the savedata
override layers over them through the existing `ApplyConfigEntries` routine. `ConfigLoaderSystem` also
records the resolved savedata path (`m_SavedataPath`) so the runtime knows where to persist.

**Live state:** `WindowConfigSingletonComponent` holds the live resolution + mode, seeded once from the
engine config — mirroring `AudioVolumeSingletonComponent`.

**Runtime change:** an engine-typed one-frame request `SetWindowResolutionRequestOneFrameComponent`
carries a desired resolution + mode. The engine `WindowConfigSystem`:

- seeds `WindowConfigSingletonComponent` from the engine config on the first frame it can (and applies
  that startup resolution to the live window),
- on each request, updates the live singleton, applies the change to the live window, and **persists**
  `windowWidth`/`windowHeight`/`windowMode` into the savedata `Config.json` (merging into the existing
  file so other overrides are preserved).

**Reaching the live window from a system:** `WindowConfigSystem` is stateless (no member variables, per
the ECS contract) and reaches the live window through the Application host —
`pg::Application::Get().GetWindow().ApplyWindowConfig(width, height, mode)`, guarded by
`Application::HasInstance()`. The window is a platform resource owned by `Application`, not by ECS; this
is the same path `WindowsInput`, the DX11 layer, and `ImGuiLayer` already use to reach it. The Testing
build constructs no `Application`, so `HasInstance()` is false and the window apply is skipped, keeping
the seed/request/persist logic fully unit-testable without a real window.

Storing the `pg::Window*` as a system member was rejected because the ECS contract forbids system member
variables (systems are stateless transformers; all shared state lives in ECS). Bridging the window
through a singleton component (as `AudioDevice` is) was considered, but the window — unlike the
factory-created audio device — is created and owned by `Application` before the world's first frame, and
there is no non-test path for `Application` to seed an ECS component at boot; reaching it through the
host is the smaller, precedent-backed change.

**Fullscreen = borderless windowed.** `ApplyWindowConfig` in `WindowsWindow` switches the window style
(`WS_POPUP` for fullscreen, the normal caption style for windowed) and resizes via `SetWindowPos`; the
resulting `WM_SIZE` drives the existing swapchain/renderer resize path. Exclusive DXGI fullscreen was
rejected as brittle with the current swapchain and ImGui integration.

**App widget:** SandboxApp's `WindowDebugPanelSystem` is an ImGui panel (a preset-resolution dropdown +
a windowed/fullscreen radio + an Apply button) that emits `SetWindowResolutionRequestOneFrameComponent`
— the same shape as `AudioDebugPanelSystem` emitting `SetAudioVolumeRequestOneFrameComponent`.

## Trade-offs

- **Seed-and-apply on the first frame (chosen):** one code path serves both startup and runtime; the
  window is created at the `WindowProps` default and then resized to the configured resolution on the
  first frame the config is available. Cost: a possible one-frame resize flash when the configured
  resolution differs from the default. Reading the config synchronously in `Application::Init` to size
  the window from the first pixel was rejected to avoid duplicating the file-read + savedata-override
  logic that already lives in `ConfigLoaderSystem`.
- **Engine-typed request (chosen):** the request must be a `pg` type because the engine
  `WindowConfigSystem` reads it and engine systems cannot read `sbx` types — identical to the audio
  request.
- **Reaching the window through the Application host (chosen) vs. a system member pointer:** a member
  pointer violates the ECS contract's "systems hold no member variables" rule; reaching the host-owned
  window through `Application::Get()` (guarded by `HasInstance()`) keeps the system stateless and matches
  how the rest of the platform layer reaches the window, at the cost of a host dependency that the test
  harness simply skips.

## Consequences

- `EngineConfigSingletonComponent` gains window fields and the savedata path; `ConfigLoaderSystem` reads
  them (optional, with defaults) and is the only writer of the engine config.
- New engine pieces: `EWindowMode`, `WindowConfigSingletonComponent`, `WindowConfigSystem`,
  `SetWindowResolutionRequestOneFrameComponent`, `Window::ApplyWindowConfig`, and `FileUtils`
  (read/write/exists) shared by `ConfigLoaderSystem` and `WindowConfigSystem`.
- The savedata `Config.json` is now written by the engine at runtime (resolution changes), not just
  read. Writes merge into the existing file to preserve other overrides.
- SandboxApp gains `WindowDebugPanelSystem`; the engine config and savedata config both carry a default
  window resolution.
