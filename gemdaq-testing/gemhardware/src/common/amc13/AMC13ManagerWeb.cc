// AMC13ManagerWeb.cc

#include "gem/hw/amc13/AMC13ManagerWeb.h"
#include "gem/hw/amc13/AMC13Manager.h"

#include "gem/hw/amc13/exception/Exception.h"

#include "xcept/tools.h"
#include "amc13/AMC13.hh"
#include "amc13/Status.hh"

gem::hw::amc13::AMC13ManagerWeb::AMC13ManagerWeb(gem::hw::amc13::AMC13Manager* amc13App) :
  gem::base::GEMWebApplication(amc13App)
{
  level = 5;
}

gem::hw::amc13::AMC13ManagerWeb::~AMC13ManagerWeb()
{
  //default destructor
}

/*To be filled in with the monitor page code
 * right now it just prints out the status page that the AMC13 generates
 * in the future it will be nice to add other monitoring to a separate tab perhaps
 * need a way to have the content be dynamic with, e.g., AJAX
*/
void gem::hw::amc13::AMC13ManagerWeb::monitorPage(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  INFO("monitorPage");

  // process the form for the debug level, how do we set the default of 2 for the first load?
  DEBUG("current level is "      << level);
  if (level != 5)
    try {
      cgicc::Cgicc cgi(in);
      int radio_i       = cgi["level"]->getIntegerValue();
      DEBUG("radio button value is " << radio_i);
      level = static_cast<size_t>(radio_i);
      DEBUG("setting AMC13 display status info to " << level);
    } catch (const xgi::exception::Exception& e) {
      level = 2;
      WARN("Caught xgi::exception " << e.what());
      XCEPT_RAISE(xgi::exception::Exception, e.what());
    } catch (const std::exception& e) {
      level = 2;
      WARN("Caught std::exception " << e.what());
      XCEPT_RAISE(xgi::exception::Exception, e.what());
    }
  else 
    level = 2;
  
  DEBUG("current level is "      << level);

  //form and control to set the display level of information
  std::string method = toolbox::toString("/%s/monitorView",gemFSMAppP_->getApplicationDescriptor()->getURN().c_str());
  *out << cgicc::form().set("method","POST").set("action",method) << std::endl;
  
  *out << cgicc::section().set("style","display:inline-block;float:left") << std::endl
       << cgicc::fieldset().set("style","display:block;padding:5px;margin:5px;list-style-type:none;margin-bottom:5px;line-height:18px;padding:2px 5px;-webkit-border-radius:5px;-moz-border-radius:5px;border-radius:5px;border:medium outset #CCC;")
       << std::endl
       << cgicc::legend("AMC13 Status")    << std::endl
       << cgicc::br()                      << std::endl
       << cgicc::div().set("align","left") << std::endl

       << cgicc::input().set("style","display:inline-block;margin-right:25px;margin-left:25px;float:center;-webkit-border-radius: 5px;-moz-border-radius: 5px;border-radius: 5px;border:medium outset #CCC;")
    .set("type","submit")
    .set("value","Set level")
    .set("name","setLevel") << std::endl

       << (level == 0 ?
	   cgicc::input().set("type","radio").set("name","level").set("value","0").set("checked") :
	   cgicc::input().set("type","radio").set("name","level").set("value","0"))
       << "version" << std::endl
       << (level == 1 ?
	   cgicc::input().set("type","radio").set("name","level").set("value","1").set("checked") :
	   cgicc::input().set("type","radio").set("name","level").set("value","1"))
       << "minimum" << std::endl
       << (level == 2 ?
	   cgicc::input().set("type","radio").set("name","level").set("value","2").set("checked") :
	   cgicc::input().set("type","radio").set("name","level").set("value","2"))
       << "default" << std::endl
       << (level == 3 ?
	   cgicc::input().set("type","radio").set("name","level").set("value","3").set("checked") :
	   cgicc::input().set("type","radio").set("name","level").set("value","3"))
       << "maximum" << std::endl

       << cgicc::br()     << std::endl
       << cgicc::div()    << std::endl
       << cgicc::span().set("style","display:block;float:left") << std::endl;
  
  try {
    if (dynamic_cast<gem::hw::amc13::AMC13Manager*>(gemFSMAppP_)->getAMC13Device()) {
      ::amc13::Status *s = dynamic_cast<gem::hw::amc13::AMC13Manager*>(gemFSMAppP_)->getAMC13Device()->getStatus();
      s->SetHTML();
      s->Report(level,*out);
    } else {
      XCEPT_RAISE(gem::hw::amc13::exception::HardwareProblem, "Unable to obtain pointer to AMC13 device");
    }
  } catch (const gem::hw::amc13::exception::HardwareProblem& e) {
    WARN("Unable to display the AMC13 status page: " << e.what());
  } catch (const std::exception& e) {
    WARN("Unable to display the AMC13 status page: " << e.what());
  }

  *out << cgicc::span()     << std::endl
       << cgicc::fieldset() << std::endl
       << cgicc::section()  << std::endl
       << cgicc::form()     << std::endl;

  //will be nice to have the "last update time" printed on the bottom of all monitoring or status pages
  //obtain this value from an infospace?
}

/*To be filled in with the expert page code*/
void gem::hw::amc13::AMC13ManagerWeb::expertPage(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  INFO("expertPage");
  //fill this page with the expert views for the AMC13Manager
  *out << cgicc::section().set("style","display:inline-block;float:left") << std::endl
       << cgicc::fieldset().set("style","display:block;padding:5px;margin:5px;list-style-type:none;margin-bottom:5px;line-height:18px;padding:2px 5px;-webkit-border-radius:5px;-moz-border-radius:5px;border-radius:5px;border:medium outset #CCC;")
       << std::endl
       << cgicc::legend("GEM AMC13Manager expert page")    << std::endl
       << cgicc::br()                      << std::endl
       << cgicc::span()     << std::endl
       << cgicc::fieldset() << std::endl
       << cgicc::section()  << std::endl;
}

