#include "vtkTriangleReader.h"

#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkCell.h"
#include "vtkCellType.h"
#include "vtkCellData.h"
#include "vtkIdTypeArray.h"
#include "vtkCellArray.h"
#include <vtkIntArray.h>
#include "vtkPoints.h"
#include "vtkUnstructuredGrid.h"
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

template <class T>
void endswap(T *objp)
{
  unsigned char *memp = reinterpret_cast<unsigned char*>(objp);
  std::reverse(memp, memp + sizeof(T));
}

//vtkCxxRevisionMacro(vtkTriangleReader, "$Revision: 0.0$");
vtkStandardNewMacro(vtkTriangleReader);

vtkTriangleReader::vtkTriangleReader(){
  this->FileName=NULL;
  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(1);
};
vtkTriangleReader::~vtkTriangleReader(){
 this->SetFileName(0);
};

int readTriangleNodes(std::ifstream& TriangleNodeFile,vtkPoints* outpoints)
{
  int nnodes, dim, temp1, temp2;
  TriangleNodeFile >> nnodes >> dim >> temp1 >> temp2;
  outpoints->Allocate(nnodes);
  for(vtkIdType i=0;i<nnodes;i++) {
    int nodeno;
    double x,y,z;
    switch(dim)
      {
      case 2:
	TriangleNodeFile >> nodeno >> x >> y; z=0;
	break;
      case 3 :
	TriangleNodeFile >> nodeno >> x >> y >> z;
	break;
      }
    outpoints->InsertNextPoint(x,y,z);
  }
  return dim;
}


int readTriangleElements(std::ifstream& TriangleElementFile,vtkUnstructuredGrid* output, int npe)
{ 
  vtkIdType nelements, ntags;
  if (npe) {
    TriangleElementFile >> nelements >> ntags;
  } else {
    TriangleElementFile >> nelements >> npe >> ntags;
  }

  std::cout << "No. of elements: " << nelements << " " << output->GetNumberOfCells() << std::endl;
  
  vtkSmartPointer<vtkIntArray> PhysicalIds;
  if (output->GetCellData()->HasArray("PhysicalIds")) {
    PhysicalIds = vtkIntArray::SafeDownCast(output->GetCellData()->GetArray("PhysicalIds"));
  } else {
    PhysicalIds = vtkSmartPointer<vtkIntArray>::New();
    output->Allocate(nelements);
    PhysicalIds->SetName("PhysicalIds");
    PhysicalIds->Allocate(nelements);
  }
  int eleNo, CellType,tagNo;
  for (vtkIdType i=0;i<nelements;i++){
    TriangleElementFile>> eleNo ;
    switch (npe)
      {
      case 2:
	// Line
	{
	  int cellPoints[2];
	  vtkIdType cellPointsVTK[2];
	  TriangleElementFile>> cellPoints[0] >> cellPoints[1];
	  for(int j=0;j<npe;j++){
	    cellPointsVTK[j]=cellPoints[j]-1;
	  }
	  output->InsertNextCell((int)3,(vtkIdType)2,cellPointsVTK);
	  break;
	}
      case 3:
	// Triangle
	{
	  int cellPoints[3];
	  vtkIdType cellPointsVTK[3];
	  TriangleElementFile>> cellPoints[0] >> cellPoints[1] >> cellPoints[2];
	  for(int j=0;j<3;j++){
	    cellPointsVTK[j]=cellPoints[j]-1;
	  }
	  output->InsertNextCell((int)5,(vtkIdType)3,cellPointsVTK);
	  break;
	}	
      case 4:
	// Tetrahedron
	{
	  int cellPoints[4];
	  vtkIdType cellPointsVTK[4];
	  TriangleElementFile>> cellPoints[0] >> cellPoints[1] >> cellPoints[2] >> cellPoints[3];
	  for(int j=0;j<4;j++){
	    cellPointsVTK[j]=cellPoints[j]-1;
	  }
	  output->InsertNextCell((int)10,(vtkIdType)4,cellPointsVTK);
	    break;
	}
      }
     for (int j=0;j<ntags;j++){
      int tag;
      TriangleElementFile >> tag;
      PhysicalIds->InsertNextValue(tag);
      }      
  }
  if (~output->GetCellData()->HasArray("PhysicalIds")) {
    output->GetCellData()->AddArray(PhysicalIds);
  }
  return 1;
}

int vtkTriangleReader::RequestData(
		      vtkInformation* vtkNotUsed(request),
		      vtkInformationVector **inputVector,
		      vtkInformationVector* outputVector )
{
  vtkInformation* outInfo=outputVector->GetInformationObject(0);
  vtkUnstructuredGrid* output= vtkUnstructuredGrid::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT() ) );

#ifndef NDEBUG
  this->DebugOn();
#endif

  // try to open the Triangle node file

  std::string basename=this->FileName;

  ifstream TriangleNodeFile;
  ifstream TriangleBoundaryFile;
  ifstream TriangleElementFile;
  TriangleNodeFile.open(basename+".node");

  vtkSmartPointer<vtkPoints> outpoints= vtkSmartPointer<vtkPoints>::New();
  // try to read the point data
  int dim = readTriangleNodes(TriangleNodeFile,outpoints);
  output->SetPoints(outpoints);

  TriangleNodeFile.close();

  int npe=dim+1;
  // deal with boundaries
  switch(dim) {
   case 2:
     TriangleBoundaryFile.open(basename+".edge");
     break;
   case 3:
     TriangleBoundaryFile.open(basename+".face");
     break;
  }
  if (TriangleBoundaryFile.is_open()) {
  std::cout << "Reading boundary data" << std::endl; 
  int npe=dim;
  readTriangleElements(TriangleBoundaryFile,output,npe);
}
  // try to open the Triangle element file

  TriangleElementFile.open(basename+".ele");

  // try to read the element data
  npe = 0;
  readTriangleElements(TriangleElementFile,output, npe);

  // close the file
  TriangleElementFile.close();

  return 1;
}

void vtkTriangleReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
 
  os << indent << "File Name: "
      << (this->FileName ? this->FileName : "(none)") << "\n";
}
