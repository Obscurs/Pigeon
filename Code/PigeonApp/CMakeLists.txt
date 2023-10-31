file(GLOB SRCS
    "main.cpp"
    "pch.cpp"
)

file(GLOB BT
    "BT/sample_nodes/*.cpp"
    "BT/sample_nodes/*.h"
)

source_group("" FILES ${SRCS})
source_group("BT" FILES ${BT})

# Properties->C/C++->General->Additional Include Directories
include_directories (
    "${PROJECT_SOURCE_DIR}/3rdParty/entt"
    "${PROJECT_SOURCE_DIR}/3rdParty/Optick_1.4.0/include"
    "${PROJECT_SOURCE_DIR}/3rdParty/behaviortree_cpp_v3/include"
    "${PROJECT_SOURCE_DIR}/Code/PigeonApp"
)

link_directories (
    "${PROJECT_SOURCE_DIR}/3rdParty/behaviortree_cpp_v3/lib"
    "${PROJECT_SOURCE_DIR}/3rdParty/Optick_1.4.0/lib/x64"
)

# Set Properties->General->Configuration Type to Application(.exe)
# Creates app.exe with the listed sources (main.cxx)
# Adds sources to the Solution Explorer
add_executable (PigeonApp ${ECS} ${SRCS} ${BT})

set_property(TARGET PigeonApp PROPERTY ENABLE_EXPORTS 1)
set_property(TARGET PigeonApp PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/Data)
# Properties->Linker->Input->Additional Dependencies

SET(BT optimized behaviortree_cpp_v3.lib debug behaviortree_cpp_v3d.lib)

#to do make this work
#target_precompile_headers(PigeonApp PUBLIC pch.h)

target_link_libraries (
    PigeonApp
    ${BT}
    bt_sample_nodes.lib
    debug OptickCore.lib optimized OptickCore
    gmock.lib
    gmock_main.lib
    gtest.lib
    gtest_main.lib
)

# Creates a folder "executables" and adds target 
# project (app.vcproj) under it
#set_property(TARGET app PROPERTY FOLDER "executables")

# Properties->General->Output Directory
set_target_properties(PigeonApp PROPERTIES
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

add_custom_command(TARGET PigeonApp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${DLLS_TO_COPY}/$(Configuration)
    ${PROJECT_BINARY_DIR}/bin/$(Configuration)
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${DATA_TO_COPY}
    ${PROJECT_BINARY_DIR}/bin/$(Configuration)
)