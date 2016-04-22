//#include "gemtreewriter/src/common/gemTreeWriter.cxx"
//#include "dqm-root/src/common/treeReader.cxx"
#include "dqm-root/src/common/gemTreeReader.cxx"
#include "dqm-root/src/common/GEMClusterization/GEMStrip.cc"
#include "dqm-root/src/common/GEMClusterization/GEMCluster.cc"
#include "dqm-root/src/common/GEMClusterization/GEMClusterizer.cc"
#include "TApplication.h"
#define DEBUG 1


using namespace std;
int main(int argc, char** argv)
{

  if (argc<2) 
    {
      cout << "Please provide input filenames" << endl;
      cout << "Usage: <path>/rundqm inputFile.root" << endl;
      return 0;
    }
  string m_root_filename = argv[1];
  // string m_slot_file = argv[2];
  bool print_hist = false;
  if (argc > 2){
    string option = argv[2];
    if ((option == "--with-print") || (option == "-p")) {
      print_hist = true;
      std::cout << "[MAIN]: will print histograms" << std::endl;
    }
  }

  string m_raw_filename = m_root_filename;

  std::cout << "[MAIN]: Beginning DQM treeReader" << std::endl;
  std::cout << "[MAIN]: Input file name: " << m_raw_filename << std::endl;
  treeReader *reader = new treeReader(m_raw_filename);
  std::cout << "[MAIN]: DQM analysis done" << std::endl;

  return 0;


}
