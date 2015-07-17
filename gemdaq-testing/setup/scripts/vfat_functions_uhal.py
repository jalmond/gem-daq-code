import sys, os, time, signal, random
sys.path.append('/opt/gemdaq/firmware/testing/src')

import uhal
from registers_uhal import *

def setupDefaultCRs(device,chip, sleep=False):
    baseNode = "OptoHybrid.GEB.VFATS.VFAT%d"%(chip)
    if not sleep:
        writeRegister(device,"%s.ContReg0"%(baseNode),  0x36)
    else:
        writeRegister(device,"%s.ContReg0"%(baseNode),  0x37)
    writeRegister(device,"%s.ContReg1"%(    baseNode),  0x00)
    writeRegister(device,"%s.ContReg2"%(    baseNode),  0x30)
    writeRegister(device,"%s.ContReg3"%(    baseNode),  0x00)
    return

def biasVFAT(device,chip):
    baseNode = "OptoHybrid.GEB.VFATS.VFAT%d"%(chip)
    writeRegister(device,"%s.ContReg0"%(   baseNode), 0x37)
    writeRegister(device,"%s.ContReg1"%(   baseNode), 0x00)
    writeRegister(device,"%s.ContReg2"%(   baseNode), 0x30)
    writeRegister(device,"%s.ContReg3"%(   baseNode), 0x00)
    writeRegister(device,"%s.Latency"%(    baseNode),   20)
    writeRegister(device,"%s.IPreampIn"%(  baseNode),  168)
    writeRegister(device,"%s.IPreampFeed"%(baseNode),   80)
    writeRegister(device,"%s.IPreampOut"%( baseNode),  150)
    writeRegister(device,"%s.IShaper"%(    baseNode),  150)
    writeRegister(device,"%s.IShaperFeed"%(baseNode),  100)
    writeRegister(device,"%s.IComp"%(      baseNode),   75)
    writeRegister(device,"%s.VCal"%(       baseNode), 0x00)
    writeRegister(device,"%s.VThreshold1"%(baseNode),   50)
    writeRegister(device,"%s.VThreshold2"%(baseNode), 0x00)
    writeRegister(device,"%s.CalPhase"%(   baseNode), 0x00)

    for chan in range(128):
        writeRegister(device,"%s.VFATChannels.ChanReg%d"%(baseNode,chan+1),0x40)
    return

