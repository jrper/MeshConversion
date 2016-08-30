#include "vtkAppendFilter.h"
#include "vtkCellData.h"
#include "vtkIntArray.h"
#include "vtkIdTypeArray.h"
#include "vtkVersion.h"

#include <unistd.h>
#include <stdio.h>
#include <getopt.h>
#include <string>

#include "mesh_converter.h"


template <typename T>
vtkUnstructuredGrid* read(char* fname, int verbosity) 
{
  if (verbosity) {
    std::cout << "Reading from file: " <<fname<<std::endl;
  }
  T* reader= T::New();
  reader->SetFileName(fname);
  reader->Update();
  vtkUnstructuredGrid* ugrid = vtkUnstructuredGrid::New();
  ugrid->ShallowCopy(as_unstructured_grid(reader->GetOutput()));
  reader->Delete();
  return ugrid;
}

template <typename T>
int write(vtkUnstructuredGrid* data, char* fname, int verbosity)
{
  if (verbosity) {
    std::cout << "Writing to file: " <<fname<<std::endl;
  }
  T* writer = T::New();  
  writer->SetFileName(fname);
#if VTK_MAJOR_VERSION <= 5
  writer->SetInput(data);
#else
  writer->SetInputData(data);
#endif
  writer->Write();
  writer->Delete();
  return 0;
}

#ifdef CONVERTER

int main(int argc, char *argv[]) {

  std::string input_format="exodus";
  std::string output_format="gmsh";
  int opt, option_index=0, verbosity=1;

  static struct option long_options[] = {
            {"input_format",     required_argument, 0,  'i' },
            {"output_format",  required_argument,       0,  'o' },
            {"help",  no_argument, 0,  'h' },
	    {"quiet", no_argument,       0,  'v' },
            {"verbose", no_argument,       0,  'v' },
            {"version",  no_argument, 0, 'V'},
  };

  while (  (opt = getopt_long(argc, argv, "i::o::hvV",
				long_options, &option_index)) != -1 ) { 
    switch ( opt ) {
    case 'i':
      if (optarg) {
	input_format = optarg;
      }
      break;
    case 'o':
      if (optarg) {
	output_format = optarg;
      }
      break;
    case 'h':
      print_help(argv[0]);
      return EXIT_SUCCESS;
      break;
    case 'q':
      verbosity = 0;
      break;
    case 'v':
      verbosity = 2;
      break;
    case 'V':
      print_version();
      return EXIT_SUCCESS;
      break;
    case '?': 
      cerr << "Unknown option: '" << char(optopt) << "'!" << endl;
      break;
    }
  }

  if (argc <3) {
    print_usage(argv[0]);
    return 1;
  }

  vtkUnstructuredGrid* ugdata  = NULL;

  if (input_format.compare("exodus")==0 ) {
    ugdata = read_exodusII(argv[optind++], verbosity);
  }  else if (input_format.compare("gmsh")==0 ) {
    ugdata = read<vtkGmshReader>(argv[optind++], verbosity);
  }  else if (input_format.compare("vtu")==0 ) {
    ugdata = read_vtu(argv[optind++], verbosity);
  }  else if (input_format.compare("pvtu")==0 ) {
    ugdata = read_pvtu(argv[optind++], verbosity);
  }  else if (input_format.compare("triangle")==0 ) {
    ugdata = read_triangle(argv[optind++], verbosity);
  }  else {
    std::cout<< "Unrecognised input format: "<< input_format << std::endl;
    return 1;
  }
  
  int flag;

  if (output_format.compare("gmsh")==0) {
    flag=write_gmsh(ugdata,argv[optind], verbosity);
  } else if (output_format.compare("vtu")==0) {
    flag=write<vtkXMLUnstructuredGridWriter>(ugdata,argv[optind], verbosity);
  } else if (output_format.compare("vtm")==0) {
    flag=write_vtm(as_multiblock(ugdata),argv[optind], verbosity);
  } else if (output_format.compare("triangle")==0) {
    flag=write_triangle(ugdata,argv[optind], verbosity);
  } else {
    std::cout<< "Unrecognised output format: "<<output_format<<std::endl;
    return 1;
  }
  
  if (ugdata){
    ugdata->Delete();
  }

  return flag;
}

