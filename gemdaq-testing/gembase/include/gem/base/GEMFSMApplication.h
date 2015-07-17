#ifndef gem_base_GEMFSMApplication_h
#define gem_base_GEMFSMApplication_h

#include "gem/base/GEMApplication.h"
#include "gem/base/GEMFSM.h"

#include "toolbox/task/exception/Exception.h"

#include "xdata/Boolean.h"
#include "xdata/Integer.h"

#include "gem/utils/Lock.h"
#include "gem/utils/LockGuard.h"

#define MAX_AMCS_PER_CRATE 12

namespace toolbox {
  namespace task{
    class WorkLoop;
    class ActionSignature;
  }
}

namespace gem {
  namespace base {
    class GEMFSM;
    class GEMApplication;
    class GEMWebApplication;

    class GEMFSMApplication : public GEMApplication
      {
      public:
	friend class GEMFSM;
	friend class GEMApplication;
	friend class GEMWebApplication;
	
	GEMFSMApplication(xdaq::ApplicationStub *stub)
	  throw (xdaq::exception::Exception);

	virtual ~GEMFSMApplication();

      protected:
	//xgi interface
	virtual void xgiInitialize(xgi::Input *in, xgi::Output *out );
	virtual void xgiEnable(    xgi::Input *in, xgi::Output *out );
	virtual void xgiConfigure( xgi::Input *in, xgi::Output *out );
	virtual void xgiStart(     xgi::Input *in, xgi::Output *out );
	virtual void xgiPause(     xgi::Input *in, xgi::Output *out );
	virtual void xgiResume(    xgi::Input *in, xgi::Output *out );
	virtual void xgiStop(      xgi::Input *in, xgi::Output *out );
	virtual void xgiHalt(      xgi::Input *in, xgi::Output *out );
 	virtual void xgiReset(     xgi::Input *in, xgi::Output *out );


	/*
	// SOAP interface
	virtual xoap::MessageReference onEnable(     xoap::MessageReference message)
	  throw (xoap::exception::Exception);
	virtual xoap::MessageReference onConfigure(  xoap::MessageReference message)
	  throw (xoap::exception::Exception);
	virtual xoap::MessageReference onStart(      xoap::MessageReference message)
	  throw (xoap::exception::Exception);
	virtual xoap::MessageReference onPause(      xoap::MessageReference message)
	  throw (xoap::exception::Exception);
	virtual xoap::MessageReference onResume(     xoap::MessageReference message)
	  throw (xoap::exception::Exception);
	virtual xoap::MessageReference onStop(       xoap::MessageReference message)
	  throw (xoap::exception::Exception);
	virtual xoap::MessageReference onHalt(       xoap::MessageReference message)
	  throw (xoap::exception::Exception);
	virtual xoap::MessageReference onReset(      xoap::MessageReference message)
	  throw (xoap::exception::Exception);
	virtual xoap::MessageReference onRunSequence(xoap::MessageReference message)
	  throw (xoap::exception::Exception);
	virtual xoap::MessageReference reset(        xoap::MessageReference message)
	  throw (xoap::exception::Exception);
	virtual xoap::MessageReference fireEvent(    xoap::MessageReference message)
	  throw (xoap::exception::Exception);
	virtual xoap::MessageReference createReply(  xoap::MessageReference message)
	  throw (xoap::exception::Exception);
	*/

	// work loop call-back functions
	void workloopDriver(std::string const& command)
	  throw (::toolbox::task::exception::Exception);
	
	std::string workLoopName;
	toolbox::task::ActionSignature* initSig_  ;
	toolbox::task::ActionSignature* confSig_  ;
	toolbox::task::ActionSignature* startSig_ ;
	toolbox::task::ActionSignature* stopSig_  ;
	toolbox::task::ActionSignature* pauseSig_ ;
	toolbox::task::ActionSignature* resumeSig_;
	toolbox::task::ActionSignature* haltSig_  ;
	toolbox::task::ActionSignature* resetSig_ ;
	
