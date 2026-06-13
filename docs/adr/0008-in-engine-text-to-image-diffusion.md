# ADR 0008 — In-engine text-to-image diffusion

**Status:** Accepted
**Date:** 2026-06-12

## Context

We want a running game to generate images at runtime from a text prompt — conditioned by a
Checkpoint, a per-job LoRA set, a single ControlNet (notably OpenPose), and an optional Input Image
(img2img blending) — entirely **in-process**, with no external backend server (no ComfyUI/A1111,
no out-of-process worker).

This is a poor fit for the engine as it stands. PigeonLib is a pure DirectX11 **rasterization**
engine: shaders are vertex + pixel only (`Dx11Shader` exposes no compute stage), and there is no
GPGPU/tensor infrastructure anywhere. A Stable Diffusion pipeline is the opposite shape — a CLIP text
encoder, an N-step U-Net denoiser, a VAE decoder, plus side-networks — i.e. neural-net **compute**,
not draw calls. Hand-writing that in HLSL compute is a multi-year effort and is exactly what existing
frameworks exist to avoid.

Hard constraints from the existing architecture:

- **One writer per component**, execution order derived only from declared access.
- The renderer resolves textures **only** through `ResourceMapSingletonComponent::m_TextureMap` by
  UUID, and that component has a single writer (`ResourceManagerSystem`). Any generated image must
  reach consumers as an ordinary entry in that map (the same constraint ADR 0007 faced for the 3D
  render target).
- `World::Update` runs every system every frame and ends with a buffer swap; a single generation takes
  **seconds to tens of seconds** on a consumer NVIDIA card and cannot block the frame loop.
- The target machine is a single NVIDIA GPU shared between the game's own VRAM and the diffusion model
  (~2 GB resident for an SD 1.5 Checkpoint).

## Decision

Embed a diffusion **inference runtime as a library** and drive it from a background-threaded ECS
subsystem whose output is an ordinary resource-map texture.

- **Runtime: `stable-diffusion.cpp` (ggml), CUDA backend.** Linked into PigeonLib; no Python, no
  server. It loads `.safetensors`/`.ckpt` Checkpoints **at runtime**, applies LoRAs per generation
  from the prompt, supports img2img, and takes a prepared ControlNet hint image — covering the whole
  requirement set natively. Built with `GGML_CUDA=ON` for the NVIDIA target.
- **Model family is a config field; the engine is family-agnostic.** SD 1.5 is the lightest target
  (co-resides in VRAM with a running game on one consumer GPU, richest OpenPose ControlNet ecosystem),
  but the **bundled SandboxApp demo targets SDXL** (Illustrious) per the provided assets, accepting the
  higher VRAM (~7 GB resident + ~4 GB working). LoRAs/ControlNets are family-specific and only coherent
  within one family.
- **Checkpoint + ControlNet are startup-resident; LoRAs are per-request.** New Resource Manifest
  categories declare them; `ResourceManagerSystem` loads the Checkpoint and ControlNet into VRAM once
  at startup, resident for the session — **one Checkpoint per session** (switching = relaunch). LoRAs
  are declared assets applied on top of the resident Checkpoint **per Generate Image Request** (with
  weights), so the LoRA set varies per job without reloading the Checkpoint.
- **`DiffusionSystem` + background worker.** The app emits an engine-typed `GenerateImageRequest`
  one-frame component (the established request pattern — cf. camera/audio/window/savedata) carrying the
  Generation Config, a caller-assigned target UUID (like an audio Voice Handle), the LoRA set, an
  optional Input Image + Denoise Strength, and an optional single ControlNet hint + strength.
  `DiffusionSystem` hands the job to a worker thread and polls it; the frame loop never blocks. It owns
  the Diffusion Job state and never writes the resource map.
- **Single ControlNet per job.** sd.cpp does not robustly stack multiple ControlNets; one ControlNet
  (+ optional img2img init + multi-LoRA) is the per-job envelope. Stacking multiple ControlNets is out
  of scope.
- **OpenPose hint is rasterized in-engine from a loadable skeleton.** An OpenPose Skeleton resource
  holds a pregenerated, fixed pose as **COCO-18** keypoints (joint positions only — not an image). The
  engine owns the OpenPose colour/limb convention and rasterizes the canonical Control Hint image,
  applying a rigid whole-skeleton Skeleton Transform driven by a **world entity's Transform projected
  through the gameplay orthographic camera** (so the generated pose aligns with where the entity
  appears on screen). The engine never detects pose from a photo — no detector model is embedded.
