#ifndef gem_hw_optohybrid_HwOPTOHYBRID_h
#define gem_hw_optohybrid_HwOPTOHYBRID_h

#include "gem/hw/GEMHwDevice.h"
#include "gem/hw/glib/HwGLIB.h"

#include "gem/hw/optohybrid/exception/Exception.h"
//#include "gem/hw/optohybrid/OptoHybridMonitor.h"

#define MAX_VFATS 24

namespace gem {
  namespace hw {
    namespace optohybrid {

      //class OptoHybridMonitor;
      class HwOptoHybrid: public gem::hw::GEMHwDevice
	{
	public:
	  HwOptoHybrid();
	  HwOptoHybrid(gem::hw::glib::HwGLIB const& glib, int const& slot);

	  ~HwOptoHybrid();
	
	  //virtual void connectDevice();
	  //virtual void releaseDevice();
	  //virtual void initDevice();
	  //virtual void enableDevice();
	  virtual void configureDevice();
	  virtual void configureDevice(std::string const& xmlSettings);
	  //virtual void configureDevice(std::string const& dbConnectionString);
	  //virtual void disableDevice();
	  //virtual void pauseDevice();
	  //virtual void startDevice();
	  //virtual void stopDevice();
	  //virtual void resumeDevice();
	  //virtual void haltDevice();

	  virtual bool isHwConnected();

	  /** Read the board ID registers
	   * @returns the OptoHybrid board ID 
	   **/
	  //std::string getBoardID()   const;

	  /** Read the firmware register using m_controlLink
	   * @returns a hex number corresponding to the build date
	   **/

	  uint32_t getFirmware() {
	    return getFirmware(m_controlLink);
	  };
	    
	  /** Read the firmware register for a given link
	   * @returns a hex number corresponding to the build date
	   **/

	  uint32_t getFirmware(uint8_t const& link) {
	    std::stringstream regName;
	    regName << "OptoHybrid_LINKS.LINK" << (int)link << ".FIRMWARE";
	    uint32_t fwver = readReg(getDeviceBaseNode(),regName.str());
	    INFO("OH link" << (int)link << " has firmware version 0x" 
		 << std::hex << fwver << std::dec);
	    return fwver;
	  };
	    
	  /** Read the firmware register
	   * @returns a string corresponding to the build date
	   **/
	  std::string getFirmwareDate(uint8_t const& link) {
	    std::stringstream retval;
	    retval << "0x" << std::hex << getFirmware(link) << std::dec << std::endl;
	    return retval.str();
	  };
	  	  
	  /** Read the link status registers, store the information in a struct
	   * @param uint8_t link is the number of the link to query
	   * @retval _status a struct containing the status bits of the optical link
	   * @throws gem::hw::optohybrid::exception::InvalidLink if the link number is outside of 0-2
	   **/
	  GEMHwDevice::OpticalLinkStatus LinkStatus(uint8_t const& link) ;

	  /** Reset the link status registers
	   * @param uint8_t link is the number of the link to query
	   * @param uint8_t resets control which bits to reset
	   * 0x00
	   * bit 1 - ErrCnt      0x01
	   * bit 2 - VFATI2CRec  0x02
	   * bit 3 - VFATI2CSnt  0x04
	   * bit 4 - RegisterRec 0x08
	   * bit 5 - RegisterSnt 0x10
	   * @throws gem::hw::optohybrid::exception::InvalidLink if the link number is outside of 0-2
	   **/
	  void LinkReset(uint8_t const& link, uint8_t const& resets);

	  /** Reset the all link status registers
	   * @param uint8_t resets control which bits to reset
	   **/
	  void ResetLinks(uint8_t const& resets) {
	    for (auto link = activeLinks.begin(); link != activeLinks.end(); ++link)
	      LinkReset(link->first,resets);
	  };
	  
	  /** Read the trigger data
	   * @retval uint32_t returns 32 bits 6 bits for s-bits and 26 for bunch countrr
	   **/
	  //uint32_t readTriggerData();

