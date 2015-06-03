#ifndef gem_hw_glib_GLIBManager_h
#define gem_hw_glib_GLIBManager_h

#include <string>

#include "xdata/String.h"
#include "xdata/UnsignedLong.h"
#include "xdata/UnsignedInteger32.h"

#include "uhal/uhal.hpp"

#include "xgi/framework/Method.h"
#include "cgicc/HTMLClasses.h"

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

#include "gem/base/GEMFSMApplication.h"
//#include "gem/hw/glib/GLIBSettings.h"

#define MAX_GLIBS_PER_CRATE 12

namespace gem {
  namespace base {
  }
  
  namespace hw {
    namespace glib {
      class HwGLIB;
      
      class GLIBManager : public gem::base::GEMFSMApplication
	{
	  
	public:
	  GLIBManager(xdaq::ApplicationStub* s);

	  ~GLIBManager();

	  void init();
	  
	  void actionPerformed(xdata::Event& event);
	  
	protected:
	  
	  
	private:
	  
	  HwGLIB* m_GLIBs[MAX_GLIBS_PER_CRATE];
	  
	}; //end class GLIBManager

    }//end namespace gem::hw::glib
    
  }//end namespace gem::hw
  
}//end namespace gem

#endif