	virtual bool initialize(toolbox::task::WorkLoop *wl) { return false; };
	virtual bool enable(    toolbox::task::WorkLoop *wl) { return false; };
	virtual bool configure( toolbox::task::WorkLoop *wl) { return false; };
	virtual bool start(     toolbox::task::WorkLoop *wl) { return false; };
	virtual bool pause(     toolbox::task::WorkLoop *wl) { return false; };
	virtual bool resume(    toolbox::task::WorkLoop *wl) { return false; };
	virtual bool stop(      toolbox::task::WorkLoop *wl) { return false; };
	virtual bool halt(      toolbox::task::WorkLoop *wl) { return false; };
	virtual bool reset(     toolbox::task::WorkLoop *wl) { return false; };
	//virtual bool noAction(        toolbox::task::WorkLoop *wl) { return false; };
	virtual bool fail(      toolbox::task::WorkLoop *wl) { return false; };

	virtual bool calibrationAction(toolbox::task::WorkLoop *wl) { return false; };
	virtual bool calibrationSequencer(toolbox::task::WorkLoop *wl) { return false; };
	
	/* state transitions
	 * defines the behaviour of the application for each state transition
	 * most will be pure virtual to enforce derived application specific
	 * implementations, common implementations will be merged into the base
	 * application
	 */
	void transitionDriver(::toolbox::Event::Reference e)
	  throw (::toolbox::fsm::exception::Exception);
	
 	virtual void initializeAction() = 0;
 	virtual void configureAction( ) = 0;
	virtual void enableAction(    ) = 0;
	virtual void startAction(     ) = 0;
	virtual void pauseAction(     ) = 0;
	virtual void resumeAction(    ) = 0;
	virtual void stopAction(      ) = 0;
	virtual void haltAction(      ) = 0;
	
	virtual void noAction(        ) = 0; 
	//virtual void failAction(      ) = 0; 
	
	/* resetAction
	 * takes the GEMFSM from a state to the uninitialzed state
	 * recovery from a failed transition, or just a reset
	 */
	virtual void resetAction(::toolbox::Event::Reference e)
	  throw (::toolbox::fsm::exception::Exception);
	
	/* failAction
	 * determines how to handle a failed transition
	 * 
	 */
	virtual void failAction(::toolbox::Event::Reference e)
	  throw (::toolbox::fsm::exception::Exception);
	
	/* stateChanged
	 * 
	 * 
	 */
	virtual void stateChanged(    toolbox::fsm::FiniteStateMachine &fsm)
	  throw (::toolbox::fsm::exception::Exception);

	/* transitionFailed
	 * 
	 * 
	 */
	virtual void transitionFailed(::toolbox::Event::Reference event)
	  throw (::toolbox::fsm::exception::Exception);

	/* fireEvent
	 * Forwards a state change to the GEMFSM object
	 * @param std::string event name of the event to pass to the GEMFSM
	 */
	virtual void fireEvent(std::string event)
	  throw (::toolbox::fsm::exception::Exception);
	
	/* changeState
	 * Forwards a state change to the GEMFSM object
	 * @param xoap::MessageReference msg message containing the state transition
	 * @returns xoap::MessageReference response of the SOAP transaction
	 */
	virtual xoap::MessageReference changeState(xoap::MessageReference msg);

	/* getCurrentState
	 * @returns std::string name of the current state of the GEMFSM object
	 */
	std::string getCurrentState() const {
	  return gemfsm_.getCurrentState();
	};
	
      private:
	GEMFSM gemfsm_;
	
	toolbox::BSem wl_semaphore_;

	toolbox::task::WorkLoop *wl_;

	toolbox::task::ActionSignature *enable_signature_, *configure_signature_;
	toolbox::task::ActionSignature *start_signature_,  *pause_signature_, *resume_signature_;
	toolbox::task::ActionSignature *stop_signature_,   *halt_signature_;
	
	xdata::InfoSpace* gemAppStateInfoSpaceP_;

      };
    
  } // namespace gem::base
} // namespace gem

#endif
