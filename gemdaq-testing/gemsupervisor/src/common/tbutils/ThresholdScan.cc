#include "gem/supervisor/tbutils/ThresholdScan.h"

//#include "gem/supervisor/tbutils/ThresholdEvent.h"
#include "gem/readout/GEMDataParker.h"
#include "gem/readout/GEMDataAMCformat.h"
#include "gem/hw/vfat/HwVFAT2.h"

#include "TH1.h"
#include "TFile.h"
#include "TCanvas.h"
#include "TROOT.h"
#include "TString.h"
#include "TError.h"
 
#include <algorithm>
#include <iomanip>
#include <ctime>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

#include "cgicc/HTTPRedirectHeader.h"

#include "gem/supervisor/tbutils/VFAT2XMLParser.h"

#include "TStopwatch.h"

XDAQ_INSTANTIATOR_IMPL(gem::supervisor::tbutils::ThresholdScan)

bool First = true, Last = false;

TH1F* RTime = NULL;
TCanvas *can = NULL;

void gem::supervisor::tbutils::ThresholdScan::ConfigParams::registerFields(xdata::Bag<ConfigParams> *bag)
{
  latency      = 12U;
  minThresh    = -80;
  maxThresh    = 20;
  stepSize     = 5U;
  currentHisto = 0U;

  deviceVT1    = 0x0;
  deviceVT2    = 0x0;

  bag->addField("minThresh",   &minThresh);
  bag->addField("maxThresh",   &maxThresh);
  bag->addField("stepSize",    &stepSize );
  bag->addField("currentHisto",&currentHisto);
  bag->addField("deviceVT1",   &deviceVT1   );
  bag->addField("deviceVT2",   &deviceVT2   );

}

gem::supervisor::tbutils::ThresholdScan::ThresholdScan(xdaq::ApplicationStub * s)
  throw (xdaq::exception::Exception) :
  //  xdaq::WebApplication(s),
  gem::supervisor::tbutils::GEMTBUtil(s)
{
  // Detect when the setting of default parameters has been performed
  //SB this->getApplicationInfoSpace()->addListener(this, "urn:xdaq-event:setDefaultValues");

  getApplicationInfoSpace()->fireItemAvailable("scanParams", &scanParams_);
  getApplicationInfoSpace()->fireItemValueRetrieve("scanParams", &scanParams_);

  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::ThresholdScan::webDefault,      "Default"    );
  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::ThresholdScan::webConfigure,    "Configure"  );
  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::ThresholdScan::webStart,        "Start"      );
  runSig_   = toolbox::task::bind(this, &ThresholdScan::run,        "run"       );
  readSig_  = toolbox::task::bind(this, &ThresholdScan::readFIFO,   "readFIFO"  );
  
  wl_ = toolbox::task::getWorkLoopFactory()->getWorkLoop("urn:xdaq-workloop:GEMTestBeamSupervisor:ThresholdScan","waiting");
  wl_->activate();

}

gem::supervisor::tbutils::ThresholdScan::~ThresholdScan()
{
  wl_ = toolbox::task::getWorkLoopFactory()->getWorkLoop("urn:xdaq-workloop:GEMTestBeamSupervisor:ThresholdScan","waiting");
  //should we check to see if it's running and try to stop?
  wl_->cancel();
  wl_ = 0;
  
  if (histo) delete histo;
  histo = 0;

  for (int hi = 0; hi < 128; ++hi) {
    if (histos[hi]) delete histos[hi];
    histos[hi] = 0;
  }

  if (outputCanvas) delete outputCanvas;
  outputCanvas = 0;

}

