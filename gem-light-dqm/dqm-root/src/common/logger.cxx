#include <string>
#include <iostream>
#include <sstream>
class logger{
  public:
    logger(std::string m_filename, std::string m_run){
      filename_ = m_filename;
      run_ = m_run;
      nEvents_ = 0;
      nGoodEvents_ = 0;
      nBadEvents_ = 0;
      nVFATBlocks_ = 0;
      nGoodVFATBlocks_ = 0;
      nBadVFATBlocks_ = 0;
      averageBlockPerEvent_ = 0;
      averageBadBlockPerBadEvent_ = 0;
      nNonEmptyResponses_ = 0;
      badEventsList_.clear();
      logReady_ = false;
     }
    void addEvent(long int m_eventNumber, bool m_isEventGood, int m_nVFATBlocks, int m_nGoodVFATBlocks, int m_nBadVFATBlocks){
      nEvents_++;
      if (m_isEventGood){ nGoodEvents_++;} else { nBadEvents_++; badEventsList_.push_back(m_eventNumber);}
      nVFATBlocks_ += m_nVFATBlocks;
      nGoodVFATBlocks_ += m_nGoodVFATBlocks;
      nBadVFATBlocks_ += m_nBadVFATBlocks;
    }
    void addResponseEfficiency(int nNonEmptyResponses, int nEmptyResponses){
      nNonEmptyResponses_ = nNonEmptyResponses;
      nEmptyResponses_ = nEmptyResponses;
    }
    void writeLog(){
      averageBlockPerEvent_ = (double) nVFATBlocks_/nEvents_;
      averageBadBlockPerBadEvent_ = (double) nBadVFATBlocks_/nBadEvents_;
      responseEfficiency_ = (double) nNonEmptyResponses_/(nNonEmptyResponses_+nEmptyResponses_);
      responseInefficiency_ = (double) nEmptyResponses_/(nNonEmptyResponses_+nEmptyResponses_);
      responseEfficiency_ *=100;
      responseInefficiency_ *=100;
      ge_fraction_ = (double) nGoodEvents_/nEvents_;
      ge_fraction_ *=100;
      ofstream myfile;
      myfile.open (filename_);
      myfile << "GEM Log For Run " << run_ << "\n" ;
      myfile << "Number of events processed     : " << nEvents_ << "\n" ;
      myfile << "Number of good events          : " << nGoodEvents_ << "\n" ;
      myfile << "Number of bad events           : " << nBadEvents_ << "\n" ;
      myfile << "Fraction of good events        : " << setprecision(4) << ge_fraction_ << "%\n" ;
      myfile << "Number of VFAT blocks          : " << nVFATBlocks_ << "\n" ;
      myfile << "Number of good VFAT blocks     : " << nGoodVFATBlocks_ << "\n" ;
      myfile << "Number of bad VFAT blocks      : " << nBadVFATBlocks_ << "\n" ;
      myfile << "Average N blocks per event     : " << averageBlockPerEvent_ << "\n" ;
      myfile << "Average N bad blocks per event : " << averageBadBlockPerBadEvent_ << "\n" ;
      myfile << "========Good Events Only========\n" ;
      myfile << "Detector response efficiency   : " << setprecision(4) << responseEfficiency_ << "%\n" ;
      myfile << "Detector response inefficiency : " << setprecision(4) << responseInefficiency_ << "%\n" ;
      if (!badEventsList_.empty()) {
        std::string tmp = "{ ";
        for (int it = 0; it < (badEventsList_.size() - 1); it++){
          std::stringstream ss;
          ss << badEventsList_.at(it);
          tmp += ss.str();
          tmp += ", ";
        }
        std::stringstream ss;
        ss << badEventsList_.back();
        tmp += ss.str();
        tmp += " }";
        myfile << "List of the bad events : \n" << tmp << "\n" ;
      } else {
        myfile << "No bad events found \n" ;
      }
      myfile.close();
      logReady_ = true;
    }
    void printLog(){
      if (logReady_) {
        std::cout << "GEM Log For Run " << run_ << "\n" ;
        std::cout << "Number of events processed     : " << nEvents_ << "\n" ;
        std::cout << "Number of good events          : " << nGoodEvents_ << "\n" ;
        std::cout << "Number of bad events           : " << nBadEvents_ << "\n" ;
        std::cout << "Fraction of good events        : " << setprecision(4) << ge_fraction_ << "%\n" ;
        std::cout << "Number of VFAT blocks          : " << nVFATBlocks_ << "\n" ;
        std::cout << "Number of good VFAT blocks     : " << nGoodVFATBlocks_ << "\n" ;
        std::cout << "Number of bad VFAT blocks      : " << nBadVFATBlocks_ << "\n" ;
        std::cout << "Average N blocks per event     : " << averageBlockPerEvent_ << "\n" ;
        std::cout << "Average N bad blocks per event : " << averageBadBlockPerBadEvent_ << "\n" ;
        std::cout << "========Good Events Only========\n" ;
        std::cout << "Detector response efficiency   : " << setprecision(4) << responseEfficiency_ << "%\n" ;
        std::cout << "Detector response inefficiency : " << setprecision(4) << responseInefficiency_ << "%\n" ;

      } else {
        std::cout << " Run log is not ready " << std::endl;
      }
    }
    ~logger(){}
  private:
    std::string filename_;
    std::string run_;
    long int nEvents_;
    long int nGoodEvents_;
    long int nBadEvents_;
    long long int nVFATBlocks_;
    long long int nGoodVFATBlocks_;
    long long int nBadVFATBlocks_;
    double averageBlockPerEvent_;
    double averageBadBlockPerBadEvent_;
    std::vector<long int> badEventsList_;
    int nNonEmptyResponses_;
    int nEmptyResponses_;
    double responseEfficiency_;
    double responseInefficiency_;
    double ge_fraction_;
    bool logReady_;
};
