import sys, os, time, signal, random
sys.path.append('/opt/gemdaq/firmware/testing/src')

from PyChipsUser import *

gMAX_RETRIES = 11
gRetries = 0
class colors:
	WHITE   = '\033[97m'
	CYAN    = '\033[96m'
	MAGENTA = '\033[95m'
	BLUE    = '\033[94m'
	YELLOW  = '\033[93m'
	GREEN   = '\033[92m'
	RED     = '\033[91m'
	ENDC    = '\033[0m'


def readRegister(glib,register):
	global gRetries
	nRetries = 0
	while (nRetries < gMAX_RETRIES):
		try:
			controlChar = glib.read(register)
			return controlChar
		
		except ChipsException, e:
			nRetries += 1
			gRetries += 1
			if ((nRetries % 10)==0):
				print colors.MAGENTA,"read error encountered (%s), retrying operation (%d,%d)"%(register,nRetries,gRetries),e,colors.ENDC
			continue
		pass
	#print colors.RED, "error encountered, retried read operation (%d)"%(nRetries)
	return 0x0
	
def writeRegister(glib,register, value):
	global gRetries
	nRetries = 0
	while (nRetries < gMAX_RETRIES):
		try:
			glib.write(register, value)
			return

		except ChipsException, e:
			#if ('amount of data' in e.value):
			#	print colors.BLUE, "bad header",register, "-> Error : ", e, colors.ENDC
			#elif ('INFO CODE = 0x4L' in e.value):
			#	print colors.CYAN, "read error",register, "-> Error : ", e, colors.ENDC
			#elif ('INFO CODE = 0x6L' in e.value or 'timed out' in e.value):
			#	print colors.YELLOW, "timed out",register, "-> Error : ", e, colors.ENDC
			#else:
			#	print colors.MAGENTA, "other error",register, "-> Error : ", e, colors.ENDC
			nRetries += 1
			gRetries += 1
			if ((nRetries % 10)==0):
				print colors.MAGENTA,"write error encountered (%s), retrying operation (%d,%d)"%(register,nRetries,gRetries),e,colors.ENDC
			continue
		pass
	#print colors.RED, "error encountered, retried test write operation (%d)"%(nRetries)
	pass

