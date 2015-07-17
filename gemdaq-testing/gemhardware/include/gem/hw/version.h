#ifndef gem_hw_version_h
#define gem_hw_version_h

#ifndef DOXYGEN_IGNORE_THIS

#include "config/PackageInfo.h"

//namespace gem {
//  namespace hw {
namespace gemhardware {

#define GEMHARDWARE_VERSION_MAJOR 0
#define GEMHARDWARE_VERSION_MINOR 1
#define GEMHARDWARE_VERSION_PATCH 0
//#define GEMHARDWARE_PREVIOUS_VERSIONS 

    //
    // Template macros
    //
#define GEMHARDWARE_VERSION_CODE PACKAGE_VERSION_CODE(GEMHARDWARE_VERSION_MAJOR,GEMHARDWARE_VERSION_MINOR,GEMHARDWARE_VERSION_PATCH)
#ifndef GEMHARDWARE_PREVIOUS_VERSIONS
#define GEMHARDWARE_FULL_VERSION_LIST  PACKAGE_VERSION_STRING(GEMHARDWARE_VERSION_MAJOR,GEMHARDWARE_VERSION_MINOR,GEMHARDWARE_VERSION_PATCH)
#else
#define GEMHARDWARE_FULL_VERSION_LIST  GEMHARDWARE_PREVIOUS_VERSIONS "," PACKAGE_VERSION_STRING(GEMHARDWARE_VERSION_MAJOR,GEMHARDWARE_VERSION_MINOR,GEMHARDWARE_VERSION_PATCH)
#endif

  
    const std::string package = "gem/hw";
    const std::string versions = GEMHARDWARE_FULL_VERSION_LIST;
    const std::string summary = "HW interfaces for GEM devices";
    const std::string description = "";
    const std::string authors = "GEM Online Systems Group";
    const std::string link = "";
  
    config::PackageInfo getPackageInfo();
    void checkPackageDependencies() throw (config::PackageInfo::VersionException);
    std::set<std::string,std::less<std::string> > getPackageDependencies();
    //  }
}

#endif // DOXYGEN_IGNORE_THIS

#endif // gem_hw_version_h
