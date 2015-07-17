#ifndef gem_hw_vfat_GEMController_h
#define gem_hw_vfat_GEMController_h

#include <string>

#include "xdaq/WebApplication.h"

#include "xdata/String.h"
#include "xdata/UnsignedLong.h"
#include "xdata/UnsignedInteger32.h"

#include "uhal/uhal.hpp"

#include "xgi/framework/Method.h"
#include "cgicc/HTMLClasses.h"

#include "gem/hw/vfat/HwVFAT2.h"

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/framework/XMLPScanToken.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>

namespace cgicc {
  BOOLEAN_ELEMENT(section,"section");
}
namespace gem {
  namespace hw {
    namespace vfat {
      class HwVFAT2;

      class GEMController: public xdaq::WebApplication, public xdata::ActionListener
	{

	public:
	  XDAQ_INSTANTIATOR();

	  GEMController(xdaq::ApplicationStub * s)
	    throw (xdaq::exception::Exception);

	  ~GEMController();

	  /**
	   *   Generate default web interface, currently generates control panel web interface
	   */
	  void Default(xgi::Input *in, xgi::Output *out )
	    throw (xgi::exception::Exception);

	  /**
	   *   Generate empty dummy web interface, useful for future and not implemented features
	   */
	  void Dummy(xgi::Input *in, xgi::Output *out )
	    throw (xgi::exception::Exception);

	  /**
	   *   Generate control panel web interface
	   */
	  void ControlPanel(xgi::Input *in, xgi::Output *out )
	    throw (xgi::exception::Exception);

	  /**
	   *   Select one of available crates
	   */
	  void CrateSelection(xgi::Input *in, xgi::Output *out )
	    throw (xgi::exception::Exception);

	  /**
	   *   Set XML configuration file
	   */
	  void setConfFile(xgi::Input *in, xgi::Output *out )
	    throw (xgi::exception::Exception);

	  /**
	   *   Status of MCH board(s)
	   */
	  void MCHStatus(xgi::Input *in, xgi::Output *out )
	    throw (xgi::exception::Exception);

	  /**
	   *   Access to MCH board(s) utils
	   */
	  void MCHUtils(xgi::Input *in, xgi::Output *out )
	    throw (xgi::exception::Exception);

	  /**
	   *   Status of AMC board(s)
	   */
	  void AMCStatus(xgi::Input *in, xgi::Output *out )
	    throw (xgi::exception::Exception);

	  /**
	   *   Access to AMC board(s) utils
	   */
	  void AMCUtils(xgi::Input *in, xgi::Output *out )
	    throw (xgi::exception::Exception);

	  /**
	   *   Status of GLIB board(s)
	   */
	  void GLIBStatus(xgi::Input *in, xgi::Output *out )
	    throw (xgi::exception::Exception);

	  /**
	   *   Access to GLIB board(s) utils
	   */
	  void GLIBUtils(xgi::Input *in, xgi::Output *out )
	    throw (xgi::exception::Exception);

	  /**
	   *   Status of OH board(s)
	   */
	  void OHStatus(xgi::Input *in, xgi::Output *out )
	    throw (xgi::exception::Exception);

	  /**
	   *   Access to OH board(s) utils
	   */
	  void OHUtils(xgi::Input *in, xgi::Output *out )
	    throw (xgi::exception::Exception);

	  /**
	   *   Access to VFAT2 manager
	   */
	  void VFAT2Manager(xgi::Input *in, xgi::Output *out )
	    throw (xgi::exception::Exception);

	  /**
	   *   The main function behind VFAT2 manager
	   */
	  void controlVFAT2(xgi::Input * in, xgi::Output * out)
	    throw (xgi::exception::Exception);

	  /**
	   *   Check what registers are selected in VFAT2 manager
	   */
	  void getCheckedRegisters(cgicc::Cgicc cgi, std::vector<std::pair<std::string,uint8_t> > &regValsToSet)
	    throw (xgi::exception::Exception);

	  /**
	   *   Perform selected action on selected registers in VFAT2 manager
	   */
	  void performAction(cgicc::Cgicc cgi, std::vector<std::pair<std::string,uint8_t> > regValsToSet)
	    throw (xgi::exception::Exception);

	  HwVFAT2* vfatDevice;

	  /**
	   *   Parse XML configuration file
	   */
	  void parseXMLFile();

	  /**
	   *   Parse section of XML configuration file describing GEM system
	   */
	  void parseGEMSystem(xercesc::DOMNode * pNode);

