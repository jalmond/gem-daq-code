#!/bin/env python

import sys, re, time, datetime, os

sys.path.append('/opt/gemdaq/firmware/testing/src')

from PyChipsUser import *
from registers import *
from glib_system_info import *
from vfat_functions import *

glibAddrTable = AddressTable("/opt/gemdaq/firmware/testing/glibAddrTable.dat")

from optparse import OptionParser
parser = OptionParser()
parser.add_option("-s", "--slot", type="int", dest="slot",
		  help="slot in uTCA crate", metavar="slot", default=15)
parser.add_option("-d", "--debug", action="store_true", dest="debug",
		  help="print extra debugging information", metavar="debug")
parser.add_option("-o", "--links", type="string", dest="activeLinks", action='append',
		  help="pair of connected optical links", metavar="activeLinks", default=[])
parser.add_option("-z", "--sleep", action="store_true", dest="sleepAll",
		  help="set all chips into sleep mode", metavar="sleepAll")
parser.add_option("-b", "--bias", action="store_true", dest="biasAll",
		  help="set all chips with default bias parameters", metavar="biasAll")
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

########################################
# IP address
########################################
#ipaddr = '137.138.115.146'
glib = ChipsBusUdp(glibAddrTable, ipaddr, 50001)
if options.debug:
	print
	print "--=======================================--"
	print "  Opening GLIB with IP", ipaddr
	print "--=======================================--"
	getSystemInfo(glib)

print
print "--=======================================--"
print

ebmask = 0x000000ff

vfats = []
for gebslot in range(24):
	vfats.append("slot%d"%(gebslot+1))

##time.sleep(5)
chipids = []
controlReg0 = []
controlReg1 = []
controlReg2 = []
controlReg3 = []
emptyMask = 0xFFFF
thechipid = 0x0000
## these need to be done in a link specific way, so select the first available link
for link in (links.keys()):
	print "GLIB link%d User FW: 0x%08x"%(link,readRegister(glib,"GLIB_link%d_FW"%(link)))
for link in (links.keys()):
	print "OH link%d FW:        0x%08x"%(links[link],readRegister(glib,"OH_link%d_FW"%(links[link])))

print "GLIB links", links.keys()
for chip in range(0,24):
	if (0 not in links.keys()):
		if (chip < 8):
			#print "link 0 and chip %d check %d"%(chip,chip < 8)
			if options.debug:
				print "GLIB link 0 missing, skipping %d"%chip
			continue
	if (1 not in links.keys()):
		if ((chip > 7) and (chip < 16)):
			#print "link 1 and chip %d check %d,%d"%(chip,chip > 7, chip < 16)
			if options.debug:
				print "GLIB link 1 missing, skipping %d"%chip
			continue
	if (2 not in links.keys()):
		if (chip > 15):
			#print "link 2 and chip %d check %d,%d"%(chip,chip > 15, chip < 24)
			if options.debug:
				print "GLIB link 2 missing, skipping %d"%chip
			continue

	if options.biasAll:
		biasVFAT(glib,"slot%d"%(chip))
	if options.sleepAll:
		regVal = readRegister(glib,"vfat2_slot%d_ctrl0"%(chip))
		writeRegister(glib,"vfat2_slot%d_ctrl0"%(chip),regVal&0xFE)

	thechipid  = readRegister(glib,"vfat2_slot%d_chipid1"%(chip))
	if options.debug:
		print "chip%d chipid1 0x%08x"%(chip,thechipid)
	if (((thechipid>>16)&emptyMask) != ((0x050<<4)+(chip))):
		thechipid  = (thechipid & ebmask)<<8
		thechipid |= (readRegister(glib,"vfat2_slot%d_chipid0"%(chip)) & ebmask)
	else:
		thechipid = 0xdead
	chipids.append(("slot%d"%(chip),thechipid))
	controlReg0.append(("slot%d"%(chip),
			    readRegister(glib,"vfat2_slot%d_ctrl0"%(chip))&ebmask))
	controlReg1.append(("slot%d"%(chip),
			    readRegister(glib,"vfat2_slot%d_ctrl1"%(chip))&ebmask))
	controlReg2.append(("slot%d"%(chip),
			    readRegister(glib,"vfat2_slot%d_ctrl2"%(chip))&ebmask))
	controlReg3.append(("slot%d"%(chip),
			    readRegister(glib,"vfat2_slot%d_ctrl3"%(chip))&ebmask))


print "%6s  %6s  %02s  %02s  %02s  %02s"%("chip", "ID", "ctrl0", "ctrl1", "ctrl2", "ctrl3")
for i,chip in enumerate(chipids):
	if (int(chip[0][4:])%8==0):
		print "-------------GEB Column %d-----------------"%(int(chip[0][4:])/8)

	print "%s%6s%s  %s0x%04x%s   0x%02x   0x%02x   0x%02x   0x%02x"%(colors.GREEN,chip[0],colors.ENDC,
									 colors.CYAN,chip[1],colors.ENDC,
									 controlReg0[i][1],
									 controlReg1[i][1],
									 controlReg2[i][1],
									 controlReg3[i][1])
	

print
print "--=======================================--"
print

