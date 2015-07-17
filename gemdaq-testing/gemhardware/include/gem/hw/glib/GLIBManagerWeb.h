#ifndef gem_hw_glib_GLIBManagerWeb_h
#define gem_hw_glib_GLIBManagerWeb_h

#include <memory>

#include "gem/base/GEMWebApplication.h"

namespace gem {
  namespace hw {
    namespace glib {
      
      class GLIBManager;

      class GLIBManagerWeb : public gem::base::GEMWebApplication
	{
	  //friend class GLIBMonitor;
	  //friend class GLIBManager;

	public:
	  GLIBManagerWeb(GLIBManager *glibApp);
	  
	  ~GLIBManagerWeb();

	protected:
	  virtual void monitorPage(xgi::Input *in, xgi::Output *out)
	    throw (xgi::exception::Exception);
	
	  virtual void expertPage(xgi::Input *in, xgi::Output *out)
	    throw (xgi::exception::Exception);

	  void cardPage(xgi::Input *in, xgi::Output *out)
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
	  size_t activeCard;
	  
	  //GLIBManagerWeb(GLIBManagerWeb const&);
	  
	};
    } // namespace gem::glib
  } // namespace gem::hw
} // namespace gem

#endif
