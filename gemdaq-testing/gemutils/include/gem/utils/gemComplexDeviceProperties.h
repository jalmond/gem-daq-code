/*
 * This class provides base interface for the device properties.
 */
#ifndef gem_utils_gemComplexDeviceProperties_h
#define gem_utils_gemComplexDeviceProperties_h

#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <cstdlib>

#include "gemDeviceProperties.h"

namespace gem {
  namespace utils {
    template <class T>
      class gemComplexDeviceProperties: public gemDeviceProperties {
    public:
      gemComplexDeviceProperties(){}
      ~gemComplexDeviceProperties(){for_each(subDevicesRefs_.begin(), subDevicesRefs_.end(), free);} //Unsafe code. Deletes objects created in different place.
      const std::vector<T*>& getSubDevicesRefs() {return subDevicesRefs_;}
      void addSubDeviceRef(T* &&subDeviceRef) {subDevicesRefs_.push_back(subDeviceRef);}
      
    private:
      std::vector <T*> subDevicesRefs_;
    };

    typedef gemDeviceProperties gemVFATProperties;
    typedef gemComplexDeviceProperties<gemVFATProperties> gemOHProperties;
    typedef gemComplexDeviceProperties<gemOHProperties> gemGLIBProperties;
    typedef gemComplexDeviceProperties<gemGLIBProperties> gemCrateProperties;
    typedef gemComplexDeviceProperties<gemCrateProperties> gemSystemProperties;
  } // end namespace gem::utils
} // end namespace gem

#endif