// State transitions
bool gem::supervisor::tbutils::ThresholdScan::run(toolbox::task::WorkLoop* wl)
{
  wl_semaphore_.take();
  if (!is_running_) {
    //hw_semaphore_.take();
    //vfatDevice_->setRunMode(1);
    //hw_semaphore_.give();
    //if stop action has killed the run, take the final readout
    wl_semaphore_.give();
    wl_->submit(readSig_);
    return false;
  }

  /*
  if (First) {
    First = false;

    if (can) delete can; can = 0;
    can = new TCanvas("can","Dynamic Filling Example",0,0,400,400);
    can->SetFillColor(42);

    if (RTime) delete RTime; RTime = 0;
    RTime = new TH1F("RTime", "Real Time, 1000 triggers", 50, 0, 20);
    RTime->SetFillColor(48);
  } */

  //send triggers
  hw_semaphore_.take();
  vfatDevice_->setDeviceBaseNode("OptoHybrid.FAST_COM");

  // trigger times calculation
  // timer.Start();
  for (size_t trig = 0; trig < 500; ++trig) vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"Send.L1A",0x1);

  //count triggers
  vfatDevice_->setDeviceBaseNode("OptoHybrid.COUNTERS");
  confParams_.bag.triggersSeen = vfatDevice_->readReg(vfatDevice_->getDeviceBaseNode(),"L1A.Internal");

  //confParams_.bag.triggersSeen = vfatDevice_->readReg(vfatDevice_->getDeviceBaseNode(),"L1A.External");
  vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());

  //ADC Voltage, Current, update
  vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB");
  confParams_.bag.ADCVoltage = vfatDevice_->readReg(vfatDevice_->getDeviceBaseNode(),"VFAT_ADC.Voltage");
  confParams_.bag.ADCurrent = vfatDevice_->readReg(vfatDevice_->getDeviceBaseNode(),"VFAT_ADC.Current");
  vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());

  hw_semaphore_.give();
  
  //timer.Stop(); Float_t RT = (Float_t)timer.RealTime();

  LOG4CPLUS_INFO(getApplicationLogger(),
		 " ABC TriggersSeen " << confParams_.bag.triggersSeen);
  LOG4CPLUS_DEBUG(getApplicationLogger(),
		 " TriggersSeen " << confParams_.bag.triggersSeen << " ADCVoltage " << confParams_.bag.ADCVoltage 
                  << " ADCurrent " << confParams_.bag.ADCurrent );

  /* trigger times calculation
  timer.Stop(); Float_t RT = (Float_t)timer.RealTime();

  if (can) {
    can->cd();
    if (RTime) {
      RTime->Fill(RT);
      RTime->Draw();
    }
    can->Update();
    can->SaveAs(TString("RTime.png")); 
  } */

  if ((uint64_t)(confParams_.bag.triggersSeen) < (uint64_t)(confParams_.bag.nTriggers)) {
    hw_semaphore_.take();
    vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());

    LOG4CPLUS_INFO(getApplicationLogger(),
      "ABC Not enough triggers, run mode 0x" << std::hex << (unsigned)vfatDevice_->getRunMode() << std::dec);
    
    vfatDevice_->setDeviceBaseNode("GLIB");
    uint32_t bufferDepth = vfatDevice_->readReg(vfatDevice_->getDeviceBaseNode(),"LINK1.TRK_FIFO.DEPTH");
    vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
    hw_semaphore_.give();
    if (bufferDepth < 10) {
      //update
      //sleep(1);
      hw_semaphore_.take();

      //triggersSeen update 
      vfatDevice_->setDeviceBaseNode("OptoHybrid.COUNTERS");
      confParams_.bag.triggersSeen = vfatDevice_->readReg(vfatDevice_->getDeviceBaseNode(),"L1A.Internal");
      //confParams_.bag.triggersSeen = vfatDevice_->readReg(vfatDevice_->getDeviceBaseNode(),"L1A.External");
      vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());

      //ADC Voltage, Current, update
      vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB");
      confParams_.bag.ADCVoltage = vfatDevice_->readReg(vfatDevice_->getDeviceBaseNode(),"VFAT_ADC.Voltage");
      confParams_.bag.ADCurrent = vfatDevice_->readReg(vfatDevice_->getDeviceBaseNode(),"VFAT_ADC.Current");
      vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
   
      LOG4CPLUS_INFO(getApplicationLogger(),"ABC Not enough entries in the buffer, run mode 0x" 
                      << std::hex << (unsigned)vfatDevice_->getRunMode() << std::dec);

      hw_semaphore_.give();
      wl_semaphore_.give();
      return true;
    }
    else {
      //maybe don't do the readout as a workloop?
      hw_semaphore_.take();
      vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());

      LOG4CPLUS_INFO(getApplicationLogger(),"ABC Buffer full, reading out, run mode 0x" 
                     << std::hex << (unsigned)vfatDevice_->getRunMode() << std::dec);

      hw_semaphore_.give();
      wl_semaphore_.give();
      wl_->submit(readSig_);

      return true;
    }
    //wl_semaphore_.give();
  }
  else {

    hw_semaphore_.take();
    vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
    LOG4CPLUS_INFO(getApplicationLogger(),"ABC Enough triggers, reading out, run mode 0x" 
                   << std::hex << (unsigned)vfatDevice_->getRunMode() << std::dec);

    hw_semaphore_.give();
    wl_semaphore_.give();

    wl_->submit(readSig_);

    wl_semaphore_.take();
    hw_semaphore_.take();
    vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
    vfatDevice_->setRunMode(0);

    //flush FIFO
    vfatDevice_->setDeviceBaseNode("GLIB.LINK1");
    vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"TRK_FIFO.FLUSH",0x1);

    vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
    hw_semaphore_.give();

    LOG4CPLUS_INFO(getApplicationLogger()," ABC Scan point TriggersSeen " 
		   << confParams_.bag.triggersSeen );

    if ( (unsigned)scanParams_.bag.deviceVT1 == (unsigned)0x0 ) {
    // ( (unsigned)scanParams_.bag.deviceVT1 == (unsigned)0x0 )

      //wl_semaphore_.take();
      hw_semaphore_.take();

      LOG4CPLUS_INFO(getApplicationLogger(),
	  "ABC VT1 is 0, reading out, run mode 0x" << std::hex << (unsigned)vfatDevice_->getRunMode() << std::dec );

      hw_semaphore_.give();
      wl_semaphore_.give();
      wl_->submit(stopSig_);
      return false;

    }
    else if ( (scanParams_.bag.deviceVT2-scanParams_.bag.deviceVT1) <= scanParams_.bag.maxThresh ) {
    // else if ( (scanParams_.bag.deviceVT2-scanParams_.bag.deviceVT1) <= scanParams_.bag.maxThresh ) { 

      hw_semaphore_.take();

      vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());

      LOG4CPLUS_INFO(getApplicationLogger()," ABC run: VT1= " 
         << scanParams_.bag.deviceVT1 << " VT2-VT1= " << scanParams_.bag.deviceVT2-scanParams_.bag.deviceVT1 
         << " bag.maxThresh= " << scanParams_.bag.maxThresh 
	 << " abs(VT2-VT1) " << abs(scanParams_.bag.deviceVT2-scanParams_.bag.deviceVT1) );

      LOG4CPLUS_INFO(getApplicationLogger(),
        "ABC VT2-VT1 is less than the max threshold, run mode 0x" << std::hex << (unsigned)vfatDevice_->getRunMode() << std::dec);

      hw_semaphore_.give();
      //how to ensure that the VT1 never goes negative
      hw_semaphore_.take();
      if (scanParams_.bag.deviceVT1 > scanParams_.bag.stepSize) {
	vfatDevice_->setVThreshold1(scanParams_.bag.deviceVT1 - scanParams_.bag.stepSize);
      } else {
	vfatDevice_->setVThreshold1(0);
      }

      scanParams_.bag.deviceVT1    = vfatDevice_->getVThreshold1();
      scanParams_.bag.deviceVT2    = vfatDevice_->getVThreshold2();
      confParams_.bag.triggersSeen = 0;
      vfatDevice_->setDeviceBaseNode("OptoHybrid.COUNTERS.RESETS");
      vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"L1A.Internal",0x1);
      vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"L1A.External",0x1);
      vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"L1A.Delayed",0x1);
      vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"L1A.Total",0x1);

      vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
      vfatDevice_->setRunMode(1);
      vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());

      LOG4CPLUS_INFO(getApplicationLogger(),"ABC Resubmitting the run workloop, run mode 0x" 
                     << std::hex << (unsigned)vfatDevice_->getRunMode() << std::dec);

      hw_semaphore_.give();
      wl_semaphore_.give();	
      return true;	
    }
    else {
 
      //wl_semaphore_.take();
      hw_semaphore_.take();

      vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());

      LOG4CPLUS_INFO(getApplicationLogger(),"ABC reached max threshold, stopping out, run mode 0x" 
                     << std::hex << (unsigned)vfatDevice_->getRunMode() << std::dec);

      hw_semaphore_.give();
      wl_semaphore_.give();
      wl_->submit(stopSig_);
      return false; 
    }
  }
}

