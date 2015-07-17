#
# Makefile for gemutils package
#
BUILD_HOME:=$(shell pwd)/../..

include $(XDAQ_ROOT)/config/mfAutoconf.rules
include $(XDAQ_ROOT)/config/mfDefs.$(XDAQ_OS)

#Project=gem
Project=gemdaq-testing
Package=gemutils
LongPackage=gemutils
ShortPackage=utils
PackageName=utils

GEMUTILS_VER_MAJOR=0
GEMUTILS_VER_MINOR=1
GEMUTILS_VER_PATCH=0

Sources =version.cc
Sources+=Lock.cc gemXMLparser.cc soap/GEMSOAPToolBox.cc

DynamicLibrary=gem_utils

IncludeDirs =$(BUILD_HOME)/$(Project)/$(Package)/include
IncludeDirs+=$(XDAQ_ROOT)/include
IncludeDirs+=$(uHALROOT)/include

DependentLibraryDirs =$(XDAQ_ROOT)/lib
DependentLibraryDirs+=$(uHALROOT)/lib

LibraryDirs =$(XDAQ_ROOT)/lib
LibraryDirs+=$(uHALROOT)/lib

PROFILING_Flags = -pg
DEBUG_CFlags  =  -O0 -g3 -fno-inline
DEBUG_CCFlags =  ${DEBUG_CFlags}
GCC47Flags = -std=c++11 -std=gnu++11
GCC48Flags = -std=c++14 -std=gnu++14
UserCFlags  =-std=c++0x -std=gnu++0x ${DEBUG_CFlags}
UserCCFlags =-std=c++0x -std=gnu++0x ${DEBUG_CCFlags}
DEBUG_LDFlags = ${PROFILING_Flags}
UserDynamicLinkFlags = 
UserStaticLinkFlags =

#UserExecutableLinkFlags =-Wl,-rpath-link ${XDAQ_ROOT}/lib -Wl,-rpath-link $(uHALROOT)/lib -Wl,-rpath-link ${HCAL_XDAQ_ROOT}/lib
#UserExecutableLinkFlags+=-L$(CACTUS_HOME)/lib
#UserExecutableLinkFlags+=-lreadline -lncurses -lpthread -lboost_filesystem -lboost_regex -lboost_system -lcactus_extern_pugixml
#UserExecutableLinkFlags+=-lcactus_uhal_log -lcactus_uhal_grammars -lcactus_uhal_uhal

DEBUG_LIBS = profiler tcmalloc
DependentLibraries = log4cplus config xcept xerces-c boost_system cactus_uhal_uhal
Libraries          = log4cplus config xcept xerces-c numa toolbox xdaq2rc asyncresolv uuid cactus_uhal_uhal

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
