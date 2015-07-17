#
# Makefile for gemHwMonitor package
#
BUILD_HOME:=$(shell pwd)/../..

include $(XDAQ_ROOT)/config/mfAutoconf.rules
include $(XDAQ_ROOT)/config/mfDefs.$(XDAQ_OS)

#Project=gem
Project=gemdaq-testing
Package=gemHwMonitor
LongPackage=gemHwMonitor
ShortPackage=hwMonitor
PackageName=hwMonitor

GEMHWMONITOR_VER_MAJOR=0
GEMHWMONITOR_VER_MINOR=1
GEMHWMONITOR_VER_PATCH=0

Sources =version.cc
Sources+=gemHwMonitorWeb.cc gemHwMonitorBase.cc

DynamicLibrary=gem_hwMonitor

IncludeDirs =$(BUILD_HOME)/$(Project)/$(Package)/include 
IncludeDirs+=$(BUILD_HOME)/$(Project)/gemutils/include 
IncludeDirs+=$(BUILD_HOME)/$(Project)/gembase/include 
IncludeDirs+=$(BUILD_HOME)/$(Project)/gemhardware/include 
IncludeDirs+=$(AMC13_STANDALONE_ROOT)/amc13/include
IncludeDirs+=$(XDAQ_ROOT)/include 
IncludeDirs+=$(uHALROOT)/include 

DependentLibraryDirs =$(BUILD_HOME)/$(Project)/gemutils/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) 
DependentLibraryDirs+=$(BUILD_HOME)/$(Project)/gembase/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) 
DependentLibraryDirs+=$(BUILD_HOME)/$(Project)/gemhardware/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) 
DependentLibraryDirs+=$(AMC13_STANDALONE_ROOT)/amc13/lib
DependentLibraryDirs+=$(XDAQ_ROOT)/lib 
DependentLibraryDirs+=$(uHALROOT)/lib 

LibraryDirs =$(BUILD_HOME)/$(Project)/gemutils/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) 
LibraryDirs+=$(BUILD_HOME)/$(Project)/gembase/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) 
LibraryDirs+=$(BUILD_HOME)/$(Project)/gemhardware/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM) 
LibraryDirs+=$(AMC13_STANDALONE_ROOT)/amc13/lib
LibraryDirs+=$(XDAQ_ROOT)/lib 
LibraryDirs+=$(uHALROOT)/lib 

PROFILING_Flags =-pg
DEBUG_CFlags  =-O0 -g3 -fno-inline
DEBUG_CCFlags =${DEBUG_CFlags}
GCC47Flags = -std=c++11 -std=gnu++11
GCC48Flags = -std=c++14 -std=gnu++14
UserCFlags  =-std=c++0x -std=gnu++0x ${DEBUG_CFlags}
UserCCFlags =-std=c++0x -std=gnu++0x ${DEBUG_CCFlags}
DEBUG_LDFlags =${PROFILING_Flags}
UserDynamicLinkFlags =
UserStaticLinkFlags =

#UserExecutableLinkFlags =-Wl,-rpath-link ${XDAQ_ROOT}/lib -Wl,-rpath-link $(uHALROOT)/lib -Wl,-rpath-link ${HCAL_XDAQ_ROOT}/lib 
#UserExecutableLinkFlags+=-L$(CACTUS_HOME)/lib 
#UserExecutableLinkFlags+=-lreadline -lncurses -lpthread -lboost_filesystem -lboost_regex -lboost_system -lcactus_extern_pugixml
#UserExecutableLinkFlags+=-lcactus_uhal_log -lcactus_uhal_grammars -lcactus_uhal_uhal

DEBUG_LIBS =profiler tcmalloc
DependentLibraries =uuid numa boost_system
DependentLibraries+=log4cplus xerces-c asyncresolv
DependentLibraries+=xdaq2rc config xcept toolbox
DependentLibraries+=cactus_uhal_uhal cactus_amc13_amc13
DependentLibraries+=gem_utils gem_base gem_hw
#DependentLibraries =log4cplus boost_system
#DependentLibraries+=config xcept toolbox
#DependentLibraries+=cactus_uhal_uhal
#DependentLibraries+=gem_base gem_hw gem_utils

#what's the difference between these two library lists?
Libraries =uuid numa boost_system
Libraries+=log4cplus xerces-c asyncresolv
Libraries+=xdaq2rc config xcept toolbox
Libraries+=cactus_uhal_uhal cactus_amc13_amc13
Libraries+=gem_utils gem_base gem_hw
#Libraries =log4cplus config xcept xerces-c
#Libraries+=numa toolbox xdaq2rc asyncresolv uuid
#Libraries+=cactus_uhal_uhal
#Libraries+=gem_base gem_hw gem_utils

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