	  //Clocking
	  /** Setup the VFAT clock 
	   * @param bool source true uses the external clock, false uses the onboard clock
	   * @param bool fallback uses the external clock, false uses the onboard clock
	   **/
	  void SetVFATClock(bool source, bool fallback, uint8_t const& link=0x0) {
	    std::stringstream regName;
	    regName << "OptoHybrid_LINKS.LINK" << (int)m_controlLink;
	    writeReg(getDeviceBaseNode(),regName.str()+".CLOCKING.VFAT.SOURCE"  ,(uint32_t)source  );
	    writeReg(getDeviceBaseNode(),regName.str()+".CLOCKING.VFAT.FALLBACK",(uint32_t)fallback);
	  };
	  /** VFAT clock status
	   * @param bool source true uses the external clock, false uses the onboard clock
	   * @param bool fallback uses the external clock, false uses the onboard clock
	   **/
	  std::pair<bool,bool> StatusVFATClock(uint8_t const& link=0x0) {
	    std::stringstream regName;
	    regName << "OptoHybrid_LINKS.LINK" << (int)m_controlLink;
	    uint32_t src = readReg(getDeviceBaseNode(),regName.str()+".CLOCKING.VFAT.SOURCE");
	    uint32_t flb = readReg(getDeviceBaseNode(),regName.str()+".CLOCKING.VFAT.FALLBACK");
	    //maybe do a check to ensure that the value has been read properly?
	    return std::make_pair(src,flb);
	  };

	  /** Setup the CDCE clock 
	   * @param bool source true uses the external clock, false uses the onboard clock
	   * @param bool fallback uses the external clock, false uses the onboard clock
	   **/
	  void SetCDCEClock(bool source, bool fallback, uint8_t const& link=0x0) {
	    std::stringstream regName;
	    regName << "OptoHybrid_LINKS.LINK" << (int)m_controlLink;
	    writeReg(getDeviceBaseNode(),regName.str()+".CLOCKING.CDCE.SOURCE"  ,(uint32_t)source  );
	    writeReg(getDeviceBaseNode(),regName.str()+".CLOCKING.CDCE.FALLBACK",(uint32_t)fallback);
	  };
      
	  /** CDCE clock status
	   * @param bool source true uses the external clock, false uses the onboard clock
	   * @param bool fallback uses the external clock, false uses the onboard clock
	   **/
	  std::pair<bool,bool> StatusCDCEClock(uint8_t link) {
	    std::stringstream regName;
	    regName << "OptoHybrid_LINKS.LINK" << (int)m_controlLink;
	    uint32_t src = readReg(getDeviceBaseNode(),regName.str()+".CLOCKING.CDCE.SOURCE");
	    uint32_t flb = readReg(getDeviceBaseNode(),regName.str()+".CLOCKING.CDCE.FALLBACK");
	    //maybe do a check to ensure that the value has been read properly?
	    return std::make_pair(src,flb);
	  };

	  ///** Read the VFAT clock source
	  // * @retval bool true uses the external clock, false uses the onboard clock
	  // **/
	  //bool getVFATClockSource();
	  //
	  //
	  ///** Set the VFAT onboard clock fallback
	  // * Allows the clock to fall back to 
	  // * @param bool true allows the optohybrid to switch back to the onboard clock
	  // * if it fails to lock
	  // **/
	  //void setVFATFallback(bool);
	  //
	  ///** Read the VFAT clock fallback status
	  // * @retval returns status of VFAT clock fallback (false does not allow automatic
	  // * fallback to onboard clock
	  // **/
	  //bool getVFATFallback();
	  //
	  //
	  ///** Set the CDCE clock source
	  // * @param bool true uses the external clock, false uses the onboard clock
	  // **/
	  //void setCDCEClockSource(bool);
	  //
	  ///** Read the CDCE clock source
	  // * @retval bool true uses the external clock, false uses the onboard clock
	  // **/
	  //bool getCDCEClockSource();
	  //
	  ///** Set the CDCE clock fallback mode
	  // * @param bool true allows the optohybrid to switch back to the onboard clock if the
	  // * CDCE doesn't lock
	  // **/
	  //void setCDCEFallback(bool);
	  //
	  ///** Read the CDCE clock fallback mode
	  // * @retval bool false does not allow the optohybrid to switch the clock back
	  // **/
	  //bool getCDCEFallback();


	  /** Set the Trigger source
	   * @param uint8_t mode 0 from GLIB, 1 from external (LEMO), 2 from both
	   **/
	  void setTrigSource(uint8_t const& mode, uint8_t const& link=0x0) {
	    std::stringstream regName;
	    regName << "OptoHybrid_LINKS.LINK" << (int)m_controlLink;
	    switch (mode) {
	    case(0):
	      writeReg(getDeviceBaseNode(),regName.str()+".TRIGGER.SOURCE",mode);
	      return;
	    case(1):
	      writeReg(getDeviceBaseNode(),regName.str()+".TRIGGER.SOURCE",mode);
	      return;
	    case(2):
	      writeReg(getDeviceBaseNode(),regName.str()+".TRIGGER.SOURCE",mode);
	      return;
	    default:
	      writeReg(getDeviceBaseNode(),regName.str()+".TRIGGER.SOURCE",0x2);
	      return;
	    }
	  };

