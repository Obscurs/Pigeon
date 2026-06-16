# ADR 0009 — In-engine text generation (llama.cpp)

**Status:** Accepted — the **CPU-only** inference decision is superseded by ADR 0010 (GPU offload is now
the default; everything else here still stands)
**Date:** 2026-06-14

## Context

We want a running game to generate text at runtime from a prompt — a local Large Language Model
driven entirely **in-process**, with no external backend server (no Ollama, no llama-server, no
out-of-process worker). The provided asset is a GGUF model (`L3-Dark-Planet-8B-max-D_AU-Q8_0.gguf`,
a Llama-3 8B at Q8_0), and GGUF is the native model format of **llama.cpp** (ggml).

This is the same shape of problem ADR 0008 solved for text-to-image: PigeonLib is a pure DirectX11
rasterization engine with no tensor/GPGPU infrastructure, so neural-net inference has to come from an
embedded library rather than hand-written HLSL. The architectural constraints are identical:

- **One writer per component**, execution order derived only from declared access.
- `World::Update` runs every system every frame and ends with a buffer swap; an LLM completion takes
  **seconds** and cannot block the frame loop.
- The Diffusion module (ADR 0008) already established the in-engine inference pattern —
  Resource → System → Request → background worker → result — which this feature mirrors closely.

## Decision

Embed an LLM **inference runtime as a library** and drive it from a background-threaded ECS subsystem,
mirroring the Diffusion module (ADR 0008).

- **Runtime: `llama.cpp` (ggml), CPU backend.** Linked into PigeonLib; no Python, no server. It loads
  `.gguf` models at runtime and runs the full prompt → completion loop in-process.
- **CPU-only inference (no CUDA).** The bundled SandboxApp already loads an SDXL Checkpoint resident in
  VRAM (~7 GB+) for Diffusion; an 8B Q8 model (~9.5 GB) cannot co-reside on one consumer GPU without
  OOM. Running the LLM on the CPU (RAM) keeps it off the contended GPU — slower (a few tokens/sec) but
  robust and always available. GPU offload is a possible later config field, not the default.
- **Model is startup-resident, one per session.** A new `languageModels` Resource Manifest category
  declares the GGUF; `ResourceManagerSystem` records its resolved path (the backend loads the weights
  itself, exactly as it records Checkpoint/LoRA paths). `TextGenSystem` loads the first declared model
  once at startup, resident for the session — switching models is a relaunch.
- **`TextGenSystem` + background worker.** The app emits an engine-typed `GenerateTextRequest`
  one-frame component (the established request pattern — cf. camera/audio/window/diffusion) carrying the
  prompt, an optional system prompt, the Generation Config overrides, and a caller-assigned target text
  UUID (like an audio Voice Handle / the diffusion target texture UUID). `TextGenSystem` hands the job
  to a worker thread and polls it; the frame loop never blocks. One generation runs at a time;
  overlapping requests are dropped while a job is in flight.
- **Whole-result delivery, no streaming.** The worker produces the **full completion**, then publishes
  it at once. The Text Gen Job exposes only a coarse pending/running → done/failed state — the simplest
  UX and the simplest mock, matching Diffusion's "no progress surface". Token-by-token streaming is out
  of scope.
- **Generated text is published into an engine-owned result store, not the resource map.** Unlike a
  generated texture — which *must* route through `ResourceMapSingletonComponent` because the renderer
  resolves textures only from there under the one-writer rule (ADR 0007/0008) — generated text is not a
  renderer resource. `TextGenSystem` is therefore the **sole writer** of a new
  `TextGenResultSingletonComponent` (target UUID → completion string) and writes the result directly
  when a job finishes. No second system and no register-request hop are needed; one-writer is preserved.
- **Generation defaults in `Config.json`.** Default max tokens / temperature / top-p seed
  `EngineConfigSingletonComponent` (as the window/audio/camera/diffusion defaults already do); each
  request overrides any subset. Prompt / system prompt / seed are always per-request.

## Trade-offs

- **Embed `llama.cpp` (chosen) vs. an external server / Ollama (rejected):** a server is "no in-process
  inference" but adds a process to manage, an IPC/HTTP layer, and a deployment dependency. llama.cpp is
  a C/C++ library that loads a raw GGUF and maps onto the existing Resource → System → Request pattern,
  exactly like sd.cpp for Diffusion.
- **CPU inference (chosen) vs. CUDA offload (rejected as default):** GPU is far faster, but the 8B Q8
  model and the resident SDXL Checkpoint cannot both fit on one consumer GPU. CPU inference trades speed
  for never contending with the renderer/diffusion for VRAM, and keeps the build CUDA-free.
