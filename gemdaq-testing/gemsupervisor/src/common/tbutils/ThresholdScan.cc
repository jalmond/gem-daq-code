#include "gem/supervisor/tbutils/ThresholdScan.h"
#include "gem/supervisor/tbutils/ThresholdEvent.h"

#include "gem/hw/vfat/HwVFAT2.h"
#include "gem/hw/glib/HwGLIB.h"
#include "gem/hw/optohybrid/HwOptoHybrid.h"

#include "gem/utils/GEMLogging.h"

#include "TH1.h"
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

XDAQ_INSTANTIATOR_IMPL(gem::supervisor::tbutils::ThresholdScan)

void gem::supervisor::tbutils::ThresholdScan::ConfigParams::registerFields(xdata::Bag<ConfigParams> *bag)
{
  latency   = 12U;
  minThresh = -80;
  maxThresh = 20;
  stepSize  = 5U;

  deviceVT1 = 0x0;
  deviceVT2 = 0x0;

  bag->addField("minThresh", &minThresh);
  bag->addField("maxThresh", &maxThresh);
  bag->addField("stepSize",  &stepSize );
  bag->addField("deviceVT1", &deviceVT1);
  bag->addField("deviceVT2", &deviceVT2);

}


gem::supervisor::tbutils::ThresholdScan::ThresholdScan(xdaq::ApplicationStub * s) throw (xdaq::exception::Exception) :
  gem::supervisor::tbutils::GEMTBUtil(s)
{

  getApplicationInfoSpace()->fireItemAvailable("scanParams", &scanParams_);
  getApplicationInfoSpace()->fireItemValueRetrieve("scanParams", &scanParams_);

  // HyperDAQ bindings
  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::ThresholdScan::webDefault,      "Default"    );
  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::ThresholdScan::webConfigure,    "Configure"  );
  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::ThresholdScan::webStart,        "Start"      );
  runSig_   = toolbox::task::bind(this, &ThresholdScan::run,        "run"       );

  // Initiate and activate main workloop  
  wl_ = toolbox::task::getWorkLoopFactory()->getWorkLoop("urn:xdaq-workloop:GEMTestBeamSupervisor:ThresholdScan","waiting");
  wl_->activate();

}

gem::supervisor::tbutils::ThresholdScan::~ThresholdScan()
{
  wl_ = toolbox::task::getWorkLoopFactory()->getWorkLoop("urn:xdaq-workloop:GEMTestBeamSupervisor:ThresholdScan","waiting");
  //should we check to see if it's running and try to stop?
  wl_->cancel();
  wl_ = 0;

}