	  /** Read the Trigger source
	   * @retval uint8_t 0 from GLIB, 1 from external, 2 from both
	   **/
	  uint8_t getTrigSource(uint8_t const& link=0x0) { 
	    std::stringstream regName;
	    regName << "OptoHybrid_LINKS.LINK" << (int)m_controlLink;
	    return readReg(getDeviceBaseNode(),regName.str()+".TRIGGER.SOURCE"); };


	  /** Set the S-bit source
	   * @param uint8_t chip
	   **/
	  void setSBitSource(uint8_t const& mode, uint8_t const& link=0x0) {
	    std::stringstream regName;
	    regName << "OptoHybrid_LINKS.LINK" << (int)m_controlLink;
	    writeReg(getDeviceBaseNode(),regName.str()+".TRIGGER.TDC.SBits",mode); };

	  /** Read the S-bit source
	   * @retval uint8_t which VFAT chip is sending the S-bits
	   **/
	  uint8_t getSBitSource(uint8_t const& link=0x0) {
	    std::stringstream regName;
	    regName << "OptoHybrid_LINKS.LINK" << (int)m_controlLink;
	    return readReg(getDeviceBaseNode(),regName.str()+".TRIGGER.TDC_SBits"); };


	  /* Generate and send specific T1 commands on the OptoHybrid */
	  /** Send an internal L1A
	   * @param uint64_t ntrigs, how many L1As to send
	   **/
	  void SendL1A(uint64_t ntrigs, uint8_t const& link=0x0) {
	    std::stringstream regName;
	    regName << "OptoHybrid_LINKS.LINK" << (int)m_controlLink;
	    for (uint64_t i = 0; i < ntrigs; ++i) 
	      writeReg(getDeviceBaseNode(),regName.str()+".FAST_COM.Send.L1A",0x1);
	  };

	  /** Send an internal CalPulse
	   * @param uint64_t npulse, how many CalPulses to send
	   **/
	  void SendCalPulse(uint64_t npulse, uint8_t const& link=0x0) {
	    std::stringstream regName;
	    regName << "OptoHybrid_LINKS.LINK" << (int)m_controlLink;
	    for (uint64_t i = 0; i < npulse; ++i) 
	      writeReg(getDeviceBaseNode(),regName.str()+".FAST_COM.Send.CalPulse",0x1);
	  };

	  /** Send an internal L1A and CalPulse
	   * @param uint64_t npulse, how many pairs to send
	   * @param uint32_t delay, how long between L1A and CalPulse
	   **/
	  void SendL1ACal(uint64_t npulse, uint32_t delay, uint8_t const& link=0x0) {
	    std::stringstream regName;
	    regName << "OptoHybrid_LINKS.LINK" << (int)m_controlLink;
	    for (uint64_t i = 0; i < npulse; ++i) 
	      writeReg(getDeviceBaseNode(),regName.str()+".FAST_COM.Send.L1ACalPulse",delay);
	  };

	  /** Send an internal Resync
	   * 
	   **/
	  void SendResync(uint8_t const& link=0x0) {
	    std::stringstream regName;
	    regName << "OptoHybrid_LINKS.LINK" << (int)m_controlLink;
	    writeReg(getDeviceBaseNode(),regName.str()+".FAST_COM.Send.Resync",0x1); };


	  /** Send an internal BC0
	   * 
	   **/
	  void SendBC0(uint8_t const& link=0x0) {
	    std::stringstream regName;
	    regName << "OptoHybrid_LINKS.LINK" << (int)m_controlLink;
	    writeReg(getDeviceBaseNode(),regName.str()+".FAST_COM.Send.BC0",0x1); };

