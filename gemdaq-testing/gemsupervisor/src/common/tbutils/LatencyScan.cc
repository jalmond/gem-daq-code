#include "gem/supervisor/tbutils/LatencyScan.h"

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

#include <iostream>
#include "xdata/Vector.h"
#include <string>

#include "xoap/MessageReference.h"
#include "xoap/MessageFactory.h"
#include "xoap/SOAPEnvelope.h"
#include "xoap/SOAPConstants.h"
#include "xoap/SOAPBody.h"
#include "xoap/Method.h"
#include "xoap/AttachmentPart.h"

#include "xoap/domutils.h"

XDAQ_INSTANTIATOR_IMPL(gem::supervisor::tbutils::LatencyScan)

//
void gem::supervisor::tbutils::LatencyScan::ConfigParams::registerFields(xdata::Bag<ConfigParams> *bag)
{

  minLatency    = 13U;
  maxLatency    = 17U;
  stepSize      = 1U;
  nTriggers     = 10;
  deviceVT1     = 25;
  deviceVT2     = 0;
  MSPulseLength = 3;
  VCal          = 100;
    
  bag->addField("minLatency",    &minLatency   );
  bag->addField("maxLatency",    &maxLatency   );
  bag->addField("stepSize",      &stepSize     );
  bag->addField("VT2",           &deviceVT1    );
  bag->addField("VT1",           &deviceVT2    );
  bag->addField("VCal",          &VCal         );
  bag->addField("MSPulseLength", &MSPulseLength);
  bag->addField("nTriggers",     &nTriggers    );

}

gem::supervisor::tbutils::LatencyScan::LatencyScan(xdaq::ApplicationStub * s)  throw (xdaq::exception::Exception) :

  gem::supervisor::tbutils::GEMTBUtil(s)
{

  getApplicationInfoSpace()->fireItemAvailable("scanParams", &scanParams_);
  getApplicationInfoSpace()->fireItemValueRetrieve("scanParams", &scanParams_);

  // HyperDAQ bindings
  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::LatencyScan::webDefault,      "Default"    );
  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::LatencyScan::webConfigure,    "Configure"  );
  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::LatencyScan::webStart,        "Start"      );
  runSig_   = toolbox::task::bind(this, &LatencyScan::run,        "run"       );

  // Initiate and activate main workloop
  wl_ = toolbox::task::getWorkLoopFactory()->getWorkLoop("urn:xdaq-workloop:GEMTestBeamSupervisor:LatencyScan","waiting");
  wl_->activate();

  currentLatency_ = 0;
}

