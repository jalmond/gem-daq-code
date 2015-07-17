#include "toolbox/version.h"
#include "xoap/version.h"
#include "xcept/version.h"
#include "xdaq/version.h"
#include "gem/base/version.h"
#include "gem/utils/version.h"

GETPACKAGEINFO(gembase)

void gembase::checkPackageDependencies() throw (config::PackageInfo::VersionException)
{
  CHECKDEPENDENCY(toolbox);
  CHECKDEPENDENCY(xcept);
  CHECKDEPENDENCY(xdaq);
  CHECKDEPENDENCY(xoap);
  CHECKDEPENDENCY(gemutils);
}

std::set<std::string, std::less<std::string> > gembase::getPackageDependencies()
{
  std::set<std::string, std::less<std::string> > dependencies;
  ADDDEPENDENCY(dependencies,toolbox);
  ADDDEPENDENCY(dependencies,xcept);
  ADDDEPENDENCY(dependencies,xdaq);
  ADDDEPENDENCY(dependencies,xoap);
  ADDDEPENDENCY(dependencies,gemutils);
  return dependencies;
}
