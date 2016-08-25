#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/TestCase.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TestRunner.h>
#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/XmlOutputter.h>

#include "vtkGmshReader.h"
#include "vtkGmshWriter.h"

using namespace CppUnit;

class TestMeshConverter : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestMeshConverter);
  CPPUNIT_TEST(testGmshReader);
  CPPUNIT_TEST(testGmshWriter);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp(void);
  void tearDown(void);

protected:
  void testNothing(void);
  void testGmshReader(void);
  void testGmshWriter(void);
  
private:

};

void TestMeshConverter::testNothing(void)
{
  CPPUNIT_ASSERT(1==1);
}

void TestMeshConverter::testGmshReader(void)
{
  vtkGmshReader* myinstance= vtkGmshReader::New();
  CPPUNIT_ASSERT(myinstance);
  myinstance->SetFileName("tests/gmsh_data.msh");
  myinstance->Update();
  CPPUNIT_ASSERT(myinstance->GetOutput()->GetNumberOfPoints() == 299);
  CPPUNIT_ASSERT(myinstance->GetOutput()->GetNumberOfCells() == 1527);
  CPPUNIT_ASSERT(myinstance->GetOutput());
}

void TestMeshConverter::testGmshWriter(void)
{
  vtkGmshWriter* myinstance = vtkGmshWriter::New();
  CPPUNIT_ASSERT(myinstance);
}


void TestMeshConverter::setUp(void) {};
void TestMeshConverter::tearDown(void) {};



CPPUNIT_TEST_SUITE_REGISTRATION( TestMeshConverter );

int main(int argc, char* argv[])
{    
  // informs test-listener about testresults
  CPPUNIT_NS::TestResult testresult;
  
  // register listener for collecting the test-results
  CPPUNIT_NS::TestResultCollector collectedresults;
  testresult.addListener (&collectedresults);
  
  // register listener for per-test progress output
  CPPUNIT_NS::BriefTestProgressListener progress;
  testresult.addListener (&progress);

  // insert test-suite at test-runner by registry

  CPPUNIT_NS::TestRunner testrunner;
  testrunner.addTest (CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest
		      ());
  testrunner.run(testresult);
  
  // output results in compiler-format
  CPPUNIT_NS::CompilerOutputter compileroutputter(&collectedresults, std::cerr
						  );
  compileroutputter.write ();

  // Output XML for Jenkins CPPunit plugin
  std::ofstream xmlFileOut("converter_test_results.xml");
  XmlOutputter xmlOut(&collectedresults, xmlFileOut);
  xmlOut.write();

  // return 0 if tests were successful
  return collectedresults.wasSuccessful() ? 0 : 1;

 }
