#
# Makefile for gemhardware package
#
BUILD_HOME:=$(shell pwd)/../..

include $(XDAQ_ROOT)/config/mfAutoconf.rules
include $(XDAQ_ROOT)/config/mfDefs.$(XDAQ_OS)

#Project=gem
Project=gemdaq-testing
Package=gemhardware
LongPackage=gemhardware
ShortPackage=hw
PackageName=hw

GEMHARDWARE_VER_MAJOR=0
GEMHARDWARE_VER_MINOR=1
GEMHARDWARE_VER_PATCH=0

##make a device specific buildfile as well, defaulting to all
#<<<<<<< HEAD
#Sources= GEMHwDevice.cc \
#	vfat/HwVFAT2.cc  vfat/VFAT2Manager.cc vfat/VFAT2ControlPanelWeb.cc \
#	amc13/AMC13Manager.cc amc13/AMC13ManagerWeb.cc \
#optohybrid/HwOptoHybrid.cc \
#glib/HwGLIB.cc 
##glib/GLIBManager.cc  glib/GLIBManagerWeb.cc \
#	GEMController.cc GEMControllerPanelWeb.cc
###version.cc
#=======
Sources =version.cc
Sources+=GEMHwDevice.cc
Sources+=vfat/HwVFAT2.cc vfat/VFAT2Manager.cc vfat/VFAT2ControlPanelWeb.cc 
Sources+=amc13/AMC13Manager.cc amc13/AMC13ManagerWeb.cc 
Sources+=optohybrid/HwOptoHybrid.cc 
Sources+=glib/HwGLIB.cc glib/GLIBManager.cc  glib/GLIBManagerWeb.cc
Sources+=GEMController.cc GEMControllerPanelWeb.cc
#>>>>>>> jsturdy-develop

DynamicLibrary=gem_hw

IncludeDirs =$(BUILD_HOME)/$(Project)/$(Package)/include
IncludeDirs+=$(BUILD_HOME)/$(Project)/gemutils/include
IncludeDirs+=$(BUILD_HOME)/$(Project)/gembase/include
IncludeDirs+=$(AMC13_STANDALONE_ROOT)/amc13/include
IncludeDirs+=$(XDAQ_ROOT)/include
IncludeDirs+=$(uHALROOT)/include

DependentLibraryDirs =$(BUILD_HOME)/$(Project)/gemutils/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)
DependentLibraryDirs+=$(BUILD_HOME)/$(Project)/gembase/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)
DependentLibraryDirs+=$(AMC13_STANDALONE_ROOT)/amc13/lib
DependentLibraryDirs+=$(XDAQ_ROOT)/lib
DependentLibraryDirs+=$(uHALROOT)/lib

LibraryDirs =$(BUILD_HOME)/$(Project)/gemutils/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)
LibraryDirs+=$(BUILD_HOME)/$(Project)/gembase/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)
LibraryDirs+=$(AMC13_STANDALONE_ROOT)/amc13/lib
LibraryDirs+=$(XDAQ_ROOT)/lib
LibraryDirs+=$(uHALROOT)/lib

PROFILING_Flags =-pg
DEBUG_CFlags=-O0 -g3 -fno-inline
DEBUG_CCFlags =${DEBUG_CFlags}
GCC47Flags = -std=c++11 -std=gnu++11
GCC48Flags = -std=c++14 -std=gnu++14
UserCFlags  =-std=c++0x -std=gnu++0x ${DEBUG_CFlags}
UserCCFlags =-std=c++0x -std=gnu++0x ${DEBUG_CCFlags}
DEBUG_LDFlags =${PROFILING_Flags}
UserDynamicLinkFlags =
UserStaticLinkFlags =

