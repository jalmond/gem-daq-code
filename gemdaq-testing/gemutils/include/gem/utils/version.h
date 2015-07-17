#ifndef gem_utils_version_h
#define gem_utils_version_h

#ifndef DOXYGEN_IGNORE_THIS

#include "config/PackageInfo.h"

//namespace gem {
//  namespace utils {
namespace gemutils {

#define GEMUTILS_VERSION_MAJOR 0
#define GEMUTILS_VERSION_MINOR 1
#define GEMUTILS_VERSION_PATCH 0
//#define GEMUTILS_PREVIOUS_VERSIONS 
#undef GEMUTILS_PREVIOUS_VERSIONS

#define GEMUTILS_VERSION_CODE PACKAGE_VERSION_CODE(GEMUTILS_VERSION_MAJOR,GEMUTILS_VERSION_MINOR,GEMUTILS_VERSION_PATCH)

#ifndef GEMUTILS_PREVIOUS_VERSIONS
#define GEMUTILS_FULL_VERSION_LIST  PACKAGE_VERSION_STRING(GEMUTILS_VERSION_MAJOR,GEMUTILS_VERSION_MINOR,GEMUTILS_VERSION_PATCH)
#else
#define GEMUTILS_FULL_VERSION_LIST  GEMUTILS_PREVIOUS_VERSIONS "," PACKAGE_VERSION_STRING(GEMUTILS_VERSION_MAJOR,GEMUTILS_VERSION_MINOR,GEMUTILS_VERSION_PATCH)
#endif
    
    const std::string package = "gem/utils";
    const std::string versions = GEMUTILS_FULL_VERSION_LIST;
    const std::string summary = "Utilities for GEM online software";
    const std::string description = "";
    const std::string authors = "GEM Online Systems Group";
    const std::string link = "";
    
    config::PackageInfo getPackageInfo();
    void checkPackageDependencies() throw (config::PackageInfo::VersionException);
    std::set<std::string,std::less<std::string> > getPackageDependencies();
    //  }
}

#endif // DOXYGEN_IGNORE_THIS

#endif // gem_utils_version_h
