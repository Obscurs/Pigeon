file(GLOB SRCS
    "App2D.cpp"
)

source_group("" FILES ${SRCS})

# Properties->C/C++->General->Additional Include Directories
include_directories (
    "${PROJECT_SOURCE_DIR}/Code/PigeonLib"
    "${PROJECT_SOURCE_DIR}/Code/App"
)

# Set Properties->General->Configuration Type to Application(.exe)
add_executable (App ${SRCS})

set_property(TARGET App PROPERTY ENABLE_EXPORTS 1)
set_property(TARGET App PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/Data)

set_property(TARGET App PROPERTY EXCLUDE_FROM_DEFAULT_BUILD_TESTING ON)

add_dependencies(App
    PigeonLib
)

if(WIN32)
    target_compile_definitions(App PRIVATE PG_PLATFORM_WINDOWS)
elseif(UNIX)
    target_compile_definitions(App PRIVATE PG_PLATFORM_UNIX)
endif()

target_precompile_headers(App PRIVATE "${PROJECT_SOURCE_DIR}/Code/PigeonLib/pch.h")

target_link_libraries (
    App PRIVATE PigeonLib
    spdlog
    glm
    d3dcompiler
)

# Properties->General->Output Directory
set_target_properties(App PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

# Copy the onnxruntime + cuDNN runtime DLLs next to the executable, mirroring SandboxApp: the matting
# backend (ADR 0012) loads them at runtime and onnxruntime GPU has no static distribution. Skipped when
# matting is disabled (e.g. the Testing/UT configure).
if(PG_ENABLE_MATTING)
    file(GLOB PG_ONNXRUNTIME_DLLS "${PG_ONNXRUNTIME_DIR}/lib/*.dll")
    file(GLOB_RECURSE PG_CUDNN_DLLS "${PG_CUDNN_DIR}/*.dll")
    add_custom_command(TARGET App POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PG_ONNXRUNTIME_DLLS} ${PG_CUDNN_DLLS} "$<TARGET_FILE_DIR:App>"
        COMMAND_EXPAND_LISTS
        COMMENT "Copying onnxruntime + cuDNN DLLs next to App")
endif()
