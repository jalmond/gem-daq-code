#ifndef gem_utils_GEMLogging_h
#define gem_utils_GEMLogging_h

#include "log4cplus/logger.h"
#include "log4cplus/loglevel.h"
#include "log4cplus/loggingmacros.h"

namespace gem {
#define DEBUG(MSG) LOG4CPLUS_DEBUG(gemLogger_ , MSG)
#define INFO( MSG) LOG4CPLUS_INFO( gemLogger_ , MSG)
#define WARN( MSG) LOG4CPLUS_WARN( gemLogger_ , MSG)
#define ERROR(MSG) LOG4CPLUS_ERROR(gemLogger_ , MSG)
#define FATAL(MSG) LOG4CPLUS_FATAL(gemLogger_ , MSG)
  
#define DEBUG_LOGGER(LOGGER,MSG) LOG4CPLUS_DEBUG(LOGGER, MSG)
#define INFO_LOGGER( LOGGER,MSG) LOG4CPLUS_INFO( LOGGER, MSG)
#define WARN_LOGGER( LOGGER,MSG) LOG4CPLUS_WARN( LOGGER, MSG)
#define ERROR_LOGGER(LOGGER,MSG) LOG4CPLUS_ERROR(LOGGER, MSG)
#define FATAL_LOGGER(LOGGER,MSG) LOG4CPLUS_FATAL(LOGGER, MSG)

  //generic function to trace hierarchy in the Logger objects from non-xdaq applications
  //copied from HCAL hcalHTR.cc
//  void setLogger(const log4cplus::Logger& parentLogger) {
//    m_logger = log4cplus::Logger::getInstance(buildLogName(parentLogger,m_slot));
//  }

}

#endif
