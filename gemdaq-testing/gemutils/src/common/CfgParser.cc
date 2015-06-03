#include "gem/utils/CfgParser.h"

uint32_t gem::utils::processRegisterSetting(std::string& regValue)
{
  //std::string myArg = regValue;
  boost::algorithm::to_lower(regValue);
  
  //ensure only numbers or hex allowed characters
  boost::regex expression("[a-fA-F0-9xX-]*$");
  //at least one char for a hex string, to differentiate from 
  boost::regex hexcheck("[a-fA-F]1*$");
  
  boost::cmatch what;
  bool result = boost::regex_match(regValue,expression);
  bool ishex  = boost::regex_search(regValue,hexcheck,boost::match_partial);
  
  bool valid = result;
  size_t n = std::count(regValue.begin(), regValue.end(), 'x');
  if (n > 1)
    valid = false;
  else if (n == 1) {
    if (regValue.find("0x") != 0)
      valid = false;
  }
  else if (ishex) {
    if (regValue.find("0x") != 0)
      valid = false;
  }

  if (valid) {
    return  strtoull(regValue.c_str(), NULL, 0);
  }
  else {
    std::cout << "Unable to parse " << regValue << " as a represenation of an integer" << std::endl;
    //logger
    return 0;
  }
}
