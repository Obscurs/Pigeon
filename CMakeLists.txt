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
# Command to output information to the console
# Useful for displaying errors, warnings, and debugging
message ("cxx Flags: " ${CMAKE_CXX_FLAGS})

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

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