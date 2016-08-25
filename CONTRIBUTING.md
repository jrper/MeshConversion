# Contributing

Any contributions gratefully accepted. Open a pull request.


## Data Model

This software is designed to produce files useful for Fluidity developers. In particular this means that any information labelling surfaces/lower dimensional should be preserved in a scalar integer array in the cell data with the name of `"PhysicalIds"`.

### Adding Readers and Writers

Any additional file types should be supported through adding new files with a name following the pattern `vtkFileTypeReader.cxx` implementing a reader into one of the intrinsic VTK data types, preferably `vtkUnstructuredGrid`. Any additional transformations necessary to rename arrays to implement physical surface labelling can be placed into the main mesh converter file.
