set(SOURCES
    "backends/imgui_impl_win32.cpp"
    "backends/imgui_impl_dx11.cpp"
    "imgui.cpp"
    "imgui_demo.cpp"
    "imgui_draw.cpp"
    "imgui_tables.cpp"
    "imgui_widgets.cpp"
)

source_group("" FILES ${SRC_MAIN})

include_directories (
    "${PROJECT_SOURCE_DIR}/3rdParty/imgui"
)

# Include directories
include_directories(
    "${PROJECT_SOURCE_DIR}/3rdParty/imgui"
    "${PROJECT_SOURCE_DIR}/3rdParty/imgui/backends"
    "$ENV{WindowsSdkDir}Include/um"
    "$ENV{WindowsSdkDir}Include/shared"
    "$ENV{DXSDK_DIR}Include"
)

link_directories (
    "${PROJECT_BINARY_DIR}/bin"
    "$ENV{DXSDK_DIR}/Lib/x86"
)

add_library (ImGui STATIC ${SOURCES} )
target_link_libraries(ImGui d3d11.lib d3dcompiler.lib)

target_compile_definitions(ImGui PRIVATE UNICODE _UNICODE)

# Properties->General->Output Directory
set_target_properties(ImGui PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

