

file(GLOB SRC_MAIN
    "Test.cpp"
)
file(GLOB SRC_TESTS
    "UT_Sample/*.cpp"
)

source_group("" FILES ${SRC_MAIN})
source_group("Tests" FILES ${SRC_TESTS})

# Properties->C/C++->General->Additional Include Directories
#include_directories (.)

include_directories (
    "${PROJECT_SOURCE_DIR}/3rdParty/Optick_1.4.0/include"
    "${PROJECT_SOURCE_DIR}/3rdParty/entt"
    "${PROJECT_SOURCE_DIR}/3rdParty/behaviortree_cpp_v3/include"
    "${PROJECT_SOURCE_DIR}/3rdParty/Catch"
    "${PROJECT_SOURCE_DIR}/Code/PigeonLib"
    "${PROJECT_SOURCE_DIR}/Code/PigeonLib/PigeonLib.dir/$(Configuration)"
)

link_directories (
    "${PROJECT_SOURCE_DIR}/3rdParty/Optick_1.4.0/lib/x64"
    "${PROJECT_SOURCE_DIR}/3rdParty/behaviortree_cpp_v3/lib"
    "${PROJECT_SOURCE_DIR}/3rdParty/behaviortree_cpp_v3/lib"
    "${PROJECT_BINARY_DIR}/bin"
)

# Set Properties->General->Configuration Type to Dynamic Library(.dll)
# Creates math.dll with the listed sources collected in the variables
# Also adds sources to the Solution Explorer
#add_library(UT SHARED ${SRC_MAIN} ${SRC_TESTS})

# Set Properties->General->Configuration Type to Application(.exe)
# Creates app.exe with the listed sources (main.cxx)
# Adds sources to the Solution Explorer

SET(OBJS
  ${CMAKE_CURRENT_BINARY_DIR}../../PigeonLib/PigeonLib.dir/$(Configuration)/pch.obj
)

add_executable (UT ${OBJS} ${SRC_MAIN} ${SRC_TESTS} )
add_dependencies(UT PigeonLib) 

SET_SOURCE_FILES_PROPERTIES(
  ${OBJS}
  PROPERTIES
  EXTERNAL_OBJECT true
  GENERATED true
)

target_link_libraries (
    UT
    bt_sample_nodes.lib
    debug OptickCore.lib optimized OptickCore
    gmock.lib
    gmock_main.lib
    gtest.lib
    gtest_main.lib
)

#target_link_libraries(UT PigeonApp.lib)
# Creates a folder "libraries" and adds target project (math.vcproj) under it
# set_property(TARGET math PROPERTY FOLDER "libraries")

# Properties->General->Output Directory
set_target_properties(UT PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

# Adds logic to INSTALL.vcproj to copy math.dll to the destination directory
#install (TARGETS math
#         RUNTIME DESTINATION ${PROJECT_SOURCE_DIR}/_install
#         LIBRARY DESTINATION ${PROJECT_SOURCE_DIR}/_install)
