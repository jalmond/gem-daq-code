import sys, os, time, signal, random
sys.path.append('${GEM_PYTHON_PATH}')

import uhal
from registers_uhal import *

def calculateLinkErrors(isGLIB,device,gtx,sampleTime):
    baseNode = "GLIB.COUNTERS.GTX%d"%(gtx)
    errorCounts = {}
    if not isGLIB:
        baseNode = "GLIB.OptoHybrid_%d.OptoHybrid.COUNTERS.GTX"%(gtx)

    for link in ("TRK","TRG"):
        writeRegister(device,"%s.%s_ERR.Reset"%(baseNode,link),0x1)
        first = readRegister(device,"%s.%s_ERR"%(baseNode,link))
        time.sleep(sampleTime)
        second = readRegister(device,"%s.%s_ERR"%(baseNode,link))
        errorCounts[link] = [first,second]
    return errorCounts


def glibCounters(device,gtx,doReset=False):
    """
    read the optical gtx counters, returning a map
    if doReset is true, just send the reset command and pass
    IPBus:Strobe,Ack
    T1:L1A,CalPulse,Resync,BC0
    GTX:
    """
    baseNode = "GLIB.COUNTERS"

    if doReset:
        for ipbcnt in ["Strobe","Ack"]:
            writeRegister(device,"%s.IPBus.%s.OptoHybrid_%d.Reset"%(baseNode, ipbcnt, gtx),0x1)
            writeRegister(device,"%s.IPBus.%s.TRK_%d.Reset"%(       baseNode, ipbcnt, gtx),0x1)
            writeRegister(device,"%s.IPBus.%s.Counters.Reset"%(     baseNode, ipbcnt),     0x1)

        #T1 counters
        for t1 in ["L1A", "CalPulse","Resync","BC0"]:
            writeRegister(device,"%s.T1.%s.Reset"%(baseNode, t1),0x1)

        writeRegister(device,"%s.GTX%d.TRK_ERR.Reset"%(     baseNode, gtx), 0x1)
        writeRegister(device,"%s.GTX%d.TRG_ERR.Reset"%(     baseNode, gtx), 0x1)
        writeRegister(device,"%s.GTX%d.DATA_Packets.Reset"%(baseNode, gtx), 0x1)
        return
    else:
        counters = {}
        
        counters["IPBus"] = {}
        for ipbcnt in ["Strobe","Ack"]:
            counters["IPBus"][ipbcnt] = {}
            for ipb in ["OptoHybrid","TRK"]:
                counters["IPBus"][ipbcnt][ipb] = readRegister(device,"%s.IPBus.%s.%s_%d"%(   baseNode, ipbcnt,ipb,gtx))
            counters["IPBus"][ipbcnt]["Counters"] = readRegister(device,"%s.IPBus.%s.Counters"%(baseNode, ipbcnt))


        #T1 counters
        counters["T1"] = {}
        for t1 in ["L1A", "CalPulse","Resync","BC0"]:
            counters["T1"][t1] = readRegister(device,"%s.T1.%s"%(baseNode, t1))

        counters["GTX%d"%(gtx)] = {}
        counters["GTX%d"%(gtx)]["TRK_ERR"]      = readRegister(device,"%s.GTX%d.TRK_ERR"%(baseNode,gtx))
        counters["GTX%d"%(gtx)]["TRG_ERR"]      = readRegister(device,"%s.GTX%d.TRG_ERR"%(baseNode,gtx))
        counters["GTX%d"%(gtx)]["DATA_Packets"] = readRegister(device,"%s.GTX%d.DATA_Packets"%(baseNode,gtx))
        return counters


def readTrackingInfo(device,gtx,nBlocks=1):
    """
    read the tracking info from given optical gtx, returning a map
    """
    baseNode = "GLIB.TRK_DATA.OptoHybrid_%d"%(gtx)
    data = readBlock(device,"%s.FIFO"%(baseNode,7*nBlocks))
    
    #for word in data:
    #    print "%s: 0x%08x"%(word,data)
    return data

def flushTrackingFIFO(device,gtx):
    """
    Flush the tracking FIFO from given optical gtx
    """
    baseNode = "GLIB.TRK_DATA.OptoHybrid_%d"%(gtx)
    writeRegister(device,"%s.FLUSH"%(baseNode),0x1)
    return

def readFIFODepth(device,gtx):
    """
    read the tracking FIFO depth from given optical gtx
    """
    baseNode = "GLIB.TRK_DATA.OptoHybrid_%d"%(gtx)

    data = {}
    data["isFULL"]    = readRegister(device,"%s.ISFULL"%(baseNode))
    data["isEMPTY"]   = readRegister(device,"%s.ISEMPTY"%(baseNode))
    data["Occupancy"] = readRegister(device,"%s.DEPTH"%(baseNode))
    return data
        
def setTriggerSBits(isGLIB,device,gtx,source):
    """
    Set the trigger sbit source
    """
    if isGLIB:
        writeRegister(device,"GLIB.GLIB_LINKS.LINK%d.TRIGGER.TDC_SBits"%(gtx),source)
    else:
        writeRegister(device,"GLIB.OptoHybrid_%d.TRIGGER.TDC.SBits"%(gtx),source)
        sendResync(device,1,1)
    return

def getTriggerSBits(isGLIB,device,gtx):
    """
    Get the trigger sbit source
    """
    if isGLIB:
        return readRegister(device,"GLIB.GLIB_LINKS.LINK%d.TRIGGER.TDC_SBits"%(gtx))
    else:
        return readRegister(device,"GLIB.OptoHybrid_%d.TRIGGER.TDC.SBits"%(gtx))
