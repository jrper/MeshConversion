int print_usage();

// converters
vtkUnstructuredGrid* multiblock_to_unstucturedgrid(vtkMultiBlockDataSet*);

// readers
vtkMultiBlockDataSet* read_exodusII(char*);
vtkUnstructuredGrid*  read_gmsh(char*);

// writers
int write_gmsh(vtkUnstructuredGrid*, char*);
int write_vtu(vtkUnstructuredGrid*, char*);
int write_vtm(vtkMultiBlockDataSet*, char*);


