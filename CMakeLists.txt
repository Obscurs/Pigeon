cmake_minimum_required (VERSION 3.20)

project(PigeonEngine)

include(FetchContent)

# Link every dependency statically into the engine/app. llama.cpp's CMake defaults BUILD_SHARED_LIBS ON
# and (added as a subdirectory) leaks that into the global cache, which would make ggml, spdlog, glm, etc.
# build as scattered DLLs the exe can't find at runtime. Forcing it OFF up front keeps the original
# self-contained, all-static layout (only the CUDA runtime, cudart/cublas from the toolkit, stays dynamic).
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm
  GIT_TAG        1.0.1
)
FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui
  GIT_TAG        v1.90.4-docking
)
FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog
  GIT_TAG        v1.13.0
)
FetchContent_Declare(
  entt
  GIT_REPOSITORY https://github.com/skypjack/entt
  GIT_TAG        v3.13.1
)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2
  GIT_TAG        v2.13.9
)

FetchContent_Declare(
  msdf-atlas-gen
  GIT_REPOSITORY https://github.com/Obscurs/msdf-atlas-gen
  GIT_TAG        main
)

FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.3
)

FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
  FetchContent_Populate(imgui)
  set(IMGUI_INCLUDE_DIRS "${imgui_SOURCE_DIR}")
  set(IMGUI_SOURCES 
      "${imgui_SOURCE_DIR}/imgui.cpp"
      "${imgui_SOURCE_DIR}/imgui_demo.cpp"
      "${imgui_SOURCE_DIR}/imgui_draw.cpp"
      "${imgui_SOURCE_DIR}/imgui_tables.cpp"
      "${imgui_SOURCE_DIR}/imgui_widgets.cpp"
      "${imgui_SOURCE_DIR}/backends/imgui_impl_dx11.cpp"
      "${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp")
endif()
add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC ${IMGUI_INCLUDE_DIRS})
target_link_libraries(imgui PUBLIC d3d11)

FetchContent_GetProperties(entt)
if(NOT entt_POPULATED)
  FetchContent_Populate(entt)
  set(ENTT_INCLUDE_DIRS "${entt_SOURCE_DIR}/src")
endif()

FetchContent_MakeAvailable(glm spdlog Catch2 msdf-atlas-gen nlohmann_json)

# In-engine AI runtimes: stable-diffusion.cpp (text-to-image, CUDA) and llama.cpp (text generation, CPU).
# Both opt-in. They each vendor ggml under the same `ggml` target and need INCOMPATIBLE ggml revisions, so
# they cannot share one ggml: a shared ggml compiles but crashes sd.cpp's model manager at generate time
# ("model manager tensor ... is not registered"). They are therefore ISOLATED — sd.cpp builds in-tree with
# its own ggml (below), and llama.cpp builds in a separate ExternalProject with its own ggml. The Testing/UT
# build leaves both OFF (the real backends are inert; tests use the Testing mocks).
# NOTE: both GIT_TAGs are pinned to verified-good commits — the ggml/sd.cpp/llama.cpp C APIs all evolve.
# Bump deliberately and re-check StableDiffusionCppBackend.cpp / LlamaCppBackend.cpp when you do.
option(PG_ENABLE_DIFFUSION "Build the stable-diffusion.cpp (CUDA) text-to-image backend" ON)
option(PG_ENABLE_TEXTGEN "Build the llama.cpp (CUDA) text-generation backend" ON)

