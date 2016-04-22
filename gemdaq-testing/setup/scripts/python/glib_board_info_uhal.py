#!/bin/env python

import sys, re
import time, datetime, os

sys.path.append('${GEM_PYTHON_PATH}')

import uhal
from registers_uhal import *
from glib_system_info_uhal import *
from rate_calculator import errorRate
from glib_user_functions_uhal import *

#from glib_clock_src import *

from optparse import OptionParser
parser = OptionParser()
parser.add_option("-s", "--slot", type="int", dest="slot",
		  help="slot in uTCA crate", metavar="slot", default=10)
parser.add_option("-g", "--gtx", type="int", dest="gtx",
		  help="GTX on the GLIB", metavar="gtx", default=0)
parser.add_option("-c", "--clksrc", type="string", dest="clksrc",
		  help="select the input for the XPoint1 outputs", metavar="clksrc", default="local")
parser.add_option("-r", "--reset", action="store_true", dest="resetCounters",
		  help="reset link error counters", metavar="resetCounters")
parser.add_option("-x", "--external", type="int", dest="trgSrc",
		  help="change trigger source", metavar="trgSrc")
parser.add_option("-b", "--sbitout", type="int", dest="sbitSrc",
		  help="use s-bit from VFAT <num>", metavar="sbitSrc")
parser.add_option("-d", "--debug", action="store_true", dest="debug",
		  help="print extra debugging information", metavar="debug")
parser.add_option("-e", "--errors", type="int", dest="errorRate", default=1,
		  help="calculate link error rates for N seconds", metavar="errorRate")
parser.add_option("-u", "--user", action="store_true", dest="userOnly",
		  help="print user information only", metavar="userOnly")
parser.add_option("--testbeam", action="store_true", dest="testbeam",
		  help="fixed IP address for testbeam", metavar="testbeam")
parser.add_option("--daq_enable", type="int", dest="daq_enable",
		  help="enable daq output", metavar="daq_enable", default=-1)
parser.add_option("--rd", type="int", dest="reset_daq",
		  help="reset daq", metavar="reset_daq", default=-1)

(options, args) = parser.parse_args()

uhal.setLogLevelTo( uhal.LogLevel.FATAL )

uTCAslot = 170
if options.slot:
	uTCAslot = 160+options.slot
print options.slot, uTCAslot
ipaddr = '192.168.0.%d'%(uTCAslot)
if options.testbeam:
        ipaddr        = '137.138.115.185'
address_table = "file://${GEM_ADDRESS_TABLE_PATH}/glib_address_table.xml"
uri = "chtcp-2.0://localhost:10203?target=%s:50001"%(ipaddr)
glib  = uhal.getDevice( "glib" , uri, address_table )

########################################
# IP address
########################################
print
print "--=======================================--"
print "  Opening GLIB with IP", ipaddr
print "--=======================================--"
print

if not options.userOnly:
	getSystemInfo(glib)
print
print "--=======================================--"
print "-> DAQ INFORMATION"
print "--=======================================--"
print

if (options.daq_enable>=0):
        #writeRegister(glib, "GLIB.DAQ.CONTROL", options.daq_enable)
        writeRegister(glib, "GLIB.DAQ.CONTROL", 0x8)
        #writeRegister(glib, "GLIB.DAQ.EXT_CONTROL.INPUT_TIMEOUT", 0x30D40)
        #writeRegister(glib, "GLIB.DAQ.CONTROL", 0x181)
        print "Reset daq_enable: %i"%(options.daq_enable)