#endif // CONVERTER

vtkUnstructuredGrid* as_unstructured_grid(vtkDataObject* data) {
  if (data->IsA("vtkUnstructuredGrid")) {
    return vtkUnstructuredGrid::SafeDownCast(data);
  } else if (data->IsA("vtkMultiBlockDataSet")) {
    vtkMultiBlockDataSet* mbdata = vtkMultiBlockDataSet::SafeDownCast(data);
    return multiblock_to_unstructured_grid(mbdata);
  }
  return NULL;
}

vtkMultiBlockDataSet* as_multiblock(vtkDataObject* data) {
  return NULL;
}

vtkUnstructuredGrid* multiblock_to_unstructured_grid(vtkMultiBlockDataSet* data) {
  vtkAppendFilter* appender = vtkAppendFilter::New();
  appender->SetMergePoints(1);

  vtkMultiBlockDataSet* sidesets = vtkMultiBlockDataSet::SafeDownCast(data->GetBlock(4));
  for (int i=0; i<sidesets->GetNumberOfBlocks(); i++) {
    vtkUnstructuredGrid* ugrid = vtkUnstructuredGrid::SafeDownCast(sidesets->GetBlock(i));
    vtkIntArray* eids = vtkIntArray::New();
    eids->SetNumberOfValues(ugrid->GetNumberOfCells());
    ugrid->GetCellData()->GetArray("ObjectId")->SetName("PhysicalIds");
    for (int j=0; j<ugrid->GetNumberOfCells(); j++) {
      eids->SetValue(j,i+1);
    }
    eids->SetName("ElementaryEntities");
    ugrid->GetCellData()->AddArray(eids);
    appender->AddInputData(ugrid);
  }
  
  vtkMultiBlockDataSet* regions = vtkMultiBlockDataSet::SafeDownCast(data->GetBlock(0));
  for (int i=0; i<regions->GetNumberOfBlocks(); i++) {
    vtkUnstructuredGrid* ugrid = vtkUnstructuredGrid::SafeDownCast(regions->GetBlock(i));
    ugrid->GetCellData()->GetArray("ObjectId")->SetName("PhysicalIds");
    ugrid->GetCellData()->GetArray("GlobalElementId")->SetName("ElementaryEntities");
    appender->AddInputData(ugrid);
  }

  appender->Update();
  vtkUnstructuredGrid* output = vtkUnstructuredGrid::New();
  output->ShallowCopy(appender->GetOutput());
  appender->Delete();

  return output;
}

// Special Readers

vtkUnstructuredGrid* read_exodusII(char* fname, int verbosity){
  if (verbosity) {
    std::cout << "Reading from file: " <<fname<<std::endl;
  }
  vtkExodusIIReader* reader = vtkExodusIIReader::New();

  reader->SetFileName(fname);
  reader->UpdateInformation();
  reader->GenerateGlobalNodeIdArrayOn();
  reader->GenerateGlobalElementIdArrayOn();
  reader->GenerateObjectIdCellArrayOn();
  for (int i=0; i<reader->GetNumberOfSideSetArrays(); i++) {
    reader->SetSideSetArrayStatus(reader->GetSideSetArrayName(i),1);
  }
  reader->Update();
  vtkUnstructuredGrid* ugrid = vtkUnstructuredGrid::New();
  ugrid->ShallowCopy(as_unstructured_grid(reader->GetOutput()));
  reader->Delete();
  return ugrid;
 }

vtkUnstructuredGrid* read_gmsh(char* fname, int verbosity) {
  if (verbosity) {
   std::cout << "Reading from GMSH file: " <<fname<<std::endl;
  }
   vtkGmshReader* reader= vtkGmshReader::New();
   reader->SetFileName(fname);
   reader->Update();
   vtkUnstructuredGrid* ugrid = vtkUnstructuredGrid::New();
   ugrid->ShallowCopy(reader->GetOutput());
   reader->Delete();
   return ugrid;
 }

