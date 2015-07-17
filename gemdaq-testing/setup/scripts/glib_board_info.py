#!/bin/env python

import sys, re
import time, datetime, os

sys.path.append('/opt/gemdaq/firmware/testing/src')

from PyChipsUser import *
from registers import *
from glib_system_info import *
from glib_clock_src import *

glibAddrTable = AddressTable("/opt/gemdaq/firmware/testing/glibAddrTable.dat")

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

uTCAslot = 175
if options.slot:
	uTCAslot = 160+options.slot
print options.slot, uTCAslot
ipaddr = '192.168.0.%d'%(uTCAslot)

#f = open('/opt/gemdaq/firmware/testing/ipaddr.dat','w')
#f.write(ipaddr)
#f.close()
########################################
# IP address
########################################
glib = ChipsBusUdp(glibAddrTable, ipaddr, 50001)
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
	#if (0 in options.activeLinks):
		writeRegister(glib,"GLIB_link%d_FIFO_Flush"%(link)   ,0x1)
		writeRegister(glib,"GLIB_link%d_Rst_ErrCnt"%(link)   ,0x1)
		writeRegister(glib,"GLIB_link%d_Rst_I2CRecCnt"%(link),0x1)
		writeRegister(glib,"GLIB_link%d_Rst_I2CSndCnt"%(link),0x1)
		writeRegister(glib,"GLIB_link%d_Rst_RegRecCnt"%(link),0x1)
		writeRegister(glib,"GLIB_link%d_Rst_RegSndCnt"%(link),0x1)

	#if (1 in options.activeLinks):
	#	writeRegister(glib,"GLIB_link1_FIFO_Flush"   ,0x1)
	#	writeRegister(glib,"GLIB_link1_Rst_ErrCnt"   ,0x1)
	#	writeRegister(glib,"GLIB_link1_Rst_I2CRecCnt",0x1)
	#	writeRegister(glib,"GLIB_link1_Rst_I2CSndCnt",0x1)
	#	writeRegister(glib,"GLIB_link1_Rst_RegRecCnt",0x1)
	#	writeRegister(glib,"GLIB_link1_Rst_RegSndCnt",0x1)
	#
	#if (2 in options.activeLinks):
	#	writeRegister(glib,"GLIB_link2_FIFO_Flush"   ,0x1)
	#	writeRegister(glib,"GLIB_link2_Rst_ErrCnt"   ,0x1)
	#	writeRegister(glib,"GLIB_link2_Rst_I2CRecCnt",0x1)
	#	writeRegister(glib,"GLIB_link2_Rst_I2CSndCnt",0x1)
	#	writeRegister(glib,"GLIB_link2_Rst_RegRecCnt",0x1)
	#	writeRegister(glib,"GLIB_link2_Rst_RegSndCnt",0x1)

print
for link in (links.keys()):
	print "-> GLIB link%d User FW :0x%08x"%(link,readRegister(glib,"GLIB_link%d_FW"%(link)))
	print "-> GLIB link%d tracking data :0x%08x"%(link,readRegister(glib,"triggerdata"))
print
errorCounts = {}
for link in (links.keys()):
	errorCounts[link] = []

for link in (links.keys()):
	errorCounts[link].append(readRegister(glib,"GLIB_link%d_ErrCnt"%(link)))
	time.sleep(options.errorRate)
	errorCounts[link].append(readRegister(glib,"GLIB_link%d_ErrCnt"%(link)))

#if (options.clksrc):
#    print
#    print "changing the xpoint1 clock routing"
#    setXPoint1Outputs(glib,options.clksrc)
#    getXpointInfo(glib)

if options.trgSrc in [0,1,2]:
	for link in (links.keys()):
		writeRegister(glib,"GLIB_link%d_TrgSrc"%(link),options.trgSrc)
	print "-> GLIB link%d trigger source:   0x%x"%(link,
						       readRegister(glib,"GLIB_link%d_TrgSrc"%(link)))
if options.sbitSrc in [1,2,3,4,5,6]:
	for link in (links.keys()):
		writeRegister(glib,"GLIB_link%d_TDC_SBit"%(link),options.sbitSrc)
	print "-> GLIB link%d SBit to TDC: 0x%x"%(link,
						  readRegister(glib,"GLIB_link%d_TDC_SBit"%(link)))
	
print
print "--=======================================--"
print "-> GLIB link num: %20s    %10s    %10s    %10s    %10s    %10s"%("ErrCnt(rate)",
									"FIFO_Occ",
									"I2CRecCnt",
									"I2CSndCnt",
									"RegRecCnt",
									"RegSndCnt")
for link in (links.keys()):
#if (0 in options.activeLinks):
	print "-> link%d        : 0x%08x (%2.2fHz)    0x%08x    0x%08x    0x%08x    0x%08x    0x%08x"%(link,
												     readRegister(glib,"GLIB_link%d_ErrCnt"%(link)),
												     ((errorCounts[link][1]-errorCounts[link][0])/(1.0*options.errorRate)),
												     readRegister(glib,"GLIB_link%d_FIFO_Occ"%(link)),
												     readRegister(glib,"GLIB_link%d_I2CRecCnt"%(link)),
												     readRegister(glib,"GLIB_link%d_I2CSndCnt"%(link)),
												     readRegister(glib,"GLIB_link%d_RegRecCnt"%(link)),
												     readRegister(glib,"GLIB_link%d_RegSndCnt"%(link)))
print "--=======================================--"
