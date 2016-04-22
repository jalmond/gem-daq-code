from django.http import HttpResponse
from django.shortcuts import render
from ldqm_db.models import Run, AMC, GEB, VFAT, HWstate, SystemState
from django.views.generic import ListView, DetailView, CreateView
from django.core.urlresolvers import reverse_lazy
from django.contrib.auth.forms import UserCreationForm
from bugtracker.models import Ticket
from django.template import Template, Context
from django.contrib.staticfiles.templatetags.staticfiles import static
import os



import csv

slot_list = ['00','01','02','03','04','05','06','07','08','09','10','11',
             '12','13','14','15','16','17','18','19','20','21','22','23'];
lslot_list = ["a","b","c","d"];

vfat_address = []; #hex ID
BASE_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
csvfilename = os.path.join(BASE_DIR,'LightDQM/test/config/slot_table.csv')
with open(csvfilename, 'rd') as csvfile:
  vfat_ids = csv.reader(csvfile, delimiter=',')
  for num in vfat_ids:
      vfat_address.extend(num)

#NEEDS REDOING
hist_list = ["b1010",
             "b1100",
             "b1110",
             "BC",
             "EC",
             "Flag",
             "ChipID",
             "FiredChannels",
             "crc",
             "crc_calc",
             "latencyScan",
             "thresholdScan"];
amc13_hist_list = ["Control_Bit5",
                   "Control_BitA",
                   "Evt_ty",
                   "LV1_id",
                   "Bx_id",
                   "Source_id",
                   "CalTyp",
                   "nAMC",
                   "OrN",
                   "CRC_amc13",
                   "Blk_Not",
                   "LV1_idT",
                   "BX_idT",
                   "EvtLength",
                   "CRC_cdf"];
amc_hist_list = ["AMCnum",
                 "L1A",
                 "BX",
                 "Dlength",
                 "FV",
                 "Rtype",
                 "Param1",
                 "Param2",
                 "Param3",
                 "Onum",
                 "BID",
                 "GEMDAV",
                 "Bstatus",
                 "GDcount",
                 "Tstate",
                 "ChamT",
                 "OOSG",
                 "CRC",
                 "L1AT",
                 "DlengthT"];

geb_hist_list = ["Errors",
                 "InputID",
                 "OHCRC",
                 "Vwh",
                 "Vwt",
                 "Warnings",
                 "ZeroSup"];

