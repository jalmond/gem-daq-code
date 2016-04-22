import sys, os, time, signal, random
sys.path.append('${GEM_PYTHON_PATH}')

import uhal
from registers_uhal import *

def getBasicSystemInfo(glib):

    #chipsLog.setLevel(logging.DEBUG)    # Verbose logging (see packets being sent and received)
    print
    print "--=======================================--"
    print "-> BOARD SYSTEM INFORMATION"
    print "--=======================================--"
    print
    brd_char 	= ['w','x','y','z']
    brd_char[0] = chr(readRegister(glib,"GLIB.SYSTEM.BOARD_ID.CHAR1"))
    brd_char[1] = chr(readRegister(glib,"GLIB.SYSTEM.BOARD_ID.CHAR2"))
    brd_char[2] = chr(readRegister(glib,"GLIB.SYSTEM.BOARD_ID.CHAR3"))
    brd_char[3] = chr(readRegister(glib,"GLIB.SYSTEM.BOARD_ID.CHAR4"))
    
    
    board_id = ''.join([brd_char[0],brd_char[1],brd_char[2],brd_char[3]])
    print "-> board type  : %s%s%s"%(colors.CYAN,board_id,colors.ENDC)
    
    sys_char 	= ['w','x','y','z']
    sys_char[0] = chr(readRegister(glib,"GLIB.SYSTEM.SYSTEM_ID.CHAR1"))
    sys_char[1] = chr(readRegister(glib,"GLIB.SYSTEM.SYSTEM_ID.CHAR2"))
    sys_char[2] = chr(readRegister(glib,"GLIB.SYSTEM.SYSTEM_ID.CHAR3"))
    sys_char[3] = chr(readRegister(glib,"GLIB.SYSTEM.SYSTEM_ID.CHAR4"))
    
    sys_id = ''.join([sys_char[0],sys_char[1],sys_char[2],sys_char[3]])
    print "-> system type : %s%s%s"%(colors.GREEN,sys_id,colors.ENDC)
    
    ver_major = readRegister(glib,"GLIB.SYSTEM.FIRMWARE.MAJOR")
    ver_minor = readRegister(glib,"GLIB.SYSTEM.FIRMWARE.MINOR")
    ver_build = readRegister(glib,"GLIB.SYSTEM.FIRMWARE.BUILD")
    
    ver = '.'.join([str(ver_major),str(ver_minor),str(ver_build)])
    print "-> version nbr : %s%s%s"%(colors.MAGENTA,ver,colors.ENDC)
    
    yyyy  = 2000+readRegister(glib,"GLIB.SYSTEM.FIRMWARE.YY")
    mm    = readRegister(glib,"GLIB.SYSTEM.FIRMWARE.MM")
    dd    = readRegister(glib,"GLIB.SYSTEM.FIRMWARE.DD")
    
    date = '/'.join([str(dd),str(mm),str(yyyy)])
    print "-> sys fw date : %s%s%s"%(colors.YELLOW,date,colors.ENDC)
    
    
    mac    = ['00','00','00','00','00','00']
    mac[5] = "%02x"%(readRegister(glib,"GLIB.SYSTEM.MAC.B5"))
    mac[4] = "%02x"%(readRegister(glib,"GLIB.SYSTEM.MAC.B4"))
    mac[3] = "%02x"%(readRegister(glib,"GLIB.SYSTEM.MAC.B3"))
    mac[2] = "%02x"%(readRegister(glib,"GLIB.SYSTEM.MAC.B2"))
    mac[1] = "%02x"%(readRegister(glib,"GLIB.SYSTEM.MAC.B1"))
    mac[0] = "%02x"%(readRegister(glib,"GLIB.SYSTEM.MAC.B0"))
    mac_addr = ':'.join([mac[5],mac[4],mac[3],mac[2],mac[1],mac[0]])

    #print "-> ip_addr           :", readRegister(glib,"ip_addr")
    print "-> ip_addr           :%d.%d.%d.%d"%(readRegister(glib,"GLIB.SYSTEM.IP_INFO.B3"),
                                               readRegister(glib,"GLIB.SYSTEM.IP_INFO.B2"),
                                               readRegister(glib,"GLIB.SYSTEM.IP_INFO.B1"),
                                               readRegister(glib,"GLIB.SYSTEM.IP_INFO.B0"))
    
    print "-> hw_addr           :%02x.%02x.%02x.%02x.%02x.%02x"%(readRegister(glib,"GLIB.SYSTEM.HW_ID.B6"),
                                                                 readRegister(glib,"GLIB.SYSTEM.HW_ID.B5"),
                                                                 readRegister(glib,"GLIB.SYSTEM.HW_ID.B4"),
                                                                 readRegister(glib,"GLIB.SYSTEM.HW_ID.B3"),
                                                                 readRegister(glib,"GLIB.SYSTEM.HW_ID.B2"),
                                                                 readRegister(glib,"GLIB.SYSTEM.HW_ID.B1"))
    amc_slot = readRegister(glib,"GLIB.SYSTEM.STATUS.V6_CPLD") & 0x0f
    
    print "-> cpld bus state    :", "%02x"%(readRegister(glib,"GLIB.SYSTEM.STATUS.V6_CPLD"))
    print "-> v6_cpld  0x%x"%readRegister(glib,"GLIB.SYSTEM.STATUS.V6_CPLD")
    print
    if ((amc_slot>0) and (amc_slot<13)):
        print "-> amc slot #        :", amc_slot
    else:
        print "-> amc slot #        :",amc_slot,"[not in crate]"
    print "-> mac address (ipb) :", mac_addr
    print "-> mac IP source :0x%x"%readRegister(glib,"GLIB.SYSTEM.MAC.IP_SOURCE")
    pass

