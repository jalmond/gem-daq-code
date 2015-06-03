#ifndef gem_utils_CfgParser
#define gem_utils_CfgParser

#include <stdlib.h>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>

namespace gem {
  namespace utils {
    class CfgParser {
	
    public:
      uint32_t processRegisterSetting(std::string regValue);
      
    };
  } //end namespace gem::utils
} //end namespace gem

#endif