sum_can_list  = ["integrity", "occupancy", "clusterMult", "clusterSize"];
hist_list_long = ['CRC', 
                  'Channels_fired_for_VFAT_chip_Slot15',
                  'CRC_2D_for_VFAT_chip_Slot13',
                  'Strips_fired_for_VFAT_chip_Slot14',
                  'Strips_fired_for_VFAT_chip_Slot6',
                  'CRC_2D_for_VFAT_chip_Slot8',
                  'VFAT_chip_Slot5_fired_per_event',
                  'VFAT_chip_Slot20_fired_per_event',
                  'CRC_2D_for_VFAT_chip_Slot2',
                  'Strips_fired_for_VFAT_chip_Slot3',
                  'Channels_fired_for_VFAT_chip_Slot5',
                  'CRC_2D_for_VFAT_chip_Slot20',
                  'Channels_fired_for_VFAT_chip_Slot4',
                  'VFAT_chip_Slot18_fired_per_event',
                  'CRC_2D_for_VFAT_chip_Slot11',
                  'VFAT_chip_Slot4_fired_per_event',
                  'Channels_fired_for_VFAT_chip_Slot17',
                  'Channels_fired_for_VFAT_chip_Slot8',
                  'Strips_fired_for_VFAT_chip_Slot17',
                  'CRC_2D_for_VFAT_chip_Slot19',
                  'Strips_fired_for_VFAT_chip_Slot22',
                  'CRC_2D_for_VFAT_chip_Slot10',
                  'Channels_fired_for_VFAT_chip_Slot14',
                  'Channels_fired_for_VFAT_chip_Slot3',
                  'Strips_fired_for_VFAT_chip_Slot4',
                  'Strips_fired_for_VFAT_chip_Slot23',
                  'VFAT_chip_Slot17_fired_per_event',
                  'Channels_fired_for_VFAT_chip_Slot16',
                  'Channels_fired_for_VFAT_chip_Slot18',
                  'VFAT_chip_Slot13_fired_per_event',
                  'Strips_fired_for_VFAT_chip_Slot21',
                  'Strips_fired_for_VFAT_chip_Slot2',
                  'Channels_fired_for_VFAT_chip_Slot9',
                  'Strips_fired_for_VFAT_chip_Slot20',
                  'Cluster_multiplicity',
                  'CRC_2D_for_VFAT_chip_Slot1',
                  'Channels_fired_for_VFAT_chip_Slot22',
                  'Strips_fired_for_VFAT_chip_Slot10',
                  'Strips_fired_for_VFAT_chip_Slot18',
                  'CRC_2D_for_VFAT_chip_Slot18',
                  'Channels_fired_for_VFAT_chip_Slot0',
                  'Number_of_bad_VFAT_blocks_in_event',
                  'VFAT_chip_Slot19_fired_per_event',
                  'Channels_fired_for_VFAT_chip_Slot6',
                  'CRC_calc_vs_CRC_VFAT',
                  'CRC_2D_for_VFAT_chip_Slot7',
                  'VFAT_chip_Slot1_fired_per_event',
                  'CRC_2D_for_VFAT_chip_Slot14',
                  'VFAT_chip_Slot2_fired_per_event',
                  'Channels_not_fired_per_event',
                  'ChipID','Flag',
                  'Control_Bits_1010',
                  'Strips_fired_for_VFAT_chip_Slot11',
                  'Cluster_size',
                  'CRC_2D_for_VFAT_chip_Slot23',
                  'Difference_of_BX_and_BC',
                  'VFAT_chip_Slot3_fired_per_event',
                  'Channels_fired_for_VFAT_chip_Slot2',
                  'CRC_2D_for_VFAT_chip_Slot9',
                  'Control_Bits_1100',
                  'Channels_fired_for_VFAT_chip_Slot10',
                  'CRC_2D_for_VFAT_chip_Slot16',
                  'Channels_fired_for_VFAT_chip_Slot12',
                  'VFAT_chip_Slot16_fired_per_event',
                  'CRC_Diff',
                  'Channels_fired_for_VFAT_chip_Slot1',
                  'VFAT_chip_Slot10_fired_per_event',
                  'VFAT_chip_Slot8_fired_per_event',
                  'Strips_fired_for_VFAT_chip_Slot16',
                  'VFAT_chip_Slot6_fired_per_event',
                  'VFAT_chip_Slot7_fired_per_event',
                  'Strips_fired_for_VFAT_chip_Slot15',
                  'Beam_Profile',
                  'CRC_2D_for_VFAT_chip_Slot12',
                  'CRC_2D_for_VFAT_chip_Slot22',
                  'Strips','VFAT_slot_number',
                  'Strips_fired_for_VFAT_chip_Slot13',
                  'Strips_fired_for_VFAT_chip_Slot1',
                  'Channels_fired_per_event',
                  'CRC_2D_for_VFAT_chip_Slot4',
                  'CRC_2D_for_VFAT_chip_Slot6',
                  'VFAT_chip_Slot14_fired_per_event',
                  'CRC_2D_for_VFAT_chip_Slot5',
                  'VFAT_chip_Slot22_fired_per_event',
                  'Channels_fired_for_VFAT_chip_Slot13',
                  'Strips_fired_for_VFAT_chip_Slot19',
                  'Strips_fired_for_VFAT_chip_Slot8',
                  'Channels_fired_for_VFAT_chip_Slot23',
                  'Strips_fired_for_VFAT_chip_Slot12',
                  'Ratio_of_BX_and_BC',
                  'CRC_2D_for_VFAT_chip_Slot17',
                  'Number_VFAT_blocks_per_event',
                  'VFAT_chip_Slot11_fired_per_event',
                  'CRC_2D_for_VFAT_chip_Slot21',
                  'VFAT_chip_Slot9_fired_per_event',
                  'VFAT_chip_Slot12_fired_per_event',
                  'Channels_fired_for_VFAT_chip_Slot21',
                  'Number_of_good_VFAT_blocks_in_event',
                  'Channels_fired_for_VFAT_chip_Slot7',
                  'CRC_2D_for_VFAT_chip_Slot3',
                  'VFAT_chip_Slot15_fired_per_event',
                  'Channels_fired_for_VFAT_chip_Slot19',
                  'Channels_fired_for_VFAT_chip_Slot20',
                  'VFAT_chip_Slot23_fired_per_event',
                  'Channels_fired_for_VFAT_chip_Slot11',
                  'Strips_fired_for_VFAT_chip_Slot0',
                  'Strips_fired_for_VFAT_chip_Slot7',
                  'Control_Bits_1110',
                  'VFAT_chip_Slot0_fired_per_event',
                  'VFAT_chip_Slot21_fired_per_event',
                  'Strips_fired_for_VFAT_chip_Slot9',
                  'CRC_2D_for_VFAT_chip_Slot15',
                  'CRC_2D_for_VFAT_chip_Slot0',
                  'Strips_fired_for_VFAT_chip_Slot5']

def dqm_help(request):
  return render(request,'test.html', {'hist_list_long':hist_list_long,})

def runs(request):
  run_list = Run.objects.all()
  return render(request,'runs.html', {'run_list':run_list,})

def main(request):
  run_list = Run.objects.all()
  return render(request,'main.html', {'run_list':run_list,})

