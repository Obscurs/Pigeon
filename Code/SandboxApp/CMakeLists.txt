file(GLOB SRCS
    "SandboxApp.cpp"
)

source_group("" FILES ${SRCS})

# Properties->C/C++->General->Additional Include Directories
include_directories (
    "${PROJECT_SOURCE_DIR}/Code/PigeonLib"
	"${PROJECT_SOURCE_DIR}/Code/SandboxApp"
#    "${PROJECT_SOURCE_DIR}/3rdParty/behaviortree_cpp_v3/include"
#    "${PROJECT_SOURCE_DIR}/3rdParty/entt"
#    "${PROJECT_SOURCE_DIR}/3rdParty/Optick_1.4.0/include"
    "${PROJECT_SOURCE_DIR}/3rdParty/"
    "${PROJECT_SOURCE_DIR}/3rdParty/spdlog-1.12.0/include"
)

link_directories (
#    "${PROJECT_SOURCE_DIR}/3rdParty/behaviortree_cpp_v3/lib"
#    "${PROJECT_SOURCE_DIR}/3rdParty/Optick_1.4.0/lib/x64"
    "${PROJECT_SOURCE_DIR}/3rdParty/spdlog-1.12.0/lib"
)

# Set Properties->General->Configuration Type to Application(.exe)
# Creates app.exe with the listed sources (main.cxx)
# Adds sources to the Solution Explorer
add_executable (SandboxApp ${SRCS})

set_property(TARGET SandboxApp PROPERTY ENABLE_EXPORTS 1)
set_property(TARGET SandboxApp PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/Data)
# Properties->Linker->Input->Additional Dependencies

add_dependencies(SandboxApp PigeonLib) 

if(WIN32)
    target_compile_definitions(SandboxApp PRIVATE PG_PLATFORM_WINDOWS)
elseif(UNIX)
    target_compile_definitions(SandboxApp PRIVATE PG_PLATFORM_UNIX)
endif()

target_precompile_headers(SandboxApp PRIVATE "${PROJECT_SOURCE_DIR}/Code/PigeonLib/pch.h")

target_link_libraries (
    SandboxApp PRIVATE PigeonLib
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
set_target_properties(SandboxApp PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
