#ifndef gem_hw_vfat_VFAT2Manager_h
#define gem_hw_vfat_VFAT2Manager_h

#include <string>

#include "xdaq/WebApplication.h"

#include "xdata/String.h"
#include "xdata/UnsignedLong.h"
#include "xdata/UnsignedInteger32.h"

//#include "uhal/uhal.hpp"

#include "xgi/framework/Method.h"
#include "cgicc/HTMLClasses.h"

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

#include "gem/hw/vfat/VFAT2Settings.h"

//typedef uhal::exception::exception uhalException;

namespace cgicc {
  BOOLEAN_ELEMENT(section,"section");
}

namespace gem {
  namespace base {
    class GEMApplication;
    class GEMFSM;
    class GEMFSMApplication;
    class GEMWebApplication;
  }
  
  namespace hw {
    namespace vfat {
      class HwVFAT2;
      
      //class VFAT2Manager: public gem::base::GEMWebApplication, public gem::base::GEMFSMApplication
      class VFAT2Manager: public xdaq::WebApplication, public xdata::ActionListener
	{
	  
	public:
	  XDAQ_INSTANTIATOR();
	  
	  VFAT2Manager(xdaq::ApplicationStub * s)
	    throw (xdaq::exception::Exception);

	  ~VFAT2Manager();
	  
	  void Default(xgi::Input *in, xgi::Output *out )
	    throw (xgi::exception::Exception);
	  void RegisterView(xgi::Input *in, xgi::Output *out )
	    throw (xgi::exception::Exception);
	  void ControlPanel(xgi::Input *in, xgi::Output *out )
	    throw (xgi::exception::Exception);
	  void ExpertView(xgi::Input *in, xgi::Output *out )
	    throw (xgi::exception::Exception);
	  void Peek(xgi::Input *in, xgi::Output *out )
	    throw (xgi::exception::Exception);
	  void controlVFAT2(xgi::Input * in, xgi::Output * out)
	    throw (xgi::exception::Exception);
	  
	  void getCheckedRegisters(cgicc::Cgicc cgi, std::vector<std::pair<std::string,uint8_t> > &regValsToSet)
	    throw (xgi::exception::Exception);
	  void performAction(cgicc::Cgicc cgi, std::vector<std::pair<std::string,uint8_t> > regValsToSet)
	    throw (xgi::exception::Exception);
	  
	  //void readCounters(  xgi::Input *in)
	  //  throw (xgi::exception::Exception);
	  //void readRegisters (xgi::Input *in)
	  //  throw (xgi::exception::Exception);
	  //void writeRegisters(xgi::Input *in)
	  //  throw (xgi::exception::Exception);
	  
	  void actionPerformed(xdata::Event& event);

	  HwVFAT2* vfatDevice;

	  void readVFAT2Registers(VFAT2ControlParams& params);
	  //void readVFAT2Registers();
	  
	  std::map<std::string,uint32_t>    vfatFullRegs_;
	  std::map<std::string,uint8_t>     vfatRegs_;
	  VFAT2ControlParams vfatParams_;

	private:
	  std::vector<std::string>          nodes_;
	  ////counters
	  //uint16_t vfat_chipid_;
	  //uint8_t  vfat_upsetcounter_;
	  //uint32_t vfat_hitcounter_;

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
	  xdata::String ipAddr_;
	  xdata::String settingsFile_;
	  
	  class VFAT2ControlPanelWeb {
  	    public:
	      static void createHeader(xgi::Output *out );
	      
	      static void createVFATInfoLayout(       xgi::Output *out,
						      const VFAT2ControlParams params);
	      static void createControlRegisterLayout(xgi::Output *out,
						      const VFAT2ControlParams params);
	      static void createSettingsLayout(       xgi::Output *out,
						      const VFAT2ControlParams params);
	      static void createCounterLayout(        xgi::Output *out,
						      const VFAT2ControlParams params);
	      static void createChannelRegisterLayout(xgi::Output *out,
						      const VFAT2ControlParams params);
	      static void createCommandLayout(        xgi::Output *out,
						      const VFAT2ControlParams params);
	      
	      static void getCurrentParametersAsXML();
	      static void saveCurrentParametersAsXML();
	      static void setParametersByXML();
	      
	      
	  };//end class VFAT2ControlPanelWeb
	}; //end class VFAT2Manager

    }//end namespace gem::hw::vfat
    
  }//end namespace gem::hw
  
}//end namespace gem

#endif