vtkUnstructuredGrid* read_vtu(char* fname, int verbosity) {
  if (verbosity) {
       std::cout << "Reading from VTK unstructured grid file: " <<fname<<std::endl;
     }
   vtkXMLUnstructuredGridReader* reader= vtkXMLUnstructuredGridReader::New();
   reader->SetFileName(fname);
   reader->Update();
   vtkUnstructuredGrid* ugrid = vtkUnstructuredGrid::New();
   ugrid->ShallowCopy(reader->GetOutput());
   reader->Delete();
   return ugrid;
 }

vtkUnstructuredGrid* read_pvtu(char* fname, int verbosity) {
   std::cout << "Reading from VTK parallel unstructured grid file: " <<fname<<std::endl;
   vtkXMLPUnstructuredGridReader* reader= vtkXMLPUnstructuredGridReader::New();
   reader->SetFileName(fname);
   reader->Update();
   vtkUnstructuredGrid* ugrid = vtkUnstructuredGrid::New();
   ugrid->ShallowCopy(reader->GetOutput());
   reader->Delete();
   return ugrid;
 }

vtkUnstructuredGrid* read_triangle(char* fname, int verbosity) {
   std::cout << "Reading from triangle files: " <<fname<<std::endl;
   vtkTriangleReader* reader= vtkTriangleReader::New();
   reader->SetFileName(fname);
   reader->Update();
   vtkUnstructuredGrid* ugrid = vtkUnstructuredGrid::New();
   ugrid->ShallowCopy(reader->GetOutput());
   reader->Delete();
   return ugrid;
 }

int print_usage(char* name){
  std::cout << "usage: " << name << " [-i input_format] [-o output_format] [-hqvV] input_file output_file"<<std::endl;
  return 0;
}

int print_help(char* name){
  print_usage(name);
  std::cout << "\t -i --input_format \t Specify mesh format of input file" << std::endl;
  std::cout << "\t -i --output_format \t Specify mesh format for output file" << std::endl;
  std::cout << "\t -h --help \t Print this message" << std::endl;
  std::cout << "\t -q --quiet \t Suppress output except error reporting" << std::endl;
  std::cout << "\t -v --verbose \t Increase verbosity" << std::endl;
  std::cout << "\t -V --version \t Report version information" << std::endl;
  return 0;
}

int print_version(){
  std::cout << "mesh_converter "
            << CONVERTER_MAJOR_VERSION<<"."<<CONVERTER_MINOR_VERSION
            <<std::endl;
  std::cout << "\t Built against VTK version: " << VTK_VERSION << std::endl;
  return 0;
}

// Special Writers



int write_gmsh(vtkUnstructuredGrid* data, char* fname, int verbosity){
  if (verbosity) {
    std::cout << "Writing to file: " <<fname<<std::endl;
  }

  vtkGmshWriter* writer = vtkGmshWriter::New();  
  writer->SetFileName(fname);
#if VTK_MAJOR_VERSION <= 5
  writer->SetInput(data);
#else
  writer->SetInputData(data);
#endif
  writer->Write();
  writer->Delete();
  return 0;
}

int write_triangle(vtkUnstructuredGrid* data, char* fname, int verbosity){
  if (verbosity) {
    std::cout << "Writing to files: " <<fname<<std::endl;
  }
  
  vtkTriangleWriter* writer = vtkTriangleWriter::New();  
  writer->SetFileName(fname);
#if VTK_MAJOR_VERSION <= 5
  writer->SetInput(data);
#else
  writer->SetInputData(data);
#endif
  writer->Write();
  writer->Delete();
  return 0;
}

int write_vtm(vtkMultiBlockDataSet* data, char* fname, int verbosity){
  if (verbosity) {
    std::cout << "Writing to file: " <<fname<<std::endl;
  }

  vtkXMLMultiBlockDataWriter* writer = vtkXMLMultiBlockDataWriter::New();  
  writer->SetFileName(fname);
#if VTK_MAJOR_VERSION <= 5
  writer->SetInput(data);
#else
  writer->SetInputData(data);
#endif
  writer->Write();
  writer->Delete();
  return 0;
}
  
