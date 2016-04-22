#!/bin/env python

import uhal
import amc13
import sys
import struct
from helper import OutputGrabber

class AMC13manager:
  def __init__(self):
    pass

  def connect(self,sn,verbosity):
    self.connection = "connection"+str(sn)+".xml" # get connection file
    self.verbosity = verbosity
    self.device = amc13.AMC13(self.connection) # connect to amc13
    #reset amc13
    self.device.AMCInputEnable(0x0)
    self.reset()

  def reset(self):
    self.device.reset(self.device.Board.T1)
    self.device.reset(self.device.Board.T2)
    self.device.resetCounters()
    self.device.resetDAQ()

  def configureInputs(self, inlist):
    mask = self.device.parseInputEnableList(inlist, True)
    self.device.AMCInputEnable(mask)

  def configureTrigger(self, ena, mode = 2, burst = 1, rate = 10, rules = 0):
    self.localTrigger = ena
    if self.localTrigger:
      self.device.configureLocalL1A(ena, mode, burst, rate, rules)

  #def startDataTaking(self, ofile, nevents):
  def startDataTaking(self, ofile):
    if self.localTrigger:
      self.device.startContinuousL1A()
    #submit work loop here
    self.isRunning = True
    c = 1
    #with open (ofile, "wb") as compdata:
    datastring = ''
    packer = struct.Struct('Q')
    pEvt = []
    read_event = self.device.readEvent
    while self.isRunning:
      nevt = self.device.read(self.device.Board.T1, 'STATUS.MONITOR_BUFFER.UNREAD_BLOCKS')
      #print "Trying to read %s events" % nevt
      for i in range(nevt):
        pEvt += read_event()
      #status = self.device.getStatus()
      #status.SetHTML()
      ## Create pipe and dup2() the write end of it on top of stdout, saving a copy
      ## of the old stdout
      #out = OutputGrabber()
      #out.start()
      #status.Report(self.verbosity)
      #out.stop()
      #shtml = out.capturedtext
      #with open("webdaq/templates/amc13status.html", "w") as text_file:
      #  text_file.write(shtml)
    for word in pEvt:
      datastring += packer.pack(word)
    with open (ofile, "wb") as compdata:
      compdata.write(datastring)

  def stopDataTaking(self):
    if self.localTrigger:
      self.device.stopContinuousL1A()
    #submit work loop here
    self.isRunning = False