def report(request, runType, runN):
  run_list = Run.objects.all()
  run = Run.objects.get(Type=runType, Number = runN)
  try:
    state = run.State
    amc_state = state.amcStates.all()
    geb_state = state.gebStates.all()
    vfat_state = state.vfatStates.all()
  except:
    print "Could not locate states for %s in Database" % chamber
  
  amc_color = []
  geb_color = []
  for i, amc in enumerate(run.amcs.all()):
    amc_color.insert(i,'default')
    geb_color.insert(i,['default','default'])
  for i, amc in enumerate(run.amcs.all()):
    try:
      code = int(next((x for x in amc_state if x.HWID==amc.BoardID),None).State)
      #print "AMC %s Code: " % amc.BoardID, code
      del amc_color[i]
      if code==0: amc_color.insert(i,'success')
      elif code==1: amc_color.insert(i,'warning')
      elif code==9: amc_color.insert(i,'default')
      elif code==3: amc_color.insert(i,'danger')
      else: amc_color.insert(i,'danger')
    except:
      print "Error locating AMC: ", amc.BoardID, amc.Type
    for j, geb in enumerate(amc.gebs.all()):
      try:
        code = int(next((x for x in geb_state if x.HWID==geb.ChamberID),None).State)
        #print "GEB %s Code: " % geb.BoardID, code
        del geb_color[i][j]
        if code==0: geb_color[i].insert(j,'success')
        elif code==1: geb_color[i].insert(j,'warning')
        elif code==9: geb_color[i].insert(j,'default')
        elif code==3: geb_color[i].insert(j,'danger')
        else: geb_color[i].insert(j,'danger')
      except:
        print "Error locating GEB: ", geb.ChamberID, geb.Type
  
  return render(request,'report.html', {'run_list':run_list,
                                        'hist_list':hist_list,
                                        'hist_list_long':hist_list_long,
                                        'run':run,
                                        'amc_color':amc_color,
                                        'geb_color':geb_color})


def chamber(request, runType, runN):
  run_list = Run.objects.all()
  run = Run.objects.get(Type=runType, Number = runN)
  try:
    state = run.State
    amc_state = state.amcStates.all()
    geb_state = state.gebStates.all()
    vfat_state = state.vfatStates.all()
  except:
    print "Could not locate states for %s in Database" % chamber
  amc_color = []
  geb_color = []
  for i, amc in enumerate(run.amcs.all()):
    amc_color.insert(i,'default')
    geb_color.insert(i,['default','default'])
  for i, amc in enumerate(run.amcs.all()):
    try:
      code = int(next((x for x in amc_state if x.HWID==amc.BoardID),None).State)
      #print "AMC %s Code: " % amc.BoardID, code
      del amc_color[i]
      if code==0: amc_color.insert(i,'success')
      elif code==1: amc_color.insert(i,'warning')
      elif code==9: amc_color.insert(i,'default')
      elif code==3: amc_color.insert(i,'danger')
      else: amc_color.insert(i,'danger')
    except:
      print "Error locating AMC: ", amc.BoardID, amc.Type
    for j, geb in enumerate(amc.gebs.all()):
      try:
        code = int(next((x for x in geb_state if x.HWID==geb.ChamberID),None).State)
        #print "GEB %s Code: " % geb.BoardID, code
        del geb_color[i][j]
        if code==0: geb_color[i].insert(j,'success')
        elif code==1: geb_color[i].insert(j,'warning')
        elif code==9: geb_color[i].insert(j,'default')
        elif code==3: geb_color[i].insert(j,'danger')
        else: geb_color[i].insert(j,'danger')
      except:
        print "Error locating GEB: ", geb.ChamberID, geb.Type
  return render(request,'chambers.html', {'run_list':run_list,
                                          'slot_list':slot_list,
                                          'hist_list':hist_list,
                                          'hist_list_long':hist_list_long,
                                          'run':run,
                                          'amc_color':amc_color,
                                          'geb_color':geb_color})

def amc_13(request, runType, runN):
  run_list = Run.objects.all()
  run = Run.objects.get(Type=runType, Number = runN)
  try:
    state = run.State
    amc_state = state.amcStates.all()
    geb_state = state.gebStates.all()
    vfat_state = state.vfatStates.all()
  except:
    print "Could not locate states for %s in Database" % run.Name
  amc_color = []
  geb_color = []
  for i, amc in enumerate(run.amcs.all()):
    amc_color.insert(i,'default')
    geb_color.insert(i,['default','default'])
  for i, amc in enumerate(run.amcs.all()):
    try:
      code = int(next((x for x in amc_state if x.HWID==amc.BoardID),None).State)
      #print "AMC %s Code: " % amc.BoardID, code
      del amc_color[i]
      if code==0: amc_color.insert(i,'success')
      elif code==1: amc_color.insert(i,'warning')
      elif code==9: amc_color.insert(i,'default')
      elif code==3: amc_color.insert(i,'danger')
      else: amc_color.insert(i,'danger')
    except:
      print "Error locating AMC: ", amc.BoardID, amc.Type
    for j, geb in enumerate(amc.gebs.all()):
      try:
        code = int(next((x for x in geb_state if x.HWID==geb.ChamberID),None).State)
        #print "GEB %s Code: " % geb.BoardID, code
        del geb_color[i][j]
        if code==0: geb_color[i].insert(j,'success')
        elif code==1: geb_color[i].insert(j,'warning')
        elif code==9: geb_color[i].insert(j,'default')
        elif code==3: geb_color[i].insert(j,'danger')
        else: geb_color[i].insert(j,'danger')
      except:
        print "Error locating GEB: ", geb.ChamberID, geb.Type  

  return render(request,'amc_13.html', {'run_list':run_list,
                                        'slot_list':slot_list,
                                        'hist_list':hist_list,
                                        'hist_list_long':hist_list_long,
                                        'amc13_hist_list':amc13_hist_list,
                                        'run':run,
                                        'sum_can_list':sum_can_list,
                                        'amc_color':amc_color,
                                        'geb_color':geb_color})

