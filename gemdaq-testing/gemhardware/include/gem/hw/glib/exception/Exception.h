#ifndef gem_hw_glib_exception_Exception_h
#define gem_hw_glib_exception_Exception_h

#include <string>

#include "gem/hw/exception/Exception.h"

#define GEM_HW_GLIB_DEFINE_EXCEPTION(EXCEPTION_NAME)			\
  namespace gem {							\
    namespace hw {							\
      namespace glib {							\
	namespace exception {						\
	  class EXCEPTION_NAME : virtual public xcept::Exception	\
	    {								\
	    public :							\
	    EXCEPTION_NAME() :						\
	      xcept::Exception()					\
		{};							\
	    EXCEPTION_NAME(std::string name,				\
			   std::string message,				\
			   std::string module,				\
			   int line,					\
			   std::string function) :			\
	      xcept::Exception(name, message, module, line, function)	\
		{};							\
	    EXCEPTION_NAME(std::string name,				\
			   std::string message,				\
			   std::string module,				\
			   int line,					\
			   std::string function,			\
			   gem::hw::exception::Exception& err) :	\
	      xcept::Exception(name, message, module, line, function, err) \
		{};							\
	    };								\
	}                                                               \
      }                                                                 \
    }                                                                   \
  } 

// The gem::hw::glib exceptions.
GEM_HW_GLIB_DEFINE_EXCEPTION(Exception)
GEM_HW_GLIB_DEFINE_EXCEPTION(ConfigurationParseProblem)
GEM_HW_GLIB_DEFINE_EXCEPTION(ConfigurationProblem)
GEM_HW_GLIB_DEFINE_EXCEPTION(ConfigurationValidationProblem)

GEM_HW_GLIB_DEFINE_EXCEPTION(HardwareProblem)
GEM_HW_GLIB_DEFINE_EXCEPTION(InvalidLink)
GEM_HW_GLIB_DEFINE_EXCEPTION(InvalidXPointRouting)
GEM_HW_GLIB_DEFINE_EXCEPTION(InvalidXPoint2Routing)

GEM_HW_GLIB_DEFINE_EXCEPTION(RCMSNotificationError)
GEM_HW_GLIB_DEFINE_EXCEPTION(SOAPTransitionProblem)
GEM_HW_GLIB_DEFINE_EXCEPTION(TransitionProblem)

GEM_HW_GLIB_DEFINE_EXCEPTION(SoftwareProblem)
GEM_HW_GLIB_DEFINE_EXCEPTION(ValueError)

// The gem::hw::glib alarms.
#define GEM_HW_GLIB_DEFINE_ALARM(ALARM_NAME) GEM_HW_GLIB_DEFINE_EXCEPTION(ALARM_NAME)

GEM_HW_GLIB_DEFINE_ALARM(MonitoringFailureAlarm)

#endif // gem_hw_glib_exception_Exception_h