// State transitions
bool gem::supervisor::tbutils::ThresholdScan::run(toolbox::task::WorkLoop* wl)
{
  wl_semaphore_.take(); //teake workloop
  if (!is_running_) {
    wl_semaphore_.give(); // give work loop if it is not running
    uint32_t bufferDepth = 0;
    bufferDepth = glibDevice_->getFIFOVFATBlockOccupancy(readout_mask);
    LOG4CPLUS_INFO(getApplicationLogger()," ******IT IS NOT RUNNIG ***** ");
    return false;
  }

  //send triggers
  hw_semaphore_.take(); //take hw to send the trigger 

  //gem::hw::amc13::AMC13Manager::sendTriggerBurst();
  sendAMC13trigger();

  //count triggers
  optohybridDevice_->setTrigSource(0x0);// trigger sources   
  confParams_.bag.triggersSeen = optohybridDevice_->getL1ACount(0x0);
  
  LOG4CPLUS_INFO(getApplicationLogger(), " ABC TriggersSeen " << confParams_.bag.triggersSeen);

  hw_semaphore_.give(); //give hw to send the trigger 
  
  // if triggersSeen < N triggers
  if ((uint64_t)(confParams_.bag.triggersSeen) < (uint64_t)(confParams_.bag.nTriggers)) {
    
    hw_semaphore_.take(); // take hw to set buffer depth

    // Get the size of GLIB data buffer       
    uint32_t bufferDepth = 0;
    bufferDepth  = glibDevice_->getFIFOOccupancy(readout_mask); 

    LOG4CPLUS_INFO(getApplicationLogger(), " Bufferdepht " << bufferDepth);    

    hw_semaphore_.give(); // give hw to set buffer depth
    wl_semaphore_.give();//give workloop to read
    return true;
  }//end if triggerSeen < nTrigger
  else {
    
    hw_semaphore_.take(); //take hw to set Runmode 0 on VFATs 
    for (auto chip = vfatDevice_.begin(); chip != vfatDevice_.end(); ++chip) {
      (*chip)->setRunMode(0);
    }// end for  
    
    uint32_t bufferDepth = 0;
    bufferDepth = glibDevice_->getFIFOVFATBlockOccupancy(readout_mask);
    
    //reset counters
    optohybridDevice_->resetL1ACount(0x5);
    optohybridDevice_->resetResyncCount();
    optohybridDevice_->resetBC0Count();
    optohybridDevice_->resetCalPulseCount(0x1);
    
    hw_semaphore_.give();  // give hw to reset counters

    LOG4CPLUS_INFO(getApplicationLogger()," ABC Scan point TriggersSeen " 
		   << confParams_.bag.triggersSeen );
    
    if ( (unsigned)scanParams_.bag.deviceVT1 == (unsigned)0x0 ) {
      wl_semaphore_.give();
      wl_->submit(stopSig_);
      return false;
      
    }//end if deviceVT1=0
    else if ( (scanParams_.bag.deviceVT2-scanParams_.bag.deviceVT1) <= scanParams_.bag.maxThresh ) {    
      //if VT2 - VT1 <= maxThreshold
      
      hw_semaphore_.take(); // take hw to set threshold values
      
      LOG4CPLUS_INFO(getApplicationLogger()," ABC run: Latency= "
		     << scanParams_.bag.latency << " VT1= "
		     << scanParams_.bag.deviceVT1 << " VT2= "
		     << scanParams_.bag.deviceVT2 << 
		     " bag.maxThresh= " << scanParams_.bag.maxThresh 
		     << " abs(VT2-VT1) " 
		     << abs(scanParams_.bag.deviceVT2-scanParams_.bag.deviceVT1) );

      //if VT1 > stepSize      
      if (scanParams_.bag.deviceVT1 > scanParams_.bag.stepSize) {

	for (auto chip = vfatDevice_.begin(); chip != vfatDevice_.end(); ++chip) {
	  (*chip)->setVThreshold1(scanParams_.bag.deviceVT1 - scanParams_.bag.stepSize);
	}
      } else { //end if VT1 > stepsize, begin else
	for (auto chip = vfatDevice_.begin(); chip != vfatDevice_.end(); ++chip) {
	  (*chip)->setVThreshold1(0);
	}
      }// end else VT1 <stepsize

      
      uint32_t bufferDepth = 0;
      bufferDepth = glibDevice_->getFIFOVFATBlockOccupancy(readout_mask);    
      
      for (auto chip = vfatDevice_.begin(); chip != vfatDevice_.end(); ++chip) {
	scanParams_.bag.deviceVT1    = (*chip)->getVThreshold1();
	scanParams_.bag.deviceVT2    = (*chip)->getVThreshold2();
      }	

      glibDevice_->setDAQLinkRunParameter(2,scanParams_.bag.deviceVT1);
      glibDevice_->setDAQLinkRunParameter(3,scanParams_.bag.deviceVT2);

      sleep(0.001);

      for (auto chip = vfatDevice_.begin(); chip != vfatDevice_.end(); ++chip) {
	(*chip)->setRunMode(1);
      }
      
      confParams_.bag.triggersSeen = 0;

      //send Resync
      optohybridDevice_->sendResync();     
      optohybridDevice_->sendBC0();          

      hw_semaphore_.give(); // give hw to set threshold values
      wl_semaphore_.give(); // emd of workloop	
      return true;	
    }//else if VT2-VT1 < maxthreshold 
    else {
      hw_semaphore_.take(); // take hw to stop workloop
      wl_->submit(stopSig_);  
      hw_semaphore_.give(); // give hw to stop workloop
      wl_semaphore_.give(); // end of workloop	      
      return true; 
    }//end else
    
    //    return true;
  }//end else triggerseen < N triggers
  
  return false; 
}//end run

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
      head.addHeader("Refresh","2");
    }
    else if (is_running_) {
      cgicc::HTTPResponseHeader &head = out->getHTTPResponseHeader();
      head.addHeader("Refresh","5");
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

      selectOptohybridDevice(out);
      selectMultipleVFAT(out);
      scanParameters(out);
      
      *out << cgicc::input().set("type", "submit")
	.set("name", "command").set("title", "Initialize hardware acces.")
	.set("value", "Initialize") << std::endl;

      *out << cgicc::form() << std::endl;
    }
    
    else if (!is_configured_) {
      //this will allow the parameters to be set to the chip and scan routine

      *out << cgicc::form().set("method","POST").set("action", "/" + getApplicationDescriptor()->getURN() + "/Configure") << std::endl;

      selectOptohybridDevice(out);
      selectMultipleVFAT(out);

      scanParameters(out);

      *out << cgicc::input().set("type","text").set("name","xmlFilename").set("size","80")
	.set("ENCTYPE","multipart/form-data").set("readonly")
	.set("value",confParams_.bag.settingsFile.toString()) << std::endl;
      
      *out << cgicc::br() << std::endl;
      *out << cgicc::input().set("type", "submit")
	.set("name", "command").set("title", "Configure threshold scan.")
	.set("value", "Configure") << std::endl;
      *out << cgicc::form()        << std::endl;
    }
    
    else if (!is_running_) {

      //hardware is initialized and configured, we can start the run
      *out << cgicc::form().set("method","POST").set("action", "/" + getApplicationDescriptor()->getURN() + "/Start") << std::endl;

      selectOptohybridDevice(out);
      selectMultipleVFAT(out);      

      scanParameters(out);
      
      *out << cgicc::input().set("type", "submit")
	.set("name", "command").set("title", "Start threshold scan.")
	.set("value", "Start") << std::endl;
      *out << cgicc::form()    << std::endl;
    }
    
    else if (is_running_) {
      *out << cgicc::form().set("method","POST").set("action", "/" + getApplicationDescriptor()->getURN() + "/Stop") << std::endl;
      
      selectOptohybridDevice(out);
      selectMultipleVFAT(out);

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
//open fast commands
    if (is_initialized_)
      fastCommandLayout(out);
    *out << "</div>" << std::endl; //close fastcommands
    
    *out << "</div>" << std::endl;

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

  }// end try
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

    //sending SOAP message
    //    sendMessage(in,out);
    
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

  //--------------------AMC13 Configure --------------

  latency_   = scanParams_.bag.latency;
  nTriggers_ = confParams_.bag.nTriggers;
  stepSize_  = scanParams_.bag.stepSize;
  minThresh_ = scanParams_.bag.minThresh;
  maxThresh_ = scanParams_.bag.maxThresh;

  NTriggersAMC13();
  sendConfigureMessageAMC13();
  sendConfigureMessageGLIB();
  
  hw_semaphore_.take();

  confParams_.bag.triggersSeen = 0;
  totaltriggers = 0;
  confParams_.bag.triggercount = 0;
  
  for (auto chip = vfatDevice_.begin(); chip != vfatDevice_.end(); ++chip) {
    (*chip)->setRunMode(0);


    LOG4CPLUS_INFO(getApplicationLogger(),"loading default settings");
    //default settings for the frontend
    (*chip)->setTriggerMode(    0x3); //set to S1 to S8
    (*chip)->setCalibrationMode(0x0); //set to normal
    (*chip)->setMSPolarity(     0x1); //negative
    (*chip)->setCalPolarity(    0x1); //negative
    
    (*chip)->setProbeMode(        0x0);
    (*chip)->setLVDSMode(         0x0);
    (*chip)->setDACMode(          0x0);
    (*chip)->setHitCountCycleTime(0x0); //maximum number of bits
    
    (*chip)->setHitCountMode( 0x0);
    (*chip)->setMSPulseLength(0x3);
    (*chip)->setInputPadMode( 0x0);
    (*chip)->setTrimDACRange( 0x0);
    (*chip)->setBandgapPad(   0x0);
    (*chip)->sendTestPattern( 0x0);
        
    (*chip)->setIPreampIn(  168);
    (*chip)->setIPreampFeed(150);
    (*chip)->setIPreampOut(  80);
    (*chip)->setIShaper(    150);
    (*chip)->setIShaperFeed(100);
    (*chip)->setIComp(      120);

    (*chip)->setLatency(latency_);
    //}
  
    (*chip)->setVThreshold1(maxThresh_-minThresh_);
    (*chip)->setVThreshold2(std::max(0,maxThresh_));
    scanParams_.bag.deviceVT1 = (*chip)->getVThreshold1();
    scanParams_.bag.deviceVT2 = (*chip)->getVThreshold2();

    scanParams_.bag.latency = (*chip)->getLatency();

  }

  // flush FIFO, how to disable a specific, misbehaving, chip
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

  glibDevice_->setDAQLinkRunParameter(2,scanParams_.bag.deviceVT1);
  glibDevice_->setDAQLinkRunParameter(3,scanParams_.bag.deviceVT2);

  //reset counters
  optohybridDevice_->resetL1ACount(0x5);
  optohybridDevice_->resetResyncCount();
  optohybridDevice_->resetBC0Count();
  optohybridDevice_->resetCalPulseCount(0x1);
  optohybridDevice_->sendResync();     
  optohybridDevice_->sendBC0();          

  is_configured_ = true;
  hw_semaphore_.give();

  is_working_    = false;
}


