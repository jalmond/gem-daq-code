#ifndef gem_base_utils_exception_Exception_h
#define gem_base_utils_exception_Exception_h

#include <string>

#include "gem/utils/exception/Exception.h"

#define GEM_BASE_UTILS_DEFINE_EXCEPTION(EXCEPTION_NAME)			\
  namespace gem {				                        \
    namespace base {				                        \
      namespace utils {				                        \
	namespace exception {					        \
	  class EXCEPTION_NAME : virtual public xcept::Exception \
	    {								\
	    public :							\
	    EXCEPTION_NAME(std::string name,				\
			   std::string message,				\
			   std::string module,				\
			   int line,					\
			   std::string function) :			\
	      xcept::Exception(name, message, module, line, function) \
		{};							\
	    EXCEPTION_NAME(std::string name,				\
			   std::string message,				\
			   std::string module,				\
			   int line,					\
			   std::string function,			\
			   xcept::Exception& err) :		\
	      xcept::Exception(name, message, module, line, function, err) \
		{};							\
	    };								\
	}								\
      }									\
    }									\
  } 

// The gem::base::utils exceptions.
GEM_BASE_UTILS_DEFINE_EXCEPTION(Exception)
GEM_BASE_UTILS_DEFINE_EXCEPTION(CfgParseProblem)
GEM_BASE_UTILS_DEFINE_EXCEPTION(CfgProblem)
GEM_BASE_UTILS_DEFINE_EXCEPTION(CfgValidationProblem)

GEM_BASE_UTILS_DEFINE_EXCEPTION(RCMSNotificationError)
GEM_BASE_UTILS_DEFINE_EXCEPTION(SOAPTransitionProblem)

GEM_BASE_UTILS_DEFINE_EXCEPTION(SoftwareProblem)
GEM_BASE_UTILS_DEFINE_EXCEPTION(TransitionProblem)
GEM_BASE_UTILS_DEFINE_EXCEPTION(ValueError)

// The gem::base::utils alarms.
#define GEM_BASE_UTILS_DEFINE_ALARM(ALARM_NAME) GEM_BASE_UTILS_DEFINE_EXCEPTION(ALARM_NAME)

GEM_BASE_UTILS_DEFINE_ALARM(MonitoringFailureAlarm)

#endif // gem_base_utils_exception_Exception_h
