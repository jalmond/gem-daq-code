#ifndef gem_hw_vfat_VFAT2Manager_h
#define gem_hw_vfat_VFAT2Manager_h

#include "gem/base/GEMFSMApplication.h"

#include "gem/hw/vfat/exception/Exception.h"



//typedef uhal::exception::exception uhalException;

namespace gem {
  namespace hw {
    namespace vfat {

      class HwVFAT2;
      class VFAT2ManagerWeb;
      class VFAT2Monitor;

      
      typedef std::shared_ptr<HwVFAT2>  vfat_shared_ptr;
      typedef std::shared_ptr<gem::base::utils::GEMInfoSpaceToolBox> is_toolbox_ptr;

      class VFAT2Manager: public gem::base::GEMFSMApplication
        {
	  
	  friend class VFAT2ManagerWeb;
	  //friend class VFAT2Monitor;

	  
        public:
          XDAQ_INSTANTIATOR();
	  
          VFAT2Manager(xdaq::ApplicationStub * s);

          ~VFAT2Manager();
	

	  	  
        protected:
          virtual void init();

          virtual void actionPerformed(xdata::Event& event);
	  
          //state transitions
          virtual void initializeAction() throw (gem::hw::vfat::exception::Exception);
          virtual void configureAction()  throw (gem::hw::vfat::exception::Exception);
          virtual void startAction()      throw (gem::hw::vfat::exception::Exception);
          virtual void pauseAction()      throw (gem::hw::vfat::exception::Exception);
          virtual void resumeAction()     throw (gem::hw::vfat::exception::Exception);
          virtual void stopAction()       throw (gem::hw::vfat::exception::Exception);
          virtual void haltAction()       throw (gem::hw::vfat::exception::Exception);
          virtual void resetAction()      throw (gem::hw::vfat::exception::Exception);
          //virtual void noAction()         throw (gem::hw::vfat::exception::Exception); 
	
          virtual void failAction(toolbox::Event::Reference e)
            throw (toolbox::fsm::exception::Exception); 
	
          virtual void resetAction(toolbox::Event::Reference e)
            throw (toolbox::fsm::exception::Exception);
	
        private:
	  uint16_t parseAMCEnableList(std::string const&);
	  bool     isValidSlotNumber( std::string const&);
          void     createVFTA2InfoSpaceItems(is_toolbox_ptr is_vfat, vfat_shared_ptr vfat);
          uint16_t m_amcEnableMask;

	  class VFATInfo {
	    
          public:
            VFATInfo();
            void registerFields(xdata::Bag<VFATManager::VFATInfo>* bag);

	    /// Need to find out what variables need adding here
            //monitoring information
            xdata::Boolean present;
            xdata::Integer crateID;
            xdata::Integer slotID;

            //configuration parameters
            xdata::String controlHubAddress;
            xdata::String deviceIPAddress;
            xdata::String ipBusProtocol;
            xdata::String addressTable;
            
            xdata::UnsignedInteger32 controlHubPort;
            xdata::UnsignedInteger32 ipBusPort;
            
            //registers to set
            xdata::Integer triggerSource;
            xdata::Integer sbitSource;            
            
            inline std::string toString() {
              // write obj to stream
              std::stringstream os;
              os << "present:" << present.toString() << std::endl
                 << "crateID:" << crateID.toString() << std::endl
                 << "slotID:"  << slotID.toString()  << std::endl
                
                 << "controlHubAddress:" << controlHubAddress.toString() << std::endl
                 << "deviceIPAddress:"   << deviceIPAddress.toString()   << std::endl
                 << "ipBusProtocol:"     << ipBusProtocol.toString()     << std::endl
                 << "addressTable:"      << addressTable.toString()      << std::endl
                 << "controlHubPort:"    << controlHubPort.value_        << std::endl
                 << "ipBusPort:"         << ipBusPort.value_             << std::endl
                 << "triggerSource:0x"   << std::hex << triggerSource.value_ << std::dec << std::endl
                 << "sbitSource:0x"      << std::hex << sbitSource.value_    << std::dec << std::endl
                 << std::endl;
              return os.str();
            };
          };


          std::vector<std::string>          nodes_;
	  
	  std::map<std::string, uint16_t> systemMap;
	  
          //xdata::UnsignedLong myParameter_;
          xdata::String device_;
          xdata::String ipAddr_;
          xdata::String settingsFile_;
	  
          /// Copied form GLIBManager.h check if needed
	  mutable gem::utils::Lock m_deviceLock;//[MAX_AMCS_PER_CRATE];
	  
          vfat_shared_ptr              m_vfats[MAX_AMCS_PER_CRATE];
          std::shared_ptr<VFAT2Monitor> m_vfatMonitors[MAX_AMCS_PER_CRATE];
          //xdata::InfoSpace*            is_vfats[MAX_AMCS_PER_CRATE];
          is_toolbox_ptr               is_vfats[MAX_AMCS_PER_CRATE];
          xdata::Vector<xdata::Bag<VFATInfo> > m_vfatInfo;//[MAX_AMCS_PER_CRATE];
          xdata::String        m_amcSlots;
          xdata::String        m_connectionFile;
        }; //end class VFATManager
            
    }//end namespace gem::hw::vfat
  }//end namespace gem::hw
}//end namespace gem

#endif

