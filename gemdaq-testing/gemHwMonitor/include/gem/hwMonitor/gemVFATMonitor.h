#ifndef gem_hwMonitor_gemHwMonitorWeb_h
#define gem_hwMonitor_gemHwMonitorWeb_h

namespace gem {
    namespace hw {
        namespace vfat {
            class HwVFAT2;
            class VFAT2Settings;
        }
    }
    namespace hwMonitor {
        class gemVFATMonitor: public xdaq::WebApplication//, xdata::ActionListener
        {
	        public:
            private:
                gem::hw::vfat::HwVFAT2* vfatDevice_;
                void printVFAThwParameters(const char* key, const char* value1, const char* value2, xgi::Output * out)
		            throw (xgi::exception::Exception);
                void printVFAThwParameters(const char* key, const char* value,  xgi::Output *out)
		            throw (xgi::exception::Exception);
                void printVFAThwParameters(const char* key, uint8_t value,  xgi::Output *out)
		            throw (xgi::exception::Exception);
                void printVFAThwParameters(const char* key, const char* value1, uint8_t value2, xgi::Output * out)
		            throw (xgi::exception::Exception);

        }; // end class gemVFATMonitor
    } // end namespace hwMonitor
} // end namespace gem
 
#endif