print "-> DAQ control reg     : 0x%08x"%(readRegister(glib,"GLIB.DAQ.CONTROL"))
print "-> DAQ status reg      : 0x%08x"%(readRegister(glib,"GLIB.DAQ.STATUS"))
print "-> DAQ L1A ID          : 0x%08x"%(readRegister(glib,"GLIB.DAQ.EXT_STATUS.L1AID"))
print "-> DAQ sent events cnt : 0x%08x"%(readRegister(glib,"GLIB.DAQ.EXT_STATUS.EVT_SENT"))
print
print "-> DAQ DAV_TIMEOUT :0x%08x"%(readRegister(glib,"GLIB.DAQ.CONTROL.DAV_TIMEOUT"))
print "-> DAQ INPUT_TIMEOUT   : 0x%08x"%(readRegister(glib,"GLIB.DAQ.EXT_CONTROL.INPUT_TIMEOUT"))
print "-> DAQ RUN_TYPE      :0x%08x"%(readRegister(glib,"GLIB.DAQ.EXT_CONTROL.RUN_TYPE"))
print "-> DAQ RUN_PARAMS    :0x%08x"%(readRegister(glib,"GLIB.DAQ.EXT_CONTROL.RUN_PARAMS"))
print
print "-> DAQ GTX NOT_IN_TABLE error counter :0x%08x"%(readRegister(glib,"GLIB.DAQ.EXT_STATUS.NOTINTABLE_ERR"))
print "-> DAQ GTX dispersion error counter   :0x%08x"%(readRegister(glib,"GLIB.DAQ.EXT_STATUS.DISPER_ERR"))
print

NGTX = 2
for olink in range(NGTX):
        print "-> DAQ GTX%d corrupted VFAT block counter : 0x%08x"%(olink,readRegister(glib,"GLIB.DAQ.GTX%d.COUNTERS.CORRUPT_VFAT_BLK_CNT"%(olink)))
        print "-> DAQ GTX%d evn                          : 0x%08x"%(olink,readRegister(glib,"GLIB.DAQ.GTX%d.COUNTERS.EVN"%(olink)))
        print

print "-> GLIB STATUS 0 :0x%08x"%(readRegister(glib,"GLIB.DAQ.GTX0.STATUS"))
print "-> GLIB STATUS 1 :0x%08x"%(readRegister(glib,"GLIB.DAQ.GTX1.STATUS"))

print "-> GLIB MAX_DAV_TIMER :0x%08x"%(readRegister(glib,"GLIB.DAQ.EXT_STATUS.MAX_DAV_TIMER"))
print "-> GLIB LAST_DAV_TIMER :0x%08x"%(readRegister(glib,"GLIB.DAQ.EXT_STATUS.LAST_DAV_TIMER"))


print "-> GLIB MAX_DAV_TIMER GTX0:0x%08x"%(readRegister(glib,"GLIB.DAQ.GTX0.DAV_STATS.MAX_DAV_TIMER"))
print "-> GLIB LAST_DAV_TIMER GTX0:0x%08x"%(readRegister(glib,"GLIB.DAQ.GTX0.DAV_STATS.LAST_DAV_TIMER"))
print "-> GLIB MAX_DAV_TIMER GTX1:0x%08x"%(readRegister(glib,"GLIB.DAQ.GTX1.DAV_STATS.MAX_DAV_TIMER"))
print "-> GLIB LAST_DAV_TIMER GTX1:0x%08x"%(readRegister(glib,"GLIB.DAQ.GTX1.DAV_STATS.LAST_DAV_TIMER"))

print "-> GLIB DAV_TIMEOUT GTX0:0x%08x"%(readRegister(glib,"GLIB.DAQ.GTX0.DAV_TIMEOUT"))
print "-> GLIB DAV_TIMEOUT GTX1:0x%08x"%(readRegister(glib,"GLIB.DAQ.GTX1.DAV_TIMEOUT"))

print "====================================================================================="
print "DEBUG INFO"
print "====================================================================================="
print "GTX0"

print "-> DAQ debug0 :0x%08x"%(readRegister(glib,"GLIB.DAQ.GTX0.LASTBLOCK.0"))
print "-> DAQ debug1 :0x%08x"%(readRegister(glib,"GLIB.DAQ.GTX0.LASTBLOCK.1"))
print "-> DAQ debug2 :0x%08x"%(readRegister(glib,"GLIB.DAQ.GTX0.LASTBLOCK.2"))
print "-> DAQ debug3 :0x%08x"%(readRegister(glib,"GLIB.DAQ.GTX0.LASTBLOCK.3"))
print "-> DAQ debug4 :0x%08x"%(readRegister(glib,"GLIB.DAQ.GTX0.LASTBLOCK.4"))
print "-> DAQ debug5 :0x%08x"%(readRegister(glib,"GLIB.DAQ.GTX0.LASTBLOCK.5"))
print "-> DAQ debug6 :0x%08x"%(readRegister(glib,"GLIB.DAQ.GTX0.LASTBLOCK.6"))
	
