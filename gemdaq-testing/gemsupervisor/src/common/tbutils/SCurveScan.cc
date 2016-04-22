#include "gem/supervisor/tbutils/SCurveScan.h"

#include "gem/readout/GEMDataAMCformat.h"
#include "gem/readout/GEMDataParker.h"

#include "gem/hw/vfat/HwVFAT2.h"
#include "gem/hw/glib/HwGLIB.h"
#include "gem/hw/optohybrid/HwOptoHybrid.h"

#include "gem/utils/GEMLogging.h"

#include "TH1D.h"
#include "TH2.h"
#include "TFile.h"
#include "TCanvas.h"
#include "TROOT.h"
#include "TString.h"
#include "TError.h"

#include <algorithm>
#include <iomanip>
#include <ctime>
#include <queue>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

#include "cgicc/HTTPRedirectHeader.h"
#include "gem/supervisor/tbutils/VFAT2XMLParser.h"
#include "TStopwatch.h"

#include "xoap/MessageReference.h"
#include "xoap/MessageFactory.h"
#include "xoap/SOAPEnvelope.h"
#include "xoap/SOAPConstants.h"
#include "xoap/SOAPBody.h"
#include "xoap/Method.h"
#include "xoap/AttachmentPart.h"

XDAQ_INSTANTIATOR_IMPL(gem::supervisor::tbutils::SCurveScan)

typedef gem::readout::GEMDataAMCformat::GEMData  AMCGEMData;
typedef gem::readout::GEMDataAMCformat::GEBData  AMCGEBData;
typedef gem::readout::GEMDataAMCformat::VFATData AMCVFATData;

//
void gem::supervisor::tbutils::SCurveScan::ConfigParams::registerFields(xdata::Bag<ConfigParams> *bag)
{

  minVCal         = 0U;
  maxVCal         = 255U;
  stepSize        = 1U;
  nTriggers       = 10U;
  deviceVT1       = 25;
  deviceVT2       = 0;
  MSPulseLength   = 3;
  Latency         = 14U;
    
  bag->addField("minVCal",    &minVCal   );
  bag->addField("maxVCal",    &maxVCal   );
  bag->addField("stepSize",      &stepSize     );
  bag->addField("VT2",           &deviceVT1    );
  bag->addField("VT1",           &deviceVT2    );
  bag->addField("Latency",          &Latency         );
  bag->addField("MSPulseLength", &MSPulseLength);
  bag->addField("nTriggers",     &nTriggers    );

}

gem::supervisor::tbutils::SCurveScan::SCurveScan(xdaq::ApplicationStub * s)  throw (xdaq::exception::Exception) :

  gem::supervisor::tbutils::GEMTBUtil(s)
{

  getApplicationInfoSpace()->fireItemAvailable("scanParams", &m_scanParams);
  getApplicationInfoSpace()->fireItemValueRetrieve("scanParams", &m_scanParams);

  // HyperDAQ bindings
  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::SCurveScan::webDefault,      "Default"    );
  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::SCurveScan::webConfigure,    "Configure"  );
  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::SCurveScan::webStart,        "Start"      );
  runSig_   = toolbox::task::bind(this, &SCurveScan::run,        "run"       );
  readSig_  = toolbox::task::bind(this, &SCurveScan::readFIFO,   "readFIFO"  );

  // Initiate and activate main workloop
  wl_ = toolbox::task::getWorkLoopFactory()->getWorkLoop("urn:xdaq-workloop:GEMTestBeamSupervisor:SCurveScan","waiting");
  wl_->activate();

  m_currentVCal = 0;
  m_vfat = 0;
  m_event = 0;
  m_eventsSeen = 0;
  m_channelSeen=0;
}

