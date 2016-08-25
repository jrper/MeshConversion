cmake_minimum_required(VERSION 2.8.8)

project(mesh_converter)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)


find_package(VTK REQUIRED)
include(${VTK_USE_FILE})


link_directories(${VTK_DIR})
include_directories(include)

set(IO_SOURCE_FILES src/vtkGmshWriter.cxx  src/vtkGmshReader.cxx)

add_executable(mesh_converter src/mesh_converter.cxx ${IO_SOURCE_FILES})
target_link_libraries(mesh_converter ${VTK_LIBRARIES})

find_package( PkgConfig REQUIRED)
pkg_search_module(CPPUNIT cppunit)
find_library(CPPUNIT_LIB cppunit)
include_directories(${CPPUNIT_INCLUDE_DIRS})
add_executable(test-converter  EXCLUDE_FROM_ALL tests/test_mesh_conversion.cxx ${IO_SOURCE_FILES})
target_link_libraries(test-converter ${CPPUNIT_LIB} ${CMAKE_DL_LIBS} ${VTK_LIBRARIES})

add_test(NAME test-converter COMMAND "${PROJECT_OUTPUT_TEST_DIR}/test-converter")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/tests/gmsh_data.msh
    ${CMAKE_CURRENT_BINARY_DIR}/tests/gmsh_data.msh COPYONLY)



install(TARGETS mesh_converter
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)