def display_amc_13(request, runType, runN, hist):
  run_list = Run.objects.all()
  run = Run.objects.get(Type=runType, Number = runN)
  try:
    state = run.State
    amc_state = state.amcStates.all()
    geb_state = state.gebStates.all()
    vfat_state = state.vfatStates.all()
  except:
    print "Could not locate states for %s in Database" % run.Name
  amc_color = []
  geb_color = []
  for i, amc in enumerate(run.amcs.all()):
    amc_color.insert(i,'default')
    geb_color.insert(i,['default','default'])
  for i, amc in enumerate(run.amcs.all()):
    try:
      code = int(next((x for x in amc_state if x.HWID==amc.BoardID),None).State)
      #print "AMC %s Code: " % amc.BoardID, code
      del amc_color[i]
      if code==0: amc_color.insert(i,'success')
      elif code==1: amc_color.insert(i,'warning')
      elif code==9: amc_color.insert(i,'default')
      elif code==3: amc_color.insert(i,'danger')
      else: amc_color.insert(i,'danger')
    except:
      print "Error locating AMC: ", amc.BoardID, amc.Type
    for j, geb in enumerate(amc.gebs.all()):
      try:
        code = int(next((x for x in geb_state if x.HWID==geb.ChamberID),None).State)
        #print "GEB %s Code: " % geb.BoardID, code
        del geb_color[i][j]
        if code==0: geb_color[i].insert(j,'success')
        elif code==1: geb_color[i].insert(j,'warning')
        elif code==9: geb_color[i].insert(j,'default')
        elif code==3: geb_color[i].insert(j,'danger')
        else: geb_color[i].insert(j,'danger')
      except:
        print "Error locating GEB: ", geb.ChamberID, geb.Type  

  return render(request,'display_amc_13.html', {'run_list':run_list,
                                        'slot_list':slot_list,
                                        'hist_list':hist_list,
                                        'hist_list_long':hist_list_long,
                                        'amc13_hist_list':amc13_hist_list,
                                        'hist':hist,
                                        'run':run,
                                        'sum_can_list':sum_can_list,
                                        'amc_color':amc_color,
                                        'geb_color':geb_color})

def amc(request, runType, runN, amc_boardid):
  run_list = Run.objects.all()
  run = Run.objects.get(Type=runType, Number = runN)
  try:
    state = run.State
    amc_state = state.amcStates.all()
    geb_state = state.gebStates.all()
    vfat_state = state.vfatStates.all()
  except:
    print "Could not locate states for %s in Database" % chamber
  amc_color = []
  geb_color = []
  for i, amc in enumerate(run.amcs.all()):
    amc_color.insert(i,'default')
    geb_color.insert(i,['default','default'])
  for i, amc in enumerate(run.amcs.all()):
    try:
      code = int(next((x for x in amc_state if x.HWID==amc.BoardID),None).State)
      #print "AMC %s Code: " % amc.BoardID, code
      del amc_color[i]
      if code==0: amc_color.insert(i,'success')
      elif code==1: amc_color.insert(i,'warning')
      elif code==9: amc_color.insert(i,'default')
      elif code==3: amc_color.insert(i,'danger')
      else: amc_color.insert(i,'danger')
    except:
      print "Error locating AMC: ", amc.BoardID, amc.Type
    for j, geb in enumerate(amc.gebs.all()):
      try:
        code = int(next((x for x in geb_state if x.HWID==geb.ChamberID),None).State)
        #print "GEB %s Code: " % geb.BoardID, code
        del geb_color[i][j]
        if code==0: geb_color[i].insert(j,'success')
        elif code==1: geb_color[i].insert(j,'warning')
        elif code==9: geb_color[i].insert(j,'default')
        elif code==3: geb_color[i].insert(j,'danger')
        else: geb_color[i].insert(j,'danger')
      except:
        print "Error locating GEB: ", geb.ChamberID, geb.Type  

  return render(request,'amc.html', {'run_list':run_list,
                                     'slot_list':slot_list,
                                     'hist_list':hist_list,
                                     'hist_list_long':hist_list_long,
                                     'amc_hist_list':amc_hist_list,
                                     'run':run,
                                     'amc_boardid':amc_boardid,
                                     'sum_can_list':sum_can_list,
                                     'amc_color':amc_color,
                                     'geb_color':geb_color})

