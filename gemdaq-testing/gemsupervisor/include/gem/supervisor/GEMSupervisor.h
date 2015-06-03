#ifndef gem_supervisor_GEMSupervisor_h
#define gem_supervisor_GEMSupervisor_h

#include "xdaq/Application.h"
#include "xdata/UnsignedLong.h"
#include "xdata/UnsignedInteger32.h"

#include "uhal/uhal.hpp"

#include "gem/base/GEMFSMApplication.h"
#include "gem/supervisor/GEMSupervisorWeb.h"

#include "gem/utils/Lock.h"
#include "gem/utils/LockGuard.h"

namespace gem {
  namespace supervisor {
    
    class GEMSupervisorWeb;
    
    class GEMSupervisor : public gem::base::GEMFSMApplication
    {
      friend class GEMSupervisorWeb;
    public:
      XDAQ_INSTANTIATOR();

      GEMSupervisor(xdaq::ApplicationStub* s);

      virtual ~GEMSupervisor();
      
      void init();

      void actionPerformed(xdata::Event& event);

    protected:
      //virtual void xgiDefault(   xgi::Input *in, xgi::Output *out );
      
      virtual bool initialize(toolbox::task::WorkLoop *wl);
      virtual bool enable(    toolbox::task::WorkLoop *wl);
      virtual bool configure( toolbox::task::WorkLoop *wl);
      virtual bool start(     toolbox::task::WorkLoop *wl);
      virtual bool pause(     toolbox::task::WorkLoop *wl);
      virtual bool resume(    toolbox::task::WorkLoop *wl);
      virtual bool stop(      toolbox::task::WorkLoop *wl);
      virtual bool halt(      toolbox::task::WorkLoop *wl);
      virtual bool reset(     toolbox::task::WorkLoop *wl);
      /* //virtual bool noAction(        toolbox::task::WorkLoop *wl); */
      /* virtual bool fail(      toolbox::task::WorkLoop *wl); */
      
      /* virtual bool calibrationAction(toolbox::task::WorkLoop *wl); */
      /* virtual bool calibrationSequencer(toolbox::task::WorkLoop *wl); */

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
      mutable gem::utils::Lock deviceLock_;
      std::vector<xdaq::ApplicationDescriptor*> v_supervisedApps;
      xdaq::ApplicationDescriptor* readoutApp;
      
    };
  } //end namespace supervisor
} //end namespace gem
#endif
