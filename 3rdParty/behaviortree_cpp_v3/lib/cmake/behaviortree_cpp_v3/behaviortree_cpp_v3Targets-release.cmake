#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "BT::behaviortree_cpp_v3" for configuration "Release"
set_property(TARGET BT::behaviortree_cpp_v3 APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(BT::behaviortree_cpp_v3 PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/behaviortree_cpp_v3.lib"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/behaviortree_cpp_v3.dll"
  )

list(APPEND _cmake_import_check_targets BT::behaviortree_cpp_v3 )
list(APPEND _cmake_import_check_files_for_BT::behaviortree_cpp_v3 "${_IMPORT_PREFIX}/lib/behaviortree_cpp_v3.lib" "${_IMPORT_PREFIX}/bin/behaviortree_cpp_v3.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