def display_amc(request, runType, runN, amc_boardid, hist):
  run_list = Run.objects.all()
  run = Run.objects.get(Type=runType, Number = runN)
  try:
    state = run.State
    amc_state = state.amcStates.all()
    geb_state = state.gebStates.all()
    vfat_state = state.vfatStates.all()
  except:
    print "Could not locate states for %s in Database" % chamber
  amc_color = []
  geb_color = []
  for i, amc in enumerate(run.amcs.all()):
    amc_color.insert(i,'default')
    geb_color.insert(i,['default','default'])
  for i, amc in enumerate(run.amcs.all()):
    try:
      code = int(next((x for x in amc_state if x.HWID==amc.BoardID),None).State)
      #print "AMC %s Code: " % amc.BoardID, code
      del amc_color[i]
      if code==0: amc_color.insert(i,'success')
      elif code==1: amc_color.insert(i,'warning')
      elif code==9: amc_color.insert(i,'default')
      elif code==3: amc_color.insert(i,'danger')
      else: amc_color.insert(i,'danger')
    except:
      print "Error locating AMC: ", amc.BoardID, amc.Type
    for j, geb in enumerate(amc.gebs.all()):
      try:
        code = int(next((x for x in geb_state if x.HWID==geb.ChamberID),None).State)
        #print "GEB %s Code: " % geb.BoardID, code
        del geb_color[i][j]
        if code==0: geb_color[i].insert(j,'success')
        elif code==1: geb_color[i].insert(j,'warning')
        elif code==9: geb_color[i].insert(j,'default')
        elif code==3: geb_color[i].insert(j,'danger')
        else: geb_color[i].insert(j,'danger')
      except:
        print "Error locating GEB: ", geb.ChamberID, geb.Type  

  return render(request,'display_amc.html', {'run_list':run_list,
                                     'slot_list':slot_list,
                                     'hist_list':hist_list,
                                     'hist_list_long':hist_list_long,
                                     'amc_hist_list':amc_hist_list,
                                     'run':run,
                                     'hist':hist,
                                     'amc_boardid':amc_boardid,
                                     'sum_can_list':sum_can_list,
                                     'amc_color':amc_color,
                                     'geb_color':geb_color})



def gebs(request, runType, runN, amc_boardid, geb_chamberid):
  run_list = Run.objects.all()
  run = Run.objects.get(Type=runType, Number = runN)
  try:
    state = run.State
    amc_state = state.amcStates.all()
    geb_state = state.gebStates.all()
    vfat_state = state.vfatStates.all()
  except:
    print "Could not locate GEB states in Database";
  
  amc_color = []
  geb_color = []
  for i, amc in enumerate(run.amcs.all()):
    amc_color.insert(i,'default')
    geb_color.insert(i,['default','default'])
  for i, amc in enumerate(run.amcs.all()):
    try:
      code = int(next((x for x in amc_state if x.HWID==amc.BoardID),None).State)
      #print "AMC %s Code: " % amc.BoardID, code
      del amc_color[i]
      if code==0: amc_color.insert(i,'success')
      elif code==1: amc_color.insert(i,'warning')
      elif code==9: amc_color.insert(i,'default')
      elif code==3: amc_color.insert(i,'danger')
      else: amc_color.insert(i,'danger')
    except:
      print "Error locating AMC: ", amc.BoardID, amc.Type
    for j, geb in enumerate(amc.gebs.all()):
      try:
        code = int(next((x for x in geb_state if x.HWID==geb.ChamberID),None).State)
        #print "GEB %s Code: " % geb.BoardID, code
        del geb_color[i][j]
        if code==0: geb_color[i].insert(j,'success')
        elif code==1: geb_color[i].insert(j,'warning')
        elif code==9: geb_color[i].insert(j,'default')
        elif code==3: geb_color[i].insert(j,'danger')
        else: geb_color[i].insert(j,'danger')
      except:
        print "Error locating GEB: ", geb.ChamberID, geb.Type

  vfats = []
  for s in slot_list: #initialize vfats to work if no states in DB
    vfats.insert(int(s),[s, vfat_address[int(s)], 0, 'default', False])
  for s in slot_list:
    try:
      code = int(next((x for x in vfat_state if x.HWID==vfat_address[int(s)]),None).State)
      del vfats[int(s)]
      if code==0: vfats.insert(int(s),[s, vfat_address[int(s)], code, 'success', False])
      elif code==1: vfats.insert(int(s),[s, vfat_address[int(s)], code, 'warning', False])
      elif code==9: vfats.insert(int(s),[s, vfat_address[int(s)], code, 'default', True])
      elif code==3: vfats.insert(int(s),[s, vfat_address[int(s)], code, 'danger', False])
      else: vfats.insert(int(s),[s, vfat_address[int(s)], code, 'danger', False])
    except:
      print "Error locating vfat: ",vfat_address[int(s)]
  
  return render(request,'gebs.html', {'run_list':run_list,
                                      'slot_list':slot_list,
                                      'hist_list':hist_list,
                                      'hist_list_long':hist_list_long,
                                      'geb_hist_list':geb_hist_list,
                                      'run':run,
                                      'amc_boardid':amc_boardid,
                                      'geb_chamberid':geb_chamberid,
                                      'sum_can_list':sum_can_list,
                                      'amc_color':amc_color,
                                      'geb_color':geb_color,
                                      'vfats':vfats})

