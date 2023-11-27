cmake_minimum_required (VERSION 3.20)

project(PigeonEngine)

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
add_subdirectory(3rdParty/imgui)
add_subdirectory(3rdParty/glm)