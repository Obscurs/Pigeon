# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Rules

- Never modify `CLAUDE.md` or `.claude/settings.local.json` without explicit user approval.
- Use the `implement-feature` skill for feature implementation tasks and the `code-review` skill for reviews. Never skip the review step.
- All project skills live in `.agents/skills/`. Never look for them in `.claude/skills/`.

## Projects

Three projects live in this repo:

- **PigeonLib** — custom 2D game engine (static library). Handles windowing, DirectX 11 rendering, ECS, events, ImGui, and UI.
- **SandboxApp** — Sandbox app to test the PigeonLib.
- **UT** — unit tests for both engine and application, built only under the `Testing` configuration.

## Build System

**Requirements**: CMake 3.20+, Visual Studio 2017+ (Windows only), VS extension "Test Adapter for Catch2" (disable other test adapters).

All dependencies are managed via CMake FetchContent — no manual installs needed.

```bash
# Configure and build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release

# Run the game (working directory must be the Data folder)
bin/Release/SandboxApp.exe

# Run tests (only built in Testing configuration)
cmake --build . --config Testing
bin/Testing/UT.exe
```

Three configurations exist: `Debug`, `Release`, `Testing`. The `UT` target is excluded from Debug/Release builds — you must use the `Testing` config explicitly.

## Detailed Documentation

Before working on any task, read the relevant files below:

| File | When to read |
|---|---|
| `.claude/docs/architecture.md` | Always — architectural rules: ECS contract, engine/game split, platform abstractions |
| `Documentation/PigeonArchitecture.info` | Always — PigeonLib module flow, boot sequence, and key files |
| `Documentation/AppArchitecture.info` | When working on SandboxApp — application boot sequence and system data flow |
| `CONTEXT.md` | Always — domain glossary (create lazily via grill-with-docs if absent) |
| `.claude/docs/coding-guidelines.md` | When implementing systems, components, or events |
| `.claude/docs/testing-guidelines.md` | When writing or modifying tests |
| `Documentation/ModuleInfo/<Module>/<SystemName>System.info` | Before touching any ECS system in that module |
| `Documentation/ModuleInfo/<Module>/<Name>.info` | Before touching infrastructure in ECS, Events, Platform, or ImGui |

The `.info` files are the **source of truth** for each system's behavior and component interface. Read the relevant system file and the architecture flow file before starting work. When code changes, update the matching `.info` file.