def display_geb(request, runType, runN, amc_boardid, geb_chamberid, hist):
  run_list = Run.objects.all()
  run = Run.objects.get(Type=runType, Number = runN)
  try:
    state = run.State
    amc_state = state.amcStates.all()
    geb_state = state.gebStates.all()
    vfat_state = state.vfatStates.all()
  except:
    print "Could not locate GEB states in Database";
  
  amc_color = []
  geb_color = []
  for i, amc in enumerate(run.amcs.all()):
    amc_color.insert(i,'default')
    geb_color.insert(i,['default','default'])
  for i, amc in enumerate(run.amcs.all()):
    try:
      code = int(next((x for x in amc_state if x.HWID==amc.BoardID),None).State)
      #print "AMC %s Code: " % amc.BoardID, code
      del amc_color[i]
      if code==0: amc_color.insert(i,'success')
      elif code==1: amc_color.insert(i,'warning')
      elif code==9: amc_color.insert(i,'default')
      elif code==3: amc_color.insert(i,'danger')
      else: amc_color.insert(i,'danger')
    except:
      print "Error locating AMC: ", amc.BoardID, amc.Type
    for j, geb in enumerate(amc.gebs.all()):
      try:
        code = int(next((x for x in geb_state if x.HWID==geb.ChamberID),None).State)
        #print "GEB %s Code: " % geb.BoardID, code
        del geb_color[i][j]
        if code==0: geb_color[i].insert(j,'success')
        elif code==1: geb_color[i].insert(j,'warning')
        elif code==9: geb_color[i].insert(j,'default')
        elif code==3: geb_color[i].insert(j,'danger')
        else: geb_color[i].insert(j,'danger')
      except:
        print "Error locating GEB: ", geb.ChamberID, geb.Type

  vfats = []
  for s in slot_list: #initialize vfats to work if no states in DB
    vfats.insert(int(s),[s, vfat_address[int(s)], 0, 'default', False])
  for s in slot_list:
    try:
      code = int(next((x for x in vfat_state if x.HWID==vfat_address[int(s)]),None).State)
      del vfats[int(s)]
      if code==0: vfats.insert(int(s),[s, vfat_address[int(s)], code, 'success', False])
      elif code==1: vfats.insert(int(s),[s, vfat_address[int(s)], code, 'warning', False])
      elif code==9: vfats.insert(int(s),[s, vfat_address[int(s)], code, 'default', True])
      elif code==3: vfats.insert(int(s),[s, vfat_address[int(s)], code, 'danger', False])
      else: vfats.insert(int(s),[s, vfat_address[int(s)], code, 'danger', False])
    except:
      print "Error locating vfat: ",vfat_address[int(s)]
  
  return render(request,'display_geb.html', {'run_list':run_list,
                                      'slot_list':slot_list,
                                      'hist_list':hist_list,
                                      'hist_list_long':hist_list_long,
                                      'geb_hist_list':geb_hist_list,
                                      'hist':hist,
                                      'run':run,
                                      'amc_boardid':amc_boardid,
                                      'geb_chamberid':geb_chamberid,
                                      'sum_can_list':sum_can_list,
                                      'amc_color':amc_color,
                                      'geb_color':geb_color,
                                      'vfats':vfats})


def vfats(request, runType, runN, amc_boardid, geb_chamberid, vfatN):
  run_list = Run.objects.all()
  run = Run.objects.get(Type=runType, Number = runN)
  try:
    state = run.State
    amc_state = state.amcStates.all()
    geb_state = state.gebStates.all()
    vfat_state = state.vfatStates.all()
  except:
    print "Could not locate states for %s in Database" % chamber

  amc_color = []
  geb_color = []
  for i, amc in enumerate(run.amcs.all()):
    amc_color.insert(i,'default')
    geb_color.insert(i,['default','default'])
  print "GEB_Color[0][1] = ",geb_color[0][1]
  for i, amc in enumerate(run.amcs.all()):
    try:
      code = int(next((x for x in amc_state if x.HWID==amc.BoardID),None).State)
      #print "AMC %s Code: " % amc.BoardID, code
      del amc_color[i]
      if code==0: amc_color.insert(i,'success')
      elif code==1: amc_color.insert(i,'warning')
      elif code==9: amc_color.insert(i,'default')
      elif code==3: amc_color.insert(i,'danger')
      else: amc_color.insert(i,'danger')
    except:
      print "Error locating state for AMC: ", amc.BoardID, amc.Type
    for j, geb in enumerate(amc.gebs.all()):
      try:
        code = int(next((x for x in geb_state if x.HWID==geb.ChamberID),None).State)
        #print "GEB %s Code: " % geb.BoardID, code
        del geb_color[i][j]
        if code==0: geb_color[i].insert(j,'success')
        elif code==1: geb_color[i].insert(j,'warning')
        elif code==9: geb_color[i].insert(j,'default')
        elif code==3: geb_color[i].insert(j,'danger')
        else: geb_color[i].insert(j,'danger')
      except:
        print "Error locating state for GEB: ", geb.ChamberID, geb.Type

  vfats = []
  for s in slot_list: #initialize vfats to work if no states in DB
    vfats.insert(int(s),[s, vfat_address[int(s)], 0, 'default', False])
  for s in slot_list:
    try:
      code = int(next((x for x in vfat_state if x.HWID==vfat_address[int(s)]),None).State)
      if code==0:
        del vfats[int(s)]
        vfats.insert(int(s),[s, vfat_address[int(s)], code, 'success', False])
      elif code==1:
        del vfats[int(s)]
        vfats.insert(int(s),[s, vfat_address[int(s)], code, 'warning', False])
      elif code==9:
        del vfats[int(s)]
        vfats.insert(int(s),[s, vfat_address[int(s)], code, 'default', True])
      elif code==3:
        del vfats[int(s)]
        vfats.insert(int(s),[s, vfat_address[int(s)], code, 'danger', False])
      else:
        del vfats[int(s)]
        vfats.insert(int(s),[s, vfat_address[int(s)], code, 'danger', False])
    except:
      print "Error locating state for vfat: ",vfat_address[int(s)]
  selected_vfat = vfats[int(vfatN)]
  return render(request,'vfats.html', {'run_list':run_list,
                                       'slot_list':slot_list,
                                       'hist_list':hist_list,
                                       'hist_list_long':hist_list_long,
                                       'geb_hist_list':geb_hist_list,
                                       'run':run,
                                       'amc_boardid':amc_boardid,
                                       'geb_chamberid':geb_chamberid,
                                       'vfat':int(vfatN),
                                       'sum_can_list':sum_can_list,
                                       'vfats':vfats,
                                       'selected_vfat':selected_vfat,
                                       'amc_color':amc_color,
                                       'geb_color':geb_color})

