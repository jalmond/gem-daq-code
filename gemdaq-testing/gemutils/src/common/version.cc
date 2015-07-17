#include "toolbox/version.h"
#include "xoap/version.h"
#include "xdaq/version.h"
#include "gem/utils/version.h"

GETPACKAGEINFO(gemutils);

void gemutils::checkPackageDependencies() throw (config::PackageInfo::VersionException) {
  CHECKDEPENDENCY(toolbox);
  CHECKDEPENDENCY(xdaq);
  CHECKDEPENDENCY(xoap);
}

std::set<std::string,std::less<std::string> > gemutils::getPackageDependencies() {
  std::set<std::string,std::less<std::string> > deps;
  ADDDEPENDENCY(deps,toolbox);
  ADDDEPENDENCY(deps,xoap);
  ADDDEPENDENCY(deps,xdaq);
  return deps;
}