# Text-to-image (stable-diffusion.cpp, CUDA), built in-tree with its OWN ggml (GGML_CUDA only scopes this
# ggml; llama's isolated ggml is unaffected). Pinned to #1650 (2026-06-13) — the bundled SDXL checkpoint +
# LoRA + OpenPose ControlNet assets load and GENERATE against this commit (verified). Newer commits drop
# the vae_decode_only / keep_vae_on_cpu fields the backend uses; the #1638 loader rejects this ControlNet's
# proj-weight shape. Heavy CUDA build; requires the CUDA Toolkit.
if(PG_ENABLE_DIFFUSION)
  set(GGML_CUDA ON CACHE BOOL "" FORCE)
  set(SD_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
  set(SD_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
  # stable-diffusion.cpp (and the diffusion backend TU) exceed MSVC's default COFF section limit.
  # Restrict to C++ TUs: passing /bigobj to nvcc makes it treat the flag as a second input file.
  add_compile_options("$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CXX_COMPILER_ID:MSVC>>:/bigobj>")
  FetchContent_Declare(
    stable_diffusion_cpp
    GIT_REPOSITORY https://github.com/leejet/stable-diffusion.cpp
    GIT_TAG        276025e054555166ec419413c6748ca79986ee93
  )
  FetchContent_MakeAvailable(stable_diffusion_cpp)

  # stable-diffusion.cpp sizes the ControlNet compute graph at 1536 nodes, which fits SD1.5 but
  # overflows an SDXL ControlNet (deep transformer stacks) -> GGML_ASSERT(n_nodes < size). Bump it to
  # the UNet's budget. Patched at configure so it survives a fresh fetch; idempotent (no-op once done).
  set(_sd_control_hpp "${stable_diffusion_cpp_SOURCE_DIR}/src/model/diffusion/control.hpp")
  if(EXISTS "${_sd_control_hpp}")
    file(READ "${_sd_control_hpp}" _sd_control_src)
    string(REPLACE "#define CONTROL_NET_GRAPH_SIZE 1536" "#define CONTROL_NET_GRAPH_SIZE 102400" _sd_control_src "${_sd_control_src}")
    file(WRITE "${_sd_control_hpp}" "${_sd_control_src}")
  endif()
endif()

# Text generation (llama.cpp, CUDA), built in an ISOLATED ExternalProject so its ggml never collides with
# sd.cpp's (both are now CUDA, but they remain SEPARATE ggml build scopes — ADR 0009's isolation contract
# is unchanged). Static, OpenMP-off (avoids a vcomp link dependency when linked by path; llama still
# threads via n_threads). Installed to a prefix; PigeonLib links the static libs by path (see
# Code/PigeonLib/CMakeLists.txt). The LLM offloads layers to the GPU at runtime via the configurable
# n_gpu_layers (default = all layers), ADR 0010.
if(PG_ENABLE_TEXTGEN)
  include(ExternalProject)
  # The PigeonLib TU that includes <llama.h> exceeds MSVC's default COFF section limit.
  add_compile_options("$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CXX_COMPILER_ID:MSVC>>:/bigobj>")
  set(PG_LLAMA_INSTALL "${CMAKE_BINARY_DIR}/llama-isolated")
  set(PG_LLAMA_INCLUDE_DIR "${PG_LLAMA_INSTALL}/include" CACHE INTERNAL "isolated llama.cpp include dir")
  # Static libs llama installs, ordered dependents-before-dependencies for the linker. ggml-cuda is the
  # CUDA backend (GGML_CUDA=ON); it needs the CUDA runtime (cudart/cublas), linked in PigeonLib's
  # CMakeLists where the diffusion build already pulls it in.
  set(PG_LLAMA_LIBS
    "${PG_LLAMA_INSTALL}/lib/llama${CMAKE_STATIC_LIBRARY_SUFFIX}"
    "${PG_LLAMA_INSTALL}/lib/ggml${CMAKE_STATIC_LIBRARY_SUFFIX}"
    "${PG_LLAMA_INSTALL}/lib/ggml-cpu${CMAKE_STATIC_LIBRARY_SUFFIX}"
    "${PG_LLAMA_INSTALL}/lib/ggml-cuda${CMAKE_STATIC_LIBRARY_SUFFIX}"
    "${PG_LLAMA_INSTALL}/lib/ggml-base${CMAKE_STATIC_LIBRARY_SUFFIX}"
    CACHE INTERNAL "isolated llama.cpp static libs")
  ExternalProject_Add(llama_isolated
    GIT_REPOSITORY https://github.com/ggml-org/llama.cpp
    GIT_TAG        6e14286edaa60a223292c8a996506905b2f66f66
    CMAKE_ARGS
      -DCMAKE_INSTALL_PREFIX=${PG_LLAMA_INSTALL}
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON
      -DBUILD_SHARED_LIBS=OFF
      -DGGML_CUDA=ON
      -DGGML_OPENMP=OFF
      -DLLAMA_BUILD_TESTS=OFF
      -DLLAMA_BUILD_EXAMPLES=OFF
      -DLLAMA_BUILD_SERVER=OFF
      -DLLAMA_BUILD_TOOLS=OFF
      -DLLAMA_CURL=OFF
      # Built standalone here, so LLAMA_STANDALONE defaults these ON — but we only need the core library.
      # The app/common/UI targets pull in a generated build-info.h and a web UI we do not build.
      -DLLAMA_BUILD_APP=OFF
      -DLLAMA_BUILD_COMMON=OFF
      -DLLAMA_BUILD_HTML=OFF
      -DLLAMA_BUILD_UI=OFF
    BUILD_COMMAND ${CMAKE_COMMAND} --build <BINARY_DIR> --config Release
    INSTALL_COMMAND ${CMAKE_COMMAND} --install <BINARY_DIR> --config Release
    BUILD_BYPRODUCTS ${PG_LLAMA_LIBS}
  )
endif()

option(PG_ENABLE_MATTING "Build the onnxruntime (CUDA) image-matting backend" ON)

# Image matting (onnxruntime, CUDA execution provider). Cuts the generated figure out of its plain
# background by its real outline so it composites into the scene without a halo (ADR 0012). Unlike the GGML
# backends this is the ONE dependency shipped as PREBUILT DLLs — onnxruntime publishes no static GPU build —
# fetched and extracted from the official GPU release zip (no source build). PigeonLib links the import lib
# and defines PG_MATTING_ENABLED (Code/PigeonLib/CMakeLists.txt); SandboxApp copies the runtime DLLs next to
# the exe. The runtime falls back to the CPU EP if CUDA cannot init, so a wrong toolkit never breaks the app.
#
# CUDA-version coupling: onnxruntime's GPU zip is built against a SPECIFIC CUDA major + cuDNN. 1.19+ targets
# CUDA 12 + cuDNN 9 (1.17 was CUDA 11.8 + cuDNN 8). The GGML backends here use CUDA 12 (cudart/cublas 12) and
# the GGML stack ships NO cuDNN, so we pin the CUDA-12 onnxruntime build AND bundle the cuDNN 9 redist DLLs
# next to the exe (the CUDA 12 runtime itself is found on PATH from the installed toolkit). Bump both versions
# together and keep them on the same CUDA major.
if(PG_ENABLE_MATTING)
  set(PG_ONNXRUNTIME_VERSION "1.20.1")   # CUDA 12 + cuDNN 9 build
  FetchContent_Declare(
    onnxruntime_gpu
    URL https://github.com/microsoft/onnxruntime/releases/download/v${PG_ONNXRUNTIME_VERSION}/onnxruntime-win-x64-gpu-${PG_ONNXRUNTIME_VERSION}.zip
  )
  FetchContent_MakeAvailable(onnxruntime_gpu)
  # The zip has no CMakeLists, so MakeAvailable only extracts it; expose the dir for the include/lib/DLLs.
  set(PG_ONNXRUNTIME_DIR "${onnxruntime_gpu_SOURCE_DIR}" CACHE INTERNAL "onnxruntime GPU prebuilt root")

  # cuDNN 9 (for CUDA 12) runtime DLLs — onnxruntime's CUDA EP needs cuDNN, which the GGML CUDA stack does
  # NOT ship, so without these onnxruntime_providers_cuda.dll fails to load (LoadLibrary error 126) and the
  # matte silently drops to CPU. NVIDIA's redist archive is downloadable without a login; bundle its DLLs.
  set(PG_CUDNN_VERSION "9.5.1.17")
  FetchContent_Declare(
    cudnn_redist
    URL https://developer.download.nvidia.com/compute/cudnn/redist/cudnn/windows-x86_64/cudnn-windows-x86_64-${PG_CUDNN_VERSION}_cuda12-archive.zip
  )
  FetchContent_MakeAvailable(cudnn_redist)
  set(PG_CUDNN_DIR "${cudnn_redist_SOURCE_DIR}" CACHE INTERNAL "cuDNN 9 redist root")
endif()

set(MSDFGEN_INCLUDE_DIRS
    "${msdf-atlas-gen_SOURCE_DIR}/msdf-atlas-gen"
    "${msdf-atlas-gen_SOURCE_DIR}/msdfgen"
    "${msdf-atlas-gen_SOURCE_DIR}/msdfgen/include"
)
# Turn on the ability to create folders to organize projects (.vcproj)
# It creates "CMakePredefinedTargets" folder by default and adds CMake
# defined projects like INSTALL.vcproj and ZERO_CHECK.vcproj
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT SandboxApp)

# Set compiler flags and options. 
# Here it is setting the Visual Studio warning level to 4
# set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release;Testing" CACHE STRING "" FORCE)
set(CMAKE_C_FLAGS_TESTING "${CMAKE_C_FLAGS_DEBUG}" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_TESTING "${CMAKE_CXX_FLAGS_DEBUG}" CACHE STRING "" FORCE)
set(CMAKE_EXE_LINKER_FLAGS_TESTING "${CMAKE_EXE_LINKER_FLAGS_DEBUG}" CACHE STRING "" FORCE)
add_compile_options(
  "$<$<CONFIG:Testing>:-DTESTS_ENABLED>"
)
# Sources are UTF-8; tell MSVC so it does not warn (C4819/C4566) under non-UTF code pages.
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
# Command to output information to the console
# Useful for displaying errors, warnings, and debugging
message ("cxx Flags: " ${CMAKE_CXX_FLAGS})

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

# Copy runsettings to build dir so VS auto-discovers it alongside the .sln
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/Minimal.runsettings"
  "${CMAKE_BINARY_DIR}/Minimal.runsettings"
  COPYONLY
)

# Sub-directories where more CMakeLists.txt exist

add_subdirectory(Code/PigeonLib)
add_subdirectory(Code/SandboxApp)
add_subdirectory(Code/UT)

set_target_properties(freetype PROPERTIES FOLDER "Dependencies")
set_target_properties(glm PROPERTIES FOLDER "Dependencies")
set_target_properties(imgui PROPERTIES FOLDER "Dependencies")
set_target_properties(msdfgen PROPERTIES FOLDER "Dependencies")
set_target_properties(msdf-atlas-gen PROPERTIES FOLDER "Dependencies")
set_target_properties(spdlog PROPERTIES FOLDER "Dependencies")