def summary(request, runType, runN, chamber, summaryN):
  run_list = Run.objects.all()
  run = Run.objects.get(Type=runType, Number = runN)
  try:
    state = run.State
    amc_state = state.amcStates.all()
    geb_state = state.gebStates.all()
    vfat_state = state.vfatStates.all()
  except:
    print "Could not locate states for %s in Database" % geb_chamberid

  amc_color = []
  geb_color = []
  for i, amc in enumerate(run.amcs.all()):
    amc_color.insert(i,'default')
    geb_color.insert(i,['default','default'])
  for i, amc in enumerate(run.amcs.all()):
    try:
      code = int(next((x for x in amc_state if x.HWID==amc.BoardID),None).State)
      #print "AMC %s Code: " % amc.BoardID, code
      del amc_color[i]
      if code==0: amc_color.insert(i,'success')
      elif code==1: amc_color.insert(i,'warning')
      elif code==9: amc_color.insert(i,'default')
      elif code==3: amc_color.insert(i,'danger')
      else: amc_color.insert(i,'danger')
    except:
      print "Error locating AMC: ", amc.BoardID, amc.Type
    for j, geb in enumerate(amc.gebs.all()):
      try:
        code = int(next((x for x in geb_state if x.HWID==geb.ChamberID),None).State)
        #print "GEB %s Code: " % geb.BoardID, code
        del geb_color[i][j]
        if code==0: geb_color[i].insert(j,'success')
        elif code==1: geb_color[i].insert(j,'warning')
        elif code==9: geb_color[i].insert(j,'default')
        elif code==3: geb_color[i].insert(j,'danger')
        else: geb_color[i].insert(j,'danger')
      except:
        print "Error locating GEB: ", geb.ChamberID, geb.Type

 

  return render(request,'summary.html', {'run_list':run_list,
                                         'slot_list':slot_list,
                                         'hist_list':hist_list,
                                         'hist_list_long':hist_list_long,
                                         'geb_hist_list':geb_hist_list,
                                         'run':run,
                                         'chamber':chamber,
                                         'sum_can_list':sum_can_list,
                                         'sumN':summaryN,
                                         'amc_color':amc_color,
                                         'geb_color':geb_color})

def display_vfat(request, runType, runN, amc_boardid, geb_chamberid, vfatN, histN):
  run_list = Run.objects.all()
  run = Run.objects.get(Type=runType, Number = runN)  
  try:
    state = run.State
    amc_state = state.amcStates.all()
    geb_state = state.gebStates.all()
    vfat_state = state.vfatStates.all()
  except:
    print "Could not locate states for %s in Database" % geb_chamberid
  amc_color = []
  geb_color = []
  for i, amc in enumerate(run.amcs.all()):
    amc_color.insert(i,'default')
    geb_color.insert(i,['default','default'])
  for i, amc in enumerate(run.amcs.all()):
    try:
      code = int(next((x for x in amc_state if x.HWID==amc.BoardID),None).State)
      #print "AMC %s Code: " % amc.BoardID, code
      del amc_color[i]
      if code==0: amc_color.insert(i,'success')
      elif code==1: amc_color.insert(i,'warning')
      elif code==9: amc_color.insert(i,'default')
      elif code==3: amc_color.insert(i,'danger')
      else: amc_color.insert(i,'danger')
    except:
      print "Error locating AMC: ", amc.BoardID, amc.Type
    for j, geb in enumerate(amc.gebs.all()):
      try:
        code = int(next((x for x in geb_state if x.HWID==geb.ChamberID),None).State)
        #print "GEB %s Code: " % geb.BoardID, code
        del geb_color[i][j]
        if code==0: geb_color[i].insert(j,'success')
        elif code==1: geb_color[i].insert(j,'warning')
        elif code==9: geb_color[i].insert(j,'default')
        elif code==3: geb_color[i].insert(j,'danger')
        else: geb_color[i].insert(j,'danger')
      except:
        print "Error locating GEB: ", geb.ChamberID, geb.Type


  vfats = []
  for s in slot_list: #initialize vfats to work if no states in DB
    vfats.insert(int(s),[s, vfat_address[int(s)], 0, 'default', False])
  #vfats['slot', hex address, state code, color code, disabled]
  for s in slot_list:
    try:
      code = int(next((x for x in vfat_state if x.HWID==vfat_address[int(s)]),None).State)
      if code==0:
        del vfats[int(s)]
        vfats.insert(int(s),[s, vfat_address[int(s)], code, 'success', False])
      elif code==1:
        del vfats[int(s)]
        vfats.insert(int(s),[s, vfat_address[int(s)], code, 'warning', False])
      elif code==9:
        del vfats[int(s)]
        vfats.insert(int(s),[s, vfat_address[int(s)], code, 'default', True])
      elif code==3:
        del vfats[int(s)]
        vfats.insert(int(s),[s, vfat_address[int(s)], code, 'danger', False])
      else:
        del vfats[int(s)]
        vfats.insert(int(s),[s, vfat_address[int(s)], code, 'danger', False])
    except:
      print "Error locating vfat: ",vfat_address[int(s)]
  
  selected_vfat = vfats[int(vfatN)]

  return render(request,'display_vfat.html', {'run_list':run_list,
                                              'slot_list':slot_list,
                                              'hist_list':hist_list,
                                              'hist_list_long':hist_list_long,
                                              'geb_hist_list':geb_hist_list,
                                              'run':run,
                                              'amc_boardid':amc_boardid,
                                              'geb_chamberid':geb_chamberid,
                                              'sum_can_list':sum_can_list,
                                              'vfat':int(vfatN),
                                              'hist':histN,
                                              'vfats':vfats,
                                              'selected_vfat':selected_vfat,
                                              'amc_color':amc_color,
                                              'geb_color':geb_color})


