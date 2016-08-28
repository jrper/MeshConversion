#include "vtkDataWriter.h"
#include "vtkUnstructuredGrid.h"

class vtkTriangleWriter : public vtkDataWriter
{
public:
  static vtkTriangleWriter *New();
  //  vtkTypeRevisionMacro(vtkTriangleWriter,vtkDataWriter);
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  // Description:
  // Get the input to this writer.
  vtkUnstructuredGrid* GetInput();
  vtkUnstructuredGrid* GetInput(int port);

  void SetBinaryWriteMode(int isBinary);

  void PrintSelf(ostream& os, vtkIndent indent);
 protected:

  vtkTriangleWriter();
  ~vtkTriangleWriter();

  void WriteData();

  virtual int FillInputPortInformation(int,vtkInformation *info);

  char* FileName;
  int isBinary;
 private:
  vtkTriangleWriter(const vtkTriangleWriter&);  // Not implemented.
  void operator=(const vtkTriangleWriter&);  // Not implemented.

};

