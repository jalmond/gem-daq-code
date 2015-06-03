BUILD_HOME:=$(shell pwd)/../..

include $(XDAQ_ROOT)/config/mfAutoconf.rules
include $(XDAQ_ROOT)/config/mfDefs.$(XDAQ_OS)

#Project=gem
Project=gemdaq-testing
Package=gembase
LongPackage=gembase
ShortPackage=base
PackageName=base

#Sources= GEMApplication.cc GEMFSMApplication.cc GEMWebApplication.cc CfgParser.cc version.cc
#Sources= GEMFSM.cc utils/GEMSOAPToolBox.cc GEMApplication.cc GEMFSMApplication.cc GEMWebApplication.cc
Sources= GEMApplication.cc GEMFSMApplication.cc GEMFSM.cc GEMWebApplication.cc
DynamicLibrary=gem_base

#IncludeDirs = $(uHALROOT)/include
#DependentLibraryDirs = $(uHALROOT)/lib
#$(BUILD_HOME)/$(Project)/gemhardware/include \

IncludeDirs = \
        $(BUILD_HOME)/$(Project)/$(Package)/include \
        $(BUILD_HOME)/$(Project)/gemutils/include \
        $(XDAQ_ROOT)/include \
        $(uHALROOT)/include

DependentLibraryDirs = \
        $(XDAQ_ROOT)/lib \
        $(BUILD_HOME)/$(Project)/gemutils/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) \
        $(uHALROOT)/lib

LibraryDirs = \
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
#UserExecutableLinkFlags = -Wl,-rpath-link ${XDAQ_ROOT}/lib -Wl,-rpath-link $(uHALROOT)/lib \
#		          -Wl,-rpath-link ${HCAL_XDAQ_ROOT}/lib \
#                          -L$(CACTUS_HOME)/lib \
#                          -lreadline -lncurses -lpthread -lboost_filesystem \
#			  -lboost_regex -lboost_system -lcactus_extern_pugixml \
#			  -lcactus_uhal_log -lcactus_uhal_grammars -lcactus_uhal_uhal

DEBUG_LIBS = profiler tcmalloc
DependentLibraries = config xcept boost_system cactus_uhal_uhal gem_utils
Libraries = config xcept numa toolbox xdaq2rc asyncresolv uuid log4cplus

include $(XDAQ_ROOT)/config/Makefile.rules
include $(XDAQ_ROOT)/config/mfRPM.rules
