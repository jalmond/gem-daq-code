#include "gem/hwMonitor/gemCrateMonitorBase.h"

gem::hwMonitor::gemHwMonitorBase::gemHwMonitorBase()
    throw (xdaq::exception::Exception)
{
    crateRef_ = crateRef;
    this->setDeviceID(crateRef->getDeviceId);
    this->setDeviceStatus(2);
}

int gem::hwMonitor::gemHwMonitorBase::getNumberOfCrates()
    throw (xgi::exception::Exception)
{ 
    return crateRefs_.size();
    //return 0;
}
std::string gem::hwMonitor::gemHwMonitorBase::getCurrentCrateId(unsigned int crateNumber)
    throw (xgi::exception::Exception)
{
    return crateRefs_.at(crateNumber)->getDeviceId();
}