	  /**
	   *   Parse section of XML configuration file describing available uTCA crates
	   */
	  void parseCrate(xercesc::DOMNode * pNode);

	  /**
	   *   Parse section of XML configuration file describing available GLIB board(s)
	   */
	  void parseGLIB(xercesc::DOMNode * pNode);

	  /**
	   *   Parse section of XML configuration file describing available OH board(s)
	   */
	  void parseOH(xercesc::DOMNode * pNode);

	  /**
	   *   Parse section of XML configuration file describing VFAT2 registers
	   */
	  void parseVFAT2Settings(xercesc::DOMNode * pNode);

	  /**
	   *   Parse section of XML configuration file describing VFAT2 control registers
	   */
	  void parseControlRegisters(xercesc::DOMNode * pNode);

	  /**
	   *   Parse section of XML configuration file describing VFAT2 control register 0
	   */
	  void parseControlRegister0(xercesc::DOMNode * pNode);

	  /**
	   *   Parse section of XML configuration file describing VFAT2 control register 1
	   */
	  void parseControlRegister1(xercesc::DOMNode * pNode);

	  /**
	   *   Parse section of XML configuration file describing VFAT2 control register 2
	   */
	  void parseControlRegister2(xercesc::DOMNode * pNode);

	  /**
	   *   Parse section of XML configuration file describing VFAT2 control register 3
	   */
	  void parseControlRegister3(xercesc::DOMNode * pNode);

	  /**
	   *   Parse section of XML configuration file describing VFAT2 bias settings
	   */
	  void parseBiasSettings(xercesc::DOMNode * pNode);

	  /**
	   *   Parse section of XML configuration file describing VFAT2 current bias settings
	   */
	  void parseCurrentBias(xercesc::DOMNode * pNode);

	  void actionPerformed(xdata::Event& event);

	  std::map<std::string,uint32_t>    vfatFullRegs_;
	  std::map<std::string,uint8_t>     vfatRegs_;
	  gem::hw::vfat::VFAT2ControlParams vfatParams_;

	private:
	  std::vector<std::string>          nodes_;

	protected:
	  /**
	   * Create a mapping between the VFAT2 chipID and the connection name specified in the
	   * address table.  Can be done at initialization of the system as this will not change
	   * while running.  Possibly able to do the system scan and compare to a hardware databse
	   * This mapping can be used to send commands to specific chips through the manager interface
	   * the string is the name in the connection file, while the uint16_t is the chipID, though
	   * it need only be a uint12_t
	   **/
	  std::map<std::string, uint16_t> systemMap;

	  //xdata::UnsignedLong myParameter_;
	  xdata::String device_;
	  xdata::String settingsFile_;

	  std::vector<std::string> crateIds; 
	  std::vector<xercesc::DOMNode *> crateNodes;
	  std::vector<std::string> MCHIds; 
	  std::vector<std::string> AMCIds; 
	  std::vector<std::string> GLIBIds; 
	  std::vector<std::string> OHIds; 
	  std::vector<std::string> VFAT2Ids; 
	  int currentCrate;
	  std::string currentCrateId;
	  std::string fileWarning;
	  std::string fileError;
	  
	  log4cplus::Logger gemLogger_;

	  class GEMControllerPanelWeb {
	  public:
	    static void createHeader(xgi::Output *out );

	    static void createVFATInfoLayout(       xgi::Output *out,
						    const gem::hw::vfat::VFAT2ControlParams params);
	    static void createControlRegisterLayout(xgi::Output *out,
						    const gem::hw::vfat::VFAT2ControlParams params);
	    static void createSettingsLayout(       xgi::Output *out,
						    const gem::hw::vfat::VFAT2ControlParams params);
	    static void createCounterLayout(        xgi::Output *out,
						    const gem::hw::vfat::VFAT2ControlParams params);
	    static void createChannelRegisterLayout(xgi::Output *out,
						    const gem::hw::vfat::VFAT2ControlParams params);
	    static void createCommandLayout(        xgi::Output *out,
						    const gem::hw::vfat::VFAT2ControlParams params);

	    static void getCurrentParametersAsXML();
	    static void saveCurrentParametersAsXML();
	    static void setParametersByXML();


	  };//end class GEMControllerPanelWeb
	}; //end class GEMController

    }//end namespace gem::hw::vfat

  }//end namespace gem::hw

}//end namespace gem

#endif

