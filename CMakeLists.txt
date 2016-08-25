cmake_minimum_required(VERSION 2.8.8)

project(mesh_converter)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)


FIND_PACKAGE(VTK REQUIRED)
INCLUDE(${VTK_USE_FILE})


link_directories(${VTK_DIR})
include_directories(include)

add_executable(mesh_converter src/mesh_converter.cxx src/vtkGmshWriter.cxx  src/vtkGmshReader.cxx)
target_link_libraries(mesh_converter ${VTK_LIBRARIES})