- **Engine-owned result singleton (chosen) vs. routing text through the resource map like a generated
  texture (rejected):** text is not sampled by the renderer, so forcing it through `m_TextureMap`'s
  one-writer funnel would need a bespoke string store on the resource map and a register-request hop for
  no benefit. A dedicated result singleton owned by `TextGenSystem` is simpler and still one-writer.
- **Whole-result (chosen) vs. token streaming (rejected):** streaming is the nicer LLM UX but needs a
  mutex-guarded partial buffer the worker appends to and the main thread samples each frame, plus a mock
  that fakes incremental output. Whole-result keeps parity with Diffusion's coarse-state job and stays
  deterministically testable.

## Consequences

- New build dependency: `llama.cpp` (+ ggml) linked into PigeonLib behind an opt-in
  `-DPG_ENABLE_TEXTGEN` flag (defines `PG_LLAMA_ENABLED`); otherwise `LlamaCppBackend` is an inert no-op
  so a plain build links without the runtime. The Testing backend mocks the runtime so generation is
  verified by data-flow tests, not real inference (cf. the audio / diffusion / render-target mocks).
- `ResourceMapSingletonComponent` gains an `m_LanguageModelMap` (UUID → path); the Resource Manifest
  gains a `languageModels` category loaded from each project's `TextGeneration` folder;
  `ResourceManagerSystem` records the path (it does not parse the weights).
- `EngineConfigSingletonComponent` gains the text-generation defaults (max tokens / temperature /
  top-p), seeded from `Assets/Engine/Config.json`.
- New engine module: `TextGenSystem` (owns the worker thread + Text Gen Job state + the result store),
  the `GenerateTextRequest` component, the `TextGenBackend` abstraction (+ `LlamaCppBackend` real /
  `TestingTextGenBackend` mock), and the result singleton. Emitting requests and displaying the result
  live in SandboxApp (an ImGui demo panel), honouring the engine/game split.
- On the CPU, a completion takes seconds and the model occupies several GB of RAM for the session;
  accepted as the cost of local, in-process, server-free generation.
- **Coexisting with the diffusion backend (ADR 0008) requires ggml ISOLATION, because both `llama.cpp`
  and `stable-diffusion.cpp` vendor `ggml`** under the same `ggml` CMake target and pin **incompatible**
  ggml revisions. They cannot share one ggml: each guards with `if(NOT TARGET ggml)` so the first added
  owns it and the other reuses it, but a shared ggml that satisfies one *compiles* the other and then
  **crashes sd.cpp's model manager at generate time** (`model manager tensor ... is not registered` ->
  `clip prepare graph weights failed` -> `GGML_ASSERT(!chunk_hidden_states.empty())`). The fix is to keep
  them in **separate ggml scopes**:
  - `stable-diffusion.cpp` is built in-tree (`FetchContent`) with its own ggml (CUDA). It is pinned to
    `276025e` / **#1650** — the bundled SDXL checkpoint + LoRA + OpenPose ControlNet assets load and
    generate against this commit (verified). The 06-13/14 churn breaks it: pre-#1638 (and #1638) reject
    this ControlNet's proj-weight shape at load, #1644+'s "model manager" refactor is what crashes under
    a foreign ggml, and a few commits later the `vae_decode_only`/`keep_vae_on_cpu` fields the backend
    uses were removed.
  - `llama.cpp` is built in a separate **`ExternalProject`** (its own ggml, CPU, `GGML_CUDA=OFF`,
    `GGML_OPENMP=OFF`, static), installed to a prefix, and linked into PigeonLib by path. Pinned to
    `6e14286`. Because it builds standalone, `LLAMA_BUILD_APP/COMMON/HTML/UI` must be forced OFF (they
    default ON via `LLAMA_STANDALONE` and pull in a generated `build-info.h` / web UI we don't build).
  - `BUILD_SHARED_LIBS` is forced **OFF** globally (llama.cpp defaults it ON, which would scatter
    ggml/spdlog/glm as DLLs the exe can't find) so everything links statically into one self-contained
    exe. Only the CUDA runtime (`cudart`/`cublas`, from the toolkit on `PATH`) stays dynamic.
  - **Verified end-to-end:** in one build, the SDXL checkpoint + ControlNet load to VRAM, the 8B GGUF
    loads on the CPU, and image generation completes (768×768) without the model-manager crash.
