#ifndef gem_hw_GEMHwDevice_h
#define gem_hw_GEMHwDevice_h

#include "xdaq/Application.h"
#include "xdata/ActionListener.h"

#include "gem/hw/exception/Exception.h"

#include "uhal/uhal.hpp"
#include "uhal/Utilities.hpp"

#include "gem/utils/GEMLogging.h"

/* would like to avoid rewriting this nice functionality,
   but the code isn't in the main xdaq release.
   have copied directly into GEM and figure out any compatibility later
*/
#include "gem/utils/Lock.h"
#include "gem/utils/LockGuard.h"

#define MAX_IPBUS_RETRIES 25

typedef uhal::exception::exception uhalException;

namespace uhal {
  class HwInterface;
}

namespace xdaq {
  class Application;;
}

namespace gem {
  namespace hw {
    
    class GEMHwDevice
      {

      public:
	typedef struct OpticalLinkStatus {
	  uint32_t linkErrCnt     ;
	  uint32_t linkVFATI2CRec ;
	  uint32_t linkVFATI2CSnt ;
	  uint32_t linkRegisterRec;
	  uint32_t linkRegisterSnt;
	} OpticalLinkStatus;
	
	typedef struct DeviceErrors {
	  int badHeader_;
	  int readError_;
	  int timeouts_;
	  int controlHubErr_;
	} DeviceErrors;
	
	/** 
	 * GEMHwDevice constructor 
	 * @param gemLogger pointer to log4cplus::Logger
	 **/
	GEMHwDevice(const log4cplus::Logger& gemLogger
		    /*xdaq::InfoSpace* const configInfoSpace
		      xdaq::InfoSpace* const monitorInfoSpace
		      gem::hw::GMEHwMonitor* const hwMonitor
		     */
		    );

	/*
	GEMHwDevice(xdaq::Application* xdaqApp,
	            std::string& addressFileName,
	            std::string& ipAddress,
	            std::string& controlHubAddress,
		    );
	*/

	virtual ~GEMHwDevice();
	
	virtual void connectDevice();
	virtual void releaseDevice();
	//virtual void initDevice();
	virtual void configureDevice()=0;
	virtual void enableDevice();
	//virtual void disableDevice();
	//virtual void startDevice();
	//virtual void stopDevice();
	//virtual void pauseDevice();
	//virtual void resumeDevice();
	//virtual void haltDevice();
	
	virtual bool isHwConnected() { return gemHWP_ != 0; };
	
	/**
	 *Generic read/write functions or IPBus devices
	 * operation will be the same for the GLIB, MP7, VFAT2/3, 
	 * and AMC13 ( we should use the already defined AMC13, rather than write our own,
	 * unless there are GEM specific functions we need to implement)
	 */

	/** readReg(std::string const& regName)
	 * @param regName name of the register to read 
	 * @retval returns the 32 bit unsigned value in the register
	 */
	uint32_t readReg( std::string const& regName);

	/** readReg(std::string const& regPrefix, std::string const& regName)
	 * @param regPrefix prefix in the address table, possibly root nodes
	 * @param regName name of the register to read from the address table
	 * @retval returns the 32 bit unsigned value
	 */
	uint32_t readReg( const std::string &regPrefix,
			  const std::string &regName) {
	  return readReg(regPrefix+"."+regName); };

	/** readRegs( std::vector<std::pair<std::string, uint32_t> > &regList)
	 * read list of registers in a single transaction (one dispatch call)
	 * into the supplied vector regList
	 * @param regList list of register name and uint32_t value to store the result
	 */
	void     readRegs( std::vector<std::pair<std::string, uint32_t> > &regList);

	/** writeReg(std::string const& regName, uint32_t const val)
	 * @param regName name of the register to read 
	 * @param val value to write to the register
	 */
	void     writeReg( std::string const& regName, uint32_t const val);

	/** writeReg(std::string const& regPrefux, std::string const& regName, uint32_t const val)
	 * @param regPrefix prefix in the address table to the register
	 * @param regName name of the register to write to 
	 * @param val value to write to the register
	 */
	void     writeReg( const std::string &regPrefix,
			   const std::string &regName,
			   uint32_t const val) {
	  return writeReg(regPrefix+"."+regName, val); };

	/** writeRegs(std::vector<std::pair<std::string, uint32_t> > const& regList)
	 * write list of registers in a single transaction (one dispatch call)
	 * using the supplied vector regList
	 * @param regList std::vector of a pairs of register names and values to write
	 */
	void     writeRegs(std::vector<std::pair<std::string, uint32_t> > const& regList);

	/** writeRegs(std::vector<std::pair<std::string, uint32_t> > const& regList)
	 * write single value to a list of registers in a single transaction
	 * (one dispatch call) using the supplied vector regList
	 * @param regList list of registers to write a value to
	 * @param regValue uint32_t value to write to the list of registers
	 */
	void     writeValueToRegs(std::vector<std::string> const& regList, uint32_t const& regValue);
	
	/** zeroReg(std::string const& regName)
	 * write zero to a single register
	 * @param regName register to zero
	 */
	void     zeroReg(  std::string const& regName) { writeReg(regName,0); };

