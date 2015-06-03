/*
 * This class provides base interface for the device properties.
 */
#ifndef gem_utils_gemDeviceProperties_h
#define gem_utils_gemDeviceProperties_h

#include <string>
#include <vector>
#include <map>

namespace gem {
  namespace utils {
    class gemDeviceProperties {
    public:
      gemDeviceProperties(){}
      virtual ~gemDeviceProperties(){}
      const std::string& getDeviceId() const {return deviceId_;}
      void setDeviceId(const char* deviceId) {deviceId_ = deviceId;}
      const std::vector<std::string>& getSubDevicesIds() {return subDevicesIds_;}
      void addSubDeviceId(const std::string& deviceId) {subDevicesIds_.push_back(deviceId);}
      const std::map<std::string, std::string>& getDeviceProperties() {return deviceProperties_;}
      void addDeviceProperty(const std::string& first, const std::string& second) {
	deviceProperties_.insert(std::pair<std::string, std::string>(first,second));
      }
      
    private:
      std::string deviceId_;
      std::vector <std::string> subDevicesIds_;
      std::map <std::string, std::string> deviceProperties_;
    };
  } // end namespace gem::utils
} // end namespace gem

#endif
