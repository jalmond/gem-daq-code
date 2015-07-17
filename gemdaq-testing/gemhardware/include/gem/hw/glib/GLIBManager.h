#ifndef gem_hw_glib_GLIBManager_h
#define gem_hw_glib_GLIBManager_h

#include <string>

#include "xdata/String.h"
#include "xdata/UnsignedLong.h"
#include "xdata/UnsignedInteger32.h"

#include "uhal/uhal.hpp"

#include "xgi/framework/Method.h"
#include "cgicc/HTMLClasses.h"

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

#include "gem/base/GEMFSMApplication.h"
//#include "gem/hw/glib/GLIBSettings.h"

namespace gem {
  namespace base {
  }
  
  namespace hw {
    namespace glib {
      class HwGLIB;
      class GLIBManagerWeb;

      class GLIBManager : public gem::base::GEMFSMApplication
	{
	  
	  friend class GLIBManagerWeb;

	public:
	  XDAQ_INSTANTIATOR();
	  
	  GLIBManager(xdaq::ApplicationStub* s);

	  virtual ~GLIBManager();
	  
	protected:
	  
	  virtual void preInit() throw (gem::base::exception::Exception);
	  virtual void init()    throw (gem::base::exception::Exception);
	  virtual void enable()  throw (gem::base::exception::Exception);
	  virtual void disable() throw (gem::base::exception::Exception);
	  
	  virtual void actionPerformed(xdata::Event& event);
	  
	  /*
	  // work loop call-back functions
	  virtual bool initializeAction(toolbox::task::WorkLoop *wl);
	  virtual bool enableAction(    toolbox::task::WorkLoop *wl);
	  virtual bool configureAction( toolbox::task::WorkLoop *wl);
	  virtual bool startAction(     toolbox::task::WorkLoop *wl);
	  virtual bool pauseAction(     toolbox::task::WorkLoop *wl);
	  virtual bool resumeAction(    toolbox::task::WorkLoop *wl);
	  virtual bool stopAction(      toolbox::task::WorkLoop *wl);
	  virtual bool haltAction(      toolbox::task::WorkLoop *wl);
	  virtual bool resetAction(     toolbox::task::WorkLoop *wl);
	  //virtual bool noAction(        toolbox::task::WorkLoop *wl);
	  virtual bool failAction(      toolbox::task::WorkLoop *wl);

	  //bool calibrationAction(toolbox::task::WorkLoop *wl);
	  //bool calibrationSequencer(toolbox::task::WorkLoop *wl);
	  */
	
	  //state transitions
	  virtual void initializeAction();
	  virtual void enableAction(    );
	  virtual void configureAction( );
	  virtual void startAction(     );
	  virtual void pauseAction(     );
	  virtual void resumeAction(    );
	  virtual void stopAction(      );
	  virtual void haltAction(      );
	  virtual void noAction(        ); 
	
	  virtual void failAction(toolbox::Event::Reference e)
	    throw (toolbox::fsm::exception::Exception); 
	
	  virtual void resetAction(toolbox::Event::Reference e)
	    throw (toolbox::fsm::exception::Exception);
	
	  
	private:
	  
	  class GLIBInfo {
	    
	  public:
	    GLIBInfo();
	    void registerFields(xdata::Bag<GLIBManager::GLIBInfo>* bag);
	    xdata::Boolean present;
	    xdata::Integer crateID;
	    xdata::Integer slotID;
	  };
	  
	  mutable gem::utils::Lock m_deviceLock;//[MAX_AMCS_PER_CRATE];
	  
	  HwGLIB* m_glibs[MAX_AMCS_PER_CRATE];
	  xdata::Bag<GLIBInfo> m_glibInfo[MAX_AMCS_PER_CRATE];
	}; //end class GLIBManager
      
    }//end namespace gem::hw::glib
    
  }//end namespace gem::hw
  
}//end namespace gem

#endif
