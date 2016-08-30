# MeshConversion
A mesh file conversion tool

## Building


Build with:

    cmake .
    make
    
## Running

Run with:
    mesh_converter [-i input_format] [-o output_format] [-h] input_file output_file

Input formats supported:
 * exodusII
 * gmsh
 * triangle(2d)/tetgen(3d)
 
Output formats supported:
 * gmsh
 * vtu
 * triangle(2d)/tetgen(3d)
