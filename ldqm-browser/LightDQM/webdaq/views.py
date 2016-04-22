from django.shortcuts import render
from webdaq.forms import *
from ldqm_db.models import *
import uhal
import amc13
import sys
import io
import os
import threading
from cStringIO import StringIO
import struct
from amc13manager import AMC13manager
from helper import OutputGrabber
from registers_uhal import *
from glib_system_info_uhal import *
from glib_user_functions_uhal import *
from amcmanager import AMCmanager
import datetime
from subprocess import call
import threading
from time import sleep

state = 'halted'
m_AMC13manager = AMC13manager()
m_AMCmanager = AMCmanager()
verbosity = 1
m_filename = "test"
form = ConfigForm()
m_monitor = False

def gemsupervisor(request):
  global state
  global m_AMC13manager
  global m_AMCmanager
  global verbosity
  global m_filename
  global form
  global m_monitor
  global lt

  def updateStatus():
    status = m_AMC13manager.device.getStatus()
    status.SetHTML()
    # Create pipe and dup2() the write end of it on top of stdout, saving a copy
    # of the old stdout
    out = OutputGrabber()
    out.start()
    status.Report(verbosity)
    out.stop()
    shtml = out.capturedtext
    with open("webdaq/templates/amc13status.html", "w") as text_file:
      text_file.write(shtml)
    with open("webdaq/templates/amcstatus.html", "w") as text_file:
        text_file.write(m_AMCmanager.getStatus(verbosity))

  def parkData():
#call root converter
    call_command =  os.getenv('BUILD_HOME')+'/gem-light-dqm/gemtreewriter/bin/'+os.getenv('XDAQ_OS')+'/'+os.getenv('XDAQ_PLATFORM')+'/unpacker'
    command_args = "/tmp/"+m_filename+".dat sdram"
    call([call_command+' '+command_args],shell=True)
#create dirs in tmp
    for i in range (24):
      call(["mkdir -p /tmp/dqm_hists/%s"%(i)],shell=True)
    call(["mkdir -p /tmp/dqm_hists/OtherData"],shell=True)
    call(["mkdir -p /tmp/dqm_hists/canvases"],shell=True)
#call dqm
    call_command =  os.getenv('BUILD_HOME')+'/gem-light-dqm/dqm-root/bin/'+os.getenv('XDAQ_OS')+'/'+os.getenv('XDAQ_PLATFORM')+'/dqm'
    command_args = "/tmp/"+m_filename+".raw.root"
    os.system(call_command+' '+command_args)
#call dqm printer
    call_command =  os.getenv('BUILD_HOME')+'/gem-light-dqm/dqm-root/bin/'+os.getenv('XDAQ_OS')+'/'+os.getenv('XDAQ_PLATFORM')+'/gtprinter'
    command_args = "/tmp/"+m_filename+".analyzed.root"
    os.system(call_command+' '+command_args)
