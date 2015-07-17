#ifndef gem_base_version_h
#define gem_base_version_h

#ifndef DOXYGEN_IGNORE_THIS

#include "config/PackageInfo.h"

//namespace gem {
//  namespace base {
namespace gembase {

#define GEMBASE_VERSION_MAJOR 0
#define GEMBASE_VERSION_MINOR 1
#define GEMBASE_VERSION_PATCH 0

//#undef GEMBASE_PREVIOUS_VERSIONS

#define GEMBASE_VERSION_CODE PACKAGE_VERSION_CODE(GEMBASE_VERSION_MAJOR,GEMBASE_VERSION_MINOR, GEMBASE_VERSION_PATCH)
#ifndef GEMBASE_PREVIOUS_VERSIONS
#define GEMBASE_FULL_VERSION_LIST PACKAGE_VERSION_STRING(GEMBASE_VERSION_MAJOR,GEMBASE_VERSION_MINOR, GEMBASE_VERSION_PATCH)
#else
#define GEMBASE_FULL_VERSION_LIST GEMBASE_PREVIOUS_VERSIONS "," PACKAGE_VERSION_STRING(GEMBASE_VERSION_MAJOR, GEMBASE_VERSION_MINOR,GEMBASE_VERSION_PATCH)
#endif

    const std::string package = "gem/base";
    const std::string versions = GEMBASE_FULL_VERSION_LIST;
    const std::string summary = "Base classes for GEM DAQ applications";
    const std::string description = "";
    const std::string authors = "GEM Online Systems Group";
    const std::string link = "";

    config::PackageInfo getPackageInfo();
    void checkPackageDependencies() throw (config::PackageInfo::VersionException);
    std::set<std::string, std::less<std::string> > getPackageDependencies();
    //  }
}

#endif // DOXYGEN_IGNORE_THIS

#endif // gem_base_version_h
