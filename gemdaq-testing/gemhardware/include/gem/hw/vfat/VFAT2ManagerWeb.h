#ifndef gem_hw_vfat_VFAT2ManagerWeb_h
#define gem_hw_vfat_VFAT2ManagerWeb_h

#include <memory>

#include "gem/base/GEMWebApplication.h"

namespace gem {
  namespace hw {
    namespace vfat {
      
      class VFAT2Manager;

      class VFAT2ManagerWeb : public gem::base::GEMWebApplication
        {
          //friend class VFAT2Monitor;
          //friend class VFAT2Manager;

        public:
          VFAT2ManagerWeb(VFAT2Manager *vfat2App);
	  
          virtual ~VFAT2ManagerWeb();

        protected:

          virtual void webDefault(  xgi::Input *in, xgi::Output *out )
            throw (xgi::exception::Exception);

          virtual void monitorPage(xgi::Input *in, xgi::Output *out)
            throw (xgi::exception::Exception);
          
          virtual void expertPage(xgi::Input *in, xgi::Output *out)
            throw (xgi::exception::Exception);
          
          virtual void jsonUpdate(xgi::Input *in, xgi::Output *out)
            throw (xgi::exception::Exception);
          
          void cardPage(xgi::Input *in, xgi::Output *out)
            throw (xgi::exception::Exception);
          
        private:
          size_t activeCard;
	  
          //VFAT2ManagerWeb(VFAT2ManagerWeb const&);
	  
        };

    } // namespace gem::vfat
  } // namespace gem::hw
} // namespace gem

#endif