def display_canvas(request, runType, runN, amc_boardid, geb_chamberid, canvas):
  run_list = Run.objects.all()
  run = Run.objects.get(Type=runType, Number = runN)
  try:
    state = run.State
    amc_state = state.amcStates.all()
    geb_state = state.gebStates.all()
    vfat_state = state.vfatStates.all()
  except:
    print "Could not locate states for %s in Database" % chamber


  amc_color = []
  geb_color = []
  for i, amc in enumerate(run.amcs.all()):
    amc_color.insert(i,'default')
    geb_color.insert(i,['default','default'])
  for i, amc in enumerate(run.amcs.all()):
    try:
      code = int(next((x for x in amc_state if x.HWID==amc.BoardID),None).State)
      #print "AMC %s Code: " % amc.BoardID, code
      del amc_color[i]
      if code==0: amc_color.insert(i,'success')
      elif code==1: amc_color.insert(i,'warning')
      elif code==9: amc_color.insert(i,'default')
      elif code==3: amc_color.insert(i,'danger')
      else: amc_color.insert(i,'danger')
    except:
      print "Error locating AMC: ", amc.BoardID, amc.Type
    for j, geb in enumerate(amc.gebs.all()):
      try:
        code = int(next((x for x in geb_state if x.HWID==geb.ChamberID),None).State)
        #print "GEB %s Code: " % geb.BoardID, code
        del geb_color[i][j]
        if code==0: geb_color[i].insert(j,'success')
        elif code==1: geb_color[i].insert(j,'warning')
        elif code==9: geb_color[i].insert(j,'default')
        elif code==3: geb_color[i].insert(j,'danger')
        else: geb_color[i].insert(j,'danger')
      except:
        print "Error locating GEB: ", geb.ChamberID, geb.Type



  vfats = []
  for s in slot_list: #initialize vfats to work if no states in DB
    vfats.insert(int(s),[s, vfat_address[int(s)], 0, 'default', False])
  for s in slot_list:
    try:
      code = int(next((x for x in vfat_state if x.HWID==vfat_address[int(s)]),None).State)
      if code==0:
        del vfats[int(s)]
        vfats.insert(int(s),[s, vfat_address[int(s)], code, 'success', False])
      elif code==1:
        del vfats[int(s)]
        vfats.insert(int(s),[s, vfat_address[int(s)], code, 'warning', False])
      elif code==9:
        del vfats[int(s)]
        vfats.insert(int(s),[s, vfat_address[int(s)], code, 'default', True])
      elif code==3:
        del vfats[int(s)]
        vfats.insert(int(s),[s, vfat_address[int(s)], code, 'danger', False])
      else:
        del vfats[int(s)]
        vfats.insert(int(s),[s, vfat_address[int(s)], code, 'danger', False])
    except:
      print "Error locating vfat: ",vfat_address[int(s)]
  
  return render(request,'display_canvas.html', {'run_list':run_list,
                                                'slot_list':slot_list,
                                                'hist_list':hist_list,
                                                'hist_list_long':hist_list_long,
                                                'geb_hist_list':geb_hist_list,
                                                'run':run,
                                                'amc_boardid':amc_boardid,
                                                'geb_chamberid':geb_chamberid,
                                                'chamber':chamber,
                                                'sum_can_list':sum_can_list,
                                                'canvas':canvas,
                                                'vfats':vfats,
                                                'amc_color':amc_color,
                                                'geb_color':geb_color})


class BugListView(ListView):
    model = Ticket
    template_name = 'list.html'

class BugDetailView(DetailView):
    model = Ticket
    template_name = 'detail.html'

class RegisterView(CreateView):
    form_class = UserCreationForm
    template_name = 'register.html'
    success_url = reverse_lazy('index')

class BugCreateView(CreateView):
    model = Ticket
    template_name = 'add.html'
    fields = ['title', 'text']
    success_url = reverse_lazy('index')

def form_valid(self, form):
        form.instance.user = self.request.user
        return super(BugCreateView, self).form_valid(form)