- **Generated image is a first-class resource-map texture.** When a Diffusion Job completes (worker
  returns a CPU RGB buffer), `DiffusionSystem` emits a `RegisterGeneratedTextureRequest`;
  `ResourceManagerSystem` — still the sole writer of the resource map — drains it each frame and
  inserts a `Texture2D` into `m_TextureMap` under the caller's UUID. The image is then sampled by
  Sprites/UI/quads exactly like a loaded texture; the UUID resolves to the default texture until done.
- **Generation defaults in `Config.json`.** Default steps/sampler/CFG/resolution/clip-skip seed
  `EngineConfigSingletonComponent` (as window/audio/camera defaults already do); each request overrides
  any subset. Prompt/negative/seed are always per-request.
- **No progress surface.** The Diffusion Job exposes only a coarse pending/running → done/failed state
  (not a per-step fraction): the simplest UX, and intermediate previews would cost extra VAE decodes
  that worsen GPU contention.

## Trade-offs

- **Embed `stable-diffusion.cpp` (chosen) vs. embed Python/PyTorch in-process (rejected):** the latter
  is technically "no server" but drags a 4–6 GB CPython + Torch + CUDA tree into the process, with the
  GIL, version/packaging hell, and opaque cross-FFI crashes. sd.cpp is a C/C++ library that loads raw
  checkpoints and maps onto the existing Resource → System → Request pattern.
- **`stable-diffusion.cpp` (chosen) vs. ONNX Runtime + DirectML (rejected):** ONNX requires every model
  pre-compiled to a static graph (Olive), which breaks "accept arbitrary Checkpoints/LoRAs dropped in a
  folder." sd.cpp loads `.safetensors` at runtime — the actual requirement.
- **SD 1.5 (chosen) vs. SDXL/SD3/Flux (rejected as default):** larger families (7–24 GB) cannot
  co-reside with a running game on one consumer GPU. SD 1.5 leaves headroom; the family is a config
  field so a bigger model is a later config change, not a rewrite.
- **Startup-resident single Checkpoint (chosen) vs. fully dynamic per-job load (rejected):** dynamic
  load/unload gives maximum flexibility but pays a multi-second load on every generation and needs a
  VRAM budget manager. Keeping LoRAs per-request preserves the switching that mattered while leaving the
  expensive Checkpoint resident; switching Checkpoints (rare) is a relaunch.
- **Generated image via the resource map (chosen) vs. a separate `DiffusionResult` singleton
  (rejected):** the separate store would force every texture consumer (sprites, UI, renderer) to learn
  a second lookup path. Routing through `m_TextureMap` keeps generated images "just textures" and
  preserves one-writer — the same resolution ADR 0007 reached for the 3D render target.
- **Engine-rasterized OpenPose hint from a keypoint resource (chosen) vs. embedding a pose detector
  (rejected) vs. a pre-rendered hint PNG (rejected):** a detector adds a second inference runtime + model
  just to feed the first; a baked PNG puts the OpenPose convention in the asset and loses crisp scaling.
  Rasterizing from COCO-18 keypoints keeps the convention in the engine and the pose authorable, and a
  world-driven Skeleton Transform makes the control track an on-screen entity.

## Consequences

- New build dependency: `stable-diffusion.cpp` (+ ggml/CUDA) linked into PigeonLib; the Testing backend
  must mock the runtime so generation is verified by data-flow tests, not real inference (cf. the audio
  and render-target mocks). GPU inference work runs only in real builds.
- `ResourceMapSingletonComponent` gains Checkpoint/LoRA/ControlNet/OpenPose-Skeleton stores; the
  Resource Manifest gains matching categories; `ResourceManagerSystem` gains a per-frame
  `RegisterGeneratedTextureRequest` drain (it no longer early-outs after frame 1) and remains the sole
  writer of the resource map.
- `EngineConfigSingletonComponent` gains the diffusion model UUIDs + Generation Config defaults (seeded
  from `Assets/Engine/Config.json`).
- New engine module: `DiffusionSystem` (owns the worker thread + Diffusion Job state), the
  `GenerateImageRequest` / `RegisterGeneratedTextureRequest` components, the OpenPose Skeleton resource
  + COCO-18 hint rasterizer, and the `GeneratedTexture` flow. Skeleton authoring (filling keypoints
  from a character) and emitting requests live in SandboxApp, honouring the engine/game split.
- On a single GPU the game's framerate dips while a job runs (sd.cpp CUDA work shares the card with the
  DX11 present); accepted, as no single-GPU remedy exists.
- ~2 GB of VRAM is permanently committed once the Checkpoint loads at startup, reducing the budget
  available to game assets for the whole session.