	  ///Counters
	  /** Get the recorded number of L1A signals
	   * @param mode specifies which L1A counter to read
	   * 0 external
	   * 1 internal
	   * 2 delayed (sent along with a CalPulse)
	   * 3 total
	   **/
	  uint32_t GetL1ACount(uint8_t const& mode, uint8_t const& link=0x0) {
	    std::stringstream regName;
	    regName << "OptoHybrid_LINKS.LINK" << (int)m_controlLink;
	    switch(mode) {
	    case 0:
	      return readReg(getDeviceBaseNode(),regName.str()+".COUNTERS.L1A.External");
	    case 1:
	      return readReg(getDeviceBaseNode(),regName.str()+".COUNTERS.L1A.Internal");
	    case 2:
	      return readReg(getDeviceBaseNode(),regName.str()+".COUNTERS.L1A.Delayed");
	    case 3:
	      return readReg(getDeviceBaseNode(),regName.str()+".COUNTERS.L1A.Total");
	    default:
	      return readReg(getDeviceBaseNode(),regName.str()+".COUNTERS.L1A.Total");
	    }
	  };
	  
	  /** Get the recorded number of CalPulse signals
	   * @param mode specifies which CalPulse counter to read
	   * 0 internal
	   * 1 delayed (sent along with a L1A)
	   * 2 total
	   **/
	  uint32_t GetCalPulseCount(uint8_t const& mode, uint8_t const& link=0x0) {
	    std::stringstream regName;
	    regName << "OptoHybrid_LINKS.LINK" << (int)m_controlLink;
	    switch(mode) {
	    case 0:
 	      return readReg(getDeviceBaseNode(),regName.str()+".COUNTERS.CalPulse.Internal");
	    case 1:
	      return readReg(getDeviceBaseNode(),regName.str()+".COUNTERS.CalPulse.Delayed");
	    case 2:
	      return readReg(getDeviceBaseNode(),regName.str()+".COUNTERS.CalPulse.Total");
	    default:
	      return readReg(getDeviceBaseNode(),regName.str()+".COUNTERS.CalPulse.Total");
	    }
	  };
	  
	  /** Get the recorded number of Resync signals
	   **/
	  uint32_t GetResyncCount(uint8_t const& link=0x0) {
	    std::stringstream regName;
	    regName << "OptoHybrid_LINKS.LINK" << (int)m_controlLink;
	    return readReg(getDeviceBaseNode(),regName.str()+".COUNTERS.Resync"); };

	  /** Get the recorded number of BC0 signals
	   **/
	  uint32_t GetBC0Count(uint8_t const& link=0x0) {
	    std::stringstream regName;
	    regName << "OptoHybrid_LINKS.LINK" << (int)m_controlLink;
	    return readReg(getDeviceBaseNode(),regName.str()+".COUNTERS.BC0"); };

	  /** Get the recorded number of BXCount signals
	   **/
	  uint32_t GetBXCountCount(uint8_t const& link=0x0) {
	    std::stringstream regName;
	    regName << "OptoHybrid_LINKS.LINK" << (int)m_controlLink;
	    return readReg(getDeviceBaseNode(),regName.str()+".COUNTERS.BXCount"); };
	  
	  ///Resets
	  /** Reset recorded number of L1A signals
	   * @param mode specifies which L1A counter to reset
	   * 0 none
	   * 1 external
	   * 2 internal
	   * 3 delayed (sent along with a CalPulse)
	   * 4 total (all)
	   **/
	  void ResetL1ACount(uint8_t const& mode) {
	    return ResetL1ACount(mode,m_controlLink); };
	  void ResetL1ACount(uint8_t const& mode, uint8_t const& link) {
	    std::stringstream regName;
	    regName << "OptoHybrid_LINKS.LINK" << (int)m_controlLink;
	    switch(mode) {
	    case 0:
	      return;
	    case 1:
	      return writeReg(getDeviceBaseNode(),regName.str()+".COUNTERS.RESETS.L1A.External", 0x1);
	    case 2:
	      return writeReg(getDeviceBaseNode(),regName.str()+".COUNTERS.RESETS.L1A.Internal", 0x1);
	    case 3:
	      return writeReg(getDeviceBaseNode(),regName.str()+".COUNTERS.RESETS.L1A.Delayed", 0x1);
	    case 4:
	      writeReg(getDeviceBaseNode(),regName.str()+".COUNTERS.RESETS.L1A.External", 0x1);
	      writeReg(getDeviceBaseNode(),regName.str()+".COUNTERS.RESETS.L1A.Internal", 0x1);
	      writeReg(getDeviceBaseNode(),regName.str()+".COUNTERS.RESETS.L1A.Delayed",  0x1);
	      writeReg(getDeviceBaseNode(),regName.str()+".COUNTERS.RESETS.L1A.Total",    0x1);
	      return;
	    default:
	      writeReg(getDeviceBaseNode(),regName.str()+".COUNTERS.RESETS.L1A.External", 0x1);
	      writeReg(getDeviceBaseNode(),regName.str()+".COUNTERS.RESETS.L1A.Internal", 0x1);
	      writeReg(getDeviceBaseNode(),regName.str()+".COUNTERS.RESETS.L1A.Delayed",  0x1);
	      writeReg(getDeviceBaseNode(),regName.str()+".COUNTERS.RESETS.L1A.Total",    0x1);
	      return;
	    }
	  };
	  
