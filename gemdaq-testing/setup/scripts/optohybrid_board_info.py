#!/bin/env python

import sys, re
import time, datetime, os

sys.path.append('/opt/gemdaq/firmware/testing/src')

from PyChipsUser import *
from registers import *

glibAddrTable = AddressTable("/opt/gemdaq/firmware/testing/glibAddrTable.dat")

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

uTCAslot = 15
if options.slot:
	uTCAslot = 160+options.slot
	print options.slot, uTCAslot
ipaddr = '192.168.0.%d'%(uTCAslot)

########################################
# IP address
########################################
#ipaddr = '137.138.115.146'
glib = ChipsBusUdp(glibAddrTable, ipaddr, 50001)
print
print "-> -----------------"
print "-> OPTOHYBRID STATUS     "
print "-> -----------------"

print

for link in (links.keys()):
	print "-> OptoHybrid link%d accessible: 0x%08x"%(links[link],
							 readRegister(glib,"OH_link%d_TEST"%(links[link])))
	print "-> OptoHybrid link%d firmware:   0x%08x"%(links[link],
							 readRegister(glib,"OH_link%d_FW"%(links[link])))
print

#for link in (links.keys()):
#	writeRegister(glib,"OH_link%d_SendResync"%(links[link]),0x1)
#	writeRegister(glib,"OH_link%d_SendBC0"%(links[link]),   0x1)

# if (options.useExtClk):
# 	for link in (links.keys()):
# 		writeRegister(glib,"OH_link%d_CDCE_SRC"%(links[link]),0x1)
# 		writeRegister(glib,"OH_link%d_CDCE_BKP"%(links[link]),0x0)
# 		writeRegister(glib,"OH_link%d_VFAT_SRC"%(links[link]),0x1)
# 		writeRegister(glib,"OH_link%d_VFAT_BKP"%(links[link]),0x0)
		
# else:
# 	for link in (links.keys()):
# 		writeRegister(glib,"OH_link%d_CDCE_SRC"%(links[link]),0x0)
# 		writeRegister(glib,"OH_link%d_CDCE_BKP"%(links[link]),0x1)
# 		writeRegister(glib,"OH_link%d_VFAT_SRC"%(links[link]),0x0)
#		writeRegister(glib,"OH_link%d_VFAT_BKP"%(links[link]),0x1)
	
#for link in (links.keys()):
#	writeRegister(glib,"OH_link%d_SendResync"%(links[link]),0x1)
#	writeRegister(glib,"OH_link%d_SendBC0"%(links[link]),   0x1)

#
#print "-> OH VFATs accessible: 0x%x"%(readRegister(glib,"VFATs_TEST"))
for link in (links.keys()):
	if options.trgSrc in [0,1,2]:
		for link in (links.keys()):
			writeRegister(glib,"OH_link%d_TrgSrc"%(links[link]),options.trgSrc)
	writeRegister(glib,"OH_link%d_SendResync"%(links[link]),0x1)
	
	print "-> GLIB link%d Trigger source: 0x%x"%(link,readRegister(glib,"GLIB_link%d_TrgSrc"%(link)))

	if options.sbitSrc in [1,2,3,4,5,6]:
		writeRegister(glib,"OH_link%d_TDC_SBit"%(links[link]),options.sbitSrc)
	writeRegister(glib,"OH_link%d_SendResync"%(links[link]),0x1)

	print "-> OH link%d Trigger source:  0x%x"%(links[link],readRegister(glib,"OH_link%d_TrgSrc"%(links[link])))
	print "-> OH link%d SBit to TDC:     0x%x"%(links[link],readRegister(glib,"OH_link%d_TDC_SBit"%(links[link])))
	print 
	print "-> OH link%d FPGA PLL Locked : 0x%x"%(links[link],readRegister(glib,"OH_link%d_FPGA_PLL_Locked"%(links[link])))
	print "-> OH link%d CDCE Locked     : 0x%x"%(links[link],readRegister(glib,"OH_link%d_CDCE_Locked"%(links[link])))
	print "-> OH link%d GTP Locked      : 0x%x"%(links[link],readRegister(glib,"OH_link%d_GTP_Locked"%(links[link])))
	print 
	print "-> OH link Clocking:    VFAT        CDCE"
	print "-> OH link%d Source         0x%x         0x%x"%(links[link],readRegister(glib,"OH_link%d_VFAT_SRC"%(links[link])),
							       readRegister(glib,"OH_link%d_CDCE_SRC"%(links[link])))
	print "-> OH link%d Backup         0x%x         0x%x"%(links[link],readRegister(glib,"OH_link%d_VFAT_BKP"%(links[link])),
							       readRegister(glib,"OH_link%d_CDCE_BKP"%(links[link])))

