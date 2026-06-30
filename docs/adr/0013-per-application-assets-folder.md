# ADR 0013 — Per-application assets folder

**Status:** Accepted
**Date:** 2026-06-29

## Context

PigeonLib is a static library reused by more than one application target (SandboxApp, and now a minimal
`App`). Each application has its own data tree under `Data/Assets/<project>` holding a
`ResourcesManifest.json` plus the category folders (`Textures`, `Fonts`, `UI`, …) the manifest paths
resolve under.

The engine's `ResourceManagerSystem` is the single place that loads the **app** resource manifest (the
engine manifest is always `Assets/Engine`). It hard-coded the app folder as `Assets/App`, which meant:

- The folder named `App` actually held *SandboxApp's* data — a misnomer.
- Only one application could exist, because every app's manifest had to live at the one hard-coded path.

We want a second application target whose data lives under its own folder, while SandboxApp keeps its
own — without the engine guessing or both apps fighting over `Assets/App`.

A constraint shapes the design: `Application::Create()` always constructs a base `pg::Application`
(`std::make_unique<Application>()`), so the client's `Sandbox`/`App` subclass is never the live instance.
A `virtual` folder accessor would therefore never dispatch to an override — the mechanism cannot rely on
subclassing.

## Decision

Make the app's assets folder a **per-application value carried on `pg::Application`**, defaulting to
`"App"`:

- `Application` gains `m_AppAssetsFolder = "App"` with `GetAppAssetsFolder()` / `SetAppAssetsFolder()`.
- The engine `ResourceManagerSystem` resolves the app manifest from it:
  `Assets/<GetAppAssetsFolder()>/ResourcesManifest.json`, and passes the same folder as the asset-path
  prefix. This is the sole place the engine names the app's folder. (The Testing build still loads
  `Assets/UT/ResourcesManifest.json` under `#ifdef TESTS_ENABLED` and never touches the Application
  instance, so unit tests are unaffected.)
- An application sets its folder in `CreateApplication()` after `Create()` and before `Run()`. The
  engine default `"App"` serves the new minimal `App` target unchanged; SandboxApp calls
  `SetAppAssetsFolder("Sandbox")` and its data was renamed `Data/Assets/App` → `Data/Assets/Sandbox`.

A setter on the host instance was chosen over a `virtual` accessor (which the always-base instance would
never dispatch) and over a new required free function like `pg::CreateApplication` (which would force
every client to define an extra symbol). The setter is opt-in: the common case — an app folder literally
named `App` — needs no code at all.

## Trade-offs

- **Reaching the value from the engine system through the Application host (chosen):**
  `ResourceManagerSystem` reads `pg::Application::Get().GetAppAssetsFolder()`, the same host-access path
  ADR 0004 established for the live window. The host dependency lives only in the non-test branch, so the
  Testing build (which constructs no `Application`) is untouched.
- **Default `"App"` (chosen):** keeps a folder literally named `App` zero-config, so the minimal `App`
  target needs no override; only an app whose folder differs (SandboxApp) sets it.
- **Set in `CreateApplication()` (chosen):** the value is needed only by the first-frame manifest load,
  which runs after `CreateApplication()` returns, so setting it there is in time. Threading it through
  `Application::Create()` as a constructor argument was rejected to avoid changing the client entry-point
  shape.

## Consequences

- `pg::Application` carries `m_AppAssetsFolder` (default `"App"`) with a getter/setter; `CreateApplication`
  is where each app declares its folder.
- `Data/Assets/App` (SandboxApp's old data) is renamed `Data/Assets/Sandbox`; SandboxApp reads
  `Assets/Sandbox/Config.json` and sets `SetAppAssetsFolder("Sandbox")`. `.gitignore` and the SandboxApp
  docs follow the rename.
- A new `Code/App` application target (minimal: defines `CreateApplication`, registers no app systems)
  uses the default `"App"` folder, with a fresh `Data/Assets/App/ResourcesManifest.json`.
- Adding further application targets is now just a new `Code/<App>` + `Data/Assets/<App>` + the
  `SetAppAssetsFolder` call.