gem::supervisor::tbutils::SCurveScan::~SCurveScan()
{
  wl_ = toolbox::task::getWorkLoopFactory()->getWorkLoop("urn:xdaq-workloop:GEMTestBeamSupervisor:SCurveScan","waiting");
  //should we check to see if it's running and try to stop?
  wl_->cancel();
  wl_ = 0;
}
bool gem::supervisor::tbutils::SCurveScan::run(toolbox::task::WorkLoop* wl)
{
  wl_semaphore_.take(); // take workloop
  if (!is_running_) {
    wl_semaphore_.give(); // give work loop if it is not running
    //++m_confParamsbag.triggercount;
    uint32_t bufferDepth = 0;
    bufferDepth = p_glibDevice->getFIFOVFATBlockOccupancy(m_readout_mask);
    if (bufferDepth>0) {
      wl_->submit(readSig_);
    }
    LOG4CPLUS_INFO(getApplicationLogger()," ******IT IS NOT RUNNIG ***** ");
    return false;
  }
  
  hw_semaphore_.take();//take hw to set the trigger source, send L1A+Cal pulses,

  //set trigger source  
  if(m_confParams.bag.triggerSource.value_ == 0x0){
    p_optohybridDevice->setTrigSource(0x0);//from AMC13   
  }else if(m_confParams.bag.triggerSource.value_ == 0x1){
    p_optohybridDevice->setTrigSource(0x1);//from T1   
  }else if(m_confParams.bag.triggerSource.value_ == 0x2){
    p_optohybridDevice->setTrigSource(0x2);//from sbits   
  }else if(m_confParams.bag.triggerSource.value_ == 0x3){
    p_optohybridDevice->setTrigSource(0x3);//from Ext_LEMO   
  }



  //send L1A and Calpulse
  if((unsigned)m_confParams.bag.triggerSource.value_ == (unsigned)0x1){
    p_optohybridDevice->setTrigSource(0x1);//from T1   
    p_optohybridDevice->sendL1ACal(1,15);  //from T1 generator
    INFO("SEND CALPULSE + L1A" <<  p_optohybridDevice->getL1ACount(0x1));
    sleep(0.5);
  }
  
  //count triggers and Calpulses coming from TTC
  m_confParams.bag.triggersSeen =  p_optohybridDevice->getL1ACount(0x1);
  m_CalPulseCount[0] = p_optohybridDevice->getCalPulseCount(0x1); 

  hw_semaphore_.give();//give hw to set the trigger source, send L1A+Cal pulses,
  
  // if triggersSeen < N triggers
  if ((uint64_t)(m_confParams.bag.triggersSeen) < (uint64_t)(m_confParams.bag.nTriggers)) {

    hw_semaphore_.take();//take hw. glib buffer depth

    // Get the size of GLIB data buffer
    uint32_t bufferDepth = 0;
    bufferDepth = p_glibDevice->getFIFOVFATBlockOccupancy(m_readout_mask);

    hw_semaphore_.give();//give hw. glib buffer depth 
    wl_semaphore_.give();//give workloop to read
    
    if (bufferDepth>0) {
      ++m_confParams.bag.triggercount;
      wl_->submit(readSig_);
    }
    return true;
  }// end triggerSeen < N triggers
  else { 
    //disable triggers
    /*    if(m_confParams.bag.triggerSource.value_ == 0x1){
	  p_optohybridDevice->stopT1Generator(true);
	  } else { 
	  p_optohybridDevice->setTrigSource(0x1);       
	  }
	  sleep(1);
    */

    hw_semaphore_.take(); //take hw to set Runmode 0 on VFATs 
    for (auto chip = p_vfatDevice.begin(); chip != p_vfatDevice.end(); ++chip) {
      (*chip)->setRunMode(0);
    }// end for  

    uint32_t bufferDepth = 0;
    bufferDepth = p_glibDevice->getFIFOVFATBlockOccupancy(m_readout_mask);

    hw_semaphore_.give(); //give hw to set Runmode 0 on VFATs       
    wl_semaphore_.give(); //give workloop to read

    if (bufferDepth>0) {
      ++m_confParams.bag.triggercount;
      wl_->submit(readSig_);
    }
    /*      for (auto chip = p_vfatDevice.begin(); chip != p_vfatDevice.end(); ++chip) {
	    wl_->submit(readSig_);
	    }// end for*/  
  
    hw_semaphore_.take();// take hw to reset counters
    wl_semaphore_.take();// take workloop after reading

    //FELIPE
    // flush FIFO, how to disable a specific, misbehaving, chip
  
    //reset counters
    p_optohybridDevice->resetL1ACount(0x1);
    p_optohybridDevice->resetResyncCount();
    p_optohybridDevice->resetBC0Count();
    p_optohybridDevice->resetCalPulseCount(0x1);
    p_optohybridDevice->sendResync();     
    p_optohybridDevice->sendBC0();          
    
    hw_semaphore_.give(); // give hw to reset counters

    INFO(" Scan point TiggersSeen " << m_confParams.bag.triggersSeen );
  
    //if max VCal - current VCal >= stepsize
    if (m_scanParams.bag.maxVCal - m_currentVCal >= m_scanParams.bag.stepSize) {

      hw_semaphore_.take();// vfat set vcal

      if ((m_currentVCal + m_scanParams.bag.stepSize) < 0xFF) {
        
        // do this with an OH broadcast write
        p_optohybridDevice->broadcastWrite("VCal", 0x0, m_currentVCal + m_scanParams.bag.stepSize);
      } else  { 

        p_optohybridDevice->broadcastWrite("VCal", 0x0, 0xFF);
      }//end else
    
      for (auto chip = p_vfatDevice.begin(); chip != p_vfatDevice.end(); ++chip) {
	m_currentVCal = (*chip)->getVCal();
	m_scanParams.bag.deviceVT1 = (*chip)->getVThreshold1();
	m_scanParams.bag.deviceVT2 = (*chip)->getVThreshold2();
	(*chip)->setRunMode(1);      
      }
        
      //set trigger source  
      if(m_confParams.bag.triggerSource.value_ == 0x0){
	p_optohybridDevice->setTrigSource(0x0);//from AMC13   
      }else if(m_confParams.bag.triggerSource.value_ == 0x1){
	p_optohybridDevice->setTrigSource(0x1);//from T1   
      }else if(m_confParams.bag.triggerSource.value_ == 0x2){
	p_optohybridDevice->setTrigSource(0x2);//from sbits   
      }else if(m_confParams.bag.triggerSource.value_ == 0x3){
	p_optohybridDevice->setTrigSource(0x3);//from Ext_LEMO   
      }
    
      //setting counters  = 0
      m_CalPulseCount[0] = 0;	  
      m_confParams.bag.triggersSeen =  0;
      m_eventsSeen   = 0;  
      m_channelSeen = 0;
 
      hw_semaphore_.give(); // give hw vfat set vcal
      wl_semaphore_.give(); // end of workloop	

      return true;	
    } // end if maxVCal - currentVCal >= step
    else {

      wl_semaphore_.give();  // end of workloop	
      wl_->submit(stopSig_);
      return false; 
    }//end else
	  
    return true;
  }//end else triggerseen < N triggers

  return true;
}//end run



