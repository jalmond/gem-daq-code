#include "toolbox/version.h"
#include "xoap/version.h"
#include "xdaq/version.h"
#include "gem/base/version.h"
#include "gem/utils/version.h"
#include "gem/readout/version.h"
#include "gem/hw/version.h"
#include "gem/supervisor/version.h"

GETPACKAGEINFO(gemsupervisor);

void gemsupervisor::checkPackageDependencies() throw (config::PackageInfo::VersionException) {
  CHECKDEPENDENCY(toolbox);
  CHECKDEPENDENCY(xdaq);
  CHECKDEPENDENCY(xoap);
  CHECKDEPENDENCY(gembase);
  CHECKDEPENDENCY(gemutils);
  CHECKDEPENDENCY(gemreadout);
  CHECKDEPENDENCY(gemhardware);
}

std::set<std::string,std::less<std::string> > gemsupervisor::getPackageDependencies() {
  std::set<std::string,std::less<std::string> > deps;
  ADDDEPENDENCY(deps,toolbox);
  ADDDEPENDENCY(deps,xoap);
  ADDDEPENDENCY(deps,xdaq);
  ADDDEPENDENCY(deps,gembase);
  ADDDEPENDENCY(deps,gemutils);
  ADDDEPENDENCY(deps,gemreadout);
  ADDDEPENDENCY(deps,gemhardware);
  return deps;
}
