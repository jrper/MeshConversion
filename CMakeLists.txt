cmake_minimum_required(VERSION 2.8.8)

project(exodus_converter)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)


FIND_PACKAGE(VTK REQUIRED)
INCLUDE(${VTK_USE_FILE})


link_directories(${VTK_DIR})

add_executable(exodus_converter exodus_converter.cxx exodus_converter.h vtkGmshWriter.cxx vtkGmshWriter.h vtkGmshReader.h vtkGmshReader.cxx)
target_link_libraries(exodus_converter ${VTK_LIBRARIES})

