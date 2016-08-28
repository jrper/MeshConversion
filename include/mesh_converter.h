#define CONVERTER_MAJOR_VERSION 0
#define CONVERTER_MINOR_VERSION 5

#include "vtkExodusIIReader.h"
#include "vtkGmshReader.h"
#include "vtkXMLUnstructuredGridReader.h"
#include "vtkXMLPUnstructuredGridReader.h"
#include "vtkTriangleReader.h"

#include "vtkXMLUnstructuredGridWriter.h"
#include "vtkXMLMultiBlockDataWriter.h"
#include "vtkGmshWriter.h"
#include "vtkTriangleWriter.h"

#include "vtkMultiBlockDataSet.h"
#include "vtkUnstructuredGrid.h"

// GNU standard functions

int print_usage();
int print_version();

// converters
vtkUnstructuredGrid* as_unstructured_grid(vtkDataObject*);
vtkMultiBlockDataSet* as_multiblock(vtkDataObject*);
vtkUnstructuredGrid* multiblock_to_unstructured_grid(vtkMultiBlockDataSet*);

// readers
vtkUnstructuredGrid*  read_exodusII(char*);
vtkUnstructuredGrid*  read_gmsh(char*);
vtkUnstructuredGrid*  read_vtu(char*);
vtkUnstructuredGrid*  read_pvtu(char*);
vtkUnstructuredGrid*  read_triangle(char*);

// special writers
int write_gmsh(vtkUnstructuredGrid*, char*);
int write_vtm(vtkMultiBlockDataSet*, char*);
int write_triangle(vtkUnstructuredGrid*, char*);

