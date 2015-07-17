import sys, os, time, signal, random
sys.path.append('/opt/gemdaq/firmware/testing/src')

import uhal
from registers_uhal import *

def calculateLinkErrors(isGLIB,device,link,sampleTime):
    baseNode = "GLIB.GLIB_LINKS.LINK%d"%(link)
    if not isGLIB:
        baseNode = "OptoHybrid.OptoHybrid_LINKS.LINK%d"%(link)

    writeRegister(device,"%s.OPTICAL_LINKS.Resets.LinkErr"%(baseNode),0x1)
    first = readRegister(device,"%s.OPTICAL_LINKS.Counter.LinkErr"%(baseNode))
    time.sleep(sampleTime)
    second = readRegister(device,"%s.OPTICAL_LINKS.Counter.LinkErr"%(baseNode))
    errorCounts = [first,second]
    return errorCounts

def linkCounters(isGLIB,device,link,doReset=False):
    """
    read the optical link counters, returning a map
    if doReset is true, just send the reset command and pass
    """
    baseNode = "GLIB.GLIB_LINKS.LINK%d"%(link)
    if not isGLIB:
        baseNode = "OptoHybrid.OptoHybrid_LINKS.LINK%d"%(link)

    if not doReset:
        counters = {}
        counters["LinkErrors"] = readRegister(device,"%s.OPTICAL_LINKS.Counter.LinkErr"%(baseNode))
        if isGLIB:
            counters["TRK_FIFO_Depth"] = readRegister(device,"%s.TRK_FIFO.DEPTH"%(       baseNode))
        else:                                                                            
            counters["L1A"]     = readRegister(device,"%s.COUNTERS.L1A.Total"%(      baseNode))
            counters["Cal"]     = readRegister(device,"%s.COUNTERS.CalPulse.Total"%( baseNode))
            counters["Resync"]  = readRegister(device,"%s.COUNTERS.Resync"%(         baseNode))
            counters["BC0"]     = readRegister(device,"%s.COUNTERS.BC0"%(            baseNode))
            counters["BXCount"] = readRegister(device,"%s.COUNTERS.BXCount"%(        baseNode))

        counters["RecI2CRequests"] = readRegister(device,"%s.OPTICAL_LINKS.Counter.RecI2CRequests"%(baseNode))
        counters["SntI2CRequests"] = readRegister(device,"%s.OPTICAL_LINKS.Counter.SntI2CRequests"%(baseNode))
        counters["RecRegRequests"] = readRegister(device,"%s.OPTICAL_LINKS.Counter.RecRegRequests"%(baseNode))
        counters["SntRegRequests"] = readRegister(device,"%s.OPTICAL_LINKS.Counter.SntRegRequests"%(baseNode))
        return counters
    else:
        writeRegister(device,"%s.OPTICAL_LINKS.Resets.LinkErr"%(       baseNode),0x1)
        if isGLIB:
            writeRegister(device,"%s.TRK_FIFO.FLUSH"%(                 baseNode),0x1)
        else:
            writeRegister(device,"%s.COUNTERS.RESETS.L1A.Total"%(     baseNode),0x1)
            writeRegister(device,"%s.COUNTERS.RESETS.CalPulse.Total"%(baseNode),0x1)
            writeRegister(device,"%s.COUNTERS.RESETS.Resync"%(        baseNode),0x1)
            writeRegister(device,"%s.COUNTERS.RESETS.BC0"%(           baseNode),0x1)

        writeRegister(device,"%s.OPTICAL_LINKS.Resets.RecI2CRequests"%(baseNode),0x1)
        writeRegister(device,"%s.OPTICAL_LINKS.Resets.SntI2CRequests"%(baseNode),0x1)
        writeRegister(device,"%s.OPTICAL_LINKS.Resets.RecRegRequests"%(baseNode),0x1)
        writeRegister(device,"%s.OPTICAL_LINKS.Resets.SntRegRequests"%(baseNode),0x1)
        return

def readTrackingInfo(device,link):
    """
    read the tracking info from given optical link, returning a map
    """
    baseNode = "GLIB.TRK_DATA.COL%d"%(link)

    data = {}
    data["hasData"] = readRegister(device,"%s.DATA_RDY"%(baseNode))
    for word in range(1,7):
        data["data%d"%(word)] = readRegister(device,"%s.DATA.%d"%(baseNode,word))
    
    for word in data.keys():
        print "%s: 0x%08x"%(word,data[word])
    return data

def setTriggerSource(isGLIB,device,link,source):
    """
    Set the trigger source
    GLIB: 0=software, 1=backplane, 2=both 
    OH:   0=GLIB,     1=external,  2=both
    """
    if isGLIB:
        writeRegister(device,"GLIB.GLIB_LINKS.LINK%d.TrgSrc"%(link),source)
    else:
        writeRegister(device,"OptoHybrid.OptoHybrid_LINKS.LINK%d.TRIGGER.SOURCE"%(link),source)
	writeRegister(device,"OptoHybrid.OptoHybrid_LINKS.LINK%d.FAST_COM.Send.Resync"%(link),0x1)

    return

def setTriggerSBits(isGLIB,device,link,source):
    """
    Set the trigger sbit source
    GLIB: 0=software, 1=backplane, 2=both 
    OH:   0=GLIB,     1=external,  2=both
    """
    if isGLIB:
        writeRegister(device,"GLIB.GLIB_LINKS.TRIGGER.TDC_SBits",source)
    else:
        writeRegister(device,"OptoHybrid.OptoHybrid_LINKS.LINK%d.TRIGGER.TDC_SBits"%(link),source)
	writeRegister(device,"OptoHybrid.OptoHybrid_LINKS.LINK%d.FAST_COM.Send.Resync"%(link),0x1)

    return

def getClockingInfo(device,link):
    """
    Get the OptoHybrid clocking information
    """
    clocking = {}

    clocking["fpgaplllock"] = readRegister(device,"OptoHybrid.OptoHybrid_LINKS.LINK%d.CLOCKING.FPGA_PLL_LOCKED"%(link))
    clocking["cdcelock"]    = readRegister(device,"OptoHybrid.OptoHybrid_LINKS.LINK%d.CLOCKING.CDCE_LOCKED"%(    link))
    clocking["gtpreclock"]  = readRegister(device,"OptoHybrid.OptoHybrid_LINKS.LINK%d.CLOCKING.GTP_REC_LOCKED"%( link))
    clocking["vfatsrc"]     = readRegister(device,"OptoHybrid.OptoHybrid_LINKS.LINK%d.CLOCKING.VFAT.SOURCE"%(    link))
    clocking["cdcesrc"]     = readRegister(device,"OptoHybrid.OptoHybrid_LINKS.LINK%d.CLOCKING.CDCE.SOURCE"%(    link))
    clocking["vfatbkp"]     = readRegister(device,"OptoHybrid.OptoHybrid_LINKS.LINK%d.CLOCKING.VFAT.FALLBACK"%(  link))
    clocking["cdcebkp"]     = readRegister(device,"OptoHybrid.OptoHybrid_LINKS.LINK%d.CLOCKING.CDCE.FALLBACK"%(  link))

    return clocking