//might be better done not as a workloop?
bool gem::supervisor::tbutils::ThresholdScan::readFIFO(toolbox::task::WorkLoop* wl)
{
  //VFATEvent vfat;
  gem::readout::VFATData vfat;

  int ievent=0;

  wl_semaphore_.take();
  hw_semaphore_.take();

  std::string tmpFileName = confParams_.bag.outFileName.toString();

  //maybe not even necessary?
  //vfatDevice_->setRunMode(0);
  //sleep(5);
  //read the fifo (x3 times fifo depth), add headers, write to disk, save disk
  boost::format linkForm("LINK%d");
  //should all links have the same fifo depth? if not is this an error?

  uint32_t fifoDepth[3];
  //set proper base address
  vfatDevice_->setDeviceBaseNode("GLIB");
  fifoDepth[0] = vfatDevice_->readReg(vfatDevice_->getDeviceBaseNode(),boost::str(linkForm%(link))+".TRK_FIFO.DEPTH");
  fifoDepth[1] = vfatDevice_->readReg(vfatDevice_->getDeviceBaseNode(),boost::str(linkForm%(link))+".TRK_FIFO.DEPTH");
  fifoDepth[2] = vfatDevice_->readReg(vfatDevice_->getDeviceBaseNode(),boost::str(linkForm%(link))+".TRK_FIFO.DEPTH");
  
  //check that the fifos are all the same size?
  int bufferDepth = 0;
  if (fifoDepth[0] != fifoDepth[1] || 
      fifoDepth[0] != fifoDepth[2] || 
      fifoDepth[1] != fifoDepth[2]) {
        LOG4CPLUS_DEBUG(getApplicationLogger(),
          "tracking data fifos had different depths:: " << fifoDepth[0] << "," << fifoDepth[0] << "," << fifoDepth[0]);

        //use the minimum
        bufferDepth = std::min(fifoDepth[0],std::min(fifoDepth[1],fifoDepth[2]));
  }
  //right now only have FIFO on LINK1
  bufferDepth = fifoDepth[1];
  
  //grab events from the fifo
  bool isFirst = true;
  uint32_t bxNum, bxExp;

  while (bufferDepth) {

    std::vector<uint32_t> data;
    //readInWords(data);
    vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.TRK_DATA.COL1");
    
    LOG4CPLUS_DEBUG(getApplicationLogger(),"Trying to read register "<<vfatDevice_->getDeviceBaseNode()<<".DATA_RDY");
    if (vfatDevice_->readReg(vfatDevice_->getDeviceBaseNode(),"DATA_RDY")) {
      LOG4CPLUS_DEBUG(getApplicationLogger(),"Trying to read the block at "<<vfatDevice_->getDeviceBaseNode()<<".DATA");
      //data = vfatDevice_->readBlock("OptoHybrid.GEB.TRK_DATA.COL1.DATA");
      //data = vfatDevice_->readBlock("OptoHybrid.GEB.TRK_DATA.COL1.DATA",7);
      for (int word = 0; word < 7; ++word) {
	std::stringstream ss9;
	ss9 << "DATA." << word;
	data.push_back(vfatDevice_->readReg(vfatDevice_->getDeviceBaseNode(),ss9.str()));
      }
    }

    uint32_t TrigReg, bxNumTr;
    uint8_t sBit;

    // read trigger data
    vfatDevice_->setDeviceBaseNode("GLIB");
    TrigReg = vfatDevice_->readReg(vfatDevice_->getDeviceBaseNode(),"TRG_DATA.DATA");
    bxNumTr = TrigReg >> 6;
    sBit = TrigReg & 0x0000003F;

    //make sure we are aligned

    //if (!checkHeaders(data)) 
    uint16_t b1010, b1100, b1110;
    b1010 = ((data.at(5) & 0xF0000000)>>28);
    b1100 = ((data.at(5) & 0x0000F000)>>12);
    b1110 = ((data.at(4) & 0xF0000000)>>28);

    bxNum = data.at(6);
    
    uint16_t bcn, evn, crc, chipid;
    uint64_t msData, lsData;
    uint8_t  flags;
    double   delVT;

    if (isFirst)
      bxExp = bxNum;
    
    if (bxNum == bxExp)
      isFirst = false;
    
    bxNum  = data.at(6);
    bcn    = (0x0fff0000 & data.at(5)) >> 16;
    evn    = (0x00000ff0 & data.at(5)) >> 4;
    chipid = (0x0fff0000 & data.at(4)) >> 16;
    flags  = (0x0000000f & data.at(5));
    crc    = (0x0000ffff & data.at(0));

    uint64_t data1  = ((0x0000ffff & data.at(4)) << 16) | ((0xffff0000 & data.at(3)) >> 16);
    uint64_t data2  = ((0x0000ffff & data.at(3)) << 16) | ((0xffff0000 & data.at(2)) >> 16);
    uint64_t data3  = ((0x0000ffff & data.at(2)) << 16) | ((0xffff0000 & data.at(1)) >> 16);
    uint64_t data4  = ((0x0000ffff & data.at(1)) << 16) | ((0xffff0000 & data.at(0)) >> 16);

    lsData = (data3 << 32) | (data4);
    msData = (data1 << 32) | (data2);

    delVT = (scanParams_.bag.deviceVT2-scanParams_.bag.deviceVT1);

    vfat.BC     = ( b1010 << 12 ) | (bcn);                // 1010     | bcn:12
    vfat.EC     = ( b1100 << 12 ) | (evn << 4) | (flags); // 1100     | EC:8      | Flag:4 (zero?)
    vfat.ChipID = ( b1110 << 12 ) | (chipid);             // 1110     | ChipID:12
    vfat.lsData = lsData;                                 // lsData:64
    vfat.msData = msData;                                 // msData:64
    vfat.crc    = crc;                                    // crc:16

    /*
    vfat.delVT = delVT;
    vfat.bxNum = (bxNum << 6) | sBit);
    */
    // keepEvent(tmpFileName, ievent, ev, ch);

    if (!(((b1010 == 0xa) && (b1100==0xc) && (b1110==0xe)))){
      // dump VFAT data
      gem::readout::printVFATdataBits(ievent, vfat);
      LOG4CPLUS_INFO(getApplicationLogger(),"VFAT headers do not match expectation");

      vfatDevice_->setDeviceBaseNode("GLIB");
      bufferDepth = vfatDevice_->readReg(vfatDevice_->getDeviceBaseNode(),"LINK1.TRK_FIFO.DEPTH");
      TrigReg = vfatDevice_->readReg(vfatDevice_->getDeviceBaseNode(),"TRG_DATA.DATA");
      continue;
    }

   /*
    * dump VFAT data */
    gem::readout::printVFATdataBits(ievent, vfat);

   /*
    * GEM data filling */
    //gem::readout::GEMDataParker::writeGEMevent(gem, geb, vfat);
    //gem::readout::GEMDataParker::fillGEMevent(gem, geb, vfat);
    //int counter_ = gemDataParker->dumpDataToDisk();

    //while (bxNum == bxExp) {
    
    //Maybe add another histogramt that is a combined all channels histogram
    histo->Fill(delVT,(lsData||msData));

    //I think it would be nice to time this...
    for (int chan = 0; chan < 128; ++chan) {
      if (chan < 64)
	histos[chan]->Fill(delVT,((lsData>>chan))&0x1);
      else
	histos[chan]->Fill(delVT,((msData>>(chan-64)))&0x1);
    }

    vfatDevice_->setDeviceBaseNode("GLIB");
    bufferDepth = vfatDevice_->readReg(vfatDevice_->getDeviceBaseNode(),"LINK1.TRK_FIFO.DEPTH");
  }

  vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
  hw_semaphore_.give();

  std::string imgRoot = "${XDAQ_DOCUMENT_ROOT}/gemdaq/gemsupervisor/html/images/tbutils/tscan/";
  std::stringstream ss;
  ss << "chanthresh0.png";
  std::string imgName = ss.str();
  outputCanvas->cd();
  histo->Draw("ep0l");
  outputCanvas->Update();
  outputCanvas->SaveAs(TString(imgRoot+imgName));

  for (int chan = 0; chan < 128; ++chan) {
    imgRoot = "${XDAQ_DOCUMENT_ROOT}/gemdaq/gemsupervisor/html/images/tbutils/tscan/";
    ss.clear();
    ss.str(std::string());
    ss << "chanthresh" << (chan+1) << ".png";
    imgName = ss.str();
    outputCanvas->cd();
    histos[chan]->Draw("ep0l");
    outputCanvas->Update();
    outputCanvas->SaveAs(TString(imgRoot+imgName));
  }

  wl_semaphore_.give();

  return false;
}

