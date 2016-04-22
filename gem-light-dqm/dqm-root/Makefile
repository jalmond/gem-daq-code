#
# Makefile for gemtreewriter package
#
BUILD_HOME:=$(shell pwd)/../..

CC=g++
CCFLAGS=-O2 -Wall -fPIC -pthread -m64
ADDFLAGS=-g -std=c++0x
RC=rootcint
LBL=-shared -O2 -m64
LS=ls -lartF

# ROOT Config
ROOTCFLAGS=$(shell root-config --cflags)
ROOTLIBS  =$(shell root-config --libs) 
ROOTGLIBS =$(shell root-config --glibs) 

ROOTLIBS += -lRHTTP

#Project=gem
Project=gem-light-dqm
Package=dqm-root
LongPackage=dqm-root
ShortPackage=dqm-root
PackageName=dqm-root

IncludeDirs = /usr/include/root
IncludeDirs+= $(BUILD_HOME)/$(Project)
IncludeDirs+= $(BUILD_HOME)/$(Project)/$(Package)
IncludeDirs+= $(BUILD_HOME)/$(Project)/$(Package)/include
IncludeDirs+= $(BUILD_HOME)/$(Project)/gemtreewriter/include
IncludeDirs+= ${BUILD_HOME}/gemdaq-testing/gemreadout/include
INC=$(IncludeDirs:%=-I%)

SRC=$(BUILD_HOME)/$(Project)/$(Package)/src/common
OBJ=$(BUILD_HOME)/$(Project)/$(Package)/src/obj
DIC=$(BUILD_HOME)/$(Project)/$(Package)/src/dic
BIN=$(BUILD_HOME)/$(Project)/$(Package)/bin/$(XDAQ_OS)/$(XDAQ_PLATFORM)

LIB=$(BUILD_HOME)/$(Project)/gemtreewriter/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)
all:
	mkdir -p $(BIN)
	$(CC) $(ADDFLAGS) $(ROOTLIBS) $($ROOTGLIBS) $(INC) $(SRC)/histogramLayeringExample.cxx $(LIB)/libEvent.so -o $(BIN)/layer
#	$(CC) $(ADDFLAGS) $(ROOTLIBS) $($ROOTGLIBS) $(INC) $(SRC)/printAll.cxx $(LIB)/libEvent.so -o $(BIN)/printer
	$(CC) $(ADDFLAGS) $(ROOTLIBS) $($ROOTGLIBS) $(INC) $(SRC)/gtprinter.cxx $(LIB)/libEvent.so -o $(BIN)/gtprinter
	$(CC) $(ADDFLAGS) $(ROOTLIBS) $($ROOTGLIBS) $(INC) $(SRC)/dummyMain.cxx $(LIB)/libEvent.so -o $(BIN)/dqm
	$(CC) $(ADDFLAGS) $(ROOTLIBS) $($ROOTGLIBS) $(INC) $(SRC)/main.cxx $(LIB)/libEvent.so -o $(BIN)/rundqm
	$(LS) $(BIN)

dqm:  	#CURRENT VERSION
	mkdir -p $(BIN)
	$(CC) $(ADDFLAGS) $(ROOTLIBS) $($ROOTGLIBS) $(INC) $(SRC)/dummyMain.cxx $(LIB)/libEvent.so -o $(BIN)/dqm

rundqm:
	mkdir -p $(BIN)
	$(CC) $(ADDFLAGS) $(ROOTLIBS) $($ROOTGLIBS) $(INC) $(SRC)/main.cxx $(LIB)/libEvent.so -o $(BIN)/rundqm

merger:
	mkdir -p $(BIN)
	$(CC) $(ADDFLAGS) $(ROOTLIBS) $($ROOTGLIBS) $(INC) $(SRC)/mergeCleanTrees.cxx $(LIB)/libEvent.so -o $(BIN)/treemerge
reader:
	mkdir -p $(BIN)
	$(CC) $(ADDFLAGS) $(ROOTLIBS) $($ROOTGLIBS) $(INC) $(SRC)/treeReaderExample.cxx $(LIB)/libEvent.so -o $(BIN)/reader
	$(LS) $(BIN)

layer:
	mkdir -p $(BIN)
	$(CC) $(ADDFLAGS) $(ROOTLIBS) $($ROOTGLIBS) $(INC) $(SRC)/histogramLayeringExample.cxx $(LIB)/libEvent.so -o $(BIN)/layer
	$(LS) $(BIN)

printer:
	mkdir -p $(BIN)
#	$(CC) $(ADDFLAGS) $(ROOTLIBS) $($ROOTGLIBS) $(INC) $(SRC)/printAll.cxx $(LIB)/libEvent.so -o $(BIN)/printer
	$(CC) $(ADDFLAGS) $(ROOTLIBS) $($ROOTGLIBS) $(INC) $(SRC)/gtprinter.cxx $(LIB)/libEvent.so -o $(BIN)/gtprinter
	$(LS) $(BIN)


clean:
	rm -rf $(BIN)
print-env:
	@echo BUILD_HOME    $(BUILD_HOME)
	@echo XDAQ_OS       $(XDAQ_OS)
	@echo XDAQ_PLATFORM $(XDAQ_PLATFORM)
	@echo INC           $(INC)