	  /** Get the recorded number of CalPulse signals
	   * @param mode specifies which CalPulse counter to reset
	   * 0 none
	   * 1 internal
	   * 2 delayed (sent along with a CalPulse)
	   * 3 total (all)
	   **/
	  void ResetCalPulseCount(uint8_t const& mode) {
	    return ResetCalPulseCount(mode,m_controlLink); };

	  void ResetCalPulseCount(uint8_t const& mode, uint8_t const& link) {
	    std::stringstream regName;
	    regName << "OptoHybrid_LINKS.LINK" << (int)m_controlLink;
	    switch(mode) {
	    case 0:
	    case 1:
	      writeReg(getDeviceBaseNode(),regName.str()+".COUNTERS.RESETS.CalPulse.Internal", 0x1);
	      return;
	    case 2:
	      writeReg(getDeviceBaseNode(),regName.str()+".COUNTERS.RESETS.CalPulse.Delayed", 0x1);
	      return;
	    case 3:
	      writeReg(getDeviceBaseNode(),regName.str()+".COUNTERS.RESETS.CalPulse.Internal", 0x1);
	      writeReg(getDeviceBaseNode(),regName.str()+".COUNTERS.RESETS.CalPulse.Delayed",  0x1);
	      writeReg(getDeviceBaseNode(),regName.str()+".COUNTERS.RESETS.CalPulse.Total",    0x1);
	      return;
	    default:
	      writeReg(getDeviceBaseNode(),regName.str()+".COUNTERS.RESETS.CalPulse.Internal", 0x1);
	      writeReg(getDeviceBaseNode(),regName.str()+".COUNTERS.RESETS.CalPulse.Delayed",  0x1);
	      writeReg(getDeviceBaseNode(),regName.str()+".COUNTERS.RESETS.CalPulse.Total",    0x1);
	      return;
	    }
	  };
	  
	  /** Get the recorded number of Resync signals
	   **/
	  void ResetResyncCount() {
	    return ResetResyncCount(m_controlLink); };
	  void ResetResyncCount(uint8_t const& link) {
	    std::stringstream regName;
	    regName << "OptoHybrid_LINKS.LINK" << (int)m_controlLink;
	    return writeReg(getDeviceBaseNode(),regName.str()+".COUNTERS.RESETS.Resync", 0x1); };

	  /** Get the recorded number of BC0 signals
	   **/
	  void ResetBC0Count() {
	    return ResetBC0Count(m_controlLink); };
	  void ResetBC0Count(uint8_t const& link) {
	    std::stringstream regName;
	    regName << "OptoHybrid_LINKS.LINK" << (int)m_controlLink;
	    return writeReg(getDeviceBaseNode(),regName.str()+".COUNTERS.RESETS.BC0", 0x1); };

	  /** Get the recorded number of BXCount signals
	   **/
	  void ResetBXCount() {
	    return ResetBXCount(m_controlLink); };
	  void ResetBXCount(uint8_t const& link) {
	    std::stringstream regName;
	    regName << "OptoHybrid_LINKS.LINK" << (int)m_controlLink;
	    return writeReg(getDeviceBaseNode(),regName.str()+".COUNTERS.RESETS.BXCount", 0x1); };
	  
	  uhal::HwInterface& getOptoHybridHwInterface(uint8_t const& link=0x0) const {
	    std::stringstream regName;
	    regName << "OptoHybrid_LINKS.LINK" << (int)m_controlLink;
	    return getGEMHwInterface(); };

	protected:
	  //uhal::ConnectionManager *manageOptoHybridConnection;
	  //log4cplus::Logger logOptoHybrid_;
	  //uhal::HwInterface *hwOptoHybrid_;

	  //uhal::HwInterface& getOptoHybridHwDevice() const;

	  //OptoHybridMonitor *monOptoHybrid_;


	  bool links[3];
	    
	  std::vector<linkStatus> activeLinks;

	private:
	  uint8_t m_controlLink;
	  int m_slot;
	  
	}; //end class HwOptoHybrid
    } //end namespace gem::hw::glib
  } //end namespace gem::hw
} //end namespace gem
#endif
