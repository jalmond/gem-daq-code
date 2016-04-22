//#include "gemtreewriter/src/common/gemTreeWriter.cxx"
#include "dqm-root/src/common/treeReader.cxx"
#include "dqm-root/src/common/GEMClusterization/GEMStrip.cc"
#include "dqm-root/src/common/GEMClusterization/GEMCluster.cc"
#include "dqm-root/src/common/GEMClusterization/GEMClusterizer.cc"

#include "TApplication.h"
using namespace std;
int main(int argc, char** argv)
{
//  if (argc<3) 
//  {
//      cout << "Please provide input filenames" << endl;
//      cout << "Usage: <path>/rundqm inputFile.dat slot_file.csv" << endl;
//      return 0;
//  }
//  string m_dat_filename = argv[1];
//  string m_slot_file = argv[2];
//  bool print_hist = false;
//  if (argc > 3){
//    string option = argv[3];
//    if ((option == "--with-print") || (option == "-p")) {
//      print_hist = true;
//      std::cout << "[MAIN]: will print histograms" << std::endl;
//    }
//  }
//
  string m_raw_filename=argv[1]; //before commit comment this and next line, uncomment all the commented ones
#ifndef __CINT__
    TApplication App("App", &argc, argv);
#endif
//  std::cout << "[MAIN]: Slot table : " << m_slot_file << std::endl;
//  gemTreeWriter *m_gemTreeWriter = new gemTreeWriter();
//  m_gemTreeWriter->makeTree(m_dat_filename, m_slot_file);
//  string m_raw_filename;
//  m_raw_filename = m_gemTreeWriter->getOutputFileName();
  std::cout << "[MAIN]: ROOT TTree is finished" << std::endl;
  std::cout << "Raw file name " << m_raw_filename << std::endl;
  gemTreeReader *m_gemTreeReader = new gemTreeReader(m_raw_filename, true);
  //gemTreeReader *m_gemTreeReader = new gemTreeReader(m_raw_filename, print_hist);
  m_gemTreeReader->createHistograms();
  std::cout << "[MAIN]: DQM analysis done" << std::endl;

  return 0;
}
