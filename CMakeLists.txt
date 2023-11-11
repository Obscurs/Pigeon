cmake_minimum_required (VERSION 3.20)

# Maps to a solution file (Tutorial.sln). The solution will 
# have all targets (exe, lib, dll) as projects (.vcproj)
project (Pigeon)

# Turn on the ability to create folders to organize projects (.vcproj)
# It creates "CMakePredefinedTargets" folder by default and adds CMake
# defined projects like INSTALL.vcproj and ZERO_CHECK.vcproj
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT SandboxApp)

# Set compiler flags and options. 
# Here it is setting the Visual Studio warning level to 4
# set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
set(CMAKE_CXX_STANDARD 17)

# Command to output information to the console
# Useful for displaying errors, warnings, and debugging
message ("cxx Flags: " ${CMAKE_CXX_FLAGS})

# Sub-directories where more CMakeLists.txt exist
add_subdirectory(Code/PigeonLib)
add_subdirectory(Code/SandboxApp)
add_subdirectory(Code/UT)
add_subdirectory(3rdParty/imgui)