	/** zeroRegs(std::vector<std::string> const& regNames)
	 * write zero to a list of registers in a single transaction (one dispatch call)
	 * using the supplied vector regNames
	 * @param regNames registers to zero
	 */
	void     zeroRegs( std::vector<std::string> const& regNames);

	/** readBlock(std::string const& regName)
	 * read from a memory block
	 * @param regName fixed size memory block to read from
	 */
	std::vector<uint32_t> readBlock( std::string const& regName);
	//size_t readBlock( std::string const& regName, size_t nWords, uint32_t* buffer); /*hcal style */

	/** readBlock(std::string const& regName, size_t const nWords)
	 * read from a memory block
	 * @param regName memory block to read from
	 * @param nWords size of the memory block to read
	 * @retval returns a vector of 32 bit unsigned value
	 */
	std::vector<uint32_t> readBlock( std::string const& regName,
					 size_t      const& nWords);

	/** writeBlock(std::string const& regName, std::vector<uint32_t> const values)
	 * write to a memory block
	 * @param regName memory block to write to
	 * @param values list of 32-bit words to write into the memory block
	 */
	void writeBlock(std::string           const& regName,
			std::vector<uint32_t> const values);

	/** zeroBlock( std::string const& regName)
	 * write zeros to a block of memory
	 * @param regName block or memory to zero
	 */
	void zeroBlock( std::string const& regName);


	// These methods provide access to the member variables
	// specifying the uhal address table name and the IPbus protocol
	// version.
	const std::string getAddressTableFileName() const { return addressTable_;   };
	const std::string getIPbusProtocolVersion() const { return ipbusProtocol_;  };
	const std::string getDeviceBaseNode()       const { return deviceBaseNode_; };
	const std::string getDeviceIPAddress()      const { return deviceIPAddr_;   };
	const std::string getDeviceID()             const { return deviceID_;       };

	void setAddressTableFileName(std::string const& name) {
	  addressTable_ = "file://${BUILD_HOME}/data/"+name; };
	void setIPbusProtocolVersion(std::string const& version) {
	  ipbusProtocol_ = version; };
	void setDeviceBaseNode(std::string const& deviceBase) {
	  deviceBaseNode_ = deviceBase; };
	void setDeviceIPAddress(std::string const& deviceIPAddr) {
	  deviceIPAddr_ = deviceIPAddr; };
	void setDeviceID(std::string const& deviceID) {
	  deviceID_ = deviceID; };
	
	uhal::HwInterface& getGEMHwInterface() const;
	
	void updateErrorCounters(std::string const& errCode);
	
	DeviceErrors ipBusErrs;
	
	std::string printErrorCounts() const;
	
      protected:
	uhal::ConnectionManager *gemConnectionManager;
	log4cplus::Logger gemLogger_;
	uhal::HwInterface *gemHWP_;
		
	std::string uint32ToString(uint32_t const val) const {
	  std::stringstream res;
	  res << char((val & uint32_t(0xff000000)) / 16777216);
	  res << char((val & uint32_t(0x00ff0000)) / 65536);
	  res << char((val & uint32_t(0x0000ff00)) / 256);
	  res << char((val & uint32_t(0x000000ff)));
	  return res.str(); };

	std::string uint32ToDottedQuad(uint32_t const val) const {
	  std::stringstream res;
	  res << std::hex << char((val & uint32_t(0xff000000)) / 16777216)<< std::dec << ".";
	  res << std::hex << char((val & uint32_t(0x00ff0000)) / 65536)   << std::dec << ".";
	  res << std::hex << char((val & uint32_t(0x0000ff00)) / 256)     << std::dec << ".";
	  res << std::hex << char((val & uint32_t(0x000000ff)))           << std::dec;
	  return res.str(); };
	
	std::string uint32ToGroupedHex(uint32_t const val1, uint32_t const val2) const {
	  std::stringstream res;
	  res << std::hex << char((val1 & uint32_t(0x0000ff00)) / 256)     << std::dec << ":";
	  res << std::hex << char((val1 & uint32_t(0x000000ff)))           << std::dec << ":";
	  res << std::hex << char((val2 & uint32_t(0xff000000)) / 16777216)<< std::dec << ":";
	  res << std::hex << char((val2 & uint32_t(0x00ff0000)) / 65536)   << std::dec << ":";
	  res << std::hex << char((val2 & uint32_t(0x0000ff00)) / 256)     << std::dec << ":";
	  res << std::hex << char((val2 & uint32_t(0x000000ff)))           << std::dec;
	  return res.str(); };

	bool is_connected_;

	mutable gem::utils::Lock hwLock_;

      private:
	std::string addressTable_;
	std::string ipbusProtocol_;
	std::string deviceBaseNode_;
	std::string deviceIPAddr_;
	std::string deviceID_;
		
	bool knownErrorCode(std::string const& errCode) const;
	
	//std::string registerToChar(uint32_t value) const;	

      }; //end class GEMHwDevice

  } //end namespace gem::hw

} //end namespace gem
#endif