#copy results to DQM display form
    #call_command = "/home/mdalchen/work/ldqm-browser/LightDQM/LightDQM/test/"
    call_command = os.getenv('LDQM_STATIC')+'/'
    # call_command += m_filename[10:15]
    # call_command += "/"
    # call_command += m_filename[:9]
    # call_command += "/"
    # call_command += "TAMU/GEB-GTX0-Long/"
    call(["mkdir -p "+call_command],shell=True)
    call(["cp -r /tmp/"+m_filename+" "+call_command],shell=True)

  if request.POST:
    if 'configure' in request.POST:
      form = ConfigForm(request.POST)
      if form.is_valid():
        amc13N = form.cleaned_data['amc13_choice']
        amc_list = form.cleaned_data['amc_list']
        amc_str = ""
        for amcN in amc_list:
          amc_str += str(amcN) + ","
        amc_str = amc_str[:-1]
        trigger_type = form.cleaned_data['trigger_type']
        if trigger_type == '1':
          lt=True
          m_monitor = True
        else:
          lt=False
          m_monitor = False
        trigger_rate = int(form.cleaned_data['trigger_rate'])
        verbosity = int(form.cleaned_data['verbosity'])
        uhal.setLogLevelTo(uhal.LogLevel.ERROR)
        #configure GLIB. Currently supports only one GLIB
        try:
          m_AMC13manager.connect(str(amc13N),verbosity)
          m_AMC13manager.configureInputs(amc_str)
          m_AMC13manager.reset()
          for amcN in amc_list:
            m_AMCmanager.connect(int(amcN))
            m_AMCmanager.reset()
            n_gtx = m_AMCmanager.activateGTX()
            print "N GTX links %s" %(n_gtx)
            # retrieve VFAT slot numberd and ChipIDs from HW
            for gtx in range(n_gtx):
              chipids = m_AMCmanager.getVFATs(gtx)
              # retrieve VFAT slot numberd and ChipIDs from DB
              vfats = VFAT.objects.all()
              # Check if the VFATs are in DB, add if not
              v_list = []
              for chip in chipids.keys():
                t_chipid = "0x%04x"%(chipids[chip])
                if t_chipid in vfats.filter(Slot=chip).values_list("ChipID", flat=True):
                  pass
                else:
                  print "Adding VFAT(ChipID = %s, Slot = %d)"%(t_chipid,chip)
                  v = VFAT(ChipID = t_chipid, Slot = chip)
                  v.save()
                v_list.append(VFAT.objects.get(ChipID = t_chipid, Slot = chip))
              #t_chamberID = 'OHv2aM'#hard code now, read from HW later when available
              t_chamberID = 'GTX-'+str(gtx) #use gtx link number now, read from HW later when available
              print "t_chamberID = %s" %(t_chamberID)
              g_list = []
              gebs = GEB.objects.filter(ChamberID=t_chamberID)
              t_flag = False
              for geb in gebs:
                if v_list == list(geb.vfats.all()):
                  t_flag = True
                  g_list.append(geb)
                  break
              if t_flag:
                pass
              else:
                print "Update DB"
                g = GEB(Type="Long",ChamberID = t_chamberID)
                g.save()
                for v in v_list:
                  g.vfats.add(v)
                  g_list.append(g)

            t_flag = False
            #t_boardID = "47-20120013"#hard code now, read from HW later when available
            t_boardID = "AMC-"+str(amcN)#hard code now, read from HW later when available
            a_list = []
            amcs = AMC.objects.filter(BoardID = t_boardID)
            for amc in amcs:
              if g_list == list(amc.gebs.all()):
                t_flag = True
                a_list.append(amc)
            if t_flag:
              pass
            else:
              print "Update DB"
              a = AMC(Type="GLIB",BoardID = t_boardID)
              a.save()
              for g in g_list:
                a.gebs.add(g)
                a_list.append(a)

          # create a new run. Some values are hard-coded for now
          runs = Run.objects.filter(Period = "2016T", Type = "bench", Station = "TAMU")
          rns = list(int(x) for x in list(runs.values_list("Number", flat=True)))
          nrs = u'%s'%(max(rns)+1)
          nrs = nrs.zfill(6)
          t_date = str(datetime.date.today())
          m_filename = "run"+str(nrs)+""+"_bench_TAMU_"+t_date
          newrun = Run(Name=m_filename, Type = "bench", Number = str(nrs), Date = datetime.date.today(), Period = "2016T", Station = "TAMU")
          newrun.save()
          for a in a_list:
            newrun.amcs.add(a)
          m_AMC13manager.configureTrigger(lt,2,1,int(trigger_rate),0)
          updateStatus()
          state = 'configured'
        except ValueError,e:
          print colors.YELLOW,e,colors.ENDC
          state = 'halted'
    elif 'halt' in request.POST:
      form = ConfigForm()
      m_AMC13manager.reset()
      state = 'halted'
    elif 'run' in request.POST:
      print "running"
      #form = ConfigForm()
      updateStatus()
      nevents = int(request.POST['nevents'])
      t = threading.Thread(target = m_AMC13manager.startDataTaking, args = ["/tmp/"+m_filename+".dat"])
      t.start()
      state = 'running'

    elif 'stop' in request.POST:
      m_AMC13manager.stopDataTaking()
      updateStatus()
      sleep(1)
      t_p = threading.Thread(target = parkData)
      t_p.start()
      state = 'configured'
    elif "monitoring" in request.POST:
      updateStatus()
      #pass
      #if lt:
      #  updateStatus()
      #else:
      #  pass
  else:
    form = ConfigForm()
    state = 'halted'
  return render(request, 'gemsupervisor.html',{'mon':m_monitor,
                                               'form':form,
                                               'state':state})

