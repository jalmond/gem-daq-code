#!/bin/env python

import sys, re
import time, datetime, os

sys.path.append('/opt/gemdaq/firmware/testing/src')

import uhal
from registers_uhal import *
from glib_system_info_uhal import *
from rate_calculator import errorRate
from glib_user_functions_uhal import *

#from glib_clock_src import *

from optparse import OptionParser
parser = OptionParser()
parser.add_option("-s", "--slot", type="int", dest="slot",
		  help="slot in uTCA crate", metavar="slot", default=15)
parser.add_option("-c", "--clksrc", type="string", dest="clksrc",
		  help="select the input for the XPoint1 outputs", metavar="clksrc", default="local")
parser.add_option("-r", "--reset", action="store_true", dest="resetCounters",
		  help="reset link error counters", metavar="resetCounters")
parser.add_option("-x", "--external", type="int", dest="trgSrc",
		  help="change trigger source", metavar="trgSrc")
parser.add_option("-b", "--sbitout", type="int", dest="sbitSrc",
		  help="use s-bit from VFAT <num>", metavar="sbitSrc")
parser.add_option("-o", "--links", type="string", dest="activeLinks", action='append',
		  help="pair of connected optical links (GLIB,OH)", metavar="activeLinks", default=[])
parser.add_option("-d", "--debug", action="store_true", dest="debug",
		  help="print extra debugging information", metavar="debug")
parser.add_option("-e", "--errors", type="int", dest="errorRate", default=1,
		  help="calculate link error rates for N seconds", metavar="errorRate")
parser.add_option("-u", "--user", action="store_true", dest="userOnly",
		  help="print user information only", metavar="userOnly")
(options, args) = parser.parse_args()

links = {}
for link in options.activeLinks:
	pair = map(int, link.split(","))
	links[pair[0]] = pair[1]
print "links", links

uhal.setLogLevelTo( uhal.LogLevel.FATAL )

uTCAslot = 175
if options.slot:
	uTCAslot = 160+options.slot
print options.slot, uTCAslot
ipaddr = '192.168.0.%d'%(uTCAslot)
address_table = "file://${BUILD_HOME}/data/glib_address_table.xml"
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
print "-> BOARD USER INFORMATION"
print "--=======================================--"
print

if (options.resetCounters):
	for link in (links.keys()):
		linkCounters(True,glib,link,True)
print
for link in (links.keys()):
	print link, links.keys()
	print "-> GLIB link%d User FW :0x%08x"%(      link,readRegister(glib,"GLIB.GLIB_LINKS.LINK%d.USER_FW"%(link)))
	print "-> GLIB link%d tracking data :0x%08x"%(link,readRegister(glib,"GLIB.GLIB_LINKS.TRG_DATA.DATA"))
print
sys.stdout.flush()
#exit(1)
errorCounts = {}
for link in (links.keys()):
	errorCounts[link] = []
SAMPLE_TIME = 1.
for link in (links.keys()):
	for trial in range(options.errorRate):
		errorCounts[link].append(calculateLinkErrors(True,glib,link,SAMPLE_TIME))
sys.stdout.flush()

#if (options.clksrc):
#    print
#    print "changing the xpoint1 clock routing"
#    setXPoint1Outputs(glib,options.clksrc)
#    getXpointInfo(glib)

if options.trgSrc in [0,1,2]:
	for link in (links.keys()):
		setTriggerSource(True,glib,link,options.trgSrc)
		#writeRegister(glib,"GLIB.GLIB_LINKS.LINK%d.TrgSrc"%(link),options.trgSrc)
		print "-> GLIB link%d trigger source:   0x%x"%(link,
							       readRegister(glib,"GLIB.GLIB_LINKS.LINK%d.TrgSrc"%(link)))
if options.sbitSrc in [1,2,3,4,5,6]:
	for link in (links.keys()):
		writeRegister(glib,"GLIB.GLIB_LINKS.LINK%d.TDC_SBit"%(link),options.sbitSrc)
		print "-> GLIB link%d SBit to TDC: 0x%x"%(link,
							  readRegister(glib,"GLIB.GLIB_LINKS.LINK%d.TDC_SBit"%(link)))
	
sys.stdout.flush()
print
print "--=======================================--"
print "-> GLIB link num: %10s  %12s    %10s    %10s    %10s    %10s    %10s"%("ErrCnt","(rate)","FIFO_Occ",
									      "I2CRecCnt","I2CSndCnt",
									      "RegRecCnt","RegSndCnt")
sys.stdout.flush()

for link in (links.keys()):
	readTrackingInfo(glib,links[link])
	rates = errorRate(errorCounts[link],SAMPLE_TIME)
	counters = linkCounters(True,glib,False)
	print "-> link%d        : 0x%08x   (%6.2f%1sHz)    0x%08x    0x%08x    0x%08x    0x%08x    0x%08x"%(link,
													    counters["LinkErrors"],
													    rates[0], rates[1],
													    counters["TRK_FIFO_Depth"],
													    counters["RecI2CRequests"],
													    counters["SntI2CRequests"],
													    counters["RecRegRequests"],
													    counters["SntRegRequests"])
print "--=======================================--"
sys.stdout.flush()
