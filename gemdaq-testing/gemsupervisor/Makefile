#
# Makefile for gemsupervisor package
#
BUILD_HOME:=$(shell pwd)/../..

include $(XDAQ_ROOT)/config/mfAutoconf.rules
include $(XDAQ_ROOT)/config/mfDefs.$(XDAQ_OS)

#Project=gem
Project=gemdaq-testing
Package=gemsupervisor
LongPackage=gemsupervisor
ShortPackage=supervisor
PackageName=supervisor

GEMSUPERVISOR_VER_MAJOR=0
GEMSUPERVISOR_VER_MINOR=1
GEMSUPERVISOR_VER_PATCH=0

# ROOT Config
ROOTCFLAGS =$(shell root-config --cflags)
ROOTLIBS   =$(shell root-config --libs)
ROOTGLIBS  =$(shell root-config --glibs) 

Sources =version.cc
Sources+=tbutils/VFAT2XMLParser.cc
Sources+=tbutils/GEMTBUtil.cc tbutils/ThresholdScan.cc tbutils/LatencyScan.cc tbutils/ADCScan.cc
#Sources+=tbutils/GEMTBUtil.cc tbutils/LatencyScan.cc
Sources+=GEMGLIBSupervisorWeb.cc
Sources+=GEMSupervisor.cc GEMSupervisorWeb.cc GEMSupervisorMonitor.cc

DynamicLibrary=gem_supervisor

IncludeDirs =$(BUILD_HOME)/$(Project)/$(Package)/include
IncludeDirs+=$(BUILD_HOME)/$(Project)/gembase/include
IncludeDirs+=$(BUILD_HOME)/$(Project)/gemutils/include
IncludeDirs+=$(BUILD_HOME)/$(Project)/gemhardware/include
IncludeDirs+=$(BUILD_HOME)/$(Project)/gemreadout/include
IncludeDirs+=$(BUILD_HOME)/gem-light-dqm/dqm-root/include
IncludeDirs+=$(XDAQ_ROOT)/include
IncludeDirs+=$(uHALROOT)/include
IncludeDirs+=$(ROOTDIR)/include 

#
DependentLibraryDirs =$(XDAQ_ROOT)/lib
DependentLibraryDirs+=$(BUILD_HOME)/$(Project)/gemhardware/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)
DependentLibraryDirs+=$(BUILD_HOME)/$(Project)/gemutils/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)
DependentLibraryDirs+=$(BUILD_HOME)/$(Project)/gemreadout/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)
DependentLibraryDirs+=$(uHALROOT)/lib
#DependentLibraryDirs+=${ROOTLIBS}

LibraryDirs =$(XDAQ_ROOT)/lib
#LibraryDirs+=$(BUILD_HOME)/$(Project)/gemhardware/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)
#LibraryDirs+=$(BUILD_HOME)/$(Project)/gemutils/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)
#LibraryDirs+=$(BUILD_HOME)/$(Project)/gemreadout/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)
LibraryDirs+=$(uHALROOT)/lib
#LibraryDirs+=${ROOTLIBS}
#LibraryDirs+=$(BUILD_HOME)/gembase/lib

PROFILING_Flags =-pg
DEBUG_CFlags  =-O0 -g3 -fno-inline
DEBUG_CCFlags =${DEBUG_CFlags}
GCC47Flags = -std=c++11 -std=gnu++11
GCC48Flags = -std=c++14 -std=gnu++14
UserCFlags  =-std=c++0x -std=gnu++0x ${ROOTCFLAGS}  ${DEBUG_CFlags}
UserCCFlags =-std=c++0x -std=gnu++0x ${ROOTCFLAGS}  ${DEBUG_CCFlags}
UserDynamicLinkFlags =$(ROOTLIBS)
UserStaticLinkFlags =

#UserExecutableLinkFlags =-Wl,-rpath-link ${XDAQ_ROOT}/lib -Wl,-rpath-link $(uHALROOT)/lib -Wl,-rpath-link ${HCAL_XDAQ_ROOT}/lib 
#UserExecutableLinkFlags+=-L$(CACTUS_HOME)/lib 
#UserExecutableLinkFlags+=-lreadline -lncurses -lpthread -lboost_filesystem -lboost_regex -lboost_system
#UserExecutableLinkFlags+=-lcactus_extern_pugixml -lcactus_uhal_log -lcactus_uhal_grammars -lcactus_uhal_uhal

#LDFLAGS=`root-config --libs --glibs`
DEBUG_LIBS =profiler tcmalloc
DependentLibraries = log4cplus config xcept boost_system cactus_uhal_uhal xerces-c gem_hw gem_utils gem_readout cactus_amc13_tools
Libraries          = log4cplus config xcept xerces-c numa toolbox asyncresolv uuid

include $(XDAQ_ROOT)/config/Makefile.rules
include $(XDAQ_ROOT)/config/mfRPM.rules
include $(BUILD_HOME)/$(Project)/mfRPM_gem.rules
print-env:
	@echo BUILD_HOME    $(BUILD_HOME)
	@echo XDAQ_ROOT     $(XDAQ_ROOT)
	@echo XDAQ_OS       $(XDAQ_OS)
	@echo XDAQ_PLATFORM $(XDAQ_PLATFORM)
	@echo LIBDIR        $(LIBDIR)
	@echo ROOTCFLAGS    $(ROOTCFLAGS)
	@echo ROOTLIBS      $(ROOTLIBS)
	@echo ROOTGLIBS     $(ROOTGLIBS)
