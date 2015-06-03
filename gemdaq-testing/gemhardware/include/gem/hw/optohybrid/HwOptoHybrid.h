#ifndef gem_hw_optohybrid_HwOPTOHYBRID_h
#define gem_hw_optohybrid_HwOPTOHYBRID_h

#include "xdaq/Application.h"

#include "xdata/String.h"
#include "xdata/UnsignedLong.h"
#include "xdata/UnsignedInteger32.h"
#include "xdata/ActionListener.h"

#include "gem/hw/GEMHwDevice.h"

#include "gem/hw/optohybrid/exception/Exception.h"
//#include "gem/hw/optohybrid/OptoHybridMonitor.h"

#include "uhal/uhal.hpp"

#define MAX_NVFATS 6

typedef uhal::exception::exception uhalException;

namespace uhal {
  class HwInterface;
}

namespace xdaq {
  class Application;;
}

namespace gem {
  namespace hw {
    namespace optohybrid {

      //class OptoHybridMonitor;
      
      class HwOptoHybrid: public gem::hw::GEMHwDevice
	{
	public:
	  HwOptoHybrid(const log4cplus::Logger& gemLogger);

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

	  //bool isHwOptoHybridConnected();

	  /** Read the board ID registers
	   * @returns the OptoHybrid board ID 
	   **/
	  //std::string getBoardID()   const;

	  /** Read the firmware register
	   * @returns a string corresponding to the build date
	   **/
	  std::string getFirmware() {
	    std::stringstream retval;
	    uint32_t fwver = readReg(getDeviceBaseNode(),"FIRMWARE");
	    retval << "0x" << std::hex << fwver << std::dec << std::endl;
	    return retval.str();
	  };
	  
	  /** Read the link status registers, store the information in a struct
	   * @param uint8_t link is the number of the link to query
	   * @retval _status a struct containing the status bits of the optical link
	   * @throws gem::hw::optohybrid::exception::InvalidLink if the link number is outside of 0-2
	   **/
	  GEMHwDevice::OpticalLinkStatus LinkStatus(uint8_t link) ;

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
	  void LinkReset(uint8_t link, uint8_t resets);

	  /** Read the trigger data
	   * @retval uint32_t returns 32 bits 6 bits for s-bits and 26 for bunch countrr
	   **/
	  //uint32_t readTriggerData();

	  
	  ///Clocking
	  /** Setup the VFAT clock 
	   * @param bool source true uses the external clock, false uses the onboard clock
	   * @param bool fallback uses the external clock, false uses the onboard clock
	   **/
	  void SetVFATClock(bool source, bool fallback) {
	    writeReg(getDeviceBaseNode(),"CLOCKING.VFAT.SOURCE"  ,(uint32_t)source  );
	    writeReg(getDeviceBaseNode(),"CLOCKING.VFAT.FALLBACK",(uint32_t)fallback);
	  };

	  /** VFAT clock status
	   * @param bool source true uses the external clock, false uses the onboard clock
	   * @param bool fallback uses the external clock, false uses the onboard clock
	   **/
	  std::pair<bool,bool> StatusVFATClock() {
	    uint32_t src = readReg(getDeviceBaseNode(),"CLOCKING.VFAT.SOURCE");
	    uint32_t flb = readReg(getDeviceBaseNode(),"CLOCKING.VFAT.FALLBACK");
	    //maybe do a check to ensure that the value has been read properly?
	    return std::make_pair(src,flb);
	  };

	  /** Setup the CDCE clock 
	   * @param bool source true uses the external clock, false uses the onboard clock
	   * @param bool fallback uses the external clock, false uses the onboard clock
	   **/
	  void SetCDCEClock(bool source, bool fallback) {
	    writeReg(getDeviceBaseNode(),"CLOCKING.CDCE.SOURCE"  ,(uint32_t)source  );
	    writeReg(getDeviceBaseNode(),"CLOCKING.CDCE.FALLBACK",(uint32_t)fallback);
	  };