#UserExecutableLinkFlags=-Wl,-rpath-link ${XDAQ_ROOT}/lib -Wl,-rpath-link $(uHALROOT)/lib -Wl,-rpath-link ${HCAL_XDAQ_ROOT}/lib 
#UserExecutableLinkFlags+=-L$(CACTUS_HOME)/lib 
#UserExecutableLinkFlags+=-lreadline -lncurses -lpthread -lboost_filesystem -lboost_regex -lboost_system -lcactus_extern_pugixml
#UserExecutableLinkFlags+=-lcactus_uhal_log -lcactus_uhal_grammars -lcactus_uhal_uhal

DEBUG_LIBS =profiler tcmalloc
DependentLibraries =uuid numa boost_system
DependentLibraries+=log4cplus xerces-c asyncresolv
DependentLibraries+=xdaq2rc config xcept toolbox
DependentLibraries+=cactus_uhal_uhal cactus_amc13_amc13
DependentLibraries+=gem_utils gem_base

#what's the difference between these two library lists?
Libraries =uuid numa boost_system
Libraries+=log4cplus xerces-c asyncresolv
Libraries+=xdaq2rc config xcept toolbox
Libraries+=cactus_uhal_uhal cactus_amc13_amc13
Libraries+=gem_utils gem_base

include $(XDAQ_ROOT)/config/Makefile.rules
include $(XDAQ_ROOT)/config/mfRPM.rules
include $(BUILD_HOME)/$(Project)/mfRPM_gem.rules

#test#hwbase:
#test#	Sources=GEMHwDevice.cc 
#test#	##version.cc
#test#
#test#	DynamicLibrary=gem_hw_base
#test#
#test#controller: 
#test#	Sources+=GEMController.cc GEMControllerPanelWeb.cc
#test#
#test#	DynamicLibrary=gem_hw_control
#test#
#test#all: hwbase amc13 vfat glib controller
#test#
#test#	DynamicLibrary=gem_hw
#test#
#test#amc13:
#test#	Sources+=amc13/AMC13Manager.cc amc13/AMC13ManagerWeb.cc
#test#	##amc13/version.cc
#test#
#test#	DynamicLibrary=gem_hw_amc13
#test#
#test#vfat:
#test#	Sources=GEMHwDevice.cc
#test#	Sources+=vfat/HwVFAT2.cc vfat/VFAT2Manager.cc vfat/VFAT2ControlPanelWeb.cc
#test#	#Sources+=vfat/version.cc
#test#
#test#	DynamicLibrary=gem_hw_vfat
#test#
#test#optohybrid:
#test#	Sources=GEMHwDevice.cc
#test#	Sources+=optohybrid/HwOptoHybrid.cc optohybrid/OptoHybridManager.cc  optohybrid/OptoHybridManagerWeb.cc
#test#	#Sources+=optohybrid/version.cc
#test#
#test#	DynamicLibrary=gem_hw_oh
#test#
#test#glib:
#test#	Sources=GEMHwDevice.cc
#test#	Sources+=glib/HwGLIB.cc glib/GLIBManager.cc  glib/GLIBManagerWeb.cc
#test#	#Sources+=glib/version.cc
#test#
#test#	DynamicLibrary=gem_hw_glib
#test#
#test#mp7:
#test#	Sources=GEMHwDevice.cc 
#test#	Sources+=mp7/HwMP7.cc mp7/MP7Manager.cc  mp7/MP7ManagerWeb.cc
#test#	#Sources+=mp7/version.cc
#test#
#test#	DynamicLibrary=gem_hw_mp7


print-env:
	@echo BUILD_HOME    $(BUILD_HOME)
	@echo XDAQ_ROOT     $(XDAQ_ROOT)
	@echo XDAQ_OS       $(XDAQ_OS)
	@echo XDAQ_PLATFORM $(XDAQ_PLATFORM)
	@echo LIBDIR        $(LIBDIR)
	@echo ROOTCFLAGS    $(ROOTCFLAGS)
	@echo ROOTLIBS      $(ROOTLIBS)
	@echo ROOTGLIBS     $(ROOTGLIBS)
