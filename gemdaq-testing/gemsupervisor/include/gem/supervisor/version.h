#ifndef gem_supervisor_version_h
#define gem_supervisor_version_h

#ifndef DOXYGEN_IGNORE_THIS

#include "config/PackageInfo.h"

//namespace gem {
//  namespace supervisor {
namespace gemsupervisor {

#define GEMSUPERVISOR_VERSION_MAJOR 0
#define GEMSUPERVISOR_VERSION_MINOR 1
#define GEMSUPERVISOR_VERSION_PATCH 0
//#define GEMSUPERVISOR_PREVIOUS_VERSIONS
#undef GEMSUPERVISOR_PREVIOUS_VERSIONS

#define GEMSUPERVISOR_VERSION_CODE PACKAGE_VERSION_CODE(GEMSUPERVISOR_VERSION_MAJOR,GEMSUPERVISOR_VERSION_MINOR, GEMSUPERVISOR_VERSION_PATCH)

#ifndef GEMSUPERVISOR_PREVIOUS_VERSIONS
#define GEMSUPERVISOR_FULL_VERSION_LIST PACKAGE_VERSION_STRING(GEMSUPERVISOR_VERSION_MAJOR,GEMSUPERVISOR_VERSION_MINOR, GEMSUPERVISOR_VERSION_PATCH)
#else
#define GEMSUPERVISOR_FULL_VERSION_LIST GEMSUPERVISOR_PREVIOUS_VERSIONS "," PACKAGE_VERSION_STRING(GEMSUPERVISOR_VERSION_MAJOR, GEMSUPERVISOR_VERSION_MINOR,GEMSUPERVISOR_VERSION_PATCH)
#endif
    
    const std::string package = "gem/supervisor";
    const std::string versions = GEMSUPERVISOR_FULL_VERSION_LIST;
    const std::string summary = "GEM Supervisor";
    const std::string description = "";
    const std::string authors = "GEM Online Systems Group";
    const std::string link = "";
    
    config::PackageInfo getPackageInfo();
    void checkPackageDependencies() throw (config::PackageInfo::VersionException);
    std::set<std::string, std::less<std::string> > getPackageDependencies();
    //  }
}

#endif // DOXYGEN_IGNORE_THIS

#endif // gem_supervisor_version_h
