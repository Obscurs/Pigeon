# ADR 0010 — Text generation runs on the GPU (configurable offload)

**Status:** Accepted — supersedes the CPU-only decision of ADR 0009
**Date:** 2026-06-16

## Context

ADR 0009 deliberately ran the in-engine LLM on the **CPU** (`n_gpu_layers = 0`, llama.cpp's isolated
ggml built `GGML_CUDA=OFF`) so it never contended for VRAM with the renderer and the resident SDXL
Diffusion checkpoint (~7 GB + ~4 GB working). That rationale assumed the bundled 8B **Q8** model
(~9.5 GB), which cannot co-reside with SDXL on one consumer GPU.

The SandboxApp demo now targets a lighter quantization (8B **Q4_K_S**, ~4.7 GB), which *does* leave
room to co-reside with the resident checkpoint. CPU inference is the bottleneck for that model (a few
tokens/sec); we want the GPU's throughput now that the VRAM budget allows it.

## Decision

Run text generation on the GPU, with the offload depth a Generation-Config field.

- **`n_gpu_layers` becomes a config field, not a constant.** A new `textGenGpuLayers` key seeds
  `EngineConfigSingletonComponent.m_TextGenGpuLayers` (default **999** = offload all layers), exactly
  like the other generation defaults. `TextGenSystem` reads it and passes it to the backend's
  `LoadModel(path, gpuLayers)`; `LlamaCppBackend` sets `modelParams.n_gpu_layers` from it. This is the
  "later config field" ADR 0009 explicitly anticipated.
- **GPU by default; CPU still reachable.** The default offloads every layer. Setting
  `textGenGpuLayers` to `0` restores the old CPU-only behaviour with no code change — useful when a
  heavier model is swapped back in and the GPU budget no longer fits both features.
- **llama.cpp's isolated ggml is built `GGML_CUDA=ON`.** Without a CUDA-compiled ggml `n_gpu_layers`
  is inert, so the isolated ExternalProject (root `CMakeLists.txt`) now builds CUDA, and PigeonLib
  links the additional installed `ggml-cuda` static lib. The ggml-isolation contract of ADR 0009 is
  unchanged: sd.cpp's ggml and llama.cpp's ggml remain **separate build scopes** (in-tree FetchContent
  vs. isolated ExternalProject) — they are now both CUDA, but still never share one ggml.

## Consequences

- **VRAM is now shared between the renderer, the Diffusion checkpoint, and the language model.** This
  is only safe with a light enough model; sizing the model to fit is an **app/asset responsibility**
  (the manifest's `languageModels` entry), not an engine concern. Too large a model OOMs at load — set
  `textGenGpuLayers` to `0` (CPU) or pick a smaller quantization.
- **The text-generation build now requires the CUDA Toolkit** (as the Diffusion build already does).
  `ggml-cuda` needs the CUDA runtime (`cudart`/`cublas`) at link time; in the normal build both
  backends are enabled and the Diffusion side already pulls the runtime in. The Testing/UT build leaves
  both backends `OFF` and uses the deterministic mock, so it neither compiles CUDA nor runs real
  inference — the GPU offload is verified by a real build, the **data-flow** (config → `LoadModel`) by
  unit tests.
- On a single GPU the game's framerate dips harder while a generation runs, since the LLM now shares
  the card with the renderer and any concurrent Diffusion job; accepted as the cost of GPU speed.
