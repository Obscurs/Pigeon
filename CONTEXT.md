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
| World Space | The gameplay coordinate space, **Y-up**: +Y is up on screen, +X is right. Positions, movement, and the camera are all authored this way. The renderer maps Y-up world space to the screen directly (no vertex negation); DirectX's downward texture V is compensated per world draw (texture-V flip for quads/sprites, anchor reflection for text). Distinct from the **UI canvas**, which is y-down (y=0 at the top-left). |
| World Transform | The resolved 4×4 matrix composed from Position, Rotation, and Scale. The single value rendering reads to place geometry. |
| Transform Request | A request to change an entity's transform. App movers each emit their own request type; these are aggregated into one canonical resolved request that the engine applies. A request is absolute (it sets values, it does not accumulate deltas). |
| Transform Resolve | Applying transform requests to the canonical Position/Rotation/Scale/Local Bounds components. Two-tier: an app aggregator collapses all app requests into one resolved request, and an engine resolver applies the resolved request (plus engine-origin requests) — it is the sole adder and writer of the canonical components. |
| Transform Compose | Building the World Transform from Position/Rotation/Scale, and the render sort key from the World Transform and Local Bounds. |
| Render order (Y-sort) | World draw order derived from the world-space Y of an entity's bottom edge (Local Bounds min, transformed to world). World space is Y-up, so **higher Y (further up / further back) draws behind, lower Y (nearer the camera bottom) draws in front**. UI always draws on top of all world geometry. |

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

## Camera

| Term | Meaning |
|---|---|
| Set Camera Request | An engine-typed one-frame request (`SetCameraRequestOneFrameComponent`) carrying a new camera zoom level + world position. Camera input detection is an **app** responsibility: the app reads input and the camera's current zoom/position, computes the new absolute values, and emits this request; the engine `CameraSystem` applies the zoom to the camera, pans its position, and rebuilds the projection. Like the audio/window/savedata requests, it must be an engine (`pg`) type so the engine system can read it. |

## Save Data

| Term | Meaning |
|---|---|
| Save Slot | One persisted JSON document, identified by a UUID. Stored on disk as `<savedataPath>/SaveSlots/<uuid>.json` (the filename stem is the slot UUID) and held live in the `SaveDataSingletonComponent` map (UUID → JSON). |
| Save Data Resource | The live in-memory map of all save slots (`SaveDataSingletonComponent`), loaded once at startup from the `SaveSlots` folder. The single value the App reads to display or read back slot contents. Owned (added + written) solely by `SaveDataSystem`. |
| Set Save Data Request | An engine-typed one-frame request (`SetSaveDataRequestOneFrameComponent`) carrying a slot UUID + new JSON. The App emits it; the engine `SaveDataSystem` updates both the loaded resource (the map entry) and the data resource (the `<uuid>.json` file). Like the window/audio requests, it must be an engine (`pg`) type so the engine system can read it. |

## Resources

| Term | Meaning |
|---|---|
| Resource Map | The single live store of all loaded assets (`ResourceMapSingletonComponent`): textures, shaders, fonts, UI layouts, sound clips, JSON assets, and 3D models, each keyed by UUID. Loaded once at startup by `ResourceManagerSystem` from the engine and app resource manifests. |
| Resource Manifest | The per-project JSON file (`Assets/<project>/ResourcesManifest.json`) that declares every asset to load, grouped by category array (`textures`, `fonts`, `shaders`, `ui`, `sounds`, `json`, `models`). Each entry is an `{id, path}` pair; the path is resolved under the category's folder (`Assets/<project>/<Folder>/<path>`). |
| JSON Asset | A loadable JSON data asset, declared in the `json` array of a resource manifest and loaded from the project's `JSON` folder (`Assets/<project>/JSON/<path>`). Parsed at load time and held live in the resource map's `m_JSONMap` (UUID → parsed `nlohmann::json`). The engine and app declare these the same way as any other resource; the parsed content is the value other systems read. |
| 3D Model | A loadable 3D mesh resource (`pg::Model`), declared in the `models` array of a resource manifest and loaded from the project's `Models` folder (`Assets/<project>/Models/<path>`). Parsed at load time from a Wavefront `.obj` into CPU-side triangle geometry — an interleaved `pg::ModelVertex` list (position/normal/texcoord) plus a triangle index list, with polygon faces fan-triangulated and identical `v/vt/vn` corners deduplicated into shared vertices. Held live in the resource map's `m_ModelMap` (UUID → `pg::Model`); a future 3D render pass uploads these buffers to the GPU. |
| Model Component | An entity's reference to a loaded 3D Model by UUID (`pg::ModelComponent`), placing it in a world-space scene via the Transform components exactly as a Sprite references a texture. Authored by the app; the forthcoming 3D render pass resolves `m_ModelID` against the resource map's model map to draw the geometry. |

