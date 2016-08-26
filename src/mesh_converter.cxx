#include "vtkExodusIIReader.h"
#include "vtkGmshReader.h"
#include "vtkXMLUnstructuredGridReader.h"
#include "vtkXMLPUnstructuredGridReader.h"
#include "vtkTriangleReader.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkUnstructuredGrid.h"
#include "vtkXMLUnstructuredGridWriter.h"
#include "vtkXMLMultiBlockDataWriter.h"
#include "vtkGmshWriter.h"
#include "vtkAppendFilter.h"
#include "vtkCellData.h"
#include "vtkIntArray.h"
#include "vtkIdTypeArray.h"

#include <unistd.h>
#include <stdio.h>
#include <string>

#include "mesh_converter.h"

int main(int argc, char *argv[]) {

  std::string input_format="exodus";
  std::string output_format="gmsh";
  int opt;

  while (  (opt = getopt(argc, argv, "i::o::hV")) != -1 ) { 
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
      print_usage();
      return EXIT_SUCCESS;
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
    print_usage();
    return 1;
  }

  vtkMultiBlockDataSet* mbdata = NULL;
  vtkUnstructuredGrid* ugdata  = NULL;

  if (input_format.compare("exodus")==0 ) {
    mbdata = read_exodusII(argv[optind++]);
  }  else if (input_format.compare("gmsh")==0 ) {
    ugdata = read_gmsh(argv[optind++]);
  }  else if (input_format.compare("vtu")==0 ) {
    ugdata = read_vtu(argv[optind++]);
  }  else if (input_format.compare("pvtu")==0 ) {
    ugdata = read_pvtu(argv[optind++]);
  }  else if (input_format.compare("triangle")==0 ) {
    ugdata = read_triangle(argv[optind++]);
  }  else {
    std::cout<< "Unrecognised input format: "<< input_format << std::endl;
    return 1;
  }
  
  int flag;
  
  if (output_format.compare("gmsh")==0) {
    if (mbdata) {
      ugdata=multiblock_to_unstucturedgrid(mbdata);
    }
    flag=write_gmsh(ugdata,argv[optind]);
  } else if (output_format.compare("vtu")==0) {
    if (mbdata) {
      ugdata=multiblock_to_unstucturedgrid(mbdata);
    }
    flag=write_vtu(ugdata,argv[optind]);
  }  else if (output_format.compare("vtm")==0) {
    if (ugdata) {
      //      mbdata=unstucturedgrid_to_multiblock(ugdata);
    }
    flag=write_vtm(mbdata,argv[optind]);
  } else {
    std::cout<< "Unrecognised output format: "<<output_format<<std::endl;
    return 1;
  }
  
  if (mbdata){
    mbdata->Delete();
  }
  if (ugdata){
    ugdata->Delete();
  }

  return flag;
}

vtkUnstructuredGrid* multiblock_to_unstucturedgrid(vtkMultiBlockDataSet* data) {
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

vtkMultiBlockDataSet* read_exodusII(char* fname){

  std::cout << "Reading from file: " <<fname<<std::endl;

  vtkExodusIIReader* r = vtkExodusIIReader::New();

  r->SetFileName(fname);
  r->UpdateInformation();
  r->GenerateGlobalNodeIdArrayOn();
  r->GenerateGlobalElementIdArrayOn();
  r->GenerateObjectIdCellArrayOn();
  for (int i=0; i<r->GetNumberOfSideSetArrays(); i++) {
    r->SetSideSetArrayStatus(r->GetSideSetArrayName(i),1);
  }
  r->Update();

  vtkMultiBlockDataSet* data;
  data = r->GetOutput();

  return data;
 }

 vtkUnstructuredGrid* read_gmsh(char* fname) {
   std::cout << "Reading from GMSH file: " <<fname<<std::endl;
   vtkGmshReader* reader= vtkGmshReader::New();
   reader->SetFileName(fname);
   reader->Update();
   vtkUnstructuredGrid* ugrid = vtkUnstructuredGrid::New();
   ugrid->ShallowCopy(reader->GetOutput());
   reader->Delete();
   return ugrid;
 }

 vtkUnstructuredGrid* read_vtu(char* fname) {
   std::cout << "Reading from VTK unstructured grid file: " <<fname<<std::endl;
   vtkXMLUnstructuredGridReader* reader= vtkXMLUnstructuredGridReader::New();
   reader->SetFileName(fname);
   reader->Update();
   vtkUnstructuredGrid* ugrid = vtkUnstructuredGrid::New();
   ugrid->ShallowCopy(reader->GetOutput());
   reader->Delete();
   return ugrid;
 }

vtkUnstructuredGrid* read_pvtu(char* fname) {
   std::cout << "Reading from VTK parallel unstructured grid file: " <<fname<<std::endl;
   vtkXMLPUnstructuredGridReader* reader= vtkXMLPUnstructuredGridReader::New();
   reader->SetFileName(fname);
   reader->Update();
   vtkUnstructuredGrid* ugrid = vtkUnstructuredGrid::New();
   ugrid->ShallowCopy(reader->GetOutput());
   reader->Delete();
   return ugrid;
 }

vtkUnstructuredGrid* read_triangle(char* fname) {
   std::cout << "Reading from VTK parallel unstructured grid file: " <<fname<<std::endl;
   vtkTriangleReader* reader= vtkTriangleReader::New();
   reader->SetFileName(fname);
   reader->Update();
   vtkUnstructuredGrid* ugrid = vtkUnstructuredGrid::New();
   ugrid->ShallowCopy(reader->GetOutput());
   reader->Delete();
   return ugrid;
 }

int print_usage(){
  std::cout << "usage: mesh_converter [-i input_format] [-o output_format] [-hV] input_file output_file"<<std::endl;
  return 0;
}

int print_version(){
  std::cout << "mesh_converter "
            << CONVERTER_MAJOR_VERSION<<"."<<CONVERTER_MINOR_VERSION
            <<std::endl;
  return 0;
}

int write_gmsh(vtkUnstructuredGrid* data, char* fname){

  std::cout << "Writing to file: " <<fname<<std::endl;

  vtkGmshWriter* writer = vtkGmshWriter::New();  
  writer->SetFileName(fname);
  writer->SetInputData(data);
  writer->Write();
  writer->Delete();
  return 0;
}

int write_vtu(vtkUnstructuredGrid* data, char* fname){

  std::cout << "Writing to file: " <<fname<<std::endl;

  vtkXMLUnstructuredGridWriter* writer = vtkXMLUnstructuredGridWriter::New();  
  writer->SetFileName(fname);
  writer->SetInputData(data);
  writer->Write();
  writer->Delete();
  return 0;
}

int write_vtm(vtkMultiBlockDataSet* data, char* fname){

  std::cout << "Writing to file: " <<fname<<std::endl;

  vtkXMLMultiBlockDataWriter* writer = vtkXMLMultiBlockDataWriter::New();  
  writer->SetFileName(fname);
  writer->SetInputData(data);
  writer->Write();
  writer->Delete();
  return 0;
}
  
