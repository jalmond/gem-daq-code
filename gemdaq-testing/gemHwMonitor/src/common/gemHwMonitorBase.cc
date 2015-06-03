#include "gem/hwMonitor/gemHwMonitorBase.h"

template <class T> const std::string gem::hwMonitor::gemHwMonitorBase<T>::getDeviceId ()
    throw (xgi::exception::Exception)
{
    if (isConfigured_){
        return gemDevice_->getDeviceId();
    } else {
        return "Device is not configured";
    }
}

template <class T> void gem::hwMonitor::gemHwMonitorBase<T>::setDeviceConfiguration(T& device)
    throw (xgi::exception::Exception)
{
    gemDevice_ = &device;
    isConfigured_ = true;
}

template <class T> int gem::hwMonitor::gemHwMonitorBase<T>::getNumberOfSubDevices()
    throw (xgi::exception::Exception)
{
        return gemDevice_->getSubDevicesIds().size();
}

// [GCC bug] Bugreport https://gcc.gnu.org/bugzilla/show_bug.cgi?id=56480
namespace gem {
    namespace hwMonitor {
        template <> 
        int gem::hwMonitor::gemHwMonitorBase<gem::utils::gemVFATProperties>::getNumberOfSubDevices()
            throw (xgi::exception::Exception)
        {
                return -1;
        }
    }
}

template <class T> const std::string gem::hwMonitor::gemHwMonitorBase<T>::getCurrentSubDeviceId(unsigned int subDeviceNumber)
    throw (xgi::exception::Exception)
{
    return gemDevice_->getSubDevicesRefs().at(subDeviceNumber)->getDeviceId();
}

// [GCC bug] Bugreport https://gcc.gnu.org/bugzilla/show_bug.cgi?id=56480
namespace gem {
    namespace hwMonitor {
        template <> 
        const std::string gemHwMonitorBase<gem::utils::gemVFATProperties>::getCurrentSubDeviceId(unsigned int subDeviceNumber)
            throw (xgi::exception::Exception)
        {
                return "VFATs don't have subdevices";
        }
    }
}
template class gem::hwMonitor::gemHwMonitorBase<gem::utils::gemSystemProperties>; 
template class gem::hwMonitor::gemHwMonitorBase<gem::utils::gemCrateProperties>; 
template class gem::hwMonitor::gemHwMonitorBase<gem::utils::gemGLIBProperties>; 
template class gem::hwMonitor::gemHwMonitorBase<gem::utils::gemOHProperties>; 
template class gem::hwMonitor::gemHwMonitorBase<gem::utils::gemVFATProperties>; 
