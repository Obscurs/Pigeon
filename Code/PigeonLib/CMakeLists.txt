file(GLOB SRCS
    "Pigeon.h"
)

file(GLOB PG
    "Pigeon/Application.cpp"
    "Pigeon/Layer.cpp"
    "Pigeon/LayerStack.cpp"
    "Pigeon/Log.cpp"
    "Pigeon/Core.h"
    "Pigeon/EntryPoint.h"
)

file(GLOB PGIMGUI
    "Pigeon/ImGui/ImGuiLayer.cpp"
)

file(GLOB PLATFORM_WIN
    "Platform/Windows/WindowsWindow.cpp"
)

file(GLOB PLATFORM_TEST
    "Platform/UTMockup/UTWindow.cpp"
)

#file(GLOB BT
#    "Pigeon/BT/sample_nodes/*.cpp"
#    "Pigeon/BT/sample_nodes/*.h"
#)

source_group("" FILES ${SRCS})
source_group("Pigeon" FILES ${PG})
source_group("Pigeon/ImGui" FILES ${PGIMGUI})
#source_group("Platform" FILES ${PLATFORM})
#source_group("Pigeon/BT" FILES ${BT})

# Properties->C/C++->General->Additional Include Directories
include_directories (
    "${PROJECT_SOURCE_DIR}/Code/PigeonLib"
#    "${PROJECT_SOURCE_DIR}/3rdParty/behaviortree_cpp_v3/include"
#    "${PROJECT_SOURCE_DIR}/3rdParty/entt"
#    "${PROJECT_SOURCE_DIR}/3rdParty/Optick_1.4.0/include"
    "${PROJECT_SOURCE_DIR}/3rdParty/spdlog-1.12.0/include"
    "${PROJECT_SOURCE_DIR}/3rdParty/imgui"
    "${PROJECT_SOURCE_DIR}/3rdParty"
)

link_directories (
#    "${PROJECT_SOURCE_DIR}/3rdParty/behaviortree_cpp_v3/lib"
#    "${PROJECT_SOURCE_DIR}/3rdParty/Optick_1.4.0/lib/x64"
    "${PROJECT_SOURCE_DIR}/3rdParty/spdlog-1.12.0/lib"
)

# Set Properties->General->Configuration Type to Application(.exe)
# Creates app.exe with the listed sources (main.cxx)
# Adds sources to the Solution Explorer
add_library(PigeonLib SHARED)

# Add common sources to target
target_sources(PigeonLib PRIVATE ${ECS} ${SRCS} ${PG} ${PGIMGUI})
add_dependencies(PigeonLib ImGui) 

# Platform/Test files are included only for the "Test" configuration
#if(CMAKE_CONFIGURATION_TYPES MATCHES "Test")
#    target_sources(PigeonLib PRIVATE $<$<CONFIG:Test>:${PLATFORM_TEST}>)
#    source_group("Platform\\Test" FILES ${PLATFORM_TEST})
#endif()

# Platform/Windows files are included for configurations other than "Test"
# This assumes WIN32 is true; adjust as necessary for other platforms
if(WIN32)
    #target_sources(PigeonLib PRIVATE $<$<NOT:$<CONFIG:Test>>:${PLATFORM_WIN}>)
    target_sources(PigeonLib PRIVATE ${PLATFORM_WIN})
    source_group("Platform\\Windows" FILES ${PLATFORM_WIN})
endif()

set_property(TARGET PigeonLib PROPERTY ENABLE_EXPORTS 1)
set_property(TARGET PigeonLib PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/Data)

target_precompile_headers(PigeonLib PRIVATE pch.h)

# Properties->Linker->Input->Additional Dependencies

#target_compile_definitions(PigeonLib PRIVATE
#    $<$<CONFIG:Test>:PG_PLATFORM_TEST>
#)

if(WIN32)
    target_compile_definitions(PigeonLib PRIVATE PG_PLATFORM_WINDOWS)
    target_compile_definitions(PigeonLib PRIVATE PG_BUILD_DLL)
elseif(UNIX)
    target_compile_definitions(PigeonLib PRIVATE PG_PLATFORM_UNIX)
endif()

#SET(BT optimized behaviortree_cpp_v3.lib debug behaviortree_cpp_v3d.lib)

#to do make this work
#target_precompile_headers(PigeonLib PUBLIC pch.h)

target_link_libraries (
    PigeonLib
    ImGui
#    ${BT}
#    bt_sample_nodes.lib
#    debug OptickCore.lib optimized OptickCore
#    gmock.lib
#    gmock_main.lib
#    gtest.lib
#    gtest_main.lib
    spdlog.lib
)

# Creates a folder "executables" and adds target 
# project (app.vcproj) under it
#set_property(TARGET app PROPERTY FOLDER "executables")

# Properties->General->Output Directory
set_target_properties(PigeonLib PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)



# Adds logic to INSTALL.vcproj to copy app.exe to destination directory
#install (TARGETS app
#         RUNTIME DESTINATION ${PROJECT_SOURCE_DIR}/_install)
#"$(SolutionDir)..\External\*.dll"

file(GLOB DLLS_TO_COPY
    ${PROJECT_SOURCE_DIR}/External
)
file(GLOB DATA_TO_COPY
    ${PROJECT_SOURCE_DIR}/Data
)

add_custom_command(TARGET PigeonLib POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${DLLS_TO_COPY}/$(Configuration)
    ${PROJECT_BINARY_DIR}/bin/$(Configuration)
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${DATA_TO_COPY}
    ${PROJECT_BINARY_DIR}/bin/$(Configuration)
)