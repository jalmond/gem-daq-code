#!/bin/env python

import sys, re
import time, datetime, os

sys.path.append('/opt/gemdaq/firmware/testing/src')

import uhal
from registers_uhal import *
from glib_user_functions_uhal import *
from rate_calculator import errorRate

from optparse import OptionParser

parser = OptionParser()
parser.add_option("-s", "--slot", type="int", dest="slot",
		  help="slot in uTCA crate", metavar="slot", default=15)
parser.add_option("-r", "--reset", action="store_true", dest="resetCounters",
		  help="reset link error counters", metavar="resetCounters")
parser.add_option("-k", "--extClock", action="store_true", dest="useExtClk",
		  help="use external clock on OH", metavar="useExtClk")
parser.add_option("-x", "--extTrig", type="int", dest="trgSrc",
		  help="change trigger source", metavar="trgSrc")
parser.add_option("-b", "--sbitout", type="int", dest="sbitSrc",
		  help="use s-bit from VFAT <num>", metavar="sbitSrc")
parser.add_option("-o", "--links", type="string", dest="activeLinks", action='append',
		  help="pair of connected optical links (GLIB,OH)", metavar="activeLinks", default=[])
parser.add_option("-d", "--debug", action="store_true", dest="debug",
		  help="print extra debugging information", metavar="debug")
parser.add_option("-e", "--errors", type="int", dest="errorRate", default=1,
		  help="calculate link error rates for N seconds", metavar="errorRate")
(options, args) = parser.parse_args()

links = {}
for link in options.activeLinks:
	pair = map(int, link.split(","))
	links[pair[0]] = pair[1]
print "links", links

uhal.setLogLevelTo( uhal.LogLevel.FATAL )

uTCAslot = 15
if options.slot:
	uTCAslot = 160+options.slot
	print options.slot, uTCAslot
ipaddr = '192.168.0.%d'%(uTCAslot)
address_table = "file://${BUILD_HOME}/data/optohybrid_address_table.xml"
uri = "chtcp-2.0://localhost:10203?target=%s:50001"%(ipaddr)
optohybrid  = uhal.getDevice( "optohybrid" , uri, address_table )
address_table = "file://${BUILD_HOME}/data/glib_address_table.xml"
glib  = uhal.getDevice( "glib" , uri, address_table )

########################################
# IP address
########################################
print
print "-> -----------------"
print "-> OPTOHYBRID STATUS     "
print "-> -----------------"

print

for link in (links.keys()):
	print "-> OptoHybrid link%d firmware:   0x%08x"%(links[link],
							 readRegister(optohybrid,"OptoHybrid.OptoHybrid_LINKS.LINK%d.FIRMWARE"%(links[link])))
print
##maybe should quit here if the previous transaction failed?
#for link in (links.keys()):
#	writeRegister(optohybrid,"OptoHybrid.OptoHybrid_LINKS.LINK%d.FAST_COM.Send.Resync"%(links[link]),0x1)
#	writeRegister(optohybrid,"OptoHybrid.OptoHybrid_LINKS.LINK%d.FAST_COM.Send.BC0"%(links[link]),   0x1)

# if (options.useExtClk):
# 	for link in (links.keys()):
# 		writeRegister(optohybrid,"OptoHybrid.OptoHybrid_LINKS.LINK%d_CDCE_SRC"%(links[link]),0x1)
# 		writeRegister(optohybrid,"OptoHybrid.OptoHybrid_LINKS.LINK%d_CDCE_BKP"%(links[link]),0x0)
# 		writeRegister(optohybrid,"OptoHybrid.OptoHybrid_LINKS.LINK%d_VFAT_SRC"%(links[link]),0x1)
# 		writeRegister(optohybrid,"OptoHybrid.OptoHybrid_LINKS.LINK%d_VFAT_BKP"%(links[link]),0x0)
		
# else:
# 	for link in (links.keys()):
# 		writeRegister(optohybrid,"OptoHybrid.OptoHybrid_LINKS.LINK%d_CDCE_SRC"%(links[link]),0x0)
# 		writeRegister(optohybrid,"OptoHybrid.OptoHybrid_LINKS.LINK%d_CDCE_BKP"%(links[link]),0x1)
# 		writeRegister(optohybrid,"OptoHybrid.OptoHybrid_LINKS.LINK%d_VFAT_SRC"%(links[link]),0x0)
#		writeRegister(optohybrid,"OptoHybrid.OptoHybrid_LINKS.LINK%d_VFAT_BKP"%(links[link]),0x1)
	