## Sprites & Animation

| Term | Meaning |
|---|---|
| Sprite | A world-space quad textured with a rectangular region (sub-texture) of a single texture, placed by the Transform components. The sub-texture is selected by a UV rectangle; a full-texture sprite simply uses the whole [0,1] rectangle. |
| Sprite Sheet | A uniform grid that subdivides one texture into `columns × rows` equal Cells (`pg::SpriteSheet`). It maps a Cell to its normalized UV rectangle, with the top-left Cell at column 0 / row 0. Pixel-agnostic — Cells are fractions of the texture, so the same sheet works at any texture resolution. The reusable engine primitive underpinning Sprite Animation. |
| Cell / Frame | One grid square of a Sprite Sheet, addressed by `(column, row)`. In an animation a single row is one clip and each column within it is a successive Frame. |
| Animation Row | One row of a Sprite Sheet treated as a single animation clip; its columns are the clip's frames played in order. Different rows hold different animations (e.g. one row per facing direction). |
| Sprite Animation | Advancing a Sprite's sub-texture through the Frames of an Animation Row over time: a frame is shown for a fixed Frame Duration, then the active column steps to the next Frame, wrapping at the end of the row. Playing can be paused (the Sprite freezes on its idle Frame). Engine-owned: the engine holds the playback state (`SpriteAnimationComponent`) and steps it (`SpriteAnimationSystem`); the App only authors the configuration (sheet, Frame Duration/speed, starting row) and changes the active animation at runtime via a Set Sprite Animation Request. |
| Set Sprite Animation Request | An engine-typed one-frame request (`SetSpriteAnimationRequestOneFrameComponent`) carrying an optional target Animation Row and a playing flag. The App emits it; the engine `SpriteAnimationSystem` applies it. Like the camera/audio/window requests, it must be an engine (`pg`) type so the engine system can read it. |
| Frame Duration | How long (seconds) each Frame of a Sprite Animation is displayed before advancing to the next; the inverse of the animation's frames-per-second. The App sets it as the animation's speed. |
| Character (showcase) | The arrow-key-controlled demo entity in SandboxApp. Held arrow keys drive its Transform movement and select the Animation Row matching its facing direction; releasing the keys stops playback so it rests on an idle Frame facing its last direction. |

## UI

