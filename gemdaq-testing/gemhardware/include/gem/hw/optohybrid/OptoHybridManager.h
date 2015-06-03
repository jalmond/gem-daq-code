#ifndef gem_hw_optohybrid_OptoHybridManager_h
#define gem_hw_optohybrid_OptoHybridManager_h

#include <string>

#include "xdaq/WebApplication.h"

#include "xdata/String.h"
#include "xdata/UnsignedLong.h"
#include "xdata/UnsignedInteger32.h"

#include "uhal/uhal.hpp"

#include "xgi/framework/Method.h"
#include "cgicc/HTMLClasses.h"

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

#include "gem/hw/optohybrid/OptoHybridSettings.h"

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
    namespace optohybrid {
      class HwOptoHybrid;
      
      //class OptoHybridManager: public gem::base::GEMWebApplication, public gem::base::GEMFSMApplication
      class OptoHybridManager: public xdaq::WebApplication, public xdata::ActionListener
	{
	  
	public:
	  XDAQ_INSTANTIATOR();
	  
	  OptoHybridManager(xdaq::ApplicationStub * s)
	    throw (xdaq::exception::Exception);

	  ~OptoHybridManager();
	  
	}; //end class OptoHybridManager

    }//end namespace gem::hw::optohybrid
    
  }//end namespace gem::hw
  
}//end namespace gem

#endif
