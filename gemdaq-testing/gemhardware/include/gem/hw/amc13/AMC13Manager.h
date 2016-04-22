#ifndef gem_hw_amc13_AMC13Manager_h
#define gem_hw_amc13_AMC13Manager_h

//copying general structure of the HCAL DTCManager (HCAL name for AMC13)
#include "uhal/uhal.hpp"

#include "gem/base/GEMFSMApplication.h"
#include "gem/hw/amc13/exception/Exception.h"
//#include "gem/hw/amc13/AMC13Monitoring.hh"

namespace amc13 {
  class AMC13;
  class Status;
}

namespace gem {
  namespace hw {
    namespace amc13 {
      
      class AMC13ManagerWeb;
      
      class AMC13Manager : public gem::base::GEMFSMApplication
        {
	  
          friend class AMC13ManagerWeb;

        public:
          XDAQ_INSTANTIATOR();
	  
          AMC13Manager(xdaq::ApplicationStub * s);

          virtual ~AMC13Manager();
	  
        protected:
          virtual void init();

          virtual void actionPerformed(xdata::Event& event);
          
          ::amc13::Status *getHTMLStatus()  const;
          ::amc13::AMC13  *getAMC13Device() const { return p_amc13; };

          //state transitions
          virtual void initializeAction() throw (gem::hw::amc13::exception::Exception);
          virtual void configureAction()  throw (gem::hw::amc13::exception::Exception);
          virtual void startAction()      throw (gem::hw::amc13::exception::Exception);
          virtual void pauseAction()      throw (gem::hw::amc13::exception::Exception);
          virtual void resumeAction()     throw (gem::hw::amc13::exception::Exception);
          virtual void stopAction()       throw (gem::hw::amc13::exception::Exception);
          virtual void haltAction()       throw (gem::hw::amc13::exception::Exception);
          virtual void resetAction()      throw (gem::hw::amc13::exception::Exception);
	  virtual xoap::MessageReference sendTriggerBurst(xoap::MessageReference mns)
	    throw (xoap::exception::Exception);
          //virtual void noAction()         throw (gem::hw::amc13::exception::Exception); 
	
          virtual void failAction(toolbox::Event::Reference e)
            throw (toolbox::fsm::exception::Exception); 
	
          virtual void resetAction(toolbox::Event::Reference e)
            throw (toolbox::fsm::exception::Exception);
          
          class AMC13Info 
          {   
          public:
            void registerFields(xdata::Bag<AMC13Info> *bag);
            
            xdata::String connectionFile;
	    xdata::String amc13CardName;
            xdata::String amcInputEnableList;
            xdata::String amcIgnoreTTSList;
            
            xdata::Boolean enableDAQLink;
            xdata::Boolean enableFakeData;
            xdata::Boolean monBackPressure;
            xdata::Boolean enableLocalTTC;
	    xdata::Boolean enableLocalL1A;
	    xdata::UnsignedInteger32 internalPeriodicPeriod;
	    xdata::Integer l1Amode;
	    xdata::Integer l1Arules;
	    xdata::UnsignedInteger32 l1Aburst;
	    xdata::Boolean sendl1ATriburst;
	    xdata::Boolean startl1ATricont;

	    xdata::Boolean enableCalpulse;
            // can configure up to 4 BGO channels
            xdata::Integer           bgochannel;
            xdata::UnsignedInteger32 bgocmd;
            xdata::UnsignedInteger32 bgobx;
            xdata::UnsignedInteger32 bgoprescale;
	    xdata::Boolean           bgorepeat;
	    xdata::Boolean           bgolong;

            xdata::Integer prescaleFactor;
            xdata::Integer bcOffset;

            xdata::UnsignedInteger32 fedID;
            xdata::UnsignedInteger32 sfpMask;
            xdata::UnsignedInteger32 slotMask;

            xdata::UnsignedInteger64 localL1AMask;
          };
          
        private:
          mutable gem::utils::Lock m_amc13Lock;
	
          ::amc13::AMC13 *p_amc13;
	  //hcal::utca::DTCMonitoring m_monitoringHelper; to be developed!!!

          //paramters taken from hcal::DTCManager (the amc13 manager for hcal)
          xdata::Integer m_crateID, m_slot;

          xdata::Bag<AMC13Info> m_amc13Params;
          //seems that we've duplicated the members of the m_amc13Params as class variables themselves
          //what is the reason for this?  is it necessary/better to have these variables?
          std::string m_connectionFile, m_amcInputEnableList, m_slotEnableList, m_amcIgnoreTTSList, m_cardname;
          bool m_enableDAQLink, m_enableFakeData;
          bool m_monBackPressEnable, m_megaMonitorScale;
          bool m_enableLocalTTC, m_ignoreAMCTTS, m_enableLocalL1A, m_sendL1ATriburst, m_startL1ATricont,
            m_enableCalpulse, m_bgorepeat, m_bgolong;
          int m_localTriggerMode, m_localTriggerPeriod, m_localTriggerRate, m_L1Amode, m_L1Arules;
          int m_prescaleFactor, m_bcOffset, m_bgochannel;
	  uint8_t m_bgocmd;
	  uint16_t m_bgobx, m_bgoprescale;
          uint32_t m_fedID, m_sfpMask, m_slotMask, m_internalPeriodicPeriod, m_L1Aburst;
          uint64_t m_localL1AMask;
	  
          ////counters

        protected:
	  
        }; //end class AMC13Manager

    }//end namespace gem::hw::amc13
  }//end namespace gem::hw
}//end namespace gem

#endif
