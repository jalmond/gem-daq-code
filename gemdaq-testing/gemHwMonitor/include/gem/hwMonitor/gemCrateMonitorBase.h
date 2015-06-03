#ifndef gem_hwMonitor_gemCrateMonitorBase_h
#define gem_hwMonitor_gemCrateMonitorBase_h

#include <string>
#include <vector>

#include "xdaq/Application.h"
#include "xgi/framework/Method.h"

#include "gem/utils/gemXMLparser.h"
#include "gem/utils/gemCrateProperties.h"
#include "gem/utils/gemDeviceProperties.h"

namespace gem {
    namespace base {
        namespace utils {
            class gemXMLparcer;
            class gemCrateProperties;
            class gemDeviceProperties;
        }
    }
    namespace hwMonitor {
        class gemCrateMonitorBase: public gemHwMonitorBase
        {
            public:
                gemCrateMonitorBase(const gem::utils::gemCrateProperties & crateRef)
                    throw (xdaq::exception::Exception);

                ~gemCrateMonitorBase(){}
                /*
                 *   Get system configuration
                 */
                void getDeviceConfiguration()
                    throw (xgi::exception::Exception);
                /**
                 *   Access to board utils
                virtual void boardUtils ()
                    throw (xgi::exception::Exception);
                 */
 
            protected:
            private:
                gem::utils::gemCrateProperties* crateRef_;
        }; // end namespace hwMon
    }
} // end namespace gem
#endif
