#ifndef gem_hwMonitor_gemHwMonitorHelper_h
#define gem_hwMonitor_gemHwMonitorHelper_h

#include <string>
#include <vector>
#include <cstdlib>

#include "gem/utils/gemXMLparser.h"
#include "gem/utils/gemComplexDeviceProperties.h"
#include "gem/utils/gemDeviceProperties.h"

#include "gemHwMonitorBase.h"

namespace gem {
    namespace hwMonitor {
        class gemHwMonitorHelper
        {
            public:
                gemHwMonitorHelper(gemHwMonitorSystem* gemSystem)
                    throw (xdaq::exception::Exception)
                {
                    std::string defaulXMLcfgFile = std::getenv("BUILD_HOME");
                    defaulXMLcfgFile +="/gemdaq-testing/gembase/xml/gem_test_904.xml";
                    this->setXMLconfigFile(defaulXMLcfgFile.c_str());
                    ptr_gemSystem_ = gemSystem;
                    ptr_gemSystem_->setDeviceStatus(2);
                }

                virtual ~gemHwMonitorHelper()
                {
                    delete gemXMLparser_;
                }

                void setXMLconfigFile (const char* inputXMLfilename)
                    throw (xgi::exception::Exception)
                {
                    xmlConfigFileName_ = inputXMLfilename;
                    if (ptr_gemSystem_) 
                        ptr_gemSystem_->setIsConfigured(false);
                    else 
                        std::cout << "[WARNING] New XML file set, but device state hasn't changed (NULL-pointer)" << std::endl;
                }
                const std::string getXMLconfigFile ()
                    throw (xgi::exception::Exception)
                {
                    return xmlConfigFileName_;
                }
                void setDBSconfigFile (std::string inputDBSfilename)
                    throw (xgi::exception::Exception){}
                const std::string getDBSconfigFile ()
                    throw (xgi::exception::Exception){return "Not implemented yet";}
                void configure()
                    throw (xgi::exception::Exception)
                {
                    gemXMLparser_ = new gem::utils::gemXMLparser(xmlConfigFileName_);
                    gemXMLparser_->parseXMLFile();
                    ptr_gemSystem_->setDeviceConfiguration(*(gemXMLparser_->getGEMDevice()));
                }

            protected:
            private:
                gemHwMonitorSystem* ptr_gemSystem_;
                gem::utils::gemXMLparser* gemXMLparser_;
                std::string xmlConfigFileName_;

        };
    }
}
#endif
