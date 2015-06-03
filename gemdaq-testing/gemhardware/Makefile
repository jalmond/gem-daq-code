BUILD_HOME:=$(shell pwd)/../..

include $(XDAQ_ROOT)/config/mfAutoconf.rules
include $(XDAQ_ROOT)/config/mfDefs.$(XDAQ_OS)

#Project=gem
Project=gemdaq-testing
Package=gemhardware
LongPackage=gemhardware
ShortPackage=hw
PackageName=hw

##make a device specific buildfile as well, defaulting to all
Sources= GEMHwDevice.cc \
	vfat/HwVFAT2.cc  vfat/VFAT2Manager.cc vfat/VFAT2ControlPanelWeb.cc \
	amc13/AMC13Manager.cc amc13/AMC13ManagerWeb.cc \
#optohybrid/HwOptoHybrid.cc \
#glib/HwGLIB.cc glib/GLIBManager.cc  glib/GLIBManagerWeb.cc \
	GEMController.cc GEMControllerPanelWeb.cc
##version.cc

DynamicLibrary=gem_hw

IncludeDirs = \
        $(BUILD_HOME)/$(Project)/$(Package)/include \
        $(BUILD_HOME)/$(Project)/gembase/include \
        $(BUILD_HOME)/$(Project)/gemutils/include \
	$(AMC13_STANDALONE_ROOT)/amc13/include \
        $(XDAQ_ROOT)/include \
        $(uHALROOT)/include

DependentLibraryDirs = \
        $(BUILD_HOME)/$(Project)/gemutils/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) \
        $(BUILD_HOME)/$(Project)/gembase/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) \
	$(AMC13_STANDALONE_ROOT)/amc13/lib \
        $(XDAQ_ROOT)/lib \
        $(uHALROOT)/lib

LibraryDirs = \
        $(BUILD_HOME)/$(Project)/gemutils/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) \
        $(BUILD_HOME)/$(Project)/gembase/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) \
	$(AMC13_STANDALONE_ROOT)/amc13/lib \
        $(XDAQ_ROOT)/lib \
        $(uHALROOT)/lib

PROFILING_Flags = -pg
DEBUG_CFlags  =  -O0 -g3 -fno-inline
DEBUG_CCFlags =  ${DEBUG_CFlags}
UserCFlags  = -std=c++0x ${DEBUG_CFlags}
UserCCFlags = -std=c++0x ${DEBUG_CCFlags}
DEBUG_LDFlags = ${PROFILING_Flags}
UserDynamicLinkFlags = 
UserStaticLinkFlags =
#UserExecutableLinkFlags = -Wl,-rpath-link ${XDAQ_ROOT}/lib -Wl,-rpath-link $(uHALROOT)/lib -Wl,-rpath-link ${HCAL_XDAQ_ROOT}/lib \
#                          -L$(CACTUS_HOME)/lib \
#                          -lreadline -lncurses -lpthread -lboost_filesystem -lboost_regex -lboost_system -lcactus_extern_pugixml -lcactus_uhal_log -lcactus_uhal_grammars -lcactus_uhal_uhal

DEBUG_LIBS = profiler tcmalloc
DependentLibraries = config xcept boost_system cactus_uhal_uhal gem_utils gem_base gem_utils cactus_amc13_amc13
Libraries = config xcept numa toolbox xdaq2rc asyncresolv uuid log4cplus cactus_uhal_uhal gem_utils gem_base cactus_amc13_amc13
# gem_utils

include $(XDAQ_ROOT)/config/Makefile.rules
include $(XDAQ_ROOT)/config/mfRPM.rules