void gem::supervisor::tbutils::ThresholdScan::scanParameters(xgi::Output *out)
  throw (xgi::exception::Exception)
{
  try {
    std::string isReadonly = "";
    if (is_running_ || is_configured_)
      isReadonly = "readonly";
    *out << cgicc::span()   << std::endl
	 << cgicc::label("Latency").set("for","Latency") << std::endl
	 << cgicc::input().set("id","Latency").set(is_running_?"readonly":"").set("name","Latency")
                          .set("type","number").set("min","0").set("max","255")
                          .set("value",boost::str(boost::format("%d")%static_cast<unsigned>(scanParams_.bag.latency)))
	 << std::endl
	 << cgicc::br() << std::endl

	 << cgicc::label("MinThreshold").set("for","MinThreshold") << std::endl
	 << cgicc::input().set("id","MinThreshold").set(is_running_?"readonly":"").set("name","MinThreshold")
                          .set("type","number").set("min","-255").set("max","255")
                          .set("value",boost::str(boost::format("%d")%(scanParams_.bag.minThresh)))
	 << std::endl

	 << cgicc::label("MaxThreshold").set("for","MaxThreshold") << std::endl
	 << cgicc::input().set("id","MaxThreshold").set(is_running_?"readonly":"").set("name","MaxThreshold")
                          .set("type","number").set("min","-255").set("max","255")
                          .set("value",boost::str(boost::format("%d")%(scanParams_.bag.maxThresh)))
	 << std::endl
	 << cgicc::br() << std::endl

	 << cgicc::label("VStep").set("for","VStep") << std::endl
	 << cgicc::input().set("id","VStep").set(is_running_?"readonly":"").set("name","VStep")
                          .set("type","number").set("min","1").set("max","255")
                          .set("value",boost::str(boost::format("%d")%(scanParams_.bag.stepSize)))
	 << std::endl
	 << cgicc::br() << std::endl

	 << cgicc::label("VT1").set("for","VT1") << std::endl
	 << cgicc::input().set("id","VT1").set("name","VT1").set("readonly")
                          .set("value",boost::str(boost::format("%d")%static_cast<unsigned>(scanParams_.bag.deviceVT1)))
	 << std::endl

	 << cgicc::label("VT2").set("for","VT2") << std::endl
	 << cgicc::input().set("id","VT2").set("name","VT2").set("readonly")
                          .set("value",boost::str(boost::format("%d")%static_cast<unsigned>(scanParams_.bag.deviceVT2)))
	 << std::endl
	 << cgicc::br() << std::endl

	 << cgicc::label("NTrigsStep").set("for","NTrigsStep") << std::endl
	 << cgicc::input().set("id","NTrigsStep").set(is_running_?"readonly":"").set("name","NTrigsStep")
                          .set("type","number").set("min","0")
                          .set("value",boost::str(boost::format("%d")%(confParams_.bag.nTriggers)))
	 << cgicc::br() << std::endl
	 << cgicc::label("NTrigsSeen").set("for","NTrigsSeen") << std::endl
	 << cgicc::input().set("id","NTrigsSeen").set("name","NTrigsSeen")
                          .set("type","number").set("min","0").set("readonly")
                          .set("value",boost::str(boost::format("%d")%(confParams_.bag.triggersSeen)))
	 << cgicc::br() << std::endl
	 << cgicc::label("ADCVoltage").set("for","ADCVoltage") << std::endl
	 << cgicc::input().set("id","ADCVoltage").set("name","ADCVoltage")
                          .set("type","number").set("min","0").set("readonly")
                          .set("value",boost::str(boost::format("%d")%(confParams_.bag.ADCVoltage)))
	 << cgicc::label("ADCurrent").set("for","ADCurrent") << std::endl
	 << cgicc::input().set("id","ADCurrent").set("name","ADCurrent")
                          .set("type","number").set("min","0").set("readonly")
                          .set("value",boost::str(boost::format("%d")%(confParams_.bag.ADCurrent)))
	 << cgicc::br() << std::endl
	 << cgicc::span()   << std::endl;
  }
  catch (const xgi::exception::Exception& e) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Something went wrong displaying VFATS(xgi): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  catch (const std::exception& e) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Something went wrong displaying VFATS(std): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
}

