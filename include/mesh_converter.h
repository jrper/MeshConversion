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

int print_help(char*);
int print_usage(char*);
int print_version();

// converters
vtkUnstructuredGrid* as_unstructured_grid(vtkDataObject*);
vtkMultiBlockDataSet* as_multiblock(vtkDataObject*);
vtkUnstructuredGrid* multiblock_to_unstructured_grid(vtkMultiBlockDataSet*);

// readers
vtkUnstructuredGrid*  read_exodusII(char*, int);
vtkUnstructuredGrid*  read_gmsh(char*, int);
vtkUnstructuredGrid*  read_vtu(char*, int);
vtkUnstructuredGrid*  read_pvtu(char*, int);
vtkUnstructuredGrid*  read_triangle(char*, int);

// special writers
int write_gmsh(vtkUnstructuredGrid*, char*, int);
int write_vtm(vtkMultiBlockDataSet*, char*, int);
int write_triangle(vtkUnstructuredGrid*, char*, int);