gem::supervisor::tbutils::LatencyScan::~LatencyScan()
{
  wl_ = toolbox::task::getWorkLoopFactory()->getWorkLoop("urn:xdaq-workloop:GEMTestBeamSupervisor:LatencyScan","waiting");
  //should we check to see if it's running and try to stop?
  wl_->cancel();
  wl_ = 0;
}
bool gem::supervisor::tbutils::LatencyScan::run(toolbox::task::WorkLoop* wl)
{
  wl_semaphore_.take(); // take workloop
  if (!is_running_) {
    wl_semaphore_.give(); // give work loop if it is not running
    uint32_t bufferDepth = 0;
    bufferDepth = glibDevice_->getFIFOVFATBlockOccupancy(readout_mask);
    LOG4CPLUS_INFO(getApplicationLogger()," ******IT IS NOT RUNNIG ***** ");
    return false;
  }
  
  hw_semaphore_.take();//take hw to set the trigger source, send L1A+Cal pulses,

  //set trigger source  
  /*
  if(confParams_.bag.triggerSource_.value_ == 0x0){
    optohybridDevice_->setTrigSource(0x0);//from AMC13   
  }else if(confParams_.bag.triggerSource_.value_ == 0x1){
    optohybridDevice_->setTrigSource(0x1);//from T1   
  }else if(confParams_.bag.triggerSource_.value_ == 0x2){
    optohybridDevice_->setTrigSource(0x2);//from sbits   
  }else if(confParams_.bag.triggerSource_.value_ == 0x3){
    optohybridDevice_->setTrigSource(0x3);//from Ext_LEMO   
    }*/

  sendAMC13trigger();

  //count triggers and Calpulses coming from TTC
  confParams_.bag.triggersSeen =  optohybridDevice_->getL1ACount(0x0);
  CalPulseCount_[0] = optohybridDevice_->getCalPulseCount(0x0); 

  LOG4CPLUS_INFO(getApplicationLogger(), " ABC TriggersSeen " << confParams_.bag.triggersSeen << " Calpulse " << optohybridDevice_->getCalPulseCount(0x0));

  hw_semaphore_.give();//give hw to set the trigger source, send L1A+Cal pulses,
  
  // if triggersSeen < N triggers
  if ((uint64_t)(confParams_.bag.triggersSeen) < (uint64_t)(confParams_.bag.nTriggers)) {

    hw_semaphore_.take();//take hw. glib buffer depth

    // Get the size of GLIB data buffer
    uint32_t bufferDepth = 0;
    bufferDepth = glibDevice_->getFIFOVFATBlockOccupancy(readout_mask);
    LOG4CPLUS_INFO(getApplicationLogger(), " Bufferdepht " << bufferDepth);    

    ++confParams_.bag.triggercount;

    hw_semaphore_.give();//give hw. glib buffer depth 
    wl_semaphore_.give();//give workloop to read
    
    if (bufferDepth>0) {
      ++confParams_.bag.triggercount;
    }
    return true;
  }// end triggerSeen < N triggers
  else { 
    //disable triggers
    /*    if(confParams_.bag.triggerSource_.value_ == 0x1){
	  optohybridDevice_->stopT1Generator(true);
	  } else { 
	  optohybridDevice_->setTrigSource(0x1);       
	  }

    */

    hw_semaphore_.take(); //take hw to set Runmode 0 on VFATs 
    for (auto chip = vfatDevice_.begin(); chip != vfatDevice_.end(); ++chip) {
      (*chip)->setRunMode(0);
    }// end for  

    uint32_t bufferDepth = 0;
    bufferDepth = glibDevice_->getFIFOVFATBlockOccupancy(readout_mask);

    ++confParams_.bag.triggercount;
  
    //reset counters
    optohybridDevice_->resetL1ACount(0x5);
    optohybridDevice_->resetResyncCount();
    optohybridDevice_->resetBC0Count();
    optohybridDevice_->resetCalPulseCount(0x0);
    optohybridDevice_->sendResync();     
    optohybridDevice_->sendBC0();          
    
    hw_semaphore_.give(); // give hw to reset counters

    LOG4CPLUS_INFO(getApplicationLogger()," ABC Scan point TriggersSeen " 
		   << confParams_.bag.triggersSeen << " Calpulse " << optohybridDevice_->getCalPulseCount(0x0));
  
    //if max Latency - current Latency >= stepsize
    if (scanParams_.bag.maxLatency - currentLatency_ >= scanParams_.bag.stepSize) {

      hw_semaphore_.take();// vfat set latency

      LOG4CPLUS_INFO(getApplicationLogger()," ABC run: Latency= "
		     << (int)currentLatency_ << " VT1= "
		     << scanParams_.bag.deviceVT1 << " VT2= "
		     << scanParams_.bag.deviceVT2
		     << " abs(VT2-VT1) " 
		     << abs(scanParams_.bag.deviceVT2-scanParams_.bag.deviceVT1) );

      if ((currentLatency_ + scanParams_.bag.stepSize) < 0xFF) {
        optohybridDevice_->broadcastWrite("Latency", 0x0, currentLatency_ + scanParams_.bag.stepSize);
      } else  { 
        optohybridDevice_->broadcastWrite("Latency", 0x0, 0xFF);
      }//end else
      
      uint32_t bufferDepth = 0;
      bufferDepth = glibDevice_->getFIFOVFATBlockOccupancy(readout_mask);    
      
      for (auto chip = vfatDevice_.begin(); chip != vfatDevice_.end(); ++chip) {
	currentLatency_ = (*chip)->getLatency();
	scanParams_.bag.deviceVT1 = (*chip)->getVThreshold1();
	scanParams_.bag.deviceVT2 = (*chip)->getVThreshold2();
      }
      
      glibDevice_->setDAQLinkRunParameter(1,currentLatency_);
      sleep(0.001);

      for (auto chip = vfatDevice_.begin(); chip != vfatDevice_.end(); ++chip) {
	(*chip)->setRunMode(1);      
      }
        
      //set trigger source  
      /*      if(confParams_.bag.triggerSource_.value_ == 0x0){
	optohybridDevice_->setTrigSource(0x0);//from AMC13   
      }else if(confParams_.bag.triggerSource_.value_ == 0x1){
	optohybridDevice_->setTrigSource(0x1);//from T1   
      }else if(confParams_.bag.triggerSource_.value_ == 0x2){
	optohybridDevice_->setTrigSource(0x2);//from sbits   
      }else if(confParams_.bag.triggerSource_.value_ == 0x3){
	optohybridDevice_->setTrigSource(0x3);//from Ext_LEMO   
	}*/
    
      //setting counters  = 0
      CalPulseCount_[0] = 0;	  
      confParams_.bag.triggersSeen =  0;
 
      hw_semaphore_.give(); // give hw vfat set latency
      wl_semaphore_.give(); // end of workloop	

      return true;	
    } // end if maxLat - curreLat >= step
    else {
      hw_semaphore_.take(); // take hw to stop workloop
      wl_->submit(stopSig_);  
      hw_semaphore_.give(); // give hw to stop workloop
      wl_semaphore_.give(); // end of workloop	      

      /*      wl_semaphore_.give();  // end of workloop	
	      wl_->submit(stopSig_);*/
      return false; 
    }//end else
	  
    return true;
  }//end else triggerseen < N triggers

  return true;
}//end run