void gem::supervisor::tbutils::ThresholdScan::displayHistograms(xgi::Output *out)
  throw (xgi::exception::Exception)
{
  try {
    *out << cgicc::form().set("method","POST").set("action", "") << std::endl;
    
    *out << cgicc::table().set("class","xdaq-table") << std::endl
	 << cgicc::thead() << std::endl
	 << cgicc::tr()    << std::endl //open
	 << cgicc::th()    << "Select Channel" << cgicc::th() << std::endl
	 << cgicc::th()    << "Histogram"      << cgicc::th() << std::endl
	 << cgicc::tr()    << std::endl //close
	 << cgicc::thead() << std::endl 
      
	 << cgicc::tbody() << std::endl;
    
    *out << cgicc::tr()  << std::endl;
    *out << cgicc::td()
	 << cgicc::label("Channel").set("for","ChannelHist") << std::endl
	 << cgicc::input().set("id","ChannelHist").set("name","ChannelHist")
                          .set("type","number").set("min","0").set("max","128")
                          .set("value",scanParams_.bag.currentHisto.toString())
	 << std::endl
	 << cgicc::br() << std::endl;
    *out << cgicc::input().set("class","button").set("type","button")
                          .set("value","SelectChannel").set("name","DisplayHistogram")
                          .set("onClick","changeImage(this.form)");
    *out << cgicc::td() << std::endl;

    *out << cgicc::td()  << std::endl
	 << cgicc::img().set("src","/gemdaq/gemsupervisor/html/images/tbutils/tscan/chanthresh"+scanParams_.bag.currentHisto.toString()+".png")
                        .set("id","vfatChannelHisto")
	 << cgicc::td()    << std::endl;
    *out << cgicc::tr()    << std::endl
	 << cgicc::tbody() << std::endl
	 << cgicc::table() << std::endl;
    *out << cgicc::form() << cgicc::br() << std::endl;
  }
  catch (const xgi::exception::Exception& e) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Something went wrong displaying displayHistograms(xgi): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  catch (const std::exception& e) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Something went wrong displaying displayHistograms(std): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
}

