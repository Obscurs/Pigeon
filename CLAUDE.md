# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Rules

- Never modify `CLAUDE.md` or `.claude/settings.local.json` without explicit user approval.
- When resuming interrupted agent work, always re-spawn the appropriate agent. Never perform agent-owned tasks directly.

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
| `.claude/docs/architecture.md` | Always — ECS contract, project split, module list, key files |
| `.claude/docs/coding-guidelines.md` | When implementing systems, components, or events |
| `.claude/docs/testing-guidelines.md` | When writing or modifying tests |
| `Documentation/docs/diagrams/<ModuleName>.info` | Before touching any system in that module |

The diagram `.info` files are the **source of truth** for implementation and testing within each module. Always read both before starting work on a module.

