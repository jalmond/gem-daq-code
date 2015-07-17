#include "toolbox/version.h"
#include "xoap/version.h"
#include "xdaq/version.h"
#include "gem/base/version.h"
#include "gem/utils/version.h"
#include "gem/hw/version.h"
#include "gem/hwMonitor/version.h"

GETPACKAGEINFO(gemHwMonitor);

void gemHwMonitor::checkPackageDependencies() throw (config::PackageInfo::VersionException) {
  CHECKDEPENDENCY(toolbox);
  CHECKDEPENDENCY(xdaq);
  CHECKDEPENDENCY(xoap);
  CHECKDEPENDENCY(gembase);
  CHECKDEPENDENCY(gemutils);
  CHECKDEPENDENCY(gemhardware);
}

std::set<std::string,std::less<std::string> > gemHwMonitor::getPackageDependencies() {
  std::set<std::string,std::less<std::string> > deps;
  ADDDEPENDENCY(deps,toolbox);
  ADDDEPENDENCY(deps,xoap);
  ADDDEPENDENCY(deps,xdaq);
  ADDDEPENDENCY(deps,gembase);
  ADDDEPENDENCY(deps,gemutils);
  ADDDEPENDENCY(deps,gemhardware);
  return deps;
}