bool gem::supervisor::tbutils::SCurveScan::readFIFO(toolbox::task::WorkLoop* wl)    
{

  wl_semaphore_.take();
  hw_semaphore_.take();//glib getFIFO

  LOG4CPLUS_INFO(getApplicationLogger(), " CurentVCal " << (int)m_currentVCal
		 << " TrigSeen " << m_confParams.bag.triggersSeen 
		 << " CalPulses " << m_CalPulseCount[0] 
		 << " eventsSeen " << m_eventsSeen
		 << "channelSeen " << m_channelSeen 
		 ); 

  uint8_t vcal = m_currentVCal;
  uint8_t vt1 = m_scanParams.bag.deviceVT1;
  uint8_t vt2 = m_scanParams.bag.deviceVT2;
  
  /// bool tells code we are running SCurve Scan and not Latency Scan
  dumpRoutinesData(m_readout_mask, vcal, vt1, vt2, false);


  hw_semaphore_.give();
  wl_semaphore_.give();

  return false;
}



void gem::supervisor::tbutils::SCurveScan::scanParameters(xgi::Output *out)
  throw (xgi::exception::Exception)
{
  try {
    *out << cgicc::span()   << std::endl //open span

	 << cgicc::label("MinVCan").set("for","MinVCal") << std::endl
	 << cgicc::input().set("id","MinVCal").set("name","MinVCal")
      .set("type","number").set("min","0").set("max","255")
      .set("value",boost::str(boost::format("%d")%(m_scanParams.bag.minVCal)))
	 << std::endl

	 << cgicc::label("MaxVCal").set("for","MaxVCal") << std::endl
	 << cgicc::input().set("id","MaxVCal").set("name","MaxVCal")
      .set("type","number").set("min","0").set("max","255")
      .set("value",boost::str(boost::format("%d")%(m_scanParams.bag.maxVCal)))
	 << std::endl

	 << cgicc::label("VCal Step").set("for","VCalStep") << std::endl
	 << cgicc::input().set("id","VCalStep").set("name","VCalStep")
      .set("type","number").set("min","1").set("max","255")
      .set("value",boost::str(boost::format("%d")%(m_scanParams.bag.stepSize)))
	 << std::endl

	 << cgicc::br() << std::endl

	 << cgicc::label("Current VCal").set("for","CurrentVCal") << std::endl
	 << cgicc::input().set("id","CurrentVCal").set("name","CurrentVCal")
      .set("type","text").set("readonly")
      .set("value",boost::str(boost::format("%d")%((unsigned)m_currentVCal)))
	 << std::endl

	 << cgicc::br() << std::endl

	 << cgicc::label("Triggers to take").set("for","TriggersToTake") << std::endl
	 << cgicc::input().set("id","TriggersToTake").set("name","TriggersToTake")
      .set("type","number").set("min","0")
      .set("value",boost::str(boost::format("%d")%(m_confParams.bag.nTriggers)))
	 << std::endl

	 << cgicc::label("Triggers taken").set("for","TriggersSeen") << std::endl
	 << cgicc::input().set("id","TriggersSeen").set("name","TriggersSeen")
      .set("type","text").set("readonly")
      .set("value",boost::str(boost::format("%d")%(m_confParams.bag.triggersSeen)))
      //.set("value",boost::str(boost::format("%d")%(triggersSeen_)))
	 << std::endl

	 << cgicc::br() << std::endl

	 << cgicc::label("Set Threshold (VT2-VT1):  VT1 ").set("for","VT1") << std::endl
	 << cgicc::input().set("id","VT1").set("name","VT1")
      .set("type","number").set("min","0").set("max","255")
      .set("value",boost::str(boost::format("%d")%(m_scanParams.bag.deviceVT1)))
	 << std::endl

	 << cgicc::label("VT2").set("for","VT2") << std::endl
	 << cgicc::input().set("id","VT2").set("name","VT2")
      .set("type","number").set("min","0").set("max","255")
      .set("value",boost::str(boost::format("%d")%(m_scanParams.bag.deviceVT2)))
	 << std::endl
      
	 << cgicc::br()  // << std::endl

	 << cgicc::label("Latency").set("for","Latency") << std::endl
	 << cgicc::input().set("id","Latency").set("name","Latency")
      .set("type","number").set("min","0").set("max","255")
      .set("value",boost::str(boost::format("%d")%(m_scanParams.bag.Latency)))
	 << std::endl

      	 << cgicc::label("MSPulseLength").set("for","MSPulseLength") << std::endl
	 << cgicc::input().set("id","MSPulseLength").set("name","MSPulseLength")
      .set("type","number").set("min","0").set("max","3")
      .set("value",boost::str(boost::format("%d")%(m_scanParams.bag.MSPulseLength)))
	 << std::endl
	 << cgicc::br()   
	 << cgicc::span() << std::endl; //end span
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

//
void gem::supervisor::tbutils::SCurveScan::webDefault(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception)
{

  try {

    ////update the page refresh 
    if (!is_working_ && !is_running_) {
    }
    else if (is_working_) {
      cgicc::HTTPResponseHeader &head = out->getHTTPResponseHeader();
      head.addHeader("Refresh","2");
    }
    else if (is_running_) {
      cgicc::HTTPResponseHeader &head = out->getHTTPResponseHeader();
      head.addHeader("Refresh","5");
    }

    //generate the control buttons and display the ones that can be touched depending on the run mode
    *out << "<div class=\"xdaq-tab-wrapper\">"            << std::endl; //open all tabs
    *out << "<div class=\"xdaq-tab\" title=\"Control\">"  << std::endl; // open control tab

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
      //have a menu for selecting the OH and VFAT
      *out << cgicc::form().set("method","POST").set("action", "/" + getApplicationDescriptor()->getURN() + "/Initialize") << std::endl;

      selectOptohybridDevice(out);
      selectMultipleVFAT(out);
      selectMultipleChannels(out);
      scanParameters(out);

      *out << cgicc::input().set("type", "submit")
	.set("name", "command").set("title", "Initialize hardware acces.")
	.set("value", "Initialize") << std::endl;

      *out << cgicc::form() << std::endl;
     }

    else if (!is_configured_) {
      //this will allow the parameters to be set to the chip and scan routine and the trigger source
      *out << cgicc::form().set("method","POST").set("action", "/" + getApplicationDescriptor()->getURN() + "/Configure") << std::endl;

      selectOptohybridDevice(out);
      selectMultipleVFAT(out);
      selectMultipleChannels(out);
      selectTrigSource(out);
      scanParameters(out);

      *out << cgicc::input().set("type","text").set("name","xmlFilename").set("size","80")
	.set("ENCTYPE","multipart/form-data").set("readonly")
	.set("value",m_confParams.bag.settingsFile.toString()) << std::endl;

      *out << cgicc::br() << std::endl;
      *out << cgicc::input().set("type", "submit")
	.set("name", "command").set("title", "Configure scan.")
	.set("value", "Configure") << std::endl;
      *out << cgicc::form()        << std::endl;
    }

    else if (!is_running_) {
     
      //hardware is initialized and configured, we can start the run
      *out << cgicc::form().set("method","POST").set("action", "/" + getApplicationDescriptor()->getURN() + "/Start") << std::endl;

      selectOptohybridDevice(out);
      selectMultipleVFAT(out);
      selectMultipleChannels(out);
      selectTrigSource(out);
      scanParameters(out);

      *out << cgicc::input().set("type", "submit")
	.set("name", "command").set("title", "Start scan.")
	.set("value", "Start") << std::endl;
      *out << cgicc::form()    << std::endl;
    }

    else if (is_running_) {
      *out << cgicc::form().set("method","POST").set("action", "/" + getApplicationDescriptor()->getURN() + "/Stop") << std::endl;

      selectOptohybridDevice(out);
      selectMultipleVFAT(out);
      selectMultipleChannels(out);
      selectTrigSource(out);
      scanParameters(out);

      *out << cgicc::input().set("type", "submit")
	.set("name", "command").set("title", "Stop scan.")
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
      .set("name", "command").set("title", "Halt scan.")
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

    *out << "</div>" << std::endl;//close control

    *out << "<div class=\"xdaq-tab\" title=\"Counters\">"  << std::endl; // open counters
    if (is_initialized_)
      showCounterLayout(out);
    *out << "</div>" << std::endl;//close counters

    *out << "<div class=\"xdaq-tab\" title=\"Fast Commands/Trigger Setup\">"  << std::endl;//open fast commands
    if (is_initialized_)
      fastCommandLayout(out);
    *out << "</div>" << std::endl; //close fastcommands

    *out << "</div>" << std::endl;//close total tabs

    *out << cgicc::br() << cgicc::br() << std::endl;

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

  }//end try
  catch (const xgi::exception::Exception& e) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Something went wrong displaying SCurveScan control panel(xgi): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  catch (const std::exception& e) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Something went wrong displaying SCurveScan control panel(std): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
}

//
void gem::supervisor::tbutils::SCurveScan::webConfigure(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception) {

  try {
    cgicc::Cgicc cgi(in);

    //sending SOAP message
    sendMessage(in,out);

    //aysen's xml parser
    m_confParams.bag.settingsFile = cgi.getElement("xmlFilename")->getValue();

    cgicc::const_form_iterator element  = cgi.getElement("MinVCal");
    if (element != cgi.getElements().end())
      m_scanParams.bag.minVCal = element->getIntegerValue();

    element = cgi.getElement("MaxVCal");
    if (element != cgi.getElements().end())
      m_scanParams.bag.maxVCal = element->getIntegerValue();

    element = cgi.getElement("VCalStep");
    if (element != cgi.getElements().end())
      m_scanParams.bag.stepSize  = element->getIntegerValue();

    element = cgi.getElement("TriggersToTake");
    if (element != cgi.getElements().end())
      m_confParams.bag.nTriggers  = element->getIntegerValue();

    element = cgi.getElement("Latency");
    if (element != cgi.getElements().end())
      m_scanParams.bag.Latency  = element->getIntegerValue();

    element = cgi.getElement("MSPulseLength");
    if (element != cgi.getElements().end())
      m_scanParams.bag.MSPulseLength  = element->getIntegerValue();


    cgicc::form_iterator new_triggersource = cgi.getElement("SetTrigSrc");
    INFO("triggersource output : " << (**new_triggersource).c_str());
    
    if (strcmp((**new_triggersource).c_str(),"Calpulse+L1A") == 0) {
      m_confParams.bag.triggerSource = 0x1;
      p_optohybridDevice->setTrigSource(0x1);//from T1   
      INFO("Fake SCurve Scan sending Calpulses+L1As. TrigSource : " << m_confParams.bag.triggerSource);    
    }
    if (strcmp((**new_triggersource).c_str(),"Internal loopback of s-bits") == 0) {
      m_confParams.bag.triggerSource = 0x3;
      p_optohybridDevice->setTrigSource(0x3);//from sbits   
      INFO("Sending Calpulses and the s-bits come back from the OH. TrigSource : " << m_confParams.bag.triggerSource );     
    }
    if (strcmp((**new_triggersource).c_str(),"External Trigger from AMC13") == 0) {
      m_confParams.bag.triggerSource= 0x0;
      p_optohybridDevice->setTrigSource(0x0);//from AMC13
      INFO("Real signals and the trigger comes from the AMC13. TrigSource : " << m_confParams.bag.triggerSource );   
    }
    if (strcmp((**new_triggersource).c_str(),"External Trigger from LEMO") == 0) {
      m_confParams.bag.triggerSource = 0x2;
      p_optohybridDevice->setTrigSource(0x2);//from Ext_LEMO   
      INFO("Real signals and the trigger comes from the LEMO Cable. TrigSource : " << m_confParams.bag.triggerSource );
    }

  }//end try
  catch (const xgi::exception::Exception & e) {
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  catch (const std::exception & e) {
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }

  // Initiate configure workloop 
  wl_->submit(confSig_);

  // Go back to main web interface
  redirect(in,out);
}

//
void gem::supervisor::tbutils::SCurveScan::webStart(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception) {
  // Initiate start workloop
  wl_->submit(startSig_);
  // Go back to main web interface
  redirect(in,out);
}

void gem::supervisor::tbutils::SCurveScan::configureAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {

  is_working_ = true;
  m_stepSize   = m_scanParams.bag.stepSize;
  m_minVCal = m_scanParams.bag.minVCal;
  m_maxVCal = m_scanParams.bag.maxVCal;

  m_vfat = 0;
  m_event = 0;
  m_sumVFAT = 0;
  m_counter = {0,0,0,0,0};
  m_eventsSeen = 0;
  m_channelSeen = 0;
  m_confParams.bag.triggercount = 0;

  hw_semaphore_.take();
  LOG4CPLUS_INFO(getApplicationLogger(), "attempting to configure device");


  //set trigger source
  INFO("ConfigureAction Trigger Source : " << m_confParams.bag.triggerSource );   
  if(m_confParams.bag.triggerSource.value_ == 0x0){
    p_optohybridDevice->setTrigSource(0x0);//from AMC13
    INFO("Real signals and the trigger comes from the AMC13. TrigSource : " << m_confParams.bag.triggerSource );   
  }else if(m_confParams.bag.triggerSource.value_ == 0x1){
    p_optohybridDevice->setTrigSource(0x1);//from T1   
    INFO("Fake SCurve Scan sending Calpulses+L1As. TrigSource : " << m_confParams.bag.triggerSource);
  }else if(m_confParams.bag.triggerSource.value_ == 0x3){
    p_optohybridDevice->setTrigSource(0x3);//from sbits   
    INFO("Sending Calpulses and the s-bits come back from the OH. TrigSource : " << m_confParams.bag.triggerSource );
  }else if(m_confParams.bag.triggerSource.value_ == 0x2){
    p_optohybridDevice->setTrigSource(0x2);//from Ext_LEMO   
    INFO("Real signals and the trigger comes from the LEMO Cable. TrigSource : " << m_confParams.bag.triggerSource );
  }

  //make sure device is not running
  for (auto chip = p_vfatDevice.begin(); chip != p_vfatDevice.end(); ++chip) {

    m_confParams.bag.deviceChipID = (*chip)->getChipID();
    (*chip)->setDeviceIPAddress(m_confParams.bag.deviceIP);
    (*chip)->setRunMode(0);
    
    LOG4CPLUS_INFO(getApplicationLogger(),"loading default settings");
    //default settings for the frontend
    (*chip)->setTriggerMode(    0x3); //set to S1 to S8
    (*chip)->setCalibrationMode(0x0); //set to normal
    (*chip)->setMSPolarity(     0x1); //negative
    (*chip)->setCalPolarity(    0x1); //negative

    (*chip)->setProbeMode(        0x0);
    (*chip)->setLVDSMode(         0x0);
    (*chip)->setHitCountCycleTime(0x0); //maximum number of bits

    (*chip)->setHitCountMode( 0x0);
    (*chip)->setMSPulseLength(m_scanParams.bag.MSPulseLength);
    (*chip)->setInputPadMode( 0x0);
    (*chip)->setTrimDACRange( 0x0);
    (*chip)->setBandgapPad(   0x0);
    (*chip)->sendTestPattern( 0x0);

    (*chip)->setLatency(m_scanParams.bag.Latency);


    //// ADD functionality to run on all channels
    
    LOG4CPLUS_INFO(getApplicationLogger(), "channelName[0]= " << m_confParams.bag.channelName[0].toString());
    LOG4CPLUS_INFO(getApplicationLogger(), "channelName[1]= " << m_confParams.bag.channelName[1].toString());
    bool runall=true;
    if(m_confParams.bag.channelName[0].toString().compare("All")!= 0) runall=false;
    

    for (int chan = 0; chan < 129; ++chan)
      if (runall){
	LOG4CPLUS_INFO(getApplicationLogger(), "Enabling CalPule to all channel");
	(*chip)->enableCalPulseToChannel(chan, true);
      }
      else{
	LOG4CPLUS_INFO(getApplicationLogger(), "Enabling CalPule to  channels 0,1 and 32");
	if (chan == 0 || chan == 1 || chan == 32)
	  (*chip)->enableCalPulseToChannel(chan, true);
	else
	  (*chip)->enableCalPulseToChannel(chan, false);
      }
    
    (*chip)->setIPreampIn(  168);
    (*chip)->setIPreampFeed(150);
    (*chip)->setIPreampOut(  80);
    (*chip)->setIShaper(    150);
    (*chip)->setIShaperFeed(100);
    (*chip)->setIComp(       75);//120

    (*chip)->setVThreshold1(m_scanParams.bag.deviceVT1);//50
    (*chip)->setVThreshold2(m_scanParams.bag.deviceVT2);//0

    //
    //    int islot = slotInfo->GEBslotIndex( (uint32_t)((*chip)->getChipID()));


    LOG4CPLUS_INFO(getApplicationLogger(), "setting DAC mode to normal");
    (*chip)->setDACMode(gem::hw::vfat::StringToDACMode.at("OFF"));

    LOG4CPLUS_INFO(getApplicationLogger(), "setting starting vcal value");
    (*chip)->setVCal(    m_scanParams.bag.minVCal);

    LOG4CPLUS_INFO(getApplicationLogger(), "reading back curren vcal value");
    m_currentVCal = (*chip)->getVCal();

    LOG4CPLUS_INFO(getApplicationLogger(), "Threshold " << m_scanParams.bag.deviceVT1);

    LOG4CPLUS_INFO(getApplicationLogger(), "Latency " << m_scanParams.bag.Latency);

    LOG4CPLUS_INFO(getApplicationLogger(), "device configured");
    is_configured_ = true;
  }

  //flush fifo
  INFO("Flushing the FIFOs, m_readout_mask 0x" <<std::hex << (int)m_readout_mask << std::dec);
  DEBUG("Flushing FIFO" << m_readout_mask << " (depth " << p_glibDevice->getFIFOOccupancy(m_readout_mask));
  p_glibDevice->flushFIFO(m_readout_mask);
  while (p_glibDevice->hasTrackingData(m_readout_mask)) {
    p_glibDevice->flushFIFO(m_readout_mask);
    std::vector<uint32_t> dumping = p_glibDevice->getTrackingData(m_readout_mask,
                                                                 p_glibDevice->getFIFOVFATBlockOccupancy(m_readout_mask));
  }
  // once more for luck
  p_glibDevice->flushFIFO(m_readout_mask);

  //reset counters
  p_optohybridDevice->resetL1ACount(0x1);
  p_optohybridDevice->resetCalPulseCount(0x1);
  p_optohybridDevice->resetResyncCount();
  p_optohybridDevice->resetBC0Count();
  p_optohybridDevice->sendResync();      
  p_optohybridDevice->sendBC0();          
  //  p_vfatDevice->setRunMode(1);      

  hw_semaphore_.give();

  LOG4CPLUS_INFO(getApplicationLogger(), "configure routine completed");

  is_working_    = false;

}		 

//
void gem::supervisor::tbutils::SCurveScan::startAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {

  wl_semaphore_.take();
  is_working_ = true;

  //AppHeader ah;

  m_threshold  = m_scanParams.bag.deviceVT2 -m_scanParams.bag.deviceVT1;
  m_Latency    = m_scanParams.bag.Latency;
  m_mspl       = m_scanParams.bag.MSPulseLength;
  m_nTriggers  = m_confParams.bag.nTriggers;
  m_stepSize   = m_scanParams.bag.stepSize;
  m_minVCal    = m_scanParams.bag.minVCal;
  m_maxVCal    = m_scanParams.bag.maxVCal;

  time_t now = time(0);
  tm *gmtm = gmtime(&now);
  char* utcTime = asctime(gmtm);

  std::string tmpFileName = "SCurveScan_", tmpType = "", outputType   = "Hex";
  tmpFileName.append(utcTime);
  tmpFileName.erase(std::remove(tmpFileName.begin(), tmpFileName.end(), '\n'), tmpFileName.end());
  tmpFileName.append(".dat");
  std::replace(tmpFileName.begin(), tmpFileName.end(), ' ', '_' );
  std::replace(tmpFileName.begin(), tmpFileName.end(), ':', '-');

  std::string errFileName = "ERRORS_";
  errFileName.append(tmpFileName);
  //  errFileName.append(utcTime);
  errFileName.erase(std::remove(errFileName.begin(), errFileName.end(), '\n'), errFileName.end());
  //  errFileName.append(".dat");
  std::replace(errFileName.begin(), errFileName.end(), ' ', '_' );
  std::replace(errFileName.begin(), errFileName.end(), ':', '-');

  m_confParams.bag.outFileName = tmpFileName;

  LOG4CPLUS_INFO(getApplicationLogger(),"Creating file " << m_confParams.bag.outFileName.toString());

  std::ofstream scanStream(tmpFileName.c_str(),  std::ios::app | std::ios::binary);
  std::ofstream errf(errFileName.c_str(), std::ios_base::app | std::ios::binary );

  if (scanStream.is_open()){
    LOG4CPLUS_DEBUG(getApplicationLogger(),"file " << m_confParams.bag.outFileName.toString() << "opened");
  }

  // Book GEM Data Parker
  p_gemDataParker = std::shared_ptr<gem::readout::GEMDataParker>(new gem::readout::GEMDataParker(*p_glibDevice, tmpFileName, errFileName, outputType,  m_confParams.bag.slotFileName.toString()));
  
  // Setup Scan file, information header
  tmpFileName = "ScanSetup_";
  tmpFileName.append(utcTime);
  tmpFileName.erase(std::remove(tmpFileName.begin(), tmpFileName.end(), '\n'), tmpFileName.end());
  tmpFileName.append(".txt");
  std::replace(tmpFileName.begin(), tmpFileName.end(), ' ', '_' );
  std::replace(tmpFileName.begin(), tmpFileName.end(), ':', '-');
  m_confParams.bag.outFileName = tmpFileName;

  LOG4CPLUS_DEBUG(getApplicationLogger(),"::startAction " 
		  << "Created ScanSetup file " << tmpFileName );

  std::ofstream scanSetup(tmpFileName.c_str(), std::ios::app );
  if (scanSetup.is_open()){
    LOG4CPLUS_INFO(getApplicationLogger(),"::startAction " 
		   << "file " << tmpFileName << " opened and closed");

    scanSetup << "\n The Time & Date : " << utcTime << std::endl;
    scanSetup << " ChipID        0x" << std::hex    << m_confParams.bag.deviceChipID << std::dec << std::endl;
    scanSetup << " Threshold     " << m_threshold   << std::endl;
    scanSetup << " Latency       " << m_Latency     << std::endl;
    scanSetup << " MSPulseLength " << m_mspl        << std::endl;
    scanSetup << " nTriggers     " << m_nTriggers   << std::endl;
    scanSetup << " stepSize      " << m_stepSize    << std::endl;
    scanSetup << " minVCal       " << m_minVCal     << std::endl;
    scanSetup << " maxVCal       " << m_maxVCal     << std::endl;
  }
  scanSetup.close();
  
  //char data[128/8]
  is_running_ = true;
  hw_semaphore_.take();//oh reset counters

  //set trigger source
  INFO("Start Trigger Source : " << m_confParams.bag.triggerSource );   
  if(m_confParams.bag.triggerSource.value_ == 0x0){
    p_optohybridDevice->setTrigSource(0x0);//from AMC13
  }else if(m_confParams.bag.triggerSource.value_ == 0x1){
    p_optohybridDevice->setTrigSource(0x1);//from T1   
  }else if(m_confParams.bag.triggerSource.value_ == 0x3){
    p_optohybridDevice->setTrigSource(0x3);//from sbits   
  }else if(m_confParams.bag.triggerSource.value_ == 0x2){
    p_optohybridDevice->setTrigSource(0x2);//from Ext_LEMO   
  }

  //set clock source
  //p_optohybridDevice->setVFATClock(1,1,0x0);    
  //p_optohybridDevice->setCDCEClock(1,1,0x0); 

  p_optohybridDevice->sendResync();     
  p_optohybridDevice->sendBC0();          

  //reset counters
  p_optohybridDevice->resetL1ACount(0x1);
  p_optohybridDevice->resetResyncCount();
  p_optohybridDevice->resetBC0Count();
  p_optohybridDevice->resetCalPulseCount(0x1);


  //flush fifo
  INFO("Flushing the FIFOs, m_readout_mask 0x" <<std::hex << (int)m_readout_mask << std::dec);
  DEBUG("Flushing FIFO" << m_readout_mask << " (depth " << p_glibDevice->getFIFOOccupancy(m_readout_mask));
  p_glibDevice->flushFIFO(m_readout_mask);
  while (p_glibDevice->hasTrackingData(m_readout_mask)) {
    p_glibDevice->flushFIFO(m_readout_mask);
    std::vector<uint32_t> dumping = p_glibDevice->getTrackingData(m_readout_mask,
                                                                 p_glibDevice->getFIFOVFATBlockOccupancy(m_readout_mask));
  }
  // once more for luck
  p_glibDevice->flushFIFO(m_readout_mask);


  //send Resync
  p_optohybridDevice->sendResync();      
  p_optohybridDevice->sendBC0();         
  p_optohybridDevice->sendResync();       

  // set selected VFATs 
  for (auto chip = p_vfatDevice.begin(); chip != p_vfatDevice.end(); ++chip) {
    (*chip)->setRunMode(1);
  }

  hw_semaphore_.give();//end vfat
  
  wl_->submit(runSig_);
  
  /*  scanStream.close();
      errf.close();
  */

  is_working_ = false;

  wl_semaphore_.give();

}							      
//
void gem::supervisor::tbutils::SCurveScan::resetAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {

  is_working_ = true;
  gem::supervisor::tbutils::GEMTBUtil::resetAction(e);
  {

    m_confParams.bag.nTriggers     = 10U;
    m_scanParams.bag.minVCal       = 0U;
    m_scanParams.bag.maxVCal       = 255U;
    m_scanParams.bag.stepSize      = 1U;
    m_scanParams.bag.deviceVT1     = 25U;
    m_scanParams.bag.deviceVT2     = 0U;
    m_scanParams.bag.Latency       = 15;
    m_scanParams.bag.MSPulseLength = 3;
    m_confParams.bag.triggerSource = 8;
    // m_confParams.bag.deviceName   = "";
    // m_confParams.bag.deviceChipID = 0x0;

    is_working_     = false;
  }
}


void gem::supervisor::tbutils::SCurveScan::selectTrigSource(xgi::Output *out)
  throw (xgi::exception::Exception)
{
  try {
    bool isDisabled = false;
    if (is_running_ || is_configured_)
      isDisabled = true;

    cgicc::input triggersourceselection;

    *out << "<table>"     << std::endl
	 << "<tr>"   << std::endl //open
 	 << "<td>" << "Trigger Source Select: " << "</td>" << std::endl	 
	 << "</tr>"     << std::endl
	 << "<tr>" << std::endl; //close
    

    *out << "<tr>" << std::endl //open
	 << "<td>" << std::endl; //open

    if(isDisabled){
    }else{
      
      *out << triggersourceselection.set("type","radio").set("name","SetTrigSrc").set("id","T1_source").set("value","Calpulse+L1A")
	   << cgicc::label("Calpulse+L1A").set("checked","checked").set("value","Calpulse+L1A") << std::endl
	   << cgicc::br();
      
      *out << triggersourceselection.set("type","radio").set("name","SetTrigSrc").set("id","sbits").set("value","Internal loopback of s-bits")
	   << cgicc::label("Internal loopback of s-bits").set("checked","checked").set("value","Internal loopback of s-bits") << std::endl
	   << cgicc::br();
      
      *out << triggersourceselection.set("type","radio").set("name","SetTrigSrc").set("id","T1_source").set("value","External Trigger from AMC13")
	   << cgicc::label("External Trigger from AMC13").set("checked","checked").set("value","External Trigger from AMC13") << std::endl
	   << cgicc::br();
      
      *out << triggersourceselection.set("type","radio").set("name","SetTrigSrc").set("id","T1_source").set("value","External Trigger from LEMO")
	   << cgicc::label("External Trigger from LEMO").set("checked","checked").set("value","External Trigger from LEMO") << std::endl
	   << cgicc::br();
      
    }//    else    
    
    *out << "</td>"    << std::endl //close
	 << "</tr>"    << std::endl //close 
	 << "</table>" << std::endl; //close
    
    if(is_configured_){
      INFO("Trigger Source : " << m_confParams.bag.triggerSource);
    }
    
  }//end try
  catch (const xgi::exception::Exception& e) {
    INFO("Something went wrong setting the trigger source): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  catch (const std::exception& e) {
    INFO("Something went wrong setting the trigger source): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  
}// end void selectTrigSource


// Send SOAP message                      
void gem::supervisor::tbutils::SCurveScan::sendMessage(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception) {
  //  is_working_ = true;
  INFO("------------------The message has been sent Begging--------------------");
  xoap::MessageReference msg = xoap::createMessage();
  xoap::SOAPPart soap = msg->getSOAPPart();
  xoap::SOAPEnvelope envelope = soap.getEnvelope();
  xoap::SOAPBody body = envelope.getBody();
  xoap::SOAPName command = envelope.createName("onMessage","xdaq", "urn:xdaq-soap:3.0");
  body.addBodyElement(command);

  try 
    {
      xdaq::ApplicationDescriptor * d = getApplicationContext()->getDefaultZone()->getApplicationDescriptor("gem::hw::vfat::VFAT2Manager", 3);
      xdaq::ApplicationDescriptor * o = this->getApplicationDescriptor();
      xoap::MessageReference reply = getApplicationContext()->postSOAP(msg, *o,  *d);
    }
  catch (xdaq::exception::Exception& e)
    {
      INFO("------------------Fail sending message " << e.what());
      XCEPT_RETHROW (xgi::exception::Exception, "Cannot send message", e);
    }
  this->Default(in,out);
  INFO("------------------The message has been sent--------------------");
}      



// return message;
//}

       







