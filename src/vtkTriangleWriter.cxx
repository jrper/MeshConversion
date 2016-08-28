#include "vtkTriangleWriter.h"

#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkCell.h"
#include "vtkCellType.h"
#include "vtkCellData.h"
#include "vtkIdTypeArray.h"
#include "vtkCellArray.h"
#include <vtkIntArray.h>
#include "vtkPoints.h"
#include "vtkSmartPointer.h"
#include "vtkDataObject.h"
#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkTriangle.h"
#include "vtkTetra.h"
#include "vtkQuad.h"
#include "vtkHexahedron.h"
#include "vtkPolygon.h"
#include <iostream>
#include <fstream>
#include <map>
#include <algorithm>

//vtkCxxRevisionMacro(vtkTriangleWriter, "$Revision: 0.0$");
vtkStandardNewMacro(vtkTriangleWriter);

int get_physical_tag(vtkUnstructuredGrid* ugrid, vtkIdType n)
{

  vtkCellData* cd = ugrid->GetCellData();
  if (cd->HasArray("PhysicalIds")){

    int id;
    vtkIntArray* data = vtkIntArray::SafeDownCast(cd->GetArray("PhysicalIds"));
    id = data->GetValue(n-1);

    return id;
  } else {
    return n;
  }
}

vtkTriangleWriter::vtkTriangleWriter(){
  this->FileName=NULL;
  this->isBinary=1;
  this->SetNumberOfInputPorts(1);
  this->SetNumberOfOutputPorts(0);
};
vtkTriangleWriter::~vtkTriangleWriter(){
 this->SetFileName(0);
};
void vtkTriangleWriter::SetBinaryWriteMode(int isBinary){
  //  this->DebugOn();
  vtkDebugMacro("isBinary"<<isBinary);
  this->isBinary=isBinary;
};

void vtkTriangleWriter::WriteData()
{
  vtkUnstructuredGrid *input= vtkUnstructuredGrid::SafeDownCast(
    this->GetInput());

  // try to open the GMSH file
  std::ofstream OutFile;
  std::string basename=this->FileName;

  double bounds[6];
  int dim;

  input->GetBounds(bounds);

  if (bounds[4] == 0.0 || bounds[5] == 0.0) {
    dim=2;
  } else {
    dim=3;
  }
  
  OutFile.open(basename + ".node");
  
  // write the nodes

  OutFile <<  input->GetNumberOfPoints()<< " " << dim << " " 
	  << 0 << "" << 0 <<std::endl;
  for (vtkIdType i=0;i<input->GetNumberOfPoints();i++) {
    double x[3];
    input->GetPoint(i,x);
    OutFile << i+1;
    for(int j=0;j<dim;j++) {
      OutFile << " " << x[j];
    }
    OutFile << std::endl;
  }

  OutFile.close();

  int boundary_elements=0, internal_elements=0;

  for (vtkIdType i=0;i<input->GetNumberOfCells();i++) {
    int cellDim= input->GetCell(i)->GetCellDimension();
    if (cellDim==dim) {
      internal_elements++;
    } else if (cellDim==dim-1) {
      boundary_elements++;
    }
  }


  if (boundary_elements) {
    switch(dim) {
    case 2:
      OutFile.open(basename + ".edge");
    case 3:
      OutFile.open(basename + ".face");
    }

    // Write the boundary data

    OutFile << boundary_elements 
            << " " << 1 <<std::endl;
   
    int k = 1;
    for (vtkIdType i=0; i<input->GetNumberOfCells(); i++) {

      OutFile << k++;

      int cellDim= input->GetCell(i)->GetCellDimension();
      if (cellDim==dim-1) {
	vtkIdList* ids = input->GetCell(i)->GetPointIds();
      
	for (int j=0; j<dim; j++) {	
	  OutFile << " " << ids->GetId(j);
	}
	
	OutFile << " " << get_physical_tag(input, i) <<std::endl;
      }
    }
    

  }

  OutFile.close();
 
  OutFile.open(basename + ".ele");

  OutFile << input->GetNumberOfCells() << " " << dim+1 
          << 1 << 0 << 0 <<std::endl;

  // Write the element data
  int k = 1;
    for (vtkIdType i=0; i<input->GetNumberOfCells(); i++) {

      OutFile << k++;

      int cellDim= input->GetCell(i)->GetCellDimension();
      if (cellDim==dim) {
	vtkIdList* ids = input->GetCell(i)->GetPointIds();
      
	for (int j=0; j<dim+1; j++) {	
	  OutFile << " " << ids->GetId(j);
	}
	
	OutFile << " " << get_physical_tag(input, i) <<std::endl;
      }
    }

  OutFile.close();

  return;
}

int vtkTriangleWriter::FillInputPortInformation(int,vtkInformation *info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(),"vtkUnstructuredGrid");
  return 1;
}

vtkUnstructuredGrid* vtkTriangleWriter::GetInput()
{
  return vtkUnstructuredGrid::SafeDownCast(this->Superclass::GetInput());
}

vtkUnstructuredGrid* vtkTriangleWriter::GetInput(int port)
{
  return vtkUnstructuredGrid::SafeDownCast(this->Superclass::GetInput(port));
}

void vtkTriangleWriter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
 
  os << indent << "File Name: "
      << (this->FileName ? this->FileName : "(none)") << "\n";
}
