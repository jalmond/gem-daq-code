#ifndef gem_supervisor_GEMSupervisorWeb_h
#define gem_supervisor_GEMSupervisorWeb_h

#include <memory>

#include "gem/base/GEMWebApplication.h"

namespace gem {
  namespace supervisor {

    class GEMSupervisor;

    class GEMSupervisorWeb: public gem::base::GEMWebApplication
      {
	//friend class GEMMonitor;
	friend class GEMSupervisor;
      public:
	GEMSupervisorWeb(GEMSupervisor *gemSupervisorApp);
	
	virtual ~GEMSupervisorWeb();
	
      protected:
	virtual void controlPanel(  xgi::Input *in, xgi::Output *out )
	  throw (xgi::exception::Exception);

	virtual void monitorPage(xgi::Input *in, xgi::Output *out)
	    throw (xgi::exception::Exception);
	
	virtual void expertPage(xgi::Input *in, xgi::Output *out)
	  throw (xgi::exception::Exception);
	
	virtual void webRedirect(  xgi::Input *in, xgi::Output *out )
	  throw (xgi::exception::Exception);
	
	virtual void webDefault(  xgi::Input *in, xgi::Output *out )
	  throw (xgi::exception::Exception);

	//FSM web callbacks
	/* virtual void webInitialize(xgi::Input *in, xgi::Output *out ) */
	/*   throw (xgi::exception::Exception); */
	/* virtual void webConfigure( xgi::Input *in, xgi::Output *out ) */
	/*   throw (xgi::exception::Exception); */
	/* virtual void webEnable(    xgi::Input *in, xgi::Output *out ) */
	/*   throw (xgi::exception::Exception); */
	/* virtual void webStart(     xgi::Input *in, xgi::Output *out ) */
	/*   throw (xgi::exception::Exception); */
	/* virtual void webPause(     xgi::Input *in, xgi::Output *out ) */
	/*   throw (xgi::exception::Exception); */
	/* virtual void webResume(    xgi::Input *in, xgi::Output *out ) */
	/*   throw (xgi::exception::Exception); */
	/* virtual void webStop(      xgi::Input *in, xgi::Output *out ) */
	/*   throw (xgi::exception::Exception); */
	/* virtual void webHalt(      xgi::Input *in, xgi::Output *out ) */
	/*   throw (xgi::exception::Exception); */
	/* virtual void webReset(     xgi::Input *in, xgi::Output *out ) */
	/*   throw (xgi::exception::Exception); */
	
	
      private:
	size_t level;
	//GEMSupervisor *gemSupervisorP__;
	//GEMSupervisorWeb(GEMSupervisorWeb const&);
	
      }; //class gem::supervisor::GEMSupervisorWeb
  } // namespace gem::supervisor
} // namespace gem
#endif