// HyperDAQ interface
void gem::supervisor::tbutils::ThresholdScan::webDefault(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception)
{
  //LOG4CPLUS_INFO(this->getApplicationLogger(),"gem::supervisor::tbutils::ThresholdScan::webDefaul");
  try {
    ////update the page refresh 
    if (!is_working_ && !is_running_) {
    }
    else if (is_working_) {
      cgicc::HTTPResponseHeader &head = out->getHTTPResponseHeader();
      head.addHeader("Refresh","60");
    }
    else if (is_running_) {
      cgicc::HTTPResponseHeader &head = out->getHTTPResponseHeader();
      head.addHeader("Refresh","20");
    }
    
    //generate the control buttons and display the ones that can be touched depending on the run mode
    *out << "<div class=\"xdaq-tab-wrapper\">"            << std::endl;
    *out << "<div class=\"xdaq-tab\" title=\"Control\">"  << std::endl;

    *out << "<table class=\"xdaq-table\">" << std::endl
	 << cgicc::thead() << std::endl
	 << cgicc::tr()    << std::endl //open
	 << cgicc::th()    << "Control" << cgicc::th() << std::endl
	 << cgicc::th()    << "Buffer"  << cgicc::th() << std::endl
	 << cgicc::tr()    << std::endl //close
	 << cgicc::thead() << std::endl 
      
	 << "<tbody>" << std::endl
	 << "<tr>"    << std::endl
	 << "<td>"    << std::endl;
    
    if (!is_initialized_) {
      //have a menu for selecting the VFAT
      *out << cgicc::form().set("method","POST").set("action", "/" + getApplicationDescriptor()->getURN() + "/Initialize") << std::endl;

      selectVFAT(out);
      scanParameters(out);
      
      *out << cgicc::input().set("type", "submit")
	.set("name", "command").set("title", "Initialize hardware acces.")
	.set("value", "Initialize") << std::endl;

      *out << cgicc::form() << std::endl;
    }
    
    else if (!is_configured_) {
      //this will allow the parameters to be set to the chip and scan routine

      *out << cgicc::form().set("method","POST").set("action", "/" + getApplicationDescriptor()->getURN() + "/Configure") << std::endl;
      
      selectVFAT(out);
      scanParameters(out);
      
      //adding aysen's xml parser
      //std::string setConfFile = toolbox::toString("/%s/setConfFile",getApplicationDescriptor()->getURN().c_str());
      //*out << cgicc::form().set("method","POST").set("action",setConfFile) << std::endl ;
      
      *out << cgicc::input().set("type","text").set("name","xmlFilename").set("size","80")
 	                    .set("ENCTYPE","multipart/form-data").set("readonly")
                            .set("value",confParams_.bag.settingsFile.toString()) << std::endl;
      //*out << cgicc::input().set("type","submit").set("value","Set configuration file") << std::endl ;
      //*out << cgicc::form() << std::endl ;
      
      *out << cgicc::br() << std::endl;
      *out << cgicc::input().set("type", "submit")
	.set("name", "command").set("title", "Configure threshold scan.")
	.set("value", "Configure") << std::endl;
      *out << cgicc::form()        << std::endl;
    }
    
    else if (!is_running_) {
      //hardware is initialized and configured, we can start the run
      *out << cgicc::form().set("method","POST").set("action", "/" + getApplicationDescriptor()->getURN() + "/Start") << std::endl;
      
      selectVFAT(out);
      scanParameters(out);
      
      *out << cgicc::input().set("type", "submit")
	.set("name", "command").set("title", "Start threshold scan.")
	.set("value", "Start") << std::endl;
      *out << cgicc::form()    << std::endl;
    }
    
    else if (is_running_) {
      *out << cgicc::form().set("method","POST").set("action", "/" + getApplicationDescriptor()->getURN() + "/Stop") << std::endl;
      
      selectVFAT(out);
      scanParameters(out);
      
      *out << cgicc::input().set("type", "submit")
	.set("name", "command").set("title", "Stop threshold scan.")
	.set("value", "Stop") << std::endl;
      *out << cgicc::form()   << std::endl;
    }
    
    *out << cgicc::comment() << "end the main commands, now putting the halt/reset commands" << cgicc::comment() << cgicc::br() << std::endl;
    *out << cgicc::span()  << std::endl
	 << "<table>" << std::endl
	 << "<tr>"    << std::endl
	 << "<td>"    << std::endl;
      
    //always should have a halt command
    *out << cgicc::form().set("method","POST").set("action", "/" + getApplicationDescriptor()->getURN() + "/Halt") << std::endl;
    
    *out << cgicc::input().set("type", "submit")
      .set("name", "command").set("title", "Halt threshold scan.")
      .set("value", "Halt") << std::endl;
    *out << cgicc::form() << std::endl
	 << "</td>" << std::endl;
    
    *out << "<td>"  << std::endl;
    if (!is_running_) {
      //comand that will take the system to initial and allow to change the hw device
      *out << cgicc::form().set("method","POST").set("action", "/" + getApplicationDescriptor()->getURN() + "/Reset") << std::endl;
      *out << cgicc::input().set("type", "submit")
	.set("name", "command").set("title", "Reset device.")
	.set("value", "Reset") << std::endl;
      *out << cgicc::form() << std::endl;
    }
    *out << "</td>"    << std::endl
	 << "</tr>"    << std::endl
	 << "</table>" << std::endl
	 << cgicc::br() << std::endl
	 << cgicc::span()  << std::endl;

    *out << "</td>" << std::endl;

    *out << "<td>" << std::endl;
    if (is_initialized_)
      showBufferLayout(out);
    *out << "</td>"    << std::endl
	 << "</tr>"    << std::endl
	 << "</tbody>" << std::endl
	 << "</table>" << cgicc::br() << std::endl;
    
    *out << "</div>" << std::endl;
    
    *out << "<div class=\"xdaq-tab\" title=\"Counters\">"  << std::endl;
    if (is_initialized_)
      showCounterLayout(out);
    *out << "</div>" << std::endl;

    *out << "<div class=\"xdaq-tab\" title=\"Fast Commands/Trigger Setup\">"  << std::endl;
    if (is_initialized_)
      fastCommandLayout(out);
    *out << "</div>" << std::endl;

    //place new div class=xdaq-tab here to hold the histograms
    /*
      display a single histogram and have a form that selects which channel you want to display
      use the file name of the histogram that is saved in readFIFO
    */
    *out << "<div class=\"xdaq-tab\" title=\"Channel histograms\">"  << std::endl;
    displayHistograms(out);
    
    *out << "</div>" << std::endl;
    *out << "</div>" << std::endl;
    //</div> //close the new div xdaq-tab

    *out << cgicc::br() << cgicc::br() << std::endl;
    
    //*out << "<div class=\"xdaq-tab\" title=\"Status\">"  << std::endl
    //*out << cgicc::div().set("class","xdaq-tab").set("title","Status")   << std::endl
    *out << "<table class=\"xdaq-table\">" << std::endl
	 << cgicc::thead() << std::endl
	 << cgicc::tr()    << std::endl //open
	 << cgicc::th()    << "Program" << cgicc::th() << std::endl
	 << cgicc::th()    << "System"  << cgicc::th() << std::endl
	 << cgicc::tr()    << std::endl //close
	 << cgicc::thead() << std::endl 
      
	 << "<tbody>" << std::endl
	 << "<tr>"    << std::endl
	 << "<td>"    << std::endl;

    *out << "<table class=\"xdaq-table\">" << std::endl
	 << cgicc::thead() << std::endl
	 << cgicc::tr()    << std::endl //open
	 << cgicc::th()    << "Status" << cgicc::th() << std::endl
	 << cgicc::th()    << "Value"  << cgicc::th() << std::endl
	 << cgicc::tr()    << std::endl //close
	 << cgicc::thead() << std::endl 
      
	 << "<tbody>" << std::endl

	 << "<tr>" << std::endl
	 << "<td>" << "is_working_" << "</td>"
	 << "<td>" << is_working_   << "</td>"
	 << "</tr>"   << std::endl

	 << "<tr>" << std::endl
	 << "<td>" << "is_initialized_" << "</td>"
	 << "<td>" << is_initialized_   << "</td>"
	 << "</tr>"       << std::endl

	 << "<tr>" << std::endl
	 << "<td>" << "is_configured_" << "</td>"
	 << "<td>" << is_configured_   << "</td>"
	 << "</tr>"      << std::endl

	 << "<tr>" << std::endl
	 << "<td>" << "is_running_" << "</td>"
	 << "<td>" << is_running_   << "</td>"
	 << "</tr>"   << std::endl

	 << "</tbody>" << std::endl
	 << "</table>" << cgicc::br() << std::endl
	 << "</td>"    << std::endl;
    
    *out  << "<td>"     << std::endl
	  << "<table class=\"xdaq-table\">" << std::endl
	  << cgicc::thead() << std::endl
	  << cgicc::tr()    << std::endl //open
	  << cgicc::th()    << "Device"     << cgicc::th() << std::endl
	  << cgicc::th()    << "Connected"  << cgicc::th() << std::endl
	  << cgicc::tr()    << std::endl //close
	  << cgicc::thead() << std::endl 
	  << "<tbody>" << std::endl;
    
    if (is_initialized_ && vfatDevice_) {
      hw_semaphore_.take();
      vfatDevice_->setDeviceBaseNode("TEST");
      *out << "<tr>" << std::endl
	   << "<td>" << "GLIB" << "</td>"
	   << "<td>" << vfatDevice_->readReg(vfatDevice_->getDeviceBaseNode(),"GLIB") << "</td>"
	   << "</tr>"   << std::endl
	
	   << "<tr>" << std::endl
	   << "<td>" << "OptoHybrid" << "</td>"
	   << "<td>" << vfatDevice_->readReg(vfatDevice_->getDeviceBaseNode(),"OptoHybrid") << "</td>"
	   << "</tr>"       << std::endl
	
	   << "<tr>" << std::endl
	   << "<td>" << "VFATs" << "</td>"
	   << "<td>" << vfatDevice_->readReg(vfatDevice_->getDeviceBaseNode(),"VFATs") << "</td>"
	   << "</tr>"      << std::endl;
      
      vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
      hw_semaphore_.give();
    }
    
    *out << "</tbody>" << std::endl
	 << "</table>" << std::endl
	 << "</td>"    << std::endl
	 << "</tr>"    << std::endl
	 << "</tbody>" << std::endl
	 << "</table>" << std::endl;
      //<< "</div>"   << std::endl;

    *out << cgicc::script().set("type","text/javascript")
                           .set("src","http://ajax.googleapis.com/ajax/libs/jquery/1/jquery.min.js")
	 << cgicc::script() << std::endl;
    *out << cgicc::script().set("type","text/javascript")
                           .set("src","http://ajax.googleapis.com/ajax/libs/jqueryui/1/jquery-ui.min.js")
	 << cgicc::script() << std::endl;
    *out << cgicc::script().set("type","text/javascript")
                           .set("src","/gemdaq/gemsupervisor/html/scripts/tbutils/changeImage.js")
	 << cgicc::script() << std::endl;
  }
  catch (const xgi::exception::Exception& e) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Something went wrong displaying ThresholdScan control panel(xgi): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  catch (const std::exception& e) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Something went wrong displaying ThresholdScan control panel(std): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
}


