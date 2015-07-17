#ifndef gem_hwMonitor_gemHwMonitorWeb_h
#define gem_hwMonitor_gemHwMonitorWeb_h

#include "xdaq/WebApplication.h"
#include "xgi/framework/Method.h"
#include "cgicc/HTMLClasses.h"
#include <sys/stat.h>

#include "gemHwMonitorBase.h"
#include "gemHwMonitorHelper.h"

#include "gem/hw/GEMHwDevice.h"
#include "gem/hw/glib/HwGLIB.h"
#include "gem/hw/optohybrid/HwOptoHybrid.h"
#include "gem/hw/vfat/HwVFAT2.h"
#include "gem/hw/vfat/VFAT2Settings.h"
#include "gem/hw/vfat/VFAT2SettingsEnums.h"
#include "gem/hw/vfat/VFAT2Enums2Strings.h"
#include "gem/hw/vfat/VFAT2Strings2Enums.h"

namespace cgicc {
    BOOLEAN_ELEMENT(section,"section");
}
namespace gem {
    namespace hw {
        class GEMHwDevice;
        namespace glib {
            class HwGLIB;
        }
        namespace optohybrid {
            class HwOptoHybrid;
        }
        namespace vfat {
            class HwVFAT2;
            class VFAT2Settings;
        }
    }
    namespace hwMonitor {
        class gemHwMonitorWeb: public xdaq::WebApplication//, xdata::ActionListener
        {
	        public:
                XDAQ_INSTANTIATOR();
		        gemHwMonitorWeb(xdaq::ApplicationStub *s)
                    throw (xdaq::exception::Exception);
                ~gemHwMonitorWeb();
                void Default(xgi::Input *in, xgi::Output *out )
		            throw (xgi::exception::Exception);
                void Dummy(xgi::Input *in, xgi::Output *out )
		            throw (xgi::exception::Exception);
		        void controlPanel(xgi::Input *in, xgi::Output *out)
		            throw (xgi::exception::Exception);
		        void showCratesAvailability(xgi::Input *in, xgi::Output *out)
		            throw (xgi::exception::Exception);
		        void pingCrate(xgi::Input *in, xgi::Output *out)
		            throw (xgi::exception::Exception);
		        void showCrateUtilities(xgi::Input *in, xgi::Output *out)
		            throw (xgi::exception::Exception);
                void setConfFile(xgi::Input *in, xgi::Output *out)
		            throw (xgi::exception::Exception);
                void uploadConfFile(xgi::Input *in, xgi::Output *out)
		            throw (xgi::exception::Exception);
                void getCratesConfiguration(xgi::Input *in, xgi::Output *out)
		            throw (xgi::exception::Exception);
                void expandCrate(xgi::Input *in, xgi::Output *out)
		            throw (xgi::exception::Exception);
                void selectCrate(xgi::Input *in, xgi::Output *out)
		            throw (xgi::exception::Exception);
                void cratePanel(xgi::Input *in, xgi::Output *out)
		            throw (xgi::exception::Exception);
                void expandGLIB(xgi::Input *in, xgi::Output *out)
		            throw (xgi::exception::Exception);
                void glibPanel(xgi::Input *in, xgi::Output *out)
		            throw (xgi::exception::Exception);
                void expandOH(xgi::Input *in, xgi::Output *out)
		            throw (xgi::exception::Exception);
                void ohPanel(xgi::Input *in, xgi::Output *out)
		            throw (xgi::exception::Exception);
                void expandVFAT(xgi::Input *in, xgi::Output *out)
		            throw (xgi::exception::Exception);
                void vfatPanel(xgi::Input *in, xgi::Output *out)
		            throw (xgi::exception::Exception);
/*
                static void createVFATInfoLayout(       xgi::Output *out,
                        const gem::hw::vfat::VFAT2ControlParams params){}
                static void createControlRegisterLayout(xgi::Output *out,
                        const gem::hw::vfat::VFAT2ControlParams params){}
                static void createSettingsLayout(       xgi::Output *out,
                        const gem::hw::vfat::VFAT2ControlParams params){}
                static void createCounterLayout(        xgi::Output *out,
                        const gem::hw::vfat::VFAT2ControlParams params){}
                static void createChannelRegisterLayout(xgi::Output *out,
                        const gem::hw::vfat::VFAT2ControlParams params){}
                static void createCommandLayout(        xgi::Output *out,
                        const gem::hw::vfat::VFAT2ControlParams params){}

                static void getCurrentParametersAsXML(){}
                static void saveCurrentParametersAsXML(){}
                static void setParametersByXML(){}
*/
            private:
                gemHwMonitorSystem* gemHwMonitorSystem_;
                std::vector<gemHwMonitorCrate*> gemHwMonitorCrate_;
                std::vector<gemHwMonitorGLIB*> gemHwMonitorGLIB_;
                std::vector<gemHwMonitorOH*> gemHwMonitorOH_;
                std::vector<gemHwMonitorVFAT*> gemHwMonitorVFAT_;
                gemHwMonitorHelper* gemSystemHelper_;
                bool crateCfgAvailable_;
                int nCrates_;
                int indexCrate_;
                int indexGLIB_;
                int indexOH_;
                int indexVFAT_;
                std::string crateToShow_;
                std::string glibToShow_;
                std::string ohToShow_;
                std::string vfatToShow_;
                std::string glibIP;
                gem::hw::glib::HwGLIB* glibDevice_;
                gem::hw::optohybrid::HwOptoHybrid* ohDevice_;
                gem::hw::vfat::HwVFAT2* vfatDevice_;
                std::vector<std::string> checkedCrates_;

                void printVFAThwParameters(const char* key, const char* value1, const char* value2, xgi::Output * out)
		            throw (xgi::exception::Exception);
                void printVFAThwParameters(const char* key, const char* value,  xgi::Output *out)
		            throw (xgi::exception::Exception);
                void printVFAThwParameters(const char* key, uint8_t value,  xgi::Output *out)
		            throw (xgi::exception::Exception);
                void printVFAThwParameters(const char* key, const char* value1, uint8_t value2, xgi::Output * out)
		            throw (xgi::exception::Exception);
        };// end class gemHwMonitorWeb
    }// end namespace hwMonitor
}// end namespace gem
#endif
