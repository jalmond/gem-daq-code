#ifndef gem_hw_glib_GLIBSettingsEnums_h
#define gem_hw_glib_GLIBSettingsEnums_h

namespace gem {
  namespace hw {
    namespace glib {
      class GLIBLinkSettings
      {
      public:
	struct LinkBitMasks {
	  enum ELinkBitMasks {
	      ERROR  = 0x01,
	      RECI2C = 0x02,
	      SNTI2C = 0x04,
	      RECREG = 0x08,
	      SNTREG = 0x10,
	  } LinkBitMasks;
	};

	struct LinkBitShifts {
	  enum ELinkBitShifts {
	    ERROR  = 0,
	    RECI2C = 1,
	    SNTI2C = 2,
	    RECREG = 3,
	    SNTREG = 4,
	  } LinkBitShifts;
	};
      };

      class GLIBSettings {
      public:

	//what are these for the GLIB??
	struct ContRegBitMasks {
	  enum EContRegBitMasks {
	    //Control Register 0
	    RUNMODE       = 0x01,
	    TRIGMODE      = 0x0e,
	    MSPOL         = 0x10,
	    CALPOL        = 0x20,
	    CALMODE       = 0xc0,
	  } ContRegBitMasks;
	};

	struct ContRegBitShifts {
	  enum EContRegBitShifts {
	    //Control Register 0
	    RUNMODE       = 0,
	    TRIGMODE      = 1,
	    MSPOL         = 4,
	    CALPOL        = 5,
	    CALMODE       = 6,
	  } ContRegBitShifts;
	};

	struct RunMode { //defines the run mode
	  enum ERunMode { //defines the run mode
	    SLEEP = 0x0, //sleep (default)
	    RUN   = 0x1  //running
	  } RunMode;
	};

	struct TriggerMode { //Trigger mode settings
	  enum ETriggerMode { //Trigger mode settings
	    GLIB = 0x0, //Triggers are generated internally
	    TTC  = 0x1, //Triggers come from the TTC link on the backplane
	    BOTH = 0x2, //Use all sources of triggers
	  } TriggerMode;
	};
	
      }; //end class GLIBSettings
      
    }//end namespace gem::hw::glib
    
  }//end namespace gem::hw
  
}//end namespace gem

//typedef the struct for access to the members via struct::VALUE
typedef gem::hw::glib::GLIBLinkSettings::LinkBitMasks   GLIBLinkBitMasks;
typedef gem::hw::glib::GLIBLinkSettings::LinkBitShifts  GLIBLinkBitShifts;

//typedef the enum for casting and access
typedef gem::hw::glib::GLIBSettings::RunMode::ERunMode                 GLIBRunMode;
typedef gem::hw::glib::GLIBSettings::TriggerMode::ETriggerMode         GLIBTrigMode;

#endif