def getExtendedSystemInfo(glib):
    print
    print "-> -----------------"
    print "-> BOARD STATUS     "
    print "-> -----------------"
    print "-> sfp    absent   rxlos   txfault"
    print "-> sfp1:  %4d     %3d    %5d"%(readRegister(glib,"GLIB.SYSTEM.STATUS.SFP1.Mod_abs"),
                                          readRegister(glib,"GLIB.SYSTEM.STATUS.SFP1.RxLOS"),
                                          readRegister(glib,"GLIB.SYSTEM.STATUS.SFP1.TxFault"))
    print "-> sfp2:  %4d     %3d    %5d"%(readRegister(glib,"GLIB.SYSTEM.STATUS.SFP2.Mod_abs"),
                                          readRegister(glib,"GLIB.SYSTEM.STATUS.SFP2.RxLOS"),
                                          readRegister(glib,"GLIB.SYSTEM.STATUS.SFP2.TxFault"))
    print "-> sfp3:  %4d     %3d    %5d"%(readRegister(glib,"GLIB.SYSTEM.STATUS.SFP3.Mod_abs"),
                                          readRegister(glib,"GLIB.SYSTEM.STATUS.SFP3.RxLOS"),
                                          readRegister(glib,"GLIB.SYSTEM.STATUS.SFP3.TxFault"))
    print "-> sfp4:  %4d     %3d    %5d"%(readRegister(glib,"GLIB.SYSTEM.STATUS.SFP4.Mod_abs"),
                                          readRegister(glib,"GLIB.SYSTEM.STATUS.SFP4.RxLOS"),
                                          readRegister(glib,"GLIB.SYSTEM.STATUS.SFP4.TxFault"))
    
    print "-> ethphy interrupt  :", readRegister(glib,"GLIB.SYSTEM.STATUS.GBE_INT")
    
    print "-> fmc presence     fmc1    fmc2"
    print "->                  %4d     %3d"%(readRegister(glib,"GLIB.SYSTEM.STATUS.FMC1_PRESENT"),
                                             readRegister(glib,"GLIB.SYSTEM.STATUS.FMC2_PRESENT"))
    print "-> fpga reset state  :", readRegister(glib,"GLIB.SYSTEM.STATUS.FPGA_RESET")
    print "-> cdce locked       :", readRegister(glib,"GLIB.SYSTEM.STATUS.CDCE_LOCK")
    print
    getXpointInfo(glib)
    getSFPMonStatus(glib)

