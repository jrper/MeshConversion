#include "vtkExodusIIReader.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkUnstructuredGrid.h"
#include "vtkXMLUnstructuredGridWriter.h"
#include "vtkAppendFilter.h"
#include "vtkCellData.h"
#include "vtkIntArray.h"
#include "vtkIdTypeArray.h"
#include "vtkGmshWriter.h"

#include <unistd.h>




int main(int argc, char *argv[]) {

  char* input_format="exodus";
  char* output_format="gmsh";
  int opt;

  while (  (opt = getopt(argc, argv, "i::o::")) != -1 ) { 
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
    case '?': 
      cerr << "Unknown option: '" << char(optopt) << "'!" << endl;
      break;
    }
  }

  vtkExodusIIReader* r = vtkExodusIIReader::New();

  r->SetFileName(argv[optind++]);
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

  vtkGmshWriter* gwriter = vtkGmshWriter::New();  
  gwriter->SetFileName(argv[optind]);
  gwriter->SetInputData(appender->GetOutput());
  gwriter->SetBinaryWriteMode(0);
  gwriter->Write();
  
  appender->Delete();
  data->Delete();
  return 1;
}
  
