#ifndef gem_base_GEMApplication_h
#define gem_base_GEMApplication_h

#include <cstdlib>
#include "string.h"
#include "limits.h"
#include <string>
#include <string>
#include <deque>
#include <map>

#include "xdaq/WebApplication.h"
#include "xgi/framework/UIManager.h"

#include "toolbox/TimeVal.h"

#include "log4cplus/logger.h"
#include "gem/utils/GEMLogging.h"
#include "gem/base/exception/Exception.h"
#include "gem/base/utils/exception/Exception.h"

namespace xdaq {
  class ApplicationStub;
}

namespace xgi {
  class Input;
  class Output;
}

namespace gem {
  namespace base {
    
    class GEMMonitor;
    class GEMFSMApplication;
    class GEMWebApplication;
    
    class GEMApplication : public xdaq::WebApplication, public xdata::ActionListener
      {
	friend class GEMMonitor;
	friend class GEMFSMApplication;
	friend class GEMWebApplication;

      public:
	GEMApplication(xdaq::ApplicationStub *stub)
	  throw (xdaq::exception::Exception);

	virtual ~GEMApplication();

	std::string getFullURL();
	
	/**
	 * The init method is pure virtual in the base class, to ensure
	 * that it is fully implemented in every derived application,
	 * with a specific implementation
	 */
	virtual void init() = 0;
	
	/**
	 * The actionPerformed method will have a default implementation here
	 * and can be further specified in derived applications, that will 
	 * subsequently call gem::base::GEMApplication::actionPerformed(event)
	 * to replicate the default behaviour
	 **/
	virtual void actionPerformed(xdata::Event& event);
	
	void xgiDefault(xgi::Input* in, xgi::Output* out);
	void xgiMonitor(xgi::Input* in, xgi::Output* out);
	void xgiExpert( xgi::Input* in, xgi::Output* out);

      protected:
	log4cplus::Logger gemLogger_;

	xdata::InfoSpace *appInfoSpaceP_;             /*generic application parameters */
	xdata::InfoSpace *monitorInfoSpaceP_;         /*monitoring parameters, stored in the appInfoSpace */
	xdata::InfoSpace *configInfoSpaceP_;          /*configuration parameters, stored in the appInfoSpace */
						    
	virtual void importConfigurationParameters();
	virtual void fillConfigurationInfoSpace();
	virtual void updateConfigurationInfoSpace();

	virtual void importMonitoringParameters();
	virtual void fillMonitoringInfoSpace();
	virtual void updateMonitoringInfoSpace();

	virtual GEMWebApplication*  getWebApp()  const { return gemWebInterfaceP_; };
	virtual GEMMonitor*         getMonitor() const { return gemMonitorP_;      };

	GEMWebApplication* gemWebInterfaceP_; /* */
	GEMMonitor*        gemMonitorP_;      /* */

	/**
	 * various application properties
	 */
	xdaq::ApplicationDescriptor *appDescriptorP_; /* */
	xdaq::ApplicationContext    *appContextP_;    /* */
	xdaq::ApplicationGroup      *appGroupP_;      /* */
	xdaq::Zone                  *appZoneP_;       /* */

	std::string xmlClass_;
	unsigned long instance_;
	std::string urn_;
	
      private:
	
        //xdaq2rc::RcmsStateNotifier rcmsStateNotifier_;

      };
    
  } // namespace gem::base
} // namespace gem

#endif
