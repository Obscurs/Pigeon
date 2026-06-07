file(GLOB SRC_MAIN
    "Test.cpp"
    "Utils/TestApp.cpp"
)

file(GLOB SRC_TESTS_CORE "UT_Core/*.cpp")
file(GLOB SRC_TESTS_INTERFACE "UT_Interface/*.cpp")
file(GLOB SRC_TESTS_UI "UT_UI/*.cpp")
file(GLOB SRC_TESTS_RENDERER "UT_Renderer/*.cpp")
file(GLOB SRC_TESTS_SANDBOX "UT_Sandbox/*.cpp")

set(SRC_SANDBOX_SYSTEMS
    "${PROJECT_SOURCE_DIR}/Code/SandboxApp/Sandbox/ConfigLoaderSystem.cpp"
    "${PROJECT_SOURCE_DIR}/Code/SandboxApp/Sandbox/DebugPanelSystem.cpp"
    "${PROJECT_SOURCE_DIR}/Code/SandboxApp/Sandbox/InputReadoutSystem.cpp"
    "${PROJECT_SOURCE_DIR}/Code/SandboxApp/Sandbox/LifetimeSystem.cpp"
    "${PROJECT_SOURCE_DIR}/Code/SandboxApp/Sandbox/QuadAnimationSystem.cpp"
    "${PROJECT_SOURCE_DIR}/Code/SandboxApp/Sandbox/QuadRenderSystem.cpp"
    "${PROJECT_SOURCE_DIR}/Code/SandboxApp/Sandbox/QuadSpawnSystem.cpp"
    "${PROJECT_SOURCE_DIR}/Code/SandboxApp/Sandbox/SceneSetupSystem.cpp"
    "${PROJECT_SOURCE_DIR}/Code/SandboxApp/Sandbox/SpriteRenderSystem.cpp"
    "${PROJECT_SOURCE_DIR}/Code/SandboxApp/Sandbox/TextRenderSystem.cpp"
    "${PROJECT_SOURCE_DIR}/Code/SandboxApp/Sandbox/UIButtonSystem.cpp"
    "${PROJECT_SOURCE_DIR}/Code/SandboxApp/Sandbox/UIButtonVisualSystem.cpp"
    "${PROJECT_SOURCE_DIR}/Code/SandboxApp/Sandbox/UICloseSystem.cpp"
    "${PROJECT_SOURCE_DIR}/Code/SandboxApp/Sandbox/UIElementHelpers.cpp"
    "${PROJECT_SOURCE_DIR}/Code/SandboxApp/Sandbox/UIStatusSystem.cpp"
)

source_group("" FILES ${SRC_MAIN})

source_group("Tests/Core" FILES ${SRC_TESTS_CORE})
source_group("Tests/Interface" FILES ${SRC_TESTS_INTERFACE})
source_group("Tests/UI" FILES ${SRC_TESTS_UI})
source_group("Tests/Renderer" FILES ${SRC_TESTS_RENDERER})
source_group("Tests/Sandbox" FILES ${SRC_TESTS_SANDBOX})

include_directories (
    "${PROJECT_SOURCE_DIR}/Code/PigeonLib"
    "${PROJECT_SOURCE_DIR}/Code/PigeonLib/PigeonLib.dir/Test"

    "${PROJECT_SOURCE_DIR}/Code/UT"
    "${PROJECT_SOURCE_DIR}/Code/SandboxApp"
)

link_directories (
    "${PROJECT_BINARY_DIR}/bin"
)

add_executable (UT
    ${OBJS}
    ${SRC_MAIN}
    ${SRC_TESTS_CORE}
    ${SRC_TESTS_INTERFACE}
    ${SRC_TESTS_UI}
    ${SRC_TESTS_RENDERER}
    ${SRC_TESTS_SANDBOX}
    ${SRC_SANDBOX_SYSTEMS}
)

add_dependencies(UT PigeonLib) 
set_property(TARGET UT PROPERTY EXCLUDE_FROM_DEFAULT_BUILD_RELEASE ON)
set_property(TARGET UT PROPERTY EXCLUDE_FROM_DEFAULT_BUILD_DEBUG ON)

SET_SOURCE_FILES_PROPERTIES(
  ${OBJS}
  PROPERTIES
  EXTERNAL_OBJECT true
  GENERATED true
)

if(WIN32)
    target_compile_definitions(UT PRIVATE PG_PLATFORM_WINDOWS)
elseif(UNIX)
    target_compile_definitions(UT PRIVATE PG_PLATFORM_UNIX)
endif()

target_precompile_headers(UT PRIVATE "${PROJECT_SOURCE_DIR}/Code/PigeonLib/pch.h")

target_link_libraries (
    UT PRIVATE PigeonLib
    spdlog
    d3dcompiler
    Catch2::Catch2
)

set_target_properties(UT PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
