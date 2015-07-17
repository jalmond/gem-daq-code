import sys, os, time, signal, random
sys.path.append('/opt/gemdaq/firmware/testing/src')

from PyChipsUser import *
from registers import *

def biasVFAT(glib,chip):
    writeRegister(glib,"vfat2_%s_lat"%(        chip),  15)
    writeRegister(glib,"vfat2_%s_ipreampin"%(  chip), 168)
    writeRegister(glib,"vfat2_%s_ipreampfeed"%(chip), 150)
    writeRegister(glib,"vfat2_%s_ipreampout"%( chip),  80)
    writeRegister(glib,"vfat2_%s_ishaper"%(    chip), 150)
    writeRegister(glib,"vfat2_%s_ishaperfeed"%(chip), 100)
    writeRegister(glib,"vfat2_%s_icomp"%(      chip),  75)
    writeRegister(glib,"vfat2_%s_channel1"%(   chip),0x40)
    writeRegister(glib,"vfat2_%s_channel2"%(   chip),0x40)
    writeRegister(glib,"vfat2_%s_channel10"%(  chip),0x40)
    writeRegister(glib,"vfat2_%s_vcal"%(       chip), 100)
    writeRegister(glib,"vfat2_%s_vthreshold1"%(chip),  40)
    writeRegister(glib,"vfat2_%s_vthreshold2"%(chip),   0)
    writeRegister(glib,"vfat2_%s_calphase"%(   chip),   0)
