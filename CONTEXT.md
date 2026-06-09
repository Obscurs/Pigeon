# Pigeon — Domain Glossary

## Namespaces

| Identifier | Meaning |
|---|---|
| `pg` | The primary C++ namespace for all PigeonLib engine code (headers, implementations, platform layers). Previously `pig`. |
| `pg::ui` | Sub-namespace for the UI module within PigeonLib. |
| `pg::ecs` | Sub-namespace for the ECS abstraction layer. Holds the engine's own names for the underlying ECS library's public types (`Entity`, `null`, `Registry`, `Dispatcher`, `exclude`). Engine and game code use these names exclusively; the third-party `entt::` types are named only inside the ECS abstraction header. |
| `sbx` | The C++ namespace for all SandboxApp game code. |

## Transform

| Term | Meaning |
|---|---|
| Transform | The world-space placement of a non-UI entity, decomposed into four authoring components: Position, Rotation, Scale, and Local Bounds. UI elements are **not** transforms — they live in screen space and are placed by the UI module. |
| Position | An entity's world-space translation (3D). |
| Rotation | An entity's world-space orientation, stored as a quaternion (full 3D). |
| Scale | An entity's world-space scale (3D). |
| Local Bounds | An entity's axis-aligned extent in its own local space (min/max corners). Defines the geometry a quad/sprite occupies and feeds the render order. |
| World Transform | The resolved 4×4 matrix composed from Position, Rotation, and Scale. The single value rendering reads to place geometry. |
| Transform Request | A request to change an entity's transform. App movers each emit their own request type; these are aggregated into one canonical resolved request that the engine applies. A request is absolute (it sets values, it does not accumulate deltas). |
| Transform Resolve | Applying transform requests to the canonical Position/Rotation/Scale/Local Bounds components. Two-tier: an app aggregator collapses all app requests into one resolved request, and an engine resolver applies the resolved request (plus engine-origin requests) — it is the sole adder and writer of the canonical components. |
| Transform Compose | Building the World Transform from Position/Rotation/Scale, and the render sort key from the World Transform and Local Bounds. |
| Render order (Y-sort) | World draw order derived from the world-space Y of an entity's bottom edge (Local Bounds min, transformed to world): lower Y draws behind, higher Y draws in front. UI always draws on top of all world geometry. |

## Audio

| Term | Meaning |
|---|---|
| Sound Clip | A loadable audio resource (`pg::SoundClip`). Holds the resolved file path of an audio asset; loaded by the resource manager into the resource map alongside textures, fonts, shaders, and UI layouts. The device decodes it lazily on play. |
| Audio Device | The platform-abstracted mixer/output (`pg::AudioDevice`). Owns the active voices and is the only platform-specific audio piece; the miniaudio backend serves real builds, a no-op mock serves tests. |
| Voice | One playing instance of a sound clip. Many voices play simultaneously, including multiple voices of the same clip. |
| Voice Handle | A caller-assigned UUID identifying a voice. The app passes it in the play request and reuses it to pause, resume, or stop that specific voice. Music is a looping voice whose handle the app remembers. |
| Audio Category | Whether a voice is a Sound (effect) or Music. Selects which category volume scales the voice. |
| Volume (master / sound / music) | Mix levels seeded from `Config.json` into the engine config, then held live in `AudioVolumeSingletonComponent`. A voice's effective gain is master × category volume × the per-voice base volume. |

## Window

| Term | Meaning |
|---|---|
| Window Resolution | The client-area width/height of the OS window. Declared in `Config.json` (engine default) and overridable in the savedata `Config.json`, seeded into `EngineConfigSingletonComponent`, then held live in `WindowConfigSingletonComponent`. |
| Window Mode | Whether the window is Windowed or Fullscreen (`pg::EWindowMode`). Fullscreen is **borderless windowed** — a borderless window sized to the primary monitor, not exclusive DXGI fullscreen. |
| Window Config | The live window resolution + mode (`WindowConfigSingletonComponent`), seeded once from the engine config and updated by resolution requests. |
| Set Window Resolution Request | An engine-typed one-frame request (`SetWindowResolutionRequestOneFrameComponent`) carrying a desired resolution + mode. The app emits it; the engine `WindowConfigSystem` applies it to the live window and persists it to the savedata `Config.json`. Like the audio request, it must be an engine (`pg`) type so the engine system can read it. |
| Savedata Path | The directory (relative to the working dir) holding the override `Config.json`; declared by the `savedataPath` key in the engine config and recorded in `EngineConfigSingletonComponent.m_SavedataPath` so the runtime knows where to persist changes. |
