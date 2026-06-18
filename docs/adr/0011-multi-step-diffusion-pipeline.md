# ADR 0011 — Multi-step diffusion pipelines (generated image as input + on-demand OpenPose hint texture)

**Status:** Accepted
**Date:** 2026-06-17

## Context

ADR 0008 established single-shot text-to-image generation: one `GenerateImageRequest` →
one Diffusion Job → one Generated Texture. The SandboxApp demo used a single request that
generated a posed character on a flat key colour and chroma-keyed it over a fixed living-room
photo. In practice that one-shot composite is unreliable: the character generation invents its
own background (which overlaps the intended one), or the chroma key bleeds the background into
the figure.

The fix is to break the work into **separate, individually-inspectable generations** the way an
artist would: (1) restyle the background from the original photo (img2img, structure-preserving),
(2) show the OpenPose pose that will condition the figure, (3) paint the figure into the restyled
background (img2img on it + OpenPose ControlNet + character LoRA, at a moderate denoise). Each step's
image is shown in the 2D scene so the pipeline is debuggable at every stage.

Step 3 places the figure on the restyled background. img2img on it (paint the figure straight in) was the
first design, but **img2img + ControlNet is unusable with the bundled SDXL checkpoint** — it decodes to a
flat grey (`min=max=128`) in every variant tried (GPU and CPU VAE, with and without an inpaint mask,
ControlNet on GPU and on CPU). It is a NaN in the UNet's img2img sampling that even a CPU fp32 VAE cannot
avoid, while plain img2img (the background step) and txt2img + ControlNet (no init image) both work. So
step 3 generates the figure with the proven **txt2img + ControlNet + character-LoRA** path (on a plain
background) and composites it over the restyled background using the OpenPose skeleton's silhouette as a
soft alpha mask (`m_CompositeWithSkeletonMask`) — an engine-side image blend with no sd.cpp numerics, so
it cannot go grey. It is not chroma (the figure's invented background defeats keying) and not img2img
(which NaNs); the same skeleton drives both the ControlNet pose and the composite mask. The mask is a
rough body silhouette, so edge placement is approximate — a deliberate trade for a result that reliably
renders. (Backend note for this checkpoint: its baked VAE is broken on this build — it alone decodes
grey — so the demo loads an external fp16 VAE from the manifest and runs it on the GPU, with the baked VAE
falling back to the CPU.)

This multi-step shape collides with two facts of the ADR 0008 design:

- **A step's output is a GPU-only `Texture2D`.** A completed Diffusion Job is registered into
  `m_TextureMap` as a `Texture2D` (the renderer's only texture path). Step 3 needs step 1's
  restyled background as an **img2img init image**, i.e. as CPU RGB pixels in `m_InputImageMap`.
  The app never sees a generation's pixels — they flow `DiffusionSystem` → register-request →
  `ResourceManagerSystem` → GPU texture — so the app cannot itself carry step 1's result into
  step 3.
- **The OpenPose hint is computed but never surfaced.** `DiffusionSystem` rasterizes the OpenPose
  Control Hint internally while assembling a job's params; nothing exposes that image as a texture
  the scene can show. And `RegisterGeneratedTextureRequest` has a **single adder**
  (`DiffusionSystem`), so the app cannot register a hint image as a texture on its own.

## Decision

Two small engine additions, both keeping `ResourceManagerSystem` the sole writer of the resource
map and `DiffusionSystem` the sole adder of `RegisterGeneratedTextureRequest`.

- **A Generated Texture is also retained as a reusable Input Image.** When `ResourceManagerSystem`
  drains a `RegisterGeneratedTextureRequest`, in addition to creating the `Texture2D` in
  `m_TextureMap` it stores the same CPU RGB `pg::Image` into `m_InputImageMap` under the **same
  UUID**. A later `GenerateImageRequest` may then reference that UUID as its `m_InputImageID`
  (img2img init) or `m_BackgroundImageID` (chroma composite) exactly as it references a
  manifest-loaded input image. This is what lets step 3 consume step 1's output — generated
  images become first-class generation inputs, no new store and no new lookup path.

- **OpenPose hints are rasterized to a texture on demand via `DiffusionSystem`.** A new engine-typed
  one-frame request, `RasterizeOpenPoseHintRequest` (skeleton UUID + caller-assigned target texture
  UUID + output size), is read by `DiffusionSystem`, which resolves the skeleton from the resource
  map, rasterizes the canonical COCO-18 hint (the same rasterizer it feeds to a ControlNet job, with
  the same canvas→image mapping), and emits a `RegisterGeneratedTextureRequest` for it. The hint
  becomes an ordinary resource-map texture the scene can show. It lives in `DiffusionSystem` — not a
  new system — because `RegisterGeneratedTextureRequest` may have only one adder, and the OpenPose
  rasterizer already lives in the Diffusion module. The work is a synchronous CPU rasterization (no
  job, no worker thread).

The 3-step pipeline itself (orchestration, the editable background style prompt + consistency
slider, advancing to the next step when the previous Diffusion Job finishes) is **app code** in
SandboxApp's `ImageGenDemoSystem`, honouring the engine/game split. The engine only gains the two
reusable primitives above.

## Trade-offs

- **Retain generated images in `m_InputImageMap` (chosen) vs. a separate generated-image CPU store
  (rejected):** a parallel store would force the img2img/chroma resolution in `DiffusionSystem` to
  learn a second lookup path and decide which store wins on a UUID collision. Reusing
  `m_InputImageMap` keeps "an input image is an input image" whatever produced it, mirroring how
  ADR 0008 made "a generated image is just a texture." Cost: every generation now also keeps its RGB
  buffer resident in CPU memory for the session (acceptable for the demo's handful of images).
- **Retain in `m_InputImageMap` (chosen) vs. chain all three steps inside one Diffusion Job
  (rejected):** chaining hides the intermediates, but the whole point is to **show** each stage in
  the scene for debugging, which requires three separately-registered textures and therefore three
  requests.
- **Hint rasterization in `DiffusionSystem` (chosen) vs. a dedicated `OpenPoseHintSystem`
  (rejected):** a separate system cannot emit `RegisterGeneratedTextureRequest` without becoming a
  second adder (forbidden). Routing through `DiffusionSystem` preserves one-adder; the cost is a
  little more surface on a system otherwise about running jobs.

## Consequences

- `ResourceManagerSystem` writes one extra map entry per generated texture; `m_InputImageMap` now
  holds both manifest-loaded and generated images, keyed by UUID. No consumer changes — the
  img2img/chroma paths already resolve against `m_InputImageMap`.
- `DiffusionSystem` gains `RasterizeOpenPoseHintRequest` in its read set and rasterizes hints to
  textures synchronously, alongside its job machinery.
- New engine component `RasterizeOpenPoseHintRequestOneFrameComponent` (Diffusion module).
- SandboxApp's `ImageGenDemoSystem` becomes a 3-step pipeline driver (background restyle → pose
  hint → composite) with an ImGui panel (background style prompt + consistency slider + Generate),
  advancing on Diffusion Job completion; `SceneSetupSystem` shows the three result textures side by
  side. The previous single-shot chroma-composite demo path is removed.
- Running two generations per pipeline (and more on re-runs) exposed a latent crash in `DiffusionSystem`'s
  worker: the worker captured a `shared_ptr` to the `DiffusionJob` that *owns* the worker thread, so the
  last reference could be released on the worker thread itself, running `~DiffusionJob()` → `join()` on
  its own thread (a self-join deadlock). The worker now captures a raw job pointer (the job outlives the
  worker, which always publishes its terminal state last), and a `try/catch` keeps a backend exception
  from terminating the process.