| Term | Meaning |
|---|---|
| UI Element | One node in the UI tree: a `BaseComponent` (its rect within its parent) optionally paired with an `ImageComponent` or `TextComponent`. Elements form a parent/child hierarchy; children resolve their rect relative to their parent's rect, and the root's parent is the UI Canvas. |
| UI Canvas | The virtual space all UI is authored in. Its size is the **Reference Resolution**; UI coordinates are always expressed against the canvas, never against the live window. Distinct from the gameplay world space and from the OS Window Resolution. |
| Reference Resolution | The fixed canvas size a layout is designed against (e.g. 1920×1080). Authoring is resolution-agnostic because every coordinate is in reference units; the live window may be any size. |
| UI Scale Factor | The single uniform multiplier mapping reference units to live window pixels, derived from window-vs-reference by a **Match Factor** (0 = match width, 1 = match height, blended in between, Unity-style). Scales absolute sizes and offsets; relative (anchor-driven) sizing needs no scaling. |
| Anchor Rect | An element's `anchorMin`/`anchorMax` — two normalized points (0..1) in the parent rect that the element's edges are pinned to. `min == max` gives a point anchor (the Size is the literal pixel size); `min != max` stretches the element with the parent (the Size becomes a delta added to the stretched span — fill-with-margins). Replaces the former discrete 3×3 align enum. |
| Pivot | An element's own normalized reference point (0..1); the Anchored Position places this point, and scaling/rotation pivot about it. |
| Anchored Position | The pixel offset (reference units) of the element's Pivot from the anchor reference. With a point anchor this is simply where the pivot sits relative to the anchor. The position complement to Size. |
| Size (UI) | The element's pixel dimensions when point-anchored (`anchorMin == anchorMax`); when stretched it is the delta added to the anchor-spanned size (`0` = exactly fills the span). The size complement to Anchored Position. |
| Element Rect | An element's resolved screen rect (top-left origin + size) in logical-canvas units, computed from its parent rect and its Anchor Rect / Pivot / Anchored Position / Size. The single geometry that UI rendering and hit-testing both read; the root element's parent rect is the logical canvas. |
| Text Alignment | How a TextComponent positions its glyph block within its Element Rect (horizontal left/center/right × vertical top/center/bottom), independent of the rect's own anchors. |
| Auto-fit Text | The default text sizing (TextComponent fixed font size = 0): the whole string is uniformly scaled so it fills the Element Rect. Suited to labels (button captions) where the text should grow/shrink with its box. |
| Fixed-size Text | Body/prose text sizing (TextComponent fixed font size > 0): glyphs render at an exact em height in logical-canvas units regardless of string length, so dialogue and descriptions stay a consistent, readable size. The complement to Auto-fit Text. |
| Word Wrap | A fixed-size text option that inserts soft line breaks at word boundaries (spaces → newlines) so the glyph block stays within the Element Rect width instead of overflowing or being shrunk. Implemented by `Font::WrapString`, which preserves string length (only spaces become newlines) and never breaks mid-word. |
| Typewriter Reveal | Progressive text disclosure for visual-novel-style dialogue: TextComponent's visible-character count (-1 = all) limits how many leading source characters are drawn, while layout is computed on the full string so shown glyphs never reflow. The app advances the count over time (engine renders, app drives the timing) via `UIUpdateTextRevealOneFrameComponent`, which also carries the new line's text (when non-empty) so a fresh dialogue line and its reveal arrive in one command. |
| UI Pass | The dedicated screen-space render pass that draws all UI **after** the world, through a UI-only orthographic projection spanning the reference canvas — never through the pannable/zoomable gameplay camera. |
| Input Capture | Pointer hit resolution that selects the single front-most UI Element under the cursor and consumes the click there, so it does not leak to elements or gameplay behind it. UI interaction is pointer-only (no keyboard/gamepad focus navigation). |
| Layout Container | A parent element carrying a layout that positions its children automatically — a vertical or horizontal stack (children placed along the axis, stretched to fill the cross-axis), or a grid (fixed cells wrapping after N columns) — with padding and spacing, instead of each child placing itself by its Anchor Rect. A container is itself placed by its own anchors; only its direct children are auto-laid-out. |
| Sibling Index | An element's deterministic order among its parent's children (captured from the layout authoring order). Drives placement order inside a Layout Container; ignored when the parent is not a container. |
| Clip Rect | The scissor rectangle a clip element imposes so descendant draws outside it are masked, enabling scroll views and masked panels. An element's effective clip rect is the intersection of all its ancestor clip rects; the renderer applies it as a window-pixel scissor in the UI pass. |
| Scroll Offset | A clip element's pixel translation applied to its clipped content (descendants), turning a clip into a scroll view; an app drives scrolling by changing it. |
| Nine-Slice | An image draw mode (enabled by a non-zero texture-pixel border on the image) that splits the texture into a 3×3 grid so corners keep their size while edges and the center stretch, letting panels and buttons resize without distortion. The renderer draws it as nine sub-quads, each sampling its own UV sub-rect. |
