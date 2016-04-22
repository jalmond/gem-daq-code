from registers_uhal import *
from glib_system_info_uhal import *
from glib_user_functions_uhal import *
from vfat_functions_uhal import *

class AMCmanager:
  def __init__(self):
    pass

  def connect(self,sn):
    self.uTCAslot = 160+int(sn)
    self.ipaddr= '192.168.0.%d'%(self.uTCAslot)
    self.address_table = "file://${GEM_ADDRESS_TABLE_PATH}/glib_address_table.xml"
    self.uri = "chtcp-2.0://localhost:10203?target=%s:50001"%(self.ipaddr)
    self.glib  = uhal.getDevice( "glib" , self.uri, self.address_table )
    #check if glib is really connected
    fwv = readRegister(self.glib,"GLIB.SYSTEM.FIRMWARE")
    if fwv == 0x0:
      raise ValueError('AMC %s is missing!' %(sn))

  def reset(self):
    writeRegister(self.glib,"GLIB.DAQ.CONTROL", 0x8)
    writeRegister(self.glib,"GLIB.DAQ.CONTROL.DAQ_LINK_RESET",0x1)
    writeRegister(self.glib,"GLIB.DAQ.CONTROL.DAQ_LINK_RESET",0x0)

  def activateGTX(self):
    c = 0
    for l in (0,1):#(0,1): currently hack to have only single OH connected twice
      fwv = readRegister(self.glib,"GLIB.OptoHybrid_%d.OptoHybrid.STATUS.FW"%(l))
      if fwv != 0x0:
        c += 1
    if c == 0:
      raise ValueError('No GTX connection present!')
    elif c == 1:
      writeRegister(self.glib, "GLIB.DAQ.CONTROL", 0x181)# enable GTX link 0
    elif c == 2:
      writeRegister(self.glib, "GLIB.DAQ.CONTROL", 0x381)# enable both GTX links
    return c

  def getVFATs(self,gtx):
    return getAllChipIDs(self.glib,gtx)

  def getStatus(self, verbosity):
    #DAQ REGISTERS TABLE
    self.status = '<table border="1"><tr>\n<th class="name" colspan="2">DAQ REGISTERS</th>\n</tr>'
    self.status += '<tr><th>CONTROL</th><td>0x%08X</td></tr>' % (readRegister(self.glib,"GLIB.DAQ.CONTROL"))
    self.status += '<tr><th>STATUS</th><td>0x%08X</td></tr>' % (readRegister(self.glib,"GLIB.DAQ.STATUS"))
    self.status += '<tr><th>GTX NOT IN TABLE ERR COUNTER</th><td>0x%08X</td></tr>' % (readRegister(self.glib,"GLIB.DAQ.EXT_STATUS.NOTINTABLE_ERR"))
    self.status += '<tr><th>GTX DISPERSION ERR COUNTER</th><td>0x%08X</td></tr>' % (readRegister(self.glib,"GLIB.DAQ.EXT_STATUS.DISPER_ERR"))
    self.status += '<tr><th>L1A ID</th><td>0x%08X</td></tr>' % (readRegister(self.glib,"GLIB.DAQ.EXT_STATUS.L1AID"))
    self.status += '<tr><th>SENT EVENTS COUNTER</th><td>0x%08X</td></tr>' % (readRegister(self.glib,"GLIB.DAQ.EXT_STATUS.EVT_SENT"))
    self.status += '<tr><th>DAV TIMEOUT</th><td>0x%08X</td></tr>' % (readRegister(self.glib,"GLIB.DAQ.CONTROL.DAV_TIMEOUT"))
    self.status += '<tr><th>RUN TYPE</th><td>0x%08X</td></tr>' % (readRegister(self.glib,"GLIB.DAQ.EXT_CONTROL.RUN_TYPE"))
    self.status += '<tr><th>RUN PARAMS</th><td>0x%08X</td></tr>' % (readRegister(self.glib,"GLIB.DAQ.EXT_CONTROL.RUN_PARAMS"))
    self.status += '<tr><th>MAX DAV TIMER</th><td>0x%08X</td></tr>' % (readRegister(self.glib,"GLIB.DAQ.EXT_STATUS.MAX_DAV_TIMER"))
    self.status += '<tr><th>LAST DAV TIMER</th><td>0x%08X</td></tr>' % (readRegister(self.glib,"GLIB.DAQ.EXT_STATUS.LAST_DAV_TIMER"))
    self. status += '</table>'
    #LINK STATUS TABLE
    self.status += '<table border="1"><tr>\n<th class="name" colspan="3">LINK STATUS</th>\n</tr>'
    self.status += '<tr><th></th><th>GTX0</th><th>GTX1</th></tr>'
    self.status += '<tr><th>CORRUPTED VFAT BLOCKS COUNTER</th><td>0x%08X</td><td>0x%08X</td></tr>' % (readRegister(self.glib,"GLIB.DAQ.GTX0.COUNTERS.CORRUPT_VFAT_BLK_CNT"),readRegister(self.glib,"GLIB.DAQ.GTX1.COUNTERS.CORRUPT_VFAT_BLK_CNT"))
    self.status += '<tr><th>EVENT COUNTER</th><td>0x%08X</td><td>0x%08X</td></tr>' % (readRegister(self.glib,"GLIB.DAQ.GTX0.COUNTERS.EVN"),readRegister(self.glib,"GLIB.DAQ.GTX1.COUNTERS.EVN"))
    self.status += '<tr><th>STATUS</th><td>0x%08X</td><td>0x%08X</td></tr>' % (readRegister(self.glib,"GLIB.DAQ.GTX0.STATUS"),readRegister(self.glib,"GLIB.DAQ.GTX1.STATUS"))
    self.status += '<tr><th>MAX DAV TIMER</th><td>0x%08X</td><td>0x%08X</td></tr>' % (readRegister(self.glib,"GLIB.DAQ.GTX0.DAV_STATS.MAX_DAV_TIMER"),readRegister(self.glib,"GLIB.DAQ.GTX1.DAV_STATS.MAX_DAV_TIMER"))
    self.status += '<tr><th>LAST DAV TIMER</th><td>0x%08X</td><td>0x%08X</td></tr>' % (readRegister(self.glib,"GLIB.DAQ.GTX0.DAV_STATS.MAX_DAV_TIMER"),readRegister(self.glib,"GLIB.DAQ.GTX1.DAV_STATS.MAX_DAV_TIMER"))
    self.status += '<tr><th>DAV TIMEOUT</th><td>0x%08X</td><td>0x%08X</td></tr>' % (readRegister(self.glib,"GLIB.DAQ.GTX0.DAV_TIMEOUT"),readRegister(self.glib,"GLIB.DAQ.GTX1.DAV_TIMEOUT"))
    self. status += '</table>'
    #DEBUG TABLE
    if verbosity>1:
      self.status += '<table border="1"><tr>\n<th class="name" colspan="3">DAQ_DEBUG</th>\n</tr>'
      self.status += '<tr><th></th><th>GTX0</th><th>GTX1</th></tr>'
      self.status += '<tr><th>DEBUG 0</th><td>0x%08X</td><td>0x%08X</td></tr>' % (readRegister(self.glib,"GLIB.DAQ.GTX0.LASTBLOCK.0"),readRegister(self.glib,"GLIB.DAQ.GTX1.LASTBLOCK.0"))
      self.status += '<tr><th>DEBUG 1</th><td>0x%08X</td><td>0x%08X</td></tr>' % (readRegister(self.glib,"GLIB.DAQ.GTX0.LASTBLOCK.1"),readRegister(self.glib,"GLIB.DAQ.GTX1.LASTBLOCK.1"))
      self.status += '<tr><th>DEBUG 2</th><td>0x%08X</td><td>0x%08X</td></tr>' % (readRegister(self.glib,"GLIB.DAQ.GTX0.LASTBLOCK.2"),readRegister(self.glib,"GLIB.DAQ.GTX1.LASTBLOCK.2"))
      self.status += '<tr><th>DEBUG 3</th><td>0x%08X</td><td>0x%08X</td></tr>' % (readRegister(self.glib,"GLIB.DAQ.GTX0.LASTBLOCK.3"),readRegister(self.glib,"GLIB.DAQ.GTX1.LASTBLOCK.3"))
      self.status += '<tr><th>DEBUG 4</th><td>0x%08X</td><td>0x%08X</td></tr>' % (readRegister(self.glib,"GLIB.DAQ.GTX0.LASTBLOCK.4"),readRegister(self.glib,"GLIB.DAQ.GTX1.LASTBLOCK.4"))
      self.status += '<tr><th>DEBUG 5</th><td>0x%08X</td><td>0x%08X</td></tr>' % (readRegister(self.glib,"GLIB.DAQ.GTX0.LASTBLOCK.5"),readRegister(self.glib,"GLIB.DAQ.GTX1.LASTBLOCK.5"))
      self.status += '<tr><th>DEBUG 6</th><td>0x%08X</td><td>0x%08X</td></tr>' % (readRegister(self.glib,"GLIB.DAQ.GTX0.LASTBLOCK.6"),readRegister(self.glib,"GLIB.DAQ.GTX1.LASTBLOCK.6"))
      self. status += '</table>'

    #SBITS TABLE
    self.status += '<table border="1"><tr>\n<th class="name" colspan="3">SBITS</th>\n</tr>'
    self.status += '<tr><th></th><th>GTX0</th><th>GTX1</th></tr>'
    self.status += '<tr><th>CLUSTERS 01</th><td>0x%08X</td><td>0x%08X</td></tr>' % (readRegister(self.glib,"GLIB.DAQ.GTX0_CLUSTER_01"),readRegister(self.glib,"GLIB.DAQ.GTX1_CLUSTER_01"))
    self.status += '<tr><th>CLUSTERS 23</th><td>0x%08X</td><td>0x%08X</td></tr>' % (readRegister(self.glib,"GLIB.DAQ.GTX0_CLUSTER_23"),readRegister(self.glib,"GLIB.DAQ.GTX1_CLUSTER_23"))
    self.status += '<tr><th>SBITS RATE</th><td colspan="2">0x%08X</td></tr>' % (readRegister(self.glib,"GLIB.DAQ.SBIT_RATE"))
    self. status += '</table>'

    return self.status

