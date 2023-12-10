file(GLOB SRC_MAIN
    "Test.cpp"
    "Utils/TestApp.cpp"
)

file(GLOB SRC_TESTS_SAMPLE
    "UT_Sample/*.cpp"
)

file(GLOB SRC_TESTS_LAYERS
    "UT_Layers/*.cpp"
)

source_group("" FILES ${SRC_MAIN})
source_group("Tests/Sample" FILES ${SRC_TESTS_SAMPLE})
source_group("Tests/Layers" FILES ${SRC_TESTS_LAYERS})

include_directories (
    "${PROJECT_SOURCE_DIR}/3rdParty/Catch"
    "${PROJECT_SOURCE_DIR}/Code/PigeonLib"
    "${PROJECT_SOURCE_DIR}/Code/PigeonLib/PigeonLib.dir/Test"
    "${PROJECT_SOURCE_DIR}/Code/UT"
    "${PROJECT_SOURCE_DIR}/3rdParty/"
    "${PROJECT_SOURCE_DIR}/3rdParty/spdlog-1.12.0/include"
)

link_directories (
    "${PROJECT_BINARY_DIR}/bin"
    "${PROJECT_SOURCE_DIR}/3rdParty/spdlog-1.12.0/lib"
)

add_executable (UT ${OBJS} ${SRC_MAIN} ${SRC_TESTS_LAYERS}  ${SRC_TESTS_SAMPLE})
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

target_link_libraries (
    UT PRIVATE PigeonLib
    spdlog.lib
)

set_target_properties(UT PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

