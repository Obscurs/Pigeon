cmake_minimum_required (VERSION 3.20)

project(PigeonEngine)

include(FetchContent)

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

# Text-to-image (stable-diffusion.cpp) is opt-in: OFF by default so the Testing/UT build and a plain
# build stay CUDA-free, fast, and green (the real backend is inert, tests use TestingDiffusionBackend).
# Configure with -DPG_ENABLE_DIFFUSION=ON to fetch + build the CUDA runtime and activate generation.
# NOTE: building stable-diffusion.cpp with CUDA is heavy and requires the CUDA Toolkit; validate on
# the target machine. Pin GIT_TAG to a known-good commit for reproducibility before shipping.
option(PG_ENABLE_DIFFUSION "Build the stable-diffusion.cpp (CUDA) text-to-image backend" ON)
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
    GIT_TAG        master
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