void gem::supervisor::tbutils::ThresholdScan::startAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {
  
  is_working_ = true;
  sendStartMessageGLIB();
  sendStartMessageAMC13();
  //  gem::base::GEMFSMApplication* p_gemFSMApp_glib;
  //  std::string state =  gem::hw::glib::GLIBManager->getCurrentState();
  //  dynamic_cast<gem::hw::glib::GLIBManager*>(p_gemFSMApp)->getCurrentState();
  //  std::string state = glibManager_->getCurrentState();  
  sleep(1);

  //(gem::base::GEMFSMApplication* p_gemFSMApp)->getCurrentState();

  //  std::string state = fsm_.getStateName(fsm_.getCurrentState());
  //  std::string state =  dynamic_cast<gem::hw::amc13::AMC13Manager*>(p_gemFSMApp)->getCurrentState());

//gem::hw::glib::GLIBManager::getCurrentState();
//  INFO("GLIB Manager STATE : " << state );

  
  
  //AppHeader ah;
  latency_   = scanParams_.bag.latency;
  nTriggers_ = confParams_.bag.nTriggers;
  stepSize_  = scanParams_.bag.stepSize;
  minThresh_ = scanParams_.bag.minThresh;
  maxThresh_ = scanParams_.bag.maxThresh;

  //char data[128/8]
  is_running_ = true;
  hw_semaphore_.take();
  
  //set trigger source
  optohybridDevice_->setTrigSource(0x0);// trigger sources   

  for (auto chip = vfatDevice_.begin(); chip != vfatDevice_.end(); ++chip) {
    (*chip)->setVThreshold1(maxThresh_-minThresh_);
    (*chip)->setVThreshold2(std::max(0,maxThresh_));
    scanParams_.bag.deviceVT1 = (*chip)->getVThreshold1();
    scanParams_.bag.deviceVT2 = (*chip)->getVThreshold2();
    
    scanParams_.bag.latency = (*chip)->getLatency();
  }

  //start readout

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

  optohybridDevice_->sendResync();      
  optohybridDevice_->sendBC0();          

  glibDevice_->setDAQLinkRunType(1);
  glibDevice_->setDAQLinkRunParameter(1,scanParams_.bag.latency);
  glibDevice_->setDAQLinkRunParameter(2,scanParams_.bag.deviceVT1);
  glibDevice_->setDAQLinkRunParameter(3,scanParams_.bag.deviceVT2);


  for (auto chip = vfatDevice_.begin(); chip != vfatDevice_.end(); ++chip) {
    (*chip)->setRunMode(1);
  }

  //reset counters
  optohybridDevice_->resetL1ACount(0x5);
  optohybridDevice_->resetResyncCount();
  optohybridDevice_->resetBC0Count();
  optohybridDevice_->resetCalPulseCount(0x1);
  optohybridDevice_->sendResync();      
  optohybridDevice_->sendBC0();          
  optohybridDevice_->setTrigSource(0x1);// trigger sources   


  wl_->submit(runSig_);


  hw_semaphore_.give();
  //start scan routine
  
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

//void gem::supervisor::tbutils::ThresholdScan::sendMessage(xgi::Input *in, xgi::Output *out)

void gem::supervisor::tbutils::ThresholdScan::sendConfigureMessageGLIB()
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


bool gem::supervisor::tbutils::ThresholdScan::sendStartMessageGLIB()
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

void gem::supervisor::tbutils::ThresholdScan::NTriggersAMC13()
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


void gem::supervisor::tbutils::ThresholdScan::sendConfigureMessageAMC13()
  throw (xgi::exception::Exception) {
  //  is_working_ = true;

  xoap::MessageReference msg = xoap::createMessage();
  xoap::SOAPPart soap = msg->getSOAPPart();
  xoap::SOAPEnvelope envelope = soap.getEnvelope();
  xoap::SOAPBody body = envelope.getBody();
  xoap::SOAPName command = envelope.createName("Configure","xdaq", "urn:xdaq-soap:3.0");
  body.addBodyElement(command);

  xdaq::ApplicationDescriptor * d = getApplicationContext()->getDefaultZone()->getApplicationDescriptor("gem::hw::amc13::AMC13Manager", 3);
  xdaq::ApplicationDescriptor * o = this->getApplicationDescriptor();
  std::string    appUrn   = "urn:xdaq-application:"+d->getClassName();

  try 
    {
      xoap::MessageReference reply = getApplicationContext()->postSOAP(msg, *o,  *d);
      std::string tool;
      DEBUG("dumpTree(reply)");
      xoap::dumpTree(reply->getSOAPPart().getEnvelope().getDOMNode(),tool);
      DEBUG("reply: " << tool);
    }
  catch (xdaq::exception::Exception& e)
    {
      LOG4CPLUS_INFO(getApplicationLogger(),"------------------Fail sending AMC13 configure message " << e.what());
      XCEPT_RETHROW (xgi::exception::Exception, "Cannot send message", e);
    }
  //  this->Default(in,out);
  LOG4CPLUS_INFO(getApplicationLogger(),"-----------The message to AMC13 configure has been sent------------");


}      





bool gem::supervisor::tbutils::ThresholdScan::sendStartMessageAMC13()
  throw (xgi::exception::Exception) {
  //  is_working_ = true;
  xoap::MessageReference msg = xoap::createMessage();
  xoap::SOAPPart soap = msg->getSOAPPart();
  xoap::SOAPEnvelope envelope = soap.getEnvelope();
  xoap::SOAPBody body = envelope.getBody();
  xoap::SOAPName command = envelope.createName("Start","xdaq", "urn:xdaq-soap:3.0");
  
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

void gem::supervisor::tbutils::ThresholdScan::sendAMC13trigger()
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