void gem::supervisor::tbutils::ThresholdScan::webConfigure(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception) {

  try {
    cgicc::Cgicc cgi(in);
    
    //aysen's xml parser
    confParams_.bag.settingsFile = cgi.getElement("xmlFilename")->getValue();
    
    cgicc::const_form_iterator element = cgi.getElement("Latency");
    if (element != cgi.getElements().end())
      scanParams_.bag.latency   = element->getIntegerValue();

    element = cgi.getElement("MinThreshold");
    if (element != cgi.getElements().end())
      scanParams_.bag.minThresh = element->getIntegerValue();
    
    element = cgi.getElement("MaxThreshold");
    if (element != cgi.getElements().end())
      scanParams_.bag.maxThresh = element->getIntegerValue();

    element = cgi.getElement("VStep");
    if (element != cgi.getElements().end())
      scanParams_.bag.stepSize  = element->getIntegerValue();
        
    element = cgi.getElement("NTrigsStep");
    if (element != cgi.getElements().end())
      confParams_.bag.nTriggers  = element->getIntegerValue();
  }
  catch (const xgi::exception::Exception & e) {
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  catch (const std::exception & e) {
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  
  wl_->submit(confSig_);
  
  redirect(in,out);
}


void gem::supervisor::tbutils::ThresholdScan::webStart(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception) {

  try {
    cgicc::Cgicc cgi(in);
    
    cgicc::const_form_iterator element = cgi.getElement("Latency");
    if (element != cgi.getElements().end())
      scanParams_.bag.latency   = element->getIntegerValue();

    element = cgi.getElement("MinThreshold");
    if (element != cgi.getElements().end())
      scanParams_.bag.minThresh = element->getIntegerValue();
    
    element = cgi.getElement("MaxThreshold");
    if (element != cgi.getElements().end())
      scanParams_.bag.maxThresh = element->getIntegerValue();

    element = cgi.getElement("VStep");
    if (element != cgi.getElements().end())
      scanParams_.bag.stepSize  = element->getIntegerValue();
        
    element = cgi.getElement("NTrigsStep");
    if (element != cgi.getElements().end())
      confParams_.bag.nTriggers  = element->getIntegerValue();
  }
  catch (const xgi::exception::Exception & e) {
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  catch (const std::exception & e) {
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  
  wl_->submit(startSig_);
  
  redirect(in,out);
}

// State transitions
void gem::supervisor::tbutils::ThresholdScan::configureAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {

  is_working_ = true;

  latency_   = scanParams_.bag.latency;
  nTriggers_ = confParams_.bag.nTriggers;
  stepSize_  = scanParams_.bag.stepSize;
  minThresh_ = scanParams_.bag.minThresh;
  maxThresh_ = scanParams_.bag.maxThresh;
  
  hw_semaphore_.take();
  vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());

  //make sure device is not running
  vfatDevice_->setRunMode(0);

  /****unimplemented at the moment
  if ((confParams_.bag.settingsFile.toString()).rfind("xml") != std::string::npos) {
    LOG4CPLUS_INFO(getApplicationLogger(),"loading settings from XML file");
    gem::supervisor::tbutils::VFAT2XMLParser::VFAT2XMLParser theParser(confParams_.bag.settingsFile.toString(),
								       vfatDevice_);
    theParser.parseXMLFile();
  }
  */
  
  //else {
    LOG4CPLUS_INFO(getApplicationLogger(),"loading default settings");
    //default settings for the frontend
    vfatDevice_->setTriggerMode(    0x3); //set to S1 to S8
    vfatDevice_->setCalibrationMode(0x0); //set to normal
    vfatDevice_->setMSPolarity(     0x1); //negative
    vfatDevice_->setCalPolarity(    0x1); //negative
    
    vfatDevice_->setProbeMode(        0x0);
    vfatDevice_->setLVDSMode(         0x0);
    vfatDevice_->setDACMode(          0x0);
    vfatDevice_->setHitCountCycleTime(0x0); //maximum number of bits
    
    vfatDevice_->setHitCountMode( 0x0);
    vfatDevice_->setMSPulseLength(0x3);
    vfatDevice_->setInputPadMode( 0x0);
    vfatDevice_->setTrimDACRange( 0x0);
    vfatDevice_->setBandgapPad(   0x0);
    vfatDevice_->sendTestPattern( 0x0);
        
    vfatDevice_->setIPreampIn(  168);
    vfatDevice_->setIPreampFeed(150);
    vfatDevice_->setIPreampOut(  80);
    vfatDevice_->setIShaper(    150);
    vfatDevice_->setIShaperFeed(100);
    vfatDevice_->setIComp(      120);

    vfatDevice_->setLatency(latency_);
    //}
  
  vfatDevice_->setVThreshold1(maxThresh_-minThresh_);
  vfatDevice_->setVThreshold2(max(0,maxThresh_));
  scanParams_.bag.deviceVT1 = vfatDevice_->getVThreshold1();
  scanParams_.bag.deviceVT2 = vfatDevice_->getVThreshold2();

  scanParams_.bag.latency = vfatDevice_->getLatency();
  is_configured_ = true;
  hw_semaphore_.give();

  if (histo) {
    delete histo;
    histo = 0;
  }
  std::stringstream histName, histTitle;
  histName  << "allchannels";
  histTitle << "Threshold scan for all channels";

  int minTh = scanParams_.bag.minThresh;
  int maxTh = scanParams_.bag.maxThresh;
  int nBins = ((maxTh - minTh) + 1)/(scanParams_.bag.stepSize);

  LOG4CPLUS_DEBUG(getApplicationLogger(),"histogram name and title: " << histName.str() 
		  << ", " << histTitle.str()
		  << "(" << nBins << " bins)");
  histo = new TH1F(histName.str().c_str(), histTitle.str().c_str(), nBins, minTh-0.5, maxTh+0.5);
  
  for (unsigned int hi = 0; hi < 128; ++hi) {
    if (histos[hi]) {
      delete histos[hi];
      histos[hi] = 0;
    }
    
    histName.clear();
    histName.str(std::string());
    histTitle.clear();
    histTitle.str(std::string());

    histName  << "channel"<<(hi+1);
    histTitle << "Threshold scan for channel "<<(hi+1);
    LOG4CPLUS_DEBUG(getApplicationLogger(),"histogram name and title: " << histName.str() 
		   << ", " << histTitle.str()
		   << "(" << nBins << " bins)");
    histos[hi] = new TH1F(histName.str().c_str(), histTitle.str().c_str(), nBins, minTh-0.5, maxTh+0.5);
  }
  outputCanvas = new TCanvas("outputCanvas","outputCanvas",600,800);

  is_working_    = false;
}


void gem::supervisor::tbutils::ThresholdScan::startAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {
  
  is_working_ = true;

  //AppHeader ah;

  latency_   = scanParams_.bag.latency;
  nTriggers_ = confParams_.bag.nTriggers;
  stepSize_  = scanParams_.bag.stepSize;
  minThresh_ = scanParams_.bag.minThresh;
  maxThresh_ = scanParams_.bag.maxThresh;

  time_t now = time(0);
  tm *gmtm = gmtime(&now);
  char* utcTime = asctime(gmtm);

  std::string tmpFileName = "ThresholdScan_";
  tmpFileName.append(utcTime);
  tmpFileName.erase(std::remove(tmpFileName.begin(), tmpFileName.end(), '\n'), tmpFileName.end());
  tmpFileName.append(".dat");
  std::replace(tmpFileName.begin(), tmpFileName.end(), ' ', '_' );
  std::replace(tmpFileName.begin(), tmpFileName.end(), ':', '-');

  confParams_.bag.outFileName = tmpFileName;

  LOG4CPLUS_INFO(getApplicationLogger(),"Creating file " << confParams_.bag.outFileName.toString());

  std::ofstream scanStream(tmpFileName.c_str(), std::ios::app | std::ios::binary);

  if (scanStream.is_open()){
    LOG4CPLUS_INFO(getApplicationLogger(),"::startAction " 
        << "file " << confParams_.bag.outFileName.toString() << " opened");
  }

  // Setup Scan file, information header
  tmpFileName = "ScanSetup_";
  tmpFileName.append(utcTime);
  tmpFileName.erase(std::remove(tmpFileName.begin(), tmpFileName.end(), '\n'), tmpFileName.end());
  tmpFileName.append(".txt");
  std::replace(tmpFileName.begin(), tmpFileName.end(), ' ', '_' );
  std::replace(tmpFileName.begin(), tmpFileName.end(), ':', '-');
  confParams_.bag.outFileName = tmpFileName;

  LOG4CPLUS_DEBUG(getApplicationLogger(),"::startAction " 
		  << "Created ScanSetup file " << tmpFileName );

  std::ofstream scanSetup(tmpFileName.c_str(), std::ios::app );
  if (scanSetup.is_open()){
    LOG4CPLUS_INFO(getApplicationLogger(),"::startAction " 
        << "file " << tmpFileName << " opened and closed");
    scanSetup << "\n The Time & Date : " << utcTime << endl;
    scanSetup << " ChipID        0x" << hex << confParams_.bag.deviceChipID << dec << endl;
    scanSetup << " Latency       " << latency_ << endl;
    scanSetup << " nTriggers     " << nTriggers_  << endl;
    scanSetup << " stepSize      " << stepSize_ << endl;
    scanSetup << " minThresh     " << minThresh_ << endl;
    scanSetup << " maxThresh     " << maxThresh_ << endl;
    }
  scanSetup.close();
  
  //char data[128/8]
  is_running_ = true;
  hw_semaphore_.take();

  /*
  //set clock source
  vfatDevice_->setDeviceBaseNode("OptoHybrid.CLOCKING");
  vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"VFAT.SOURCE",  0x0); // 0x1
  //vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"VFAT.FALLBACK",0x1);
  vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"CDCE.SOURCE",  0x0);
  //vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"CDCE.FALLBACK",0x1);
  */

  //send resync
  vfatDevice_->setDeviceBaseNode("OptoHybrid.FAST_COM");
  vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"Send.Resync",0x1);

  //reset counters
  vfatDevice_->setDeviceBaseNode("OptoHybrid.COUNTERS");
  vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"RESETS.L1A.External",0x1);
  vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"RESETS.L1A.Internal",0x1);
  vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"RESETS.L1A.Delayed", 0x1);
  vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"RESETS.L1A.Total",   0x1);

  vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"RESETS.CalPulse.External",0x1);
  vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"RESETS.CalPulse.Internal",0x1);
  vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"RESETS.CalPulse.Total",   0x1);

  vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"RESETS.Resync",0x1);
  vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"RESETS.BC0",   0x1);
  
  //flush FIFO
  vfatDevice_->setDeviceBaseNode("GLIB.LINK1");
  vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"TRK_FIFO.FLUSH", 0x1);
  
  //set trigger source
  /*
  vfatDevice_->setDeviceBaseNode("OptoHybrid.TRIGGER");
  vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"SOURCE",   0x2);
  vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"TDC_SBits",(unsigned)confParams_.bag.deviceNum);
  */

  vfatDevice_->setDeviceBaseNode("GLIB");
  vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"TDC_SBits",(unsigned)confParams_.bag.deviceNum);
  
  vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());

  vfatDevice_->setVThreshold1(maxThresh_-minThresh_);
  vfatDevice_->setVThreshold2(max(0,maxThresh_));
  scanParams_.bag.deviceVT1 = vfatDevice_->getVThreshold1();
  scanParams_.bag.deviceVT2 = vfatDevice_->getVThreshold2();

  scanParams_.bag.latency = vfatDevice_->getLatency();

  vfatDevice_->setRunMode(1);
  hw_semaphore_.give();

  //start readout
  scanStream.close();

  if (histo) {
    delete histo;
    histo = 0;
  }
  std::stringstream histName, histTitle;
  histName  << "allchannels";
  histTitle << "Threshold scan for all channels";
  int minTh = scanParams_.bag.minThresh;
  int maxTh = scanParams_.bag.maxThresh;
  int nBins = ((maxTh - minTh) + 1)/(scanParams_.bag.stepSize);

  /*
  //write Applicatie  header
  ah.minTh = minTh;
  ah.maxTh = maxTh;
  ah.stepSize = scanParams_.bag.stepSize;
  keepAppHeader(tmpFileName, ah);
  */

  histo = new TH1F(histName.str().c_str(), histTitle.str().c_str(), nBins, minTh-0.5, maxTh+0.5);
  
  for (unsigned int hi = 0; hi < 128; ++hi) {
    LOG4CPLUS_INFO(getApplicationLogger(),"histos[" << hi << "] = 0x" << std::hex << histos[hi] << std::dec);
    if (histos[hi]) {
      delete histos[hi];
      histos[hi] = 0;
    }
    
    histName.clear();
    histName.str(std::string());
    histTitle.clear();
    histTitle.str(std::string());

    histName  << "channel"<<(hi+1);
    histTitle << "Threshold scan for channel "<<(hi+1);
    histos[hi] = new TH1F(histName.str().c_str(), histTitle.str().c_str(), nBins, minTh-0.5, maxTh+0.5);
  }

  //start scan routine
  wl_->submit(runSig_);
  
  is_working_ = false;
}


void gem::supervisor::tbutils::ThresholdScan::resetAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {

  is_working_ = true;
  gem::supervisor::tbutils::GEMTBUtil::resetAction(e);
  
  scanParams_.bag.latency   = 12U;
  scanParams_.bag.minThresh = -80;
  scanParams_.bag.maxThresh = 20;
  scanParams_.bag.stepSize  = 5U;
  scanParams_.bag.deviceVT1 = 0x0;
  scanParams_.bag.deviceVT2 = 0x0;
  
  is_working_     = false;
}

