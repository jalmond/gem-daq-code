#ifndef gem_hwMonitor_gemGLIBMonitor_h
#define gem_hwMonitor_gemGLIBMonitor_h

#include "gemHwMonitorBase.h"

namespace gem { 
    namespace base {
       namespace utils {
           class gemXMLparcer;
           class gemCrateProperties;
           class gemGLIBProperties;
        }
    }
    namespace hwMonitor {
        class gemGLIBMonitor: public gemHwMonitorBase
            {
                public:
                gemGLIBMonitor(gem::utils::gemGLIBProperties& glibRef)
                    throw (xgi::exception::Exception);
                ~gemGLIBMonitor()
                    throw (xgi::exception::Exception);

                void setGLIBRef()
                    throw (xgi::exception::Exception);
                void getGLIBRef()
                    throw (xgi::exception::Exception);
                
                private:
                gem::utils::gemGLIBProperties glibRef_;
            }
    }
}
#endif
