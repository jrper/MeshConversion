#include "vtkUnstructuredGridAlgorithm.h"
#include "vtkSetGet.h"

class vtkTriangleReader : public vtkUnstructuredGridAlgorithm
{
public:
  static vtkTriangleReader *New();
  //  vtkTypeRevisionMacro(vtkTriangleReader,vtkUnstructuredGridAlgorithm);

  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);
  void PrintSelf(ostream& os, vtkIndent indent);
 protected:

  vtkTriangleReader();
  ~vtkTriangleReader();

  virtual int RequestData(
			  vtkInformation* request,
			  vtkInformationVector** InputVector,
			  vtkInformationVector* outputVector);

  char* FileName;

 private:
  vtkTriangleReader(const vtkTriangleReader&);  // Not implemented.
  void operator=(const vtkTriangleReader&);  // Not implemented.

};