	  /** CDCE clock status
	   * @param bool source true uses the external clock, false uses the onboard clock
	   * @param bool fallback uses the external clock, false uses the onboard clock
	   **/
	  std::pair<bool,bool> StatusCDCEClock() {
	    uint32_t src = readReg(getDeviceBaseNode(),"CLOCKING.CDCE.SOURCE");
	    uint32_t flb = readReg(getDeviceBaseNode(),"CLOCKING.CDCE.FALLBACK");
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
	   * @param uint8_t mode 0 from GLIB, 1 from external, 2 from both
	   **/
	  void setTrigSource(uint8_t mode) {
	    switch (mode) {
	    case(0):
	      writeReg(getDeviceBaseNode(),"TRIGGER.SOURCE",mode);
	      return;
	    case(1):
	      writeReg(getDeviceBaseNode(),"TRIGGER.SOURCE",mode);
	      return;
	    case(2):
	      writeReg(getDeviceBaseNode(),"TRIGGER.SOURCE",mode);
	      return;
	    default:
	      writeReg(getDeviceBaseNode(),"TRIGGER.SOURCE",0x2);
	      return;
	    }
	  };

	  /** Read the Trigger source
	   * @retval uint8_t 0 from GLIB, 1 from external, 2 from both
	   **/
	  uint8_t getTrigSource() { return readReg(getDeviceBaseNode(),"TRIGGER.SOURCE"); };


	  /** Set the S-bit source
	   * @param uint8_t chip
	   **/
	  void setSBitSource(uint8_t mode) { writeReg(getDeviceBaseNode(),"TRIGGER.TDC.SBits",mode); };

	  /** Read the S-bit source
	   * @retval uint8_t which VFAT chip is sending the S-bits
	   **/
	  uint8_t getSBitSource() { return readReg(getDeviceBaseNode(),"TRIGGER.TDC_SBits"); };


	  /* Generate and send specific T1 commands on the OptoHybrid */
	  /** Send an internal L1A
	   * @param uint64_t ntrigs, how many L1As to send
	   **/
	  void SendL1A(uint64_t ntrigs) {
	    for (uint64_t i = 0; i < ntrigs; ++i) 
	      writeReg(getDeviceBaseNode(),"FAST_COM.Send.L1A",0x1);
	  };

	  /** Send an internal CalPulse
	   * @param uint64_t npulse, how many CalPulses to send
	   **/
	  void SendCalPulse(uint64_t npulse) {
	    for (uint64_t i = 0; i < npulse; ++i) 
	      writeReg(getDeviceBaseNode(),"FAST_COM.Send.CalPulse",0x1);
	  };

	  /** Send an internal L1A and CalPulse
	   * @param uint64_t npulse, how many pairs to send
	   * @param uint32_t delay, how long between L1A and CalPulse
	   **/
	  void SendL1ACal(uint64_t npulse, uint32_t delay) {
	    for (uint64_t i = 0; i < npulse; ++i) 
	      writeReg(getDeviceBaseNode(),"FAST_COM.Send.L1ACalPulse",delay);
	  };

	  /** Send an internal Resync
	   * 
	   **/
	  void SendResync() { writeReg(getDeviceBaseNode(),"FAST_COM.Send.Resync",0x1); };


	  /** Send an internal BC0
	   * 
	   **/
	  void SendBC0() { writeReg(getDeviceBaseNode(),"FAST_COM.Send.BC0",0x1); };

	  ///Counters
	  /** Get the recorded number of L1A signals
	   * @param mode specifies which L1A counter to read
	   * 0 external
	   * 1 internal
	   * 2 delayed (sent along with a CalPulse)
	   * 3 total
	   **/
	  uint32_t GetL1ACount(uint8_t mode) {
	    switch(mode) {
	    case 0:
	      return readReg(getDeviceBaseNode(),"COUNTERS.L1A.External");
	    case 1:
	      return readReg(getDeviceBaseNode(),"COUNTERS.L1A.Internal");
	    case 2:
	      return readReg(getDeviceBaseNode(),"COUNTERS.L1A.Delayed");
	    case 3:
	      return readReg(getDeviceBaseNode(),"COUNTERS.L1A.Total");
	    default:
	      return readReg(getDeviceBaseNode(),"COUNTERS.L1A.Total");
	    }
	  };
	  
	  /** Get the recorded number of CalPulse signals
	   * @param mode specifies which CalPulse counter to read
	   * 0 internal
	   * 1 delayed (sent along with a L1A)
	   * 2 total
	   **/
	  uint32_t GetCalPulseCount(uint8_t mode) {
	    switch(mode) {
	    case 0:
	      return readReg(getDeviceBaseNode(),"COUNTERS.CalPulse.Internal");
	    case 1:
	      return readReg(getDeviceBaseNode(),"COUNTERS.CalPulse.Delayed");
	    case 2:
	      return readReg(getDeviceBaseNode(),"COUNTERS.CalPulse.Total");
	    default:
	      return readReg(getDeviceBaseNode(),"COUNTERS.CalPulse.Total");
	    }
	  };
	  
	  /** Get the recorded number of Resync signals
	   **/
	  uint32_t GetResyncCount() { return readReg(getDeviceBaseNode(),"COUNTERS.Resync"); };

	  /** Get the recorded number of BC0 signals
	   **/
	  uint32_t GetBC0Count() { return readReg(getDeviceBaseNode(),"COUNTERS.BC0"); };

	  /** Get the recorded number of BXCount signals
	   **/
	  uint32_t GetBXCountCount() { return readReg(getDeviceBaseNode(),"COUNTERS.BXCount"); };
	  
	  ///Resets
	  /** Reset recorded number of L1A signals
	   * @param mode specifies which L1A counter to reset
	   * 0 external
	   * 1 internal
	   * 2 delayed (sent along with a CalPulse)
	   * 3 total
	   **/
	  void ResetL1ACount(uint8_t mode) {
	    switch(mode) {
	    case 0:
	      return writeReg(getDeviceBaseNode(),"RESETS.L1A.External", 0x1);
	    case 1:
	      return writeReg(getDeviceBaseNode(),"RESETS.L1A.Internal", 0x1);
	    case 2:
	      return writeReg(getDeviceBaseNode(),"RESETS.L1A.Delayed", 0x1);
	    case 3:
	      return writeReg(getDeviceBaseNode(),"RESETS.L1A.Total", 0x1);
	    default:
	      return writeReg(getDeviceBaseNode(),"RESETS.L1A.Total", 0x1);
	    }
	  };
	  
	  /** Get the recorded number of CalPulse signals
	   * @param mode specifies which CalPulse counter to read
	   * 0 internal
	   * 1 delayed (sent along with a L1A)
	   * 2 total
	   **/
	  void ResetCalPulseCount(uint8_t mode) {
	    switch(mode) {
	    case 0:
	      return writeReg(getDeviceBaseNode(),"RESETS.CalPulse.Internal", 0x1);
	    case 1:
	      return writeReg(getDeviceBaseNode(),"RESETS.CalPulse.Delayed", 0x1);
	    case 2:
	      return writeReg(getDeviceBaseNode(),"RESETS.CalPulse.Total", 0x1);
	    default:
	      return writeReg(getDeviceBaseNode(),"RESETS.CalPulse.Total", 0x1);
	    }
	  };
	  
	  /** Get the recorded number of Resync signals
	   **/
	  void ResetResyncCount() { return writeReg(getDeviceBaseNode(),"RESETS.Resync", 0x1); };

	  /** Get the recorded number of BC0 signals
	   **/
	  void ResetBC0Count() { return writeReg(getDeviceBaseNode(),"RESETS.BC0", 0x1); };

	  /** Get the recorded number of BXCount signals
	   **/
	  void ResetBXCountCount() { return writeReg(getDeviceBaseNode(),"RESETS.BXCount", 0x1); };
	  
	  uhal::HwInterface& getOptoHybridHwInterface() const { return getGEMHwInterface(); };

	protected:
	  //uhal::ConnectionManager *manageOptoHybridConnection;
	  //log4cplus::Logger logOptoHybrid_;
	  //uhal::HwInterface *hwOptoHybrid_;

	  //uhal::HwInterface& getOptoHybridHwDevice() const;

	  //OptoHybridMonitor *monOptoHybrid_;

	  xdata::UnsignedLong myParameter_;
	  xdata::String myAction_;
	
	  xdata::UnsignedInteger32 testReg_;
	  xdata::UnsignedInteger32 boardID_;
	  xdata::UnsignedInteger32 systemID_;
	  xdata::UnsignedInteger32 systemFirmwareID_;
	
	private:
	
	}; //end class HwOptoHybrid
    } //end namespace gem::hw::glib
  } //end namespace gem::hw
} //end namespace gem
#endif
