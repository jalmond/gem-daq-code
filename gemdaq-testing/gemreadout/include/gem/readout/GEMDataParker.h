#ifndef gem_readout_GEMDataParker_h
#define gem_readout_GEMDataParker_h

#include "xdata/String.h"
#include <string>

namespace gem {
    namespace hw {
        namespace vfat {
            class HwVFAT2;
        }   
    }
    namespace readout {
            struct VFATData;
            struct GEBData;
            struct GEMData;
        }
    namespace readout {
        class GEMDataParker
        {
            public:
	        GEMDataParker(gem::hw::vfat::HwVFAT2& vfatDevice, std::string& outFileName, std::string& outputType);
                ~GEMDataParker() {};

                int dumpDataToDisk();

		int  getGLIBData  (gem::readout::GEMData& gem, gem::readout::GEBData& geb, 
                     gem::readout::VFATData& vfat);
                void fillGEMevent (gem::readout::GEMData& gem, gem::readout::GEBData& geb, 
                     gem::readout::VFATData& vfat);
                void writeGEMevent(gem::readout::GEMData& gem, gem::readout::GEBData& geb,
                     gem::readout::VFATData& vfat);

            private:

                gem::hw::vfat::HwVFAT2* vfatDevice_;
                std::string outFileName_;
                std::string outputType_;
                int counter_;
        };
    }
}
#endif
