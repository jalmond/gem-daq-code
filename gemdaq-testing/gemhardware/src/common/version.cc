#include "toolbox/version.h"
#include "xoap/version.h"
#include "xdaq/version.h"
#include "gem/base/version.h"
#include "gem/utils/version.h"
#include "gem/hw/version.h"

GETPACKAGEINFO(gemhardware);

void gemhardware::checkPackageDependencies() throw (config::PackageInfo::VersionException) {
  CHECKDEPENDENCY(toolbox);
  CHECKDEPENDENCY(xdaq);
  CHECKDEPENDENCY(xoap);
  CHECKDEPENDENCY(gembase);
  CHECKDEPENDENCY(gemutils);
}

std::set<std::string,std::less<std::string> > gemhardware::getPackageDependencies() {
  std::set<std::string,std::less<std::string> > deps;
  ADDDEPENDENCY(deps,toolbox);
  ADDDEPENDENCY(deps,xoap);
  ADDDEPENDENCY(deps,xdaq);
  ADDDEPENDENCY(deps,gembase);
  ADDDEPENDENCY(deps,gemutils);
  return deps;
}