#for link in (links.keys()):
#	writeRegister(optohybrid,"OptoHybrid.OptoHybrid_LINKS.LINK%d.FAST_COM.Send.Resync"%(links[link]),0x1)
#	writeRegister(optohybrid,"OptoHybrid.OptoHybrid_LINKS.LINK%d.FAST_COM.Send.BC0"%(links[link]),   0x1)

#
#print "-> OH VFATs accessible: 0x%x"%(readRegister(glib,"VFATs_TEST"))
print "OH  %4s:  %6s  %7s  %8s  %6s  %6s"%("link","TrgSrc","SBitSrc","FPGA PLL","CDCE","GTP")
for link in (links.keys()):
	if options.trgSrc in [0,1,2]:
		setTriggerSource(False,optohybrid,links[link],options.trgSrc)
	
	if options.sbitSrc in [1,2,3,4,5,6]:
		setTriggerSBits(False,optohybrid,links[link],options.sbitSrc)

	clocking = getClockingInfo(optohybrid,links[link])
#OH  link:  TrgSrc  SBitSrc  FPGA PLL    CDCE     GTP
#-> link0:     0x0      0x0       0x0     0x1     0x1

	print "-> link%d:     0x%x      0x%x       0x%x     0x%x     0x%x"%(links[link],
									    readRegister(optohybrid,"OptoHybrid.OptoHybrid_LINKS.LINK%d.TRIGGER.SOURCE"%(links[link])),
									    readRegister(optohybrid,"OptoHybrid.OptoHybrid_LINKS.LINK%d.TRIGGER.TDC_SBits"%(links[link])),
									    clocking["fpgaplllock"],
									    clocking["cdcelock"],
									    clocking["gtpreclock"])
	
print 
print "-> OH link Clocking (src, bkp):     VFAT         CDCE"
for link in (links.keys()):
	print "-> %22s%d       (0x%x  0x%x)   (0x%x  0x%x)"%("link",links[link],
							     clocking["vfatsrc"],clocking["vfatbkp"],
							     clocking["cdcesrc"],clocking["cdcebkp"])
	
#exit(1)
if (options.resetCounters):
	for link in (links.keys()):
		linkCounters(False,optohybrid,links[link],True)

errorCounts = {}
for link in (links.keys()):
	errorCounts[links[link]] = []
SAMPLE_TIME = 1.
for link in (links.keys()):
	for trial in range(options.errorRate):
		errorCounts[links[link]].append(calculateLinkErrors(False,optohybrid,links[link],SAMPLE_TIME))
sys.stdout.flush()


print 
print "-> Counters    %8s     %8s     %8s     %8s     %8s"%("L1A","Cal","Resync","BC0","BX")
for link in (links.keys()):
	counters = linkCounters(False,optohybrid,links[link],False)
	print "-> link%d     0x%08x   0x%08x   0x%08x   0x%08x   0x%08x"%(links[link],
									  counters["L1A"],
									  counters["Cal"],
									  counters["Resync"],
									  counters["BC0"],
									  counters["BXCount"])
	
print
print
print "--=======================================--"
print "-> OH link num: %10s  %12s    %10s    %10s    %10s    %10s"%("ErrCnt","(rate)",
								    "I2CRecCnt","I2CSndCnt",
								    "RegRecCnt","RegSndCnt")
for link in (links.keys()):
	rates = errorRate(errorCounts[links[link]],SAMPLE_TIME)
	counters = linkCounters(False,optohybrid,links[link],False)
	print "-> link%d      : 0x%08x   (%6.2f%1sHz)    0x%08x    0x%08x    0x%08x    0x%08x"%(links[link],
												counters["LinkErrors"],
												rates[0],rates[1],
												counters["RecI2CRequests"],
												counters["SntI2CRequests"],
												counters["RecRegRequests"],
												counters["SntRegRequests"])
exit(1)
print
vfatRange = ["0-7","8-15","16-23"]
for link in (links.keys()):
	print "FIFO%d depth = %d"%(link,readRegister(glib,"GLIB.GLIB_LINKS.LINK%d.TRK_FIFO.DEPTH"%(link)))
	print "vfat (%s) have data: 0x%x"%(link,readRegister(optohybrid,"OptoHybrid.GEB.TRK_DATA.COL%d.DATA_RDY%d"%(link)))
	

print
print "--=======================================--"