print
print "--=======================================--"
print "-> BOARD USER INFORMATION"
print "--=======================================--"
print

print "====================================================================================="
print "DEBUG INFO"
print "====================================================================================="
print "GTX1"

print "-> DAQ debug0 :0x%08x"%(readRegister(glib,"GLIB.DAQ.GTX1.LASTBLOCK.0"))
print "-> DAQ debug1 :0x%08x"%(readRegister(glib,"GLIB.DAQ.GTX1.LASTBLOCK.1"))
print "-> DAQ debug2 :0x%08x"%(readRegister(glib,"GLIB.DAQ.GTX1.LASTBLOCK.2"))
print "-> DAQ debug3 :0x%08x"%(readRegister(glib,"GLIB.DAQ.GTX1.LASTBLOCK.3"))
print "-> DAQ debug4 :0x%08x"%(readRegister(glib,"GLIB.DAQ.GTX1.LASTBLOCK.4"))
print "-> DAQ debug5 :0x%08x"%(readRegister(glib,"GLIB.DAQ.GTX1.LASTBLOCK.5"))
print "-> DAQ debug6 :0x%08x"%(readRegister(glib,"GLIB.DAQ.GTX1.LASTBLOCK.6"))

print "--=======================================--"
print "-> SSSSSSSSSSSSSSSSSSSSSSBITSSSSSSSSSSSSSS"
print "--=======================================--"
	
print "-> GTX 0 clusters 01 :0x%08x"%(readRegister(glib,"GLIB.DAQ.GTX0_CLUSTER_01"))
print "-> GTX 0 clusters 23 :0x%08x"%(readRegister(glib,"GLIB.DAQ.GTX0_CLUSTER_23"))
print "-> GTX 1 clusters 01 :0x%08x"%(readRegister(glib,"GLIB.DAQ.GTX1_CLUSTER_01"))
print "-> GTX 1 clusters 23 :0x%08x"%(readRegister(glib,"GLIB.DAQ.GTX1_CLUSTER_23"))

print "-> SBIT_RATE :0x%08x"%(readRegister(glib,"GLIB.DAQ.SBIT_RATE"))

print
print "--=======================================--"
print "-> BOARD USER INFORMATION"
print "--=======================================--"
print

if (options.resetCounters):
        glibCounters(glib,options.gtx,True)
print
sys.stdout.flush()
for olink in range(NGTX):
        print "--=====GTX%d==============================--"%(olink)
        errorCounts = []
        SAMPLE_TIME = 1.
        for trial in range(options.errorRate):
                errorCounts.append(calculateLinkErrors(True,glib,olink,SAMPLE_TIME))
        sys.stdout.flush()
        
        rates = errorRate(errorCounts,SAMPLE_TIME)
        print "-> TRK: 0x%08x  (%6.2f%1sHz)"%(rates["TRK"][0],rates["TRK"][1],rates["TRK"][2])
        print "-> TRG: 0x%08x  (%6.2f%1sHz)"%(rates["TRG"][0],rates["TRG"][1],rates["TRG"][2])
        print 
        
        sys.stdout.flush()
        print "-> Counters    %8s     %8s     %8s     %8s"%("L1A","Cal","Resync","BC0")
        counters = glibCounters(glib,olink)
        print "   %8s  0x%08x   0x%08x   0x%08x   0x%08x"%(
                "",
                counters["T1"]["L1A"],
                counters["T1"]["CalPulse"],
                counters["T1"]["Resync"],
                counters["T1"]["BC0"])
print "--=======================================--"
sys.stdout.flush()
