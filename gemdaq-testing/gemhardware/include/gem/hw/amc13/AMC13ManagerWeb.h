#ifndef gem_hw_amc13_AMC13ManagerWeb_h
#define gem_hw_amc13_AMC13ManagerWeb_h

#include <memory>

#include "gem/base/GEMWebApplication.h"
//#include "gem/hw/amc13/AMC13Manager.h"

namespace gem {
  namespace hw {
    namespace amc13 {
      
      class AMC13Manager;
      
      class AMC13ManagerWeb : public gem::base::GEMWebApplication
	{
	  //friend class AMC13Monitor;
	  //friend class AMC13Manager;
	  
	public:
	  AMC13ManagerWeb(AMC13Manager *amc13App);
	  
	  virtual ~AMC13ManagerWeb();

	protected:
	  virtual void monitorPage(xgi::Input *in, xgi::Output *out)
	    throw (xgi::exception::Exception);
	
	  virtual void expertPage(xgi::Input *in, xgi::Output *out)
	    throw (xgi::exception::Exception);

	  /*
	  //FSM web callbacks
	  virtual void webInitialize(xgi::Input *in, xgi::Output *out )
	    throw (xgi::exception::Exception);
	  virtual void webEnable(    xgi::Input *in, xgi::Output *out )
	    throw (xgi::exception::Exception);
	  virtual void webConfigure( xgi::Input *in, xgi::Output *out )
	    throw (xgi::exception::Exception);
	  virtual void webStart(     xgi::Input *in, xgi::Output *out )
	    throw (xgi::exception::Exception);
	  virtual void webPause(     xgi::Input *in, xgi::Output *out )
	    throw (xgi::exception::Exception);
	  virtual void webResume(    xgi::Input *in, xgi::Output *out )
	    throw (xgi::exception::Exception);
	  virtual void webStop(      xgi::Input *in, xgi::Output *out )
	    throw (xgi::exception::Exception);
	  virtual void webHalt(      xgi::Input *in, xgi::Output *out )
	    throw (xgi::exception::Exception);
	  virtual void webReset(     xgi::Input *in, xgi::Output *out )
	    throw (xgi::exception::Exception);
	  
	  virtual void webRedirect(  xgi::Input *in, xgi::Output *out )
	    throw (xgi::exception::Exception);
	  */
	
	private:
	  size_t level;
	  //AMC13Manager *amc13ManagerP_;
	  //AMC13ManagerWeb(AMC13ManagerWeb const&);
	  
	};
    } // namespace gem::amc13
  } // namespace gem::hw
} // namespace gem

#endif