void gem::supervisor::tbutils::LatencyScan::scanParameters(xgi::Output *out)
  throw (xgi::exception::Exception)
{
  try {
    *out << cgicc::span()   << std::endl //open span

	 << cgicc::label("MinLatency").set("for","MinLatency") << std::endl
	 << cgicc::input().set("id","MinLatency").set("name","MinLatency")
      .set("type","number").set("min","0").set("max","255")
      .set("value",boost::str(boost::format("%d")%(scanParams_.bag.minLatency)))
	 << std::endl

	 << cgicc::label("MaxLatency").set("for","MaxLatency") << std::endl
	 << cgicc::input().set("id","MaxLatency").set("name","MaxLatency")
      .set("type","number").set("min","0").set("max","255")
      .set("value",boost::str(boost::format("%d")%(scanParams_.bag.maxLatency)))
	 << std::endl

	 << cgicc::label("Latency Step").set("for","LatencyStep") << std::endl
	 << cgicc::input().set("id","LatencyStep").set("name","LatencyStep")
      .set("type","number").set("min","1").set("max","255")
      .set("value",boost::str(boost::format("%d")%(scanParams_.bag.stepSize)))
	 << std::endl

	 << cgicc::br() << std::endl

	 << cgicc::label("Current Latency").set("for","CurrentLatency") << std::endl
	 << cgicc::input().set("id","CurrentLatency").set("name","CurrentLatency")
      .set("type","text").set("readonly")
      .set("value",boost::str(boost::format("%d")%((unsigned)currentLatency_)))
	 << std::endl

	 << cgicc::br() << std::endl

	 << cgicc::label("Triggers to take").set("for","TriggersToTake") << std::endl
	 << cgicc::input().set("id","TriggersToTake").set("name","TriggersToTake")
      .set("type","number").set("min","0")
      .set("value",boost::str(boost::format("%d")%(confParams_.bag.nTriggers)))
	 << std::endl

	 << cgicc::label("Triggers taken").set("for","TriggersSeen") << std::endl
	 << cgicc::input().set("id","TriggersSeen").set("name","TriggersSeen")
      .set("type","text").set("readonly")
      .set("value",boost::str(boost::format("%d")%(confParams_.bag.triggersSeen)))
      //.set("value",boost::str(boost::format("%d")%(triggersSeen_)))
	 << std::endl

	 << cgicc::br() << std::endl

	 << cgicc::label("Set Threshold (VT2-VT1):  VT1 ").set("for","VT1") << std::endl
	 << cgicc::input().set("id","VT1").set("name","VT1")
      .set("type","number").set("min","0").set("max","255")
      .set("value",boost::str(boost::format("%d")%(scanParams_.bag.deviceVT1)))
	 << std::endl

	 << cgicc::label("VT2").set("for","VT2") << std::endl
	 << cgicc::input().set("id","VT2").set("name","VT2")
      .set("type","number").set("min","0").set("max","255")
      .set("value",boost::str(boost::format("%d")%(scanParams_.bag.deviceVT2)))
	 << std::endl
      
	 << cgicc::br()  // << std::endl

	 << cgicc::label("VCal").set("for","VCal") << std::endl
	 << cgicc::input().set("id","VCal").set("name","VCal")
      .set("type","number").set("min","0").set("max","200")
      .set("value",boost::str(boost::format("%d")%(scanParams_.bag.VCal)))
	 << std::endl

      	 << cgicc::label("MSPulseLength").set("for","MSPulseLength") << std::endl
	 << cgicc::input().set("id","MSPulseLength").set("name","MSPulseLength")
      .set("type","number").set("min","0").set("max","3")
      .set("value",boost::str(boost::format("%d")%(scanParams_.bag.MSPulseLength)))
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
void gem::supervisor::tbutils::LatencyScan::webDefault(xgi::Input *in, xgi::Output *out)
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
      //      selectTrigSource(out);
      scanParameters(out);

      *out << cgicc::input().set("type","text").set("name","xmlFilename").set("size","80")
	.set("ENCTYPE","multipart/form-data").set("readonly")
	.set("value",confParams_.bag.settingsFile.toString()) << std::endl;

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
      //      selectTrigSource(out);
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
      //      selectTrigSource(out);
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
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Something went wrong displaying LatencyScan control panel(xgi): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  catch (const std::exception& e) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Something went wrong displaying LatencyScan control panel(std): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
}

//
void gem::supervisor::tbutils::LatencyScan::webConfigure(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception) {

  try {
    cgicc::Cgicc cgi(in);

    //aysen's xml parser
    confParams_.bag.settingsFile = cgi.getElement("xmlFilename")->getValue();

    cgicc::const_form_iterator element  = cgi.getElement("MinLatency");
    if (element != cgi.getElements().end())
      scanParams_.bag.minLatency = element->getIntegerValue();

    element = cgi.getElement("MaxLatency");
    if (element != cgi.getElements().end())
      scanParams_.bag.maxLatency = element->getIntegerValue();

    element = cgi.getElement("LatencyStep");
    if (element != cgi.getElements().end())
      scanParams_.bag.stepSize  = element->getIntegerValue();

    element = cgi.getElement("TriggersToTake");
    if (element != cgi.getElements().end())
      confParams_.bag.nTriggers  = element->getIntegerValue();

    element = cgi.getElement("VCal");
    if (element != cgi.getElements().end())
      scanParams_.bag.VCal  = element->getIntegerValue();

    element = cgi.getElement("MSPulseLength");
    if (element != cgi.getElements().end())
      scanParams_.bag.MSPulseLength  = element->getIntegerValue();

    /*
    cgicc::form_iterator new_triggersource = cgi.getElement("SetTrigSrc");
    INFO("triggersource output : " << (**new_triggersource).c_str());
    
    if (strcmp((**new_triggersource).c_str(),"Calpulse+L1A") == 0) {
      confParams_.bag.triggerSource_ = 0x1;
      optohybridDevice_->setTrigSource(0x1);//from T1   
      INFO("Fake Latency Scan sending Calpulses+L1As. TrigSource : " << confParams_.bag.triggerSource_);    
    }
    if (strcmp((**new_triggersource).c_str(),"Internal loopback of s-bits") == 0) {
      confParams_.bag.triggerSource_ = 0x3;
      optohybridDevice_->setTrigSource(0x3);//from sbits   
      INFO("Sending Calpulses and the s-bits come back from the OH. TrigSource : " << confParams_.bag.triggerSource_ );     
    }
    if (strcmp((**new_triggersource).c_str(),"External Trigger from AMC13") == 0) {
      confParams_.bag.triggerSource_= 0x0;
      optohybridDevice_->setTrigSource(0x0);//from AMC13
      INFO("Real signals and the trigger comes from the AMC13. TrigSource : " << confParams_.bag.triggerSource_ );   
    }
    if (strcmp((**new_triggersource).c_str(),"External Trigger from LEMO") == 0) {
      confParams_.bag.triggerSource_ = 0x2;
      optohybridDevice_->setTrigSource(0x2);//from Ext_LEMO   
      INFO("Real signals and the trigger comes from the LEMO Cable. TrigSource : " << confParams_.bag.triggerSource_ );
    }
   */
  
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
void gem::supervisor::tbutils::LatencyScan::webStart(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception) {
  // Initiate start workloop
  wl_->submit(startSig_);
  // Go back to main web interface
  redirect(in,out);
}

void gem::supervisor::tbutils::LatencyScan::configureAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {

  is_working_ = true;

  stepSize_   = scanParams_.bag.stepSize;
  minLatency_ = scanParams_.bag.minLatency;
  maxLatency_ = scanParams_.bag.maxLatency;

  NTriggersAMC13();
  sendConfigureMessageAMC13();
  sendConfigureMessageGLIB();

  confParams_.bag.triggercount = 0;

  hw_semaphore_.take();
  LOG4CPLUS_INFO(getApplicationLogger(), "attempting to configure device");


  //set trigger source
  /*  INFO("ConfigureAction Trigger Source : " << confParams_.bag.triggerSource_ );   
  if(confParams_.bag.triggerSource_.value_ == 0x0){
    optohybridDevice_->setTrigSource(0x0);//from AMC13
    INFO("Real signals and the trigger comes from the AMC13. TrigSource : " << confParams_.bag.triggerSource_ );   
  }else if(confParams_.bag.triggerSource_.value_ == 0x1){
    optohybridDevice_->setTrigSource(0x1);//from T1   
    INFO("Fake Latency Scan sending Calpulses+L1As. TrigSource : " << confParams_.bag.triggerSource_);
  }else if(confParams_.bag.triggerSource_.value_ == 0x3){
    optohybridDevice_->setTrigSource(0x3);//from sbits   
    INFO("Sending Calpulses and the s-bits come back from the OH. TrigSource : " << confParams_.bag.triggerSource_ );
  }else if(confParams_.bag.triggerSource_.value_ == 0x2){
    optohybridDevice_->setTrigSource(0x2);//from Ext_LEMO   
    INFO("Real signals and the trigger comes from the LEMO Cable. TrigSource : " << confParams_.bag.triggerSource_ );
  }
  */

  //make sure device is not running
  for (auto chip = vfatDevice_.begin(); chip != vfatDevice_.end(); ++chip) {

    confParams_.bag.deviceChipID = (*chip)->getChipID();
    (*chip)->setDeviceIPAddress(confParams_.bag.deviceIP);
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
    (*chip)->setMSPulseLength(scanParams_.bag.MSPulseLength);
    (*chip)->setInputPadMode( 0x0);
    (*chip)->setTrimDACRange( 0x0);
    (*chip)->setBandgapPad(   0x0);
    (*chip)->sendTestPattern( 0x0);

    (*chip)->setVCal(scanParams_.bag.VCal);
    for (int chan = 0; chan < 129; ++chan)
      if (chan == 0 || chan == 1 || chan == 32)
        (*chip)->enableCalPulseToChannel(chan, true);
      else
        (*chip)->enableCalPulseToChannel(chan, false);
      
    (*chip)->setIPreampIn(  168);
    (*chip)->setIPreampFeed(150);
    (*chip)->setIPreampOut(  80);
    (*chip)->setIShaper(    150);
    (*chip)->setIShaperFeed(100);
    (*chip)->setIComp(       75);//120

    (*chip)->setVThreshold1(scanParams_.bag.deviceVT1);//50
    (*chip)->setVThreshold2(scanParams_.bag.deviceVT2);//0

    //
    //    int islot = slotInfo->GEBslotIndex( (uint32_t)((*chip)->getChipID()));


    LOG4CPLUS_INFO(getApplicationLogger(), "setting DAC mode to normal");
    (*chip)->setDACMode(gem::hw::vfat::StringToDACMode.at("OFF"));

    LOG4CPLUS_INFO(getApplicationLogger(), "setting starting latency value");
    (*chip)->setLatency(    scanParams_.bag.minLatency);

    LOG4CPLUS_INFO(getApplicationLogger(), "reading back current latency value");
    currentLatency_ = (*chip)->getLatency();

    LOG4CPLUS_INFO(getApplicationLogger(), "Threshold " << scanParams_.bag.deviceVT1);

    LOG4CPLUS_INFO(getApplicationLogger(), "VCal " << scanParams_.bag.VCal);

    LOG4CPLUS_INFO(getApplicationLogger(), "device configured");
    is_configured_ = true;
  }

  //flush fifo
  INFO("Flushing the FIFOs, readout_mask 0x" <<std::hex << (int)readout_mask << std::dec);
  DEBUG("Flushing FIFO" << readout_mask << " (depth " << glibDevice_->getFIFOOccupancy(readout_mask));
  glibDevice_->flushFIFO(readout_mask);
  while (glibDevice_->hasTrackingData(readout_mask)) {
    glibDevice_->flushFIFO(readout_mask);
    std::vector<uint32_t> dumping = glibDevice_->getTrackingData(readout_mask,
                                                                 glibDevice_->getFIFOVFATBlockOccupancy(readout_mask));
  }
  // once more for luck
  glibDevice_->flushFIFO(readout_mask);

  glibDevice_->setDAQLinkRunParameter(1,currentLatency_);

  //reset counters
  optohybridDevice_->resetL1ACount(0x5);
  optohybridDevice_->resetCalPulseCount(0x0);
  optohybridDevice_->resetResyncCount();
  optohybridDevice_->resetBC0Count();
  optohybridDevice_->sendResync();      
  optohybridDevice_->sendBC0();          
  //  vfatDevice_->setRunMode(1);      

  hw_semaphore_.give();

  LOG4CPLUS_INFO(getApplicationLogger(), "configure routine completed");

  is_working_    = false;

}		 

//
void gem::supervisor::tbutils::LatencyScan::startAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {

  wl_semaphore_.take();
  is_working_ = true;

  sendStartMessageGLIB();

  //AppHeader ah;
  threshold_     = scanParams_.bag.deviceVT2 -scanParams_.bag.deviceVT1;
  VCal           = scanParams_.bag.VCal;
  MSPulseLength  = scanParams_.bag.MSPulseLength;
  nTriggers_  = confParams_.bag.nTriggers;
  stepSize_   = scanParams_.bag.stepSize;
  minLatency_ = scanParams_.bag.minLatency;
  maxLatency_ = scanParams_.bag.maxLatency;

  time_t now = time(0);
  tm *gmtm = gmtime(&now);
  char* utcTime = asctime(gmtm);

  std::string tmpFileName = "LatencyScan_", tmpType = "", outputType   = "Bin";
  tmpFileName.append(utcTime);
  tmpFileName.erase(std::remove(tmpFileName.begin(), tmpFileName.end(), '\n'), tmpFileName.end());
  tmpFileName.append(".dat");
  std::replace(tmpFileName.begin(), tmpFileName.end(), ' ', '_' );
  std::replace(tmpFileName.begin(), tmpFileName.end(), ':', '-');

  std::string errFileName = "ERRORS_";
  errFileName.append(tmpFileName);
  errFileName.append(utcTime);
  errFileName.erase(std::remove(errFileName.begin(), errFileName.end(), '\n'), errFileName.end());
  errFileName.append(".dat");
  std::replace(errFileName.begin(), errFileName.end(), ' ', '_' );
  std::replace(errFileName.begin(), errFileName.end(), ':', '-');

  confParams_.bag.outFileName = tmpFileName;

  LOG4CPLUS_INFO(getApplicationLogger(),"Creating file " << confParams_.bag.outFileName.toString());

  std::ofstream scanStream(tmpFileName.c_str(),  std::ios::app | std::ios::binary);
  std::ofstream errf(errFileName.c_str(), std::ios_base::app | std::ios::binary );

  if (scanStream.is_open()){
    LOG4CPLUS_DEBUG(getApplicationLogger(),"file " << confParams_.bag.outFileName.toString() << "opened");
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

    scanSetup << "\n The Time & Date : " << utcTime << std::endl;
    scanSetup << " ChipID        0x" << std::hex    << confParams_.bag.deviceChipID << std::dec << std::endl;
    scanSetup << " Threshold     " << threshold_    << std::endl;
    scanSetup << " VCal          " << VCal          << std::endl;
    scanSetup << " MSPulseLength " << MSPulseLength << std::endl;
    scanSetup << " nTriggers     " << nTriggers_    << std::endl;
    scanSetup << " stepSize      " << stepSize_     << std::endl;
    scanSetup << " minLatency    " << minLatency_   << std::endl;
    scanSetup << " maxLatency    " << maxLatency_   << std::endl;
  }
  scanSetup.close();
  
  //char data[128/8]
  is_running_ = true;
  hw_semaphore_.take();//oh reset counters

  //set trigger source
  /*  INFO("Start Trigger Source : " << confParams_.bag.triggerSource_ );   
  if(confParams_.bag.triggerSource_.value_ == 0x0){
    optohybridDevice_->setTrigSource(0x0);//from AMC13
  }else if(confParams_.bag.triggerSource_.value_ == 0x1){
    optohybridDevice_->setTrigSource(0x1);//from T1   
  }else if(confParams_.bag.triggerSource_.value_ == 0x3){
    optohybridDevice_->setTrigSource(0x3);//from sbits   
  }else if(confParams_.bag.triggerSource_.value_ == 0x2){
    optohybridDevice_->setTrigSource(0x2);//from Ext_LEMO   
    }*/

  //set clock source
  //optohybridDevice_->setVFATClock(1,1,0x0);    
  //optohybridDevice_->setCDCEClock(1,1,0x0); 

  optohybridDevice_->sendResync();     
  optohybridDevice_->sendBC0();          

  //reset counters
  optohybridDevice_->resetL1ACount(0x5);
  optohybridDevice_->resetResyncCount();
  optohybridDevice_->resetBC0Count();
  optohybridDevice_->resetCalPulseCount(0x0);


  //flush fifo
  INFO("Flushing the FIFOs, readout_mask 0x" <<std::hex << (int)readout_mask << std::dec);
  DEBUG("Flushing FIFO" << readout_mask << " (depth " << glibDevice_->getFIFOOccupancy(readout_mask));
  glibDevice_->flushFIFO(readout_mask);
  while (glibDevice_->hasTrackingData(readout_mask)) {
    glibDevice_->flushFIFO(readout_mask);
    std::vector<uint32_t> dumping = glibDevice_->getTrackingData(readout_mask,
                                                                 glibDevice_->getFIFOVFATBlockOccupancy(readout_mask));
  }
  // once more for luck
  glibDevice_->flushFIFO(readout_mask);

  //send Resync
  optohybridDevice_->sendResync();      
  optohybridDevice_->sendBC0();         
  optohybridDevice_->sendResync();       

  glibDevice_->setDAQLinkRunType(1);
  glibDevice_->setDAQLinkRunParameter(1,currentLatency_);
  glibDevice_->setDAQLinkRunParameter(2,scanParams_.bag.deviceVT1);
  glibDevice_->setDAQLinkRunParameter(3,scanParams_.bag.deviceVT2);

  // set selected VFATs 
  for (auto chip = vfatDevice_.begin(); chip != vfatDevice_.end(); ++chip) {
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
void gem::supervisor::tbutils::LatencyScan::resetAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {

  is_working_ = true;
  gem::supervisor::tbutils::GEMTBUtil::resetAction(e);
  {

    confParams_.bag.nTriggers       = 10U;
    scanParams_.bag.minLatency      = 0U;
    scanParams_.bag.maxLatency      = 25U;
    scanParams_.bag.stepSize        = 1U;
    scanParams_.bag.deviceVT1       = 25U;
    scanParams_.bag.deviceVT2       = 0U;
    scanParams_.bag.VCal            = 100;
    scanParams_.bag.MSPulseLength   = 3;
    //    confParams_.bag.triggerSource_  = 8;
    // confParams_.bag.deviceName   = "";
    // confParams_.bag.deviceChipID = 0x0;

    is_working_     = false;
  }
}

/*
void gem::supervisor::tbutils::LatencyScan::selectTrigSource(xgi::Output *out)
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
      INFO("Trigger Source : " << confParams_.bag.triggerSource_);
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
*/

void gem::supervisor::tbutils::LatencyScan::sendConfigureMessageGLIB()
  throw (xgi::exception::Exception) {
  //  is_working_ = true;

  xoap::MessageReference msg = xoap::createMessage();
  xoap::SOAPPart soap = msg->getSOAPPart();
  xoap::SOAPEnvelope envelope = soap.getEnvelope();
  xoap::SOAPBody body = envelope.getBody();
  //  xoap::SOAPName command = envelope.createName("CallBackConfigure","xdaq", "urn:xdaq-soap:3.0");
  xoap::SOAPName command = envelope.createName("Configure","xdaq", "urn:xdaq-soap:3.0");
  body.addBodyElement(command);

  try 
    {
      xdaq::ApplicationDescriptor * d = getApplicationContext()->getDefaultZone()->getApplicationDescriptor("gem::hw::glib::GLIBManager", 4);
      xdaq::ApplicationDescriptor * o = this->getApplicationDescriptor();
      xoap::MessageReference reply = getApplicationContext()->postSOAP(msg, *o,  *d);
    }
  catch (xdaq::exception::Exception& e)
    {
      LOG4CPLUS_INFO(getApplicationLogger(),"------------------Fail sending configure message " << e.what());
      XCEPT_RETHROW (xgi::exception::Exception, "Cannot send message", e);
    }
  //  this->Default(in,out);
  LOG4CPLUS_INFO(getApplicationLogger(),"-----------The message to configure has been sent------------");
}      


bool gem::supervisor::tbutils::LatencyScan::sendStartMessageGLIB()
  throw (xgi::exception::Exception) {

  //  this->Default(in,out);
  LOG4CPLUS_INFO(getApplicationLogger(),"-----------The message to GLIB start sent------------");

  //  is_working_ = true;
  xoap::MessageReference msg = xoap::createMessage();
  xoap::SOAPPart soap = msg->getSOAPPart();
  xoap::SOAPEnvelope envelope = soap.getEnvelope();
  xoap::SOAPBody body = envelope.getBody();
  //  xoap::SOAPName command = envelope.createName("CallBackStart","xdaq", "urn:xdaq-soap:3.0");
  xoap::SOAPName command = envelope.createName("Start","xdaq", "urn:xdaq-soap:3.0");
  body.addBodyElement(command);

  try 
    {
      xdaq::ApplicationDescriptor * d = getApplicationContext()->getDefaultZone()->getApplicationDescriptor("gem::hw::glib::GLIBManager", 4);
      xdaq::ApplicationDescriptor * o = this->getApplicationDescriptor();
      xoap::MessageReference reply = getApplicationContext()->postSOAP(msg, *o,  *d);
      LOG4CPLUS_INFO(getApplicationLogger(),"-----------The message to start GLIB has been sent------------");
      return true;
    }
  catch (xdaq::exception::Exception& e)
    {
      LOG4CPLUS_INFO(getApplicationLogger(),"------------------Fail sending start message " << e.what());
      XCEPT_RETHROW (xgi::exception::Exception, "Cannot send message", e);
      return false;
    }
}      

void gem::supervisor::tbutils::LatencyScan::NTriggersAMC13()
  throw (xgi::exception::Exception) {
  //  is_working_ = true;

  LOG4CPLUS_INFO(getApplicationLogger(),"-----------start SOAP message modify paramteres AMC13------ ");

  xdaq::ApplicationDescriptor * d = getApplicationContext()->getDefaultZone()->getApplicationDescriptor("gem::hw::amc13::AMC13Manager", 3);
  xdaq::ApplicationDescriptor * o = this->getApplicationDescriptor();
  std::string    appUrn   = "urn:xdaq-application:"+d->getClassName();

  xoap::MessageReference msg_2 = xoap::createMessage();
  xoap::SOAPPart soap_2 = msg_2->getSOAPPart();
  xoap::SOAPEnvelope envelope_2 = soap_2.getEnvelope();
  xoap::SOAPName     parameterset   = envelope_2.createName("ParameterSet","xdaq",XDAQ_NS_URI);

  xoap::SOAPElement  container = envelope_2.getBody().addBodyElement(parameterset);
  container.addNamespaceDeclaration("xsd","http://www.w3.org/2001/XMLSchema");
  container.addNamespaceDeclaration("xsi","http://www.w3.org/2001/XMLSchema-instance");
  //  container.addNamespaceDeclaration("parameterset","http://schemas.xmlsoap.org/soap/encoding/");
  xoap::SOAPName tname_param    = envelope_2.createName("type","xsi","http://www.w3.org/2001/XMLSchema-instance");
  xoap::SOAPName pboxname_param = envelope_2.createName("Properties","props",appUrn);
  xoap::SOAPElement pbox_param = container.addChildElement(pboxname_param);
  pbox_param.addAttribute(tname_param,"soapenc:Struct");

  xoap::SOAPName pboxname_amc13config = envelope_2.createName("amc13ConfigParams","props",appUrn);
  xoap::SOAPElement pbox_amc13config = pbox_param.addChildElement(pboxname_amc13config);
  pbox_amc13config.addAttribute(tname_param,"soapenc:Struct");
  
  xoap::SOAPName    soapName_l1A = envelope_2.createName("L1Aburst","props",appUrn);
  xoap::SOAPElement cs_l1A      = pbox_amc13config.addChildElement(soapName_l1A);
  cs_l1A.addAttribute(tname_param,"xsd:unsignedInt");
  cs_l1A.addTextNode(confParams_.bag.nTriggers.toString());

  
  std::string tool;
  xoap::dumpTree(msg_2->getSOAPPart().getEnvelope().getDOMNode(),tool);
  DEBUG("msg_2: " << tool);
  
  try 
    {
      DEBUG("trying to send parameters");
      xoap::MessageReference reply_2 = getApplicationContext()->postSOAP(msg_2, *o,  *d);
      std::string tool;
      xoap::dumpTree(reply_2->getSOAPPart().getEnvelope().getDOMNode(),tool);
      DEBUG("reply_2: " << tool);
    }
  catch (xoap::exception::Exception& e)
    {
      LOG4CPLUS_ERROR(getApplicationLogger(),"------------------Fail  AMC13 configuring parameters message " << e.what());
      XCEPT_RETHROW (xoap::exception::Exception, "Cannot send message", e);
    }
  catch (xdaq::exception::Exception& e)
    {
      LOG4CPLUS_ERROR(getApplicationLogger(),"------------------Fail  AMC13 configuring parameters message " << e.what());
      XCEPT_RETHROW (xoap::exception::Exception, "Cannot send message", e);
    }
  catch (std::exception& e)
    {
      LOG4CPLUS_ERROR(getApplicationLogger(),"------------------Fail  AMC13 configuring parameters message " << e.what());
      //XCEPT_RETHROW (xoap::exception::Exception, "Cannot send message", e);
    }
  catch (...)
    {
      LOG4CPLUS_ERROR(getApplicationLogger(),"------------------Fail  AMC13 configuring parameters message ");
      XCEPT_RAISE (xoap::exception::Exception, "Cannot send message");
    }

  //  this->Default(in,out);
  LOG4CPLUS_INFO(getApplicationLogger(),"-----------The message to AMC13 configuring parameters has been sent------------");
}      

void gem::supervisor::tbutils::LatencyScan::sendConfigureMessageAMC13()
  throw (xgi::exception::Exception) {
  //  is_working_ = true;

  xoap::MessageReference msg = xoap::createMessage();
  xoap::SOAPPart soap = msg->getSOAPPart();
  xoap::SOAPEnvelope envelope = soap.getEnvelope();
  xoap::SOAPBody body = envelope.getBody();
  xoap::SOAPName command = envelope.createName("CallBackConfigure","xdaq", "urn:xdaq-soap:3.0");
  body.addBodyElement(command);

  try 
    {
      xdaq::ApplicationDescriptor * d = getApplicationContext()->getDefaultZone()->getApplicationDescriptor("gem::hw::amc13::AMC13Manager", 3);
      xdaq::ApplicationDescriptor * o = this->getApplicationDescriptor();
      xoap::MessageReference reply = getApplicationContext()->postSOAP(msg, *o,  *d);
    }
  catch (xdaq::exception::Exception& e)
    {
      LOG4CPLUS_INFO(getApplicationLogger(),"------------------Fail sending AMC13 configure message " << e.what());
      XCEPT_RETHROW (xgi::exception::Exception, "Cannot send message", e);
    }
  //  this->Default(in,out);
  LOG4CPLUS_INFO(getApplicationLogger(),"-----------The message to AMC13 configure has been sent------------");
}      


bool gem::supervisor::tbutils::LatencyScan::sendStartMessageAMC13()
  throw (xgi::exception::Exception) {
  //  is_working_ = true;
  xoap::MessageReference msg = xoap::createMessage();
  xoap::SOAPPart soap = msg->getSOAPPart();
  xoap::SOAPEnvelope envelope = soap.getEnvelope();
  xoap::SOAPBody body = envelope.getBody();
  xoap::SOAPName command = envelope.createName("CallBackStart","xdaq", "urn:xdaq-soap:3.0");
  body.addBodyElement(command);

  try 
    {
      xdaq::ApplicationDescriptor * d = getApplicationContext()->getDefaultZone()->getApplicationDescriptor("gem::hw::amc13::AMC13Manager", 3);
      xdaq::ApplicationDescriptor * o = this->getApplicationDescriptor();
      xoap::MessageReference reply = getApplicationContext()->postSOAP(msg, *o,  *d);
      LOG4CPLUS_INFO(getApplicationLogger(),"-----------The message to start the AMC13 has been sent------------");
      return true;
    }
  catch (xdaq::exception::Exception& e)
    {
      LOG4CPLUS_INFO(getApplicationLogger(),"------------------Fail sending AMC13 start message " << e.what());
      XCEPT_RETHROW (xgi::exception::Exception, "Cannot send message", e);
        return false;
    }
}      

void gem::supervisor::tbutils::LatencyScan::sendAMC13trigger()
  throw (xgi::exception::Exception) {
  //  is_working_ = true;
  xoap::MessageReference msg = xoap::createMessage();
  xoap::SOAPPart soap = msg->getSOAPPart();
  xoap::SOAPEnvelope envelope = soap.getEnvelope();
  xoap::SOAPBody body = envelope.getBody();
  xoap::SOAPName command = envelope.createName("sendtriggerburst","xdaq", "urn:xdaq-soap:3.0");

  body.addBodyElement(command);

  try 
    {
      xdaq::ApplicationDescriptor * d = getApplicationContext()->getDefaultZone()->getApplicationDescriptor("gem::hw::amc13::AMC13Manager", 3);
      xdaq::ApplicationDescriptor * o = this->getApplicationDescriptor();
      xoap::MessageReference reply = getApplicationContext()->postSOAP(msg, *o,  *d);
      
      LOG4CPLUS_INFO(getApplicationLogger(),"-----------The message to start sending burst-----------");

    }
  catch (xdaq::exception::Exception& e)
    {
      LOG4CPLUS_INFO(getApplicationLogger(),"------------------Fail sending burst message " << e.what());
      XCEPT_RETHROW (xgi::exception::Exception, "Cannot send message", e);
    }
  //  this->Default(in,out);
}      