#-> Counters::         L1A          Cal       Resync          BC0           BX
#->             0x00000800   0x00000000   0x00000000      0x00000000   0x154a86b7
if (options.resetCounters):
	for link in (links.keys()):
	#if (0 in options.activeLinks):
		writeRegister(glib,"GLIB_link%d_FIFO_Flush"%(link)  ,0x1)
		writeRegister(glib,"GLIB_link%d_Rst_ErrCnt"%(link)  ,0x1)
		writeRegister(glib,"OH_link%d_RstL1A"%(links[link])        ,0x1)
		time.sleep(1)
		writeRegister(glib,"OH_link%d_RstCal"%(links[link])        ,0x1)
		writeRegister(glib,"OH_link%d_RstResync"%(links[link])     ,0x1)
		writeRegister(glib,"OH_link%d_RstBC0"%(links[link])        ,0x1)
		writeRegister(glib,"OH_link%d_Rst_ErrCnt"%(links[link])    ,0x1)
		writeRegister(glib,"OH_link%d_Rst_I2CRecCnt"%(links[link]) ,0x1)
		writeRegister(glib,"OH_link%d_Rst_I2CSndCnt"%(links[link]) ,0x1)
		writeRegister(glib,"OH_link%d_Rst_RegRecCnt"%(links[link]) ,0x1)
		writeRegister(glib,"OH_link%d_Rst_RegSndCnt"%(links[link]) ,0x1)


print 
print "-> Counters::    %8s     %8s     %8s     %8s     %8s"%("L1A","Cal","Resync","BC0","BX")
for link in (links.keys()):
	print "-> link%d            0x%08x   0x%08x   0x%08x   0x%08x   0x%08x"%(links[link],
										 readRegister(glib,"OH_link%d_ReadL1A"%(links[link])),
										 readRegister(glib,"OH_link%d_ReadCal"%(links[link])),
										 readRegister(glib,"OH_link%d_ReadResync"%(links[link])),
										 readRegister(glib,"OH_link%d_ReadBC0"%(links[link])),
										 readRegister(glib,"OH_link%d_ReadBX"%(links[link])))

errorCounts = {}
for link in (links.keys()):
	errorCounts[link] = []

for link in (links.keys()):
	errorCounts[links[link]].append(readRegister(glib,"OH_link%d_ErrCnt"%(links[link])))
	time.sleep(options.errorRate)
	errorCounts[links[link]].append(readRegister(glib,"OH_link%d_ErrCnt"%(links[link])))

print "-> OH link num:  %28s    %10s    %10s    %10s    %10s"%("ErrCnt (rate)",
							      "I2CRecCnt",
							      "I2CSndCnt",
							      "RegRecCnt",
							      "RegSndCnt")

for link in (links.keys()):
#if (0 in options.activeLinks):
	print "-> link%d        : 0x%08x (%12.2fHz)    0x%08x    0x%08x    0x%08x    0x%08x"%(links[link],
											      readRegister(glib,"OH_link%d_ErrCnt"%(links[link])),
											      ((errorCounts[link][1]-errorCounts[link][0])/(1.0*options.errorRate)),
											      readRegister(glib,"OH_link%d_I2CRecCnt"%(links[link])),
											      readRegister(glib,"OH_link%d_I2CSndCnt"%(links[link])),
											      readRegister(glib,"OH_link%d_RegRecCnt"%(links[link])),
											      readRegister(glib,"OH_link%d_RegSndCnt"%(links[link])))
print
vfatRange = ["0-7","8-15","16-23"]
for link in (links.keys()):
#if (0 in options.activeLinks):
	print "FIFO%d depth = %d"%(link,readRegister(glib,"GLIB_link%d_FIFO_Occ"%(link)))
	print "is vfat (%s)   data: 0x%x"%(link,readRegister(glib,"ch1trackingdata%d"%(link)))
	
#if (1 in options.activeLinks):
#	print "FIFO1 depth = %d"%readRegister(glib,"GLIB_link1_FIFO_Occ")
#	print "is vfat (8-15)  data: 0x%x"%(readRegister(glib,"ch2trackingdata0"))
#
#if (2 in options.activeLinks):
#	print "FIFO2 depth = %d"%readRegister(glib,"GLIB_link2_FIFO_Occ")
#	print "is vfat (16-23) data: 0x%x"%(readRegister(glib,"ch3trackingdata0"))


print
print "--=======================================--"