def getXpointInfo(glib):
    print "-> --------------------"
    print "-> XPOINT SWITCH STATUS"
    print "-> --------------------"
    print
    print "-> xpoint2 : S10 0x%x S11 0x%x"%(readRegister(glib,"GLIB.SYSTEM.CLK_CTRL.XPOINT2.S10"),
                                            readRegister(glib,"GLIB.SYSTEM.CLK_CTRL.XPOINT2.S11"))
    print "-> xpoint1 : S10 0x%x S11 0x%x"%(readRegister(glib,"GLIB.SYSTEM.CLK_CTRL.XPOINT1.S10"),
                                            readRegister(glib,"GLIB.SYSTEM.CLK_CTRL.XPOINT1.S11"))
                                          
    print "             S20 0x%x S11 0x%x"%(readRegister(glib,"GLIB.SYSTEM.CLK_CTRL.XPOINT1.S20"),
                                            readRegister(glib,"GLIB.SYSTEM.CLK_CTRL.XPOINT1.S21"))
                                            
    print "             S30 0x%x S31 0x%x"%(readRegister(glib,"GLIB.SYSTEM.CLK_CTRL.XPOINT1.S30"),
                                            readRegister(glib,"GLIB.SYSTEM.CLK_CTRL.XPOINT1.S31"))
                                            
    print "             S40 0x%x S41 0x%x"%(readRegister(glib,"GLIB.SYSTEM.CLK_CTRL.XPOINT1.S40"),
                                            readRegister(glib,"GLIB.SYSTEM.CLK_CTRL.XPOINT1.S41"))
    
    print

def getSFPMonStatus(glib):
    print "-> -----------------"
    print "-> SFP MON STATUS   "
    print "-> -----------------"
    print
    print "-> sfp_phase_mon_done  0x%x"%(  readRegister(glib,"GLIB.SYSTEM.STATUS.SFP_PHASE_MON.DONE" ))
    print "-> sfp_phase_mon_ok    0x%x"%(  readRegister(glib,"GLIB.SYSTEM.STATUS.SFP_PHASE_MON.OK"   ))
    print "-> sfp_phase_mon_ctrl  0x%08x"%(readRegister(glib,"GLIB.SYSTEM.SFP_PHASE_MON.CTRL"        ))
    print "-> sfp_phase_mon_lower 0x%08x"%(readRegister(glib,"GLIB.SYSTEM.SFP_PHASE_MON.CTRL.LOWER"  ))
    print "-> sfp_phase_mon_upper 0x%08x"%(readRegister(glib,"GLIB.SYSTEM.SFP_PHASE_MON.CTRL.UPPER"  ))
    print "-> sfp_phase_mon_stats 0x%08x"%(readRegister(glib,"GLIB.SYSTEM.SFP_PHASE_MON.STATS"       ))
    print
    print "-> -----------------"
    print "-> FMC MON STATUS   "
    print "-> -----------------"
    print
    print "-> fmc1_phase_mon_done  0x%x"%(  readRegister(glib,"GLIB.SYSTEM.STATUS.FMC1_PHASE_MON.DONE" ))
    print "-> fmc1_phase_mon_ok    0x%x"%(  readRegister(glib,"GLIB.SYSTEM.STATUS.FMC1_PHASE_MON.OK"   ))
    print "-> fmc1_phase_mon_ctrl  0x%08x"%(readRegister(glib,"GLIB.SYSTEM.FMC1_PHASE_MON.CTRL"         ))
    print "-> fmc1_phase_mon_lower 0x%08x"%(readRegister(glib,"GLIB.SYSTEM.FMC1_PHASE_MON.CTRL.LOWER"   ))
    print "-> fmc1_phase_mon_upper 0x%08x"%(readRegister(glib,"GLIB.SYSTEM.FMC1_PHASE_MON.CTRL.UPPER"   ))
    print "-> fmc1_phase_mon_stats 0x%08x"%(readRegister(glib,"GLIB.SYSTEM.FMC1_PHASE_MON.STATS"        ))
    print
    pass

def getSystemInfo(glib):
    getBasicSystemInfo(glib)
    getExtendedSystemInfo(glib)
