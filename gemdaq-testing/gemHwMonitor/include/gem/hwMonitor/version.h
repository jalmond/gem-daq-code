#ifndef gem_hwMonitor_version_h
#define gem_hwMonitor_version_h

#ifndef DOXYGEN_IGNORE_THIS

#include "config/PackageInfo.h"

//namespace gem {
//  namespace hwMonitor {
namespace gemHwMonitor {

#define GEMHWMONITOR_VERSION_MAJOR 0
#define GEMHWMONITOR_VERSION_MINOR 1
#define GEMHWMONITOR_VERSION_PATCH 0
//#define GEMHWMONITOR_PREVIOUS_VERSIONS
#undef GEMHWMONITOR_PREVIOUS_VERSIONS

#define GEMHWMONITOR_VERSION_CODE PACKAGE_VERSION_CODE(GEMHWMONITOR_VERSION_MAJOR,GEMHWMONITOR_VERSION_MINOR, GEMHWMONITOR_VERSION_PATCH)

#ifndef GEMHWMONITOR_PREVIOUS_VERSIONS
#define GEMHWMONITOR_FULL_VERSION_LIST PACKAGE_VERSION_STRING(GEMHWMONITOR_VERSION_MAJOR,GEMHWMONITOR_VERSION_MINOR, GEMHWMONITOR_VERSION_PATCH)
#else
#define GEMHWMONITOR_FULL_VERSION_LIST GEMHWMONITOR_PREVIOUS_VERSIONS "," PACKAGE_VERSION_STRING(GEMHWMONITOR_VERSION_MAJOR, GEMHWMONITOR_VERSION_MINOR,GEMHWMONITOR_VERSION_PATCH)
#endif
    
    const std::string package = "gem/hwMonitor";
    const std::string versions = GEMHWMONITOR_FULL_VERSION_LIST;
    const std::string summary = "GEM hwMonitor";
    const std::string description = "";
    const std::string authors = "GEM Online Systems Group";
    const std::string link = "";
    
    config::PackageInfo getPackageInfo();
    void checkPackageDependencies() throw (config::PackageInfo::VersionException);
    std::set<std::string, std::less<std::string> > getPackageDependencies();
    //  }
}

#endif // DOXYGEN_IGNORE_THIS

#endif // gem_hwMonitor_version_h
