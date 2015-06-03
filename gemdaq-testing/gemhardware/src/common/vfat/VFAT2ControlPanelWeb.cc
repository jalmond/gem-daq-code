#include "gem/hw/vfat/VFAT2Manager.h"

//#include <boost/lexical_cast.hpp>
//#include <boost/format.hpp>

void gem::hw::vfat::VFAT2Manager::VFAT2ControlPanelWeb::createHeader(xgi::Output *out ) {

*out << "<!doctype html>" << std::endl
     << "<html xmlns=\"http://www.w3.org/1999/xhtml\">" << std::endl
     << "<head>" << std::endl
     << "    <meta charset=\"UTF-8\" />" << std::endl
     << "    <meta name=\"viewport\" content=\"width = device-width\">" << std::endl
     << "    <title>XDAQ HyperDAQ</title>" << std::endl
     << "    <link rel=\"shortcut icon\" href=\"/xdaq/images/xdaq.ico\">" << std::endl
     << "    <link rel=\"apple-touch-icon\" href=\"/xdaq/images/xdaq.ico\"/>" << std::endl
     << "    <link href=\"/hyperdaq/html/css/xdaq-normalize.css\" rel=\"stylesheet\" />" << std::endl
     << "    <link href=\"/hyperdaq/html/css/xdaq-buttons.css\" rel=\"stylesheet\" />" << std::endl
     << "    <link href=\"/hyperdaq/html/css/xdaq-generics.css\" rel=\"stylesheet\" />" << std::endl
     << "    <link href=\"/hyperdaq/html/css/xdaq-layout.css\" rel=\"stylesheet\" />" << std::endl
     << "    <link href=\"/hyperdaq/html/css/xdaq-fonts.css\" rel=\"stylesheet\" />" << std::endl
     << "    <link href=\"/hyperdaq/html/css/xdaq-forms.css\" rel=\"stylesheet\" />" << std::endl
     << "    <link href=\"/hyperdaq/html/css/xdaq-tables.css\" rel=\"stylesheet\" />" << std::endl
     << "    <link href=\"/hyperdaq/html/css/xdaq-trees.css\" rel=\"stylesheet\" />" << std::endl
     << "    <link href=\"/hyperdaq/html/css/xdaq-tooltips.css\" rel=\"stylesheet\" />" << std::endl
     << "    <link href=\"/hyperdaq/html/css/xdaq-tabs.css\" rel=\"stylesheet\" />" << std::endl
     << "    <link href=\"/hyperdaq/html/css/xdaq-console.css\" rel=\"stylesheet\" />" << std::endl
     << "    <link href=\"/hyperdaq/html/css/xdaq-hyperdaq-app-widgets.css\" rel=\"stylesheet\" />" << std::endl
     << "    <script type=\"text/javascript\" src=\"/hyperdaq/html/js/jquery-2.1.0.js\"></script>" << std::endl
  //<< "    <script type=\"text/javascript\" src=\"/hyperdaq/html/js/jquery-2.1.0.min.js\"></script>" << std::endl
     << "    <script type=\"text/javascript\" src=\"/hyperdaq/html/js/xdaq-utils.js\"></script>" << std::endl
     << "    <script type=\"text/javascript\" src=\"/hyperdaq/html/js/xdaq-main-layout.js\"></script>" << std::endl
     << "    <script type=\"text/javascript\" src=\"/hyperdaq/html/js/xdaq-tabs.js\"></script>" << std::endl
     << "    <script type=\"text/javascript\" src=\"/hyperdaq/html/js/xdaq-treetable.js\"></script>" << std::endl
     << "    <script type=\"text/javascript\" src=\"/hyperdaq/html/js/xdaq-tree-utils.js\"></script>" << std::endl
     << "    <script type=\"text/javascript\" src=\"/hyperdaq/html/js/xdaq-console.js\"></script>" << std::endl
     << "    <script type=\"text/javascript\" src=\"/hyperdaq/html/js/xdaq-sort.js\"></script>" << std::endl
     << "    <script type=\"text/javascript\" src=\"/hyperdaq/html/js/xdaq-window-load.js\"></script>" << std::endl

     << "<link rel=\"stylesheet\" type=\"text/css\" href=\"gemdaq/gemhardware/html/css/vfat//vfatchannelregister.css\"/>" << std::endl
     << "<link rel=\"stylesheet\" type=\"text/css\" href=\"gemdaq/gemhardware/html/css/vfat//vfatcommands.css\"/>"        << std::endl
     << "<link rel=\"stylesheet\" type=\"text/css\" href=\"gemdaq/gemhardware/html/css/vfat//vfatcontrolregisters.css\"/>"      << std::endl
     << "<link rel=\"stylesheet\" type=\"text/css\" href=\"gemdaq/gemhardware/html/css/vfat//vfatglobalsettings.css\"/>"      << std::endl
     << "<link rel=\"stylesheet\" type=\"text/css\" href=\"gemdaq/gemhardware/html/css/vfat//vfatmanager.css\"/>"<< std::endl;
 
 *out << "<script>" << std::endl;
 *out << "window.onload = function()" << std::endl
      << "    alert( \"welcome\" );"  << std::endl
      << "}"                          << std::endl
      << "</script>"     << std::endl << std::endl;
 
 *out << "<script>" << std::endl;
 *out << "$( document ).ready(function() {"           << std::endl
      << "    $( \"a\" ).click(function( event ) {"   << std::endl
      << "        alert( \"Thanks for visiting!\" );" << std::endl
      << "    });"                                    << std::endl
      << "});"                                        << std::endl;
 *out << "</script>"  << std::endl << std::endl;
 *out << "</head>" << std::endl;

}

//building the control view
void gem::hw::vfat::VFAT2Manager::VFAT2ControlPanelWeb::createVFATInfoLayout(xgi::Output *out, const gem::hw::vfat::VFAT2ControlParams params) {
  //Report the GEB information (slot, fibre, etc.,)
  *out << cgicc::label("ChipID").set("for","ChipID") << std::endl
       << cgicc::input().set("type","text").set("id","ChipID")
                        .set("name","ChipID").set("readonly")
                        .set("value",boost::str(boost::format("0x%04x")%(params.chipID)))
       << std::endl
       << cgicc::br() << std::endl
       << "          <fieldset class=\"vfatPosition\">" << std::endl
       << cgicc::legend("VFAT2 Position") << std::endl
       << "            <div>" << std::endl
       << cgicc::label("GLIB").set("for","GLIB") << std::endl
       << cgicc::input().set("id","GLIB").set("type","text")
                        .set("name","GLIB").set("readonly")
       << std::endl
       << cgicc::label("Fibre").set("for","GLIBFibre") << std::endl
       << cgicc::input().set("id","GLIBFibre").set("type","text")
                        .set("name","GLIBFibre").set("readonly")
       << std::endl
       << cgicc::br() << std::endl
       << cgicc::label("GEB").set("for","GEB") << std::endl
       << cgicc::input().set("id","GEB").set("type","text")
                        .set("name","GEB").set("readonly")
       << std::endl
       << cgicc::label("Slot").set("for","Slot") << std::endl
       << cgicc::input().set("id","Slot").set("type","text")
                        .set("name","Slot").set("readonly")
       << std::endl
       << cgicc::label("Fibre").set("for","OHFibre") << std::endl
       << cgicc::input().set("id","OHFibre").set("type","text")
                        .set("name","OHFibre").set("readonly")
       << std::endl
       << "            </div>" << std::endl
       << cgicc::comment() << "ending the VFAT2 Position fieldset" << cgicc::comment() << std::endl
       << "          </fieldset>" << std::endl;
}

//building the counter view
void gem::hw::vfat::VFAT2Manager::VFAT2ControlPanelWeb::createCounterLayout(xgi::Output *out, const gem::hw::vfat::VFAT2ControlParams params) {
  *out << "          <fieldset class=\"vfatCounters\">" << std::endl
       << cgicc::legend("VFAT2 Counters") << std::endl
       << cgicc::label("Upset counter").set("for","Upset") << std::endl
       << cgicc::input().set("class","vfatBiasInput").set("type","text")
                        .set("id","Upset").set("name","Upset").set("readonly")
    .set("value",boost::str(boost::format("0x%02x")%(static_cast<unsigned>(params.upsetCounter))))
       << std::endl
       << cgicc::br() << std::endl
       << std::endl
       << cgicc::label("HitCount").set("for","HitCount") << std::endl
       << cgicc::input().set("class","vfatBiasInput").set("type","text")
                        .set("name","HitCount").set("readonly")
                        .set("value",boost::str(boost::format("0x%06x")%(params.hitCounter)))
    /*
       << std::endl
       << cgicc::br() << std::endl
       << std::endl
       << cgicc::label("HitCount0").set("for","HitCount0") << std::endl
       << cgicc::input().set("class","vfatBiasInput").set("type","text")
                        .set("name","HitCount0").set("readonly")
                        .set("value",boost::str(boost::format("0x%02x")%(params.hitCounter&0x000000ff)))
       << std::endl
       << cgicc::br() << std::endl
       << std::endl
       << cgicc::label("HitCount1").set("for","HitCount1") << std::endl
       << cgicc::input().set("class","vfatBiasInput").set("type","text")
                        .set("name","HitCount1").set("readonly")
                        .set("value",boost::str(boost::format("0x%02x")%(params.hitCounter&0x0000ff00)))
       << std::endl
       << cgicc::br() << std::endl
       << std::endl
       << cgicc::label("HitCount2").set("for","HitCount2") << std::endl
       << cgicc::input().set("class","vfatBiasInput").set("type","text")
                        .set("name","HitCount2").set("readonly")
                        .set("value",boost::str(boost::format("0x%02x")%(params.hitCounter&0x00ff0000)))
       << std::endl
    */
       << cgicc::br() << std::endl
       << std::endl
    
       << cgicc::input().set("class","vfatButtonInput").set("type","submit")
                        //.set("value","Read counters").set("name","ReadCounters")
                        .set("value","Read counters").set("name","VFAT2ControlOption")
       << std::endl
       << cgicc::comment() << "ending the Counters fieldset" <<cgicc::comment() << std::endl
       << "          </fieldset>" << std::endl;  
}

//building the control register view
void gem::hw::vfat::VFAT2Manager::VFAT2ControlPanelWeb::createControlRegisterLayout(xgi::Output *out, const gem::hw::vfat::VFAT2ControlParams params) {
  *out << "          <fieldset class=\"vfatControl\">"           << std::endl
       << cgicc::legend("VFAT2 Control registers") << std::endl
       << "            <section>"                                << std::endl
       << "              <div class=\"controlRegister\">"        << std::endl
       << "                <div class=\"controlRegisterBits\">"  << std::endl
       << "            	     <div class=\"controlRegisterDropdownEntry\">"          << std::endl
       << cgicc::label("CalMode &lt;7:6&gt;").set("for","CalMode")      << std::endl
       << cgicc::select().set("id","CalMode").set("name","CalMode")     << std::endl
       //<< cgicc::option("Normal"  ).set("value","Normal"  ).set("selected") << std::endl
       //<< (strcmp(gem::hw::vfat::CalibrationModeToString.at(params.calibMode),"NORMAL") == 0 ?
       << ((gem::hw::vfat::CalibrationModeToString.at(params.calibMode).compare("NORMAL")) == 0 ?
	   (cgicc::option("Normal").set("value","Normal").set("selected")) :
	   (cgicc::option("Normal").set("value","Normal"))) << std::endl
       //<< cgicc::option("Normal"    ).set("value","Normal"    )   << std::endl
       //<< ((gem::hw::vfat::CalibrationModeToString.at(params.calibMode).compare("VCAL")) == 0 ?
       << ((gem::hw::vfat::CalibrationModeToString.at(params.calibMode).compare("VHI")) == 0 ?
	   (cgicc::option("VCal").set("value","VHi").set("selected")) :
	   (cgicc::option("VCal").set("value","VHi"))) << std::endl
       //<< cgicc::option("Baseline").set("value","Baseline")   << std::endl
       //<< ((gem::hw::vfat::CalibrationModeToString.at(params.calibMode).compare("BASELINE")) == 0 ?
       << ((gem::hw::vfat::CalibrationModeToString.at(params.calibMode).compare("VLOW")) == 0 ?
	   (cgicc::option("Baseline").set("value","VLow").set("selected")) :
	   (cgicc::option("Baseline").set("value","VLow"))) << std::endl
       //<< cgicc::option("External").set("value","External")   << std::endl
       << ((gem::hw::vfat::CalibrationModeToString.at(params.calibMode).compare("EXTERNAL")) == 0 ?
	   (cgicc::option("External").set("value","External").set("selected")) :
	   (cgicc::option("External").set("value","External"))) << std::endl
       << cgicc::select()                                     << std::endl
       << "                  </div>"                          << std::endl;
	             
  *out << "            	     <div class=\"controlRegisterDropdownEntry\">"  << std::endl
       << cgicc::label("CalPolarity &lt;5&gt;").set("for","CalPolarity")    << std::endl
       << cgicc::select().set("id","CalPolarity").set("name","CalPolarity") << std::endl
       //<< cgicc::option("Positive").set("value","Positive").set("selected") << std::endl
       << ((gem::hw::vfat::CalPolarityToString.at(params.calPol).compare("POSITIVE")) == 0 ?
	   (cgicc::option("Positive").set("value","Positive").set("selected")) :
	   (cgicc::option("Positive").set("value","Positive"))) << std::endl
       //<< cgicc::option("Negative").set("value","Negative")                 << std::endl
       << ((gem::hw::vfat::CalPolarityToString.at(params.calPol).compare("NEGATIVE")) == 0 ?
	   (cgicc::option("Negative").set("value","Negative").set("selected")) :
	   (cgicc::option("Negative").set("value","Negative"))) << std::endl
       << cgicc::select()                                                   << std::endl
       << "            	     </div>"                                        << std::endl;
	             
  *out << "            	     <div class=\"controlRegisterDropdownEntry\">"  << std::endl
       << cgicc::label("MSPolarity &lt;4&gt;").set("for","MSPolarity")      << std::endl
       << cgicc::select().set("id","MSPolarity").set("name","MSPolarity")   << std::endl
       //<< cgicc::option("Positive").set("value","Positive").set("selected") << std::endl
       << ((gem::hw::vfat::MSPolarityToString.at(params.msPol).compare("POSITIVE")) == 0 ?
	   (cgicc::option("Positive").set("value","Positive").set("selected")) :
	   (cgicc::option("Positive").set("value","Positive"))) << std::endl
       //<< cgicc::option("Negative").set("value","Negative")                 << std::endl
       << ((gem::hw::vfat::MSPolarityToString.at(params.msPol).compare("NEGATIVE")) == 0 ?
	   (cgicc::option("Negative").set("value","Negative").set("selected")) :
	   (cgicc::option("Negative").set("value","Negative"))) << std::endl
       << cgicc::select()                                                   << std::endl
       << "            	     </div>"                                        << std::endl;
	             
  *out << "            	     <div class=\"controlRegisterDropdownEntry\">"    << std::endl
       << cgicc::label("TriggerMode &lt;3:1&gt;").set("for","TriggerMode")    << std::endl
       << cgicc::select().set("id","TriggerMode").set("name","TriggerMode")   << std::endl
       //<< cgicc::option("NoTrigger"  ).set("value","NoTrigger"  ).set("selected") << std::endl
       << ((gem::hw::vfat::TriggerModeToString.at(params.trigMode).compare("NOTRIGGER")) == 0 ?
	   (cgicc::option("No Trigger").set("value","NoTrigger").set("selected")) :
	   (cgicc::option("No Trigger").set("value","NoTrigger"))) << std::endl
       //<< cgicc::option("S1Only"     ).set("value","S1Only"     ) << std::endl
       //<< ((gem::hw::vfat::TriggerModeToString.at(params.trigMode).compare("ONEMODE")) == 0 ?
       << ((gem::hw::vfat::TriggerModeToString.at(params.trigMode).compare("S1")) == 0 ?
	   (cgicc::option("S1 Only").set("value","S1").set("selected")) :
	   (cgicc::option("S1 Only").set("value","S1"))) << std::endl
       //<< cgicc::option("FourS1toS4" ).set("value","FourS1toS4" ) << std::endl
       //<< ((gem::hw::vfat::TriggerModeToString.at(params.trigMode).compare("FOURMODE")) == 0 ?
       << ((gem::hw::vfat::TriggerModeToString.at(params.trigMode).compare("S1-S4")) == 0 ?
	   (cgicc::option("Four (S1 to S4)").set("value","S1-S4").set("selected")) :
	   (cgicc::option("Four (S1 to S4)").set("value","S1-S4"))) << std::endl
       //<< cgicc::option("EightS1toS8").set("value","EightS1toS8") << std::endl
       //<< ((gem::hw::vfat::TriggerModeToString.at(params.trigMode).compare("EIGHTMODE")) == 0 ?
       << ((gem::hw::vfat::TriggerModeToString.at(params.trigMode).compare("S1-S8")) == 0 ?
	   (cgicc::option("Eight (S1 to S8)").set("value","S1-S8").set("selected")) :
	   (cgicc::option("Eight (S1 to S8)").set("value","S1-S8"))) << std::endl
       //<< cgicc::option("GEMMode"    ).set("value","GEMMode"    ) << std::endl
       << ((gem::hw::vfat::TriggerModeToString.at(params.trigMode).compare("GEMMODE")) == 0 ?
	   (cgicc::option("GEMMode").set("value","GEMMode").set("selected")) :
	   (cgicc::option("GEMMode").set("value","GEMMode"))) << std::endl
       << cgicc::select()                                         << std::endl
       << "                  </div>"                              << std::endl;
	             
  *out << "            	     <div class=\"controlRegisterDropdownEntry\">" << std::endl
       << cgicc::label("RunMode &lt;0&gt;").set("for","RunMode")           << std::endl
       << cgicc::select().set("id","RunMode").set("name","RunMode")        << std::endl
       << ((gem::hw::vfat::RunModeToString.at(params.runMode).compare("RUN")) == 0 ?
	   (cgicc::option("Run"  ).set("value","Run"  ).set("selected")) :
	   (cgicc::option("Run"  ).set("value","Run"  ))) << std::endl
       << ((gem::hw::vfat::RunModeToString.at(params.runMode).compare("SLEEP")) == 0 ?
	   (cgicc::option("Sleep").set("value","Sleep").set("selected")) :
	   (cgicc::option("Sleep").set("value","Sleep"))) << std::endl
       << cgicc::select()                                                  << std::endl
       << "            	     </div>"                                       << std::endl
       << "                </div>"                                         << std::endl;
  	             
  *out << "                <div class=\"controlRegisterCheckBox\">" << std::endl
       << cgicc::label("Set CR #0").set("for","CR0Set")             << std::endl
       << cgicc::input().set("type","checkbox").set("id","CR0Set")
                        .set("name","CR0Set"  ).set("checked")
       << std::endl
       << "                </div>"                       << std::endl
       << "              </div>"                         << std::endl
       << cgicc::br().set("style","clear:both;")         << std::endl
       << cgicc::comment() << "ending the CR #0 section" <<cgicc::comment() << std::endl
       << "            </section>"                       << std::endl
       << std::endl;

  *out << "            <section>" << std::endl
       << "              <div class=\"controlRegister\">"       << std::endl
       << "                <div class=\"controlRegisterBits\">" << std::endl
       << "            	     <div class=\"controlRegisterDropdownEntry\">" << std::endl
       << cgicc::label("ReHitCT &lt;7:6&gt;").set("for","ReHitCT")         << std::endl
       << cgicc::select().set("id","ReHitCT").set("name","ReHitCT")        << std::endl
       //<< ((gem::hw::vfat::ReHitCTToString.at(params.reHitCT).compare("CYCLE0")) == 0 ?
       << ((gem::hw::vfat::ReHitCTToString.at(params.reHitCT).compare("6.4MICROSEC")) == 0 ?
	   (cgicc::option("6.4 us").set("value","6.4microsec").set("selected")) :
	   (cgicc::option("6.4 us").set("value","6.4microsec"))) << std::endl
       //<< ((gem::hw::vfat::ReHitCTToString.at(params.reHitCT).compare("CYCLE1")) == 0 ?
       << ((gem::hw::vfat::ReHitCTToString.at(params.reHitCT).compare("1.6MILLISEC")) == 0 ?
	   (cgicc::option("1.6 ms").set("value","1.6millesec").set("selected")) :
	   (cgicc::option("1.6 ms").set("value","1.6millesec"))) << std::endl
       //<< ((gem::hw::vfat::ReHitCTToString.at(params.reHitCT).compare("CYCLE2")) == 0 ?
       << ((gem::hw::vfat::ReHitCTToString.at(params.reHitCT).compare("0.4SEC")) == 0 ?
	   (cgicc::option("0.4 s").set("value","0.4sec").set("selected")) :
	   (cgicc::option("0.4 s").set("value","0.4sec"))) << std::endl
       //<< ((gem::hw::vfat::ReHitCTToString.at(params.reHitCT).compare("CYCLE3")) == 0 ?
       << ((gem::hw::vfat::ReHitCTToString.at(params.reHitCT).compare("107SEC")) == 0 ?
	   (cgicc::option("107 s").set("value","107sec").set("selected")) :
	   (cgicc::option("107 s").set("value","107sec"))) << std::endl
       << cgicc::select()                                                  << std::endl
       << "            	     </div>"                                       << std::endl;
	               
  *out << "            	     <div class=\"controlRegisterDropdownEntry\">"      << std::endl
       << cgicc::label("LVDSPowerSave &lt;5&gt;").set("for","LVDSPowerSave")    << std::endl
       << cgicc::select().set("id","LVDSPowerSave").set("name","LVDSPowerSave") << std::endl
    //<< ((gem::hw::vfat::LVDSPowerSaveToString.at(params.lvdsMode).compare("ENABLEALL")) == 0 ?
       << ((gem::hw::vfat::LVDSPowerSaveToString.at(params.lvdsMode).compare("OFF")) == 0 ?
	   (cgicc::option("Off").set("value","Off").set("selected")) :
	   (cgicc::option("Off").set("value","Off"))) << std::endl
    //<< ((gem::hw::vfat::LVDSPowerSaveToString.at(params.lvdsMode).compare("POWERSAVE")) == 0 ?
       << ((gem::hw::vfat::LVDSPowerSaveToString.at(params.lvdsMode).compare("ON")) == 0 ?
	   (cgicc::option("Enabled").set("value","On").set("selected")) :
	   (cgicc::option("Enabled").set("value","On"))) << std::endl
       << cgicc::select()                                                       << std::endl
       << "            	     </div>"                                            << std::endl;
  
  *out << "            	     <div class=\"controlRegisterDropdownEntry\">" << std::endl
       << cgicc::label("ProbeMode &lt;4&gt;").set("for","ProbeMode")       << std::endl
       << cgicc::select().set("id","ProbeMode").set("name","ProbeMode")    << std::endl
       << ((gem::hw::vfat::ProbeModeToString.at(params.probeMode).compare("OFF")) == 0 ?
	   (cgicc::option("Off").set("value","Off").set("selected")) :
	   (cgicc::option("Off").set("value","Off"))) << std::endl
       << ((gem::hw::vfat::ProbeModeToString.at(params.probeMode).compare("ON")) == 0 ?
	   (cgicc::option("Enabled").set("value","On").set("selected")) :
	   (cgicc::option("Enabled").set("value","On"))) << std::endl
       << cgicc::select()                                                  << std::endl
       << "            	     </div>"                                       << std::endl;
  	               
  *out << "            	     <div class=\"controlRegisterDropdownEntry\">" << std::endl
       << cgicc::label("DACMode &lt;3:0&gt;").set("for","DACMode")         << std::endl
       << cgicc::select().set("id","DACMode").set("name","DACMode")               << std::endl
       //<< ((gem::hw::vfat::DACModeToString.at(params.dacMode).compare("NORMAL")) == 0 ?
       << ((gem::hw::vfat::DACModeToString.at(params.dacMode).compare("OFF")) == 0 ?
	   (cgicc::option("Normal").set("value","Off").set("selected")) :
	   (cgicc::option("Normal").set("value","Off"))) << std::endl
       << ((gem::hw::vfat::DACModeToString.at(params.dacMode).compare("IPREAMPIN")) == 0 ?
	   (cgicc::option("IPreampIn").set("value","IPreampIn").set("selected")) :
	   (cgicc::option("IPreampIn").set("value","IPreampIn"))) << std::endl
       << ((gem::hw::vfat::DACModeToString.at(params.dacMode).compare("IPREAMPFEED")) == 0 ?
	   (cgicc::option("IPreampFeed").set("value","IPreampFeed").set("selected")) :
	   (cgicc::option("IPreampFeed").set("value","IPreampFeed"))) << std::endl
       << ((gem::hw::vfat::DACModeToString.at(params.dacMode).compare("IPREAMPOUT")) == 0 ?
	   (cgicc::option("IPreampOut").set("value","IPreampOut").set("selected")) :
	   (cgicc::option("IPreampOut").set("value","IPreampOut"))) << std::endl
       << ((gem::hw::vfat::DACModeToString.at(params.dacMode).compare("ISHAPER")) == 0 ?
	   (cgicc::option("IShaper").set("value","IShaper").set("selected")) :
	   (cgicc::option("IShaper").set("value","IShaper"))) << std::endl
       << ((gem::hw::vfat::DACModeToString.at(params.dacMode).compare("ISHAPERFEED")) == 0 ?
	   (cgicc::option("IShaperFeed").set("value","IShaperFeed").set("selected")) :
	   (cgicc::option("IShaperFeed").set("value","IShaperFeed"))) << std::endl
       << ((gem::hw::vfat::DACModeToString.at(params.dacMode).compare("ICOMP")) == 0 ?
	   (cgicc::option("IComp").set("value","IComp").set("selected")) :
	   (cgicc::option("IComp").set("value","IComp"))) << std::endl
       //<< ((gem::hw::vfat::DACModeToString.at(params.dacMode).compare("ITHRESHOLD1")) == 0 ?
       << ((gem::hw::vfat::DACModeToString.at(params.dacMode).compare("VTHRESHOLD1")) == 0 ?
	   (cgicc::option("VThreshold1").set("value","VThreshold1").set("selected")) :
	   (cgicc::option("VThreshold1").set("value","VThreshold1"))) << std::endl
       //<< ((gem::hw::vfat::DACModeToString.at(params.dacMode).compare("ITHRESHOLD2")) == 0 ?
       << ((gem::hw::vfat::DACModeToString.at(params.dacMode).compare("VTHRESHOLD2")) == 0 ?
	   (cgicc::option("VThreshold2").set("value","VThreshold2").set("selected")) :
	   (cgicc::option("VThreshold2").set("value","VThreshold2"))) << std::endl
       << ((gem::hw::vfat::DACModeToString.at(params.dacMode).compare("VCAL")) == 0 ?
	   (cgicc::option("VCal").set("value","VCal").set("selected")) :
	   (cgicc::option("VCal").set("value","VCal"))) << std::endl
       << ((gem::hw::vfat::DACModeToString.at(params.dacMode).compare("CALOUT")) == 0 ?
	   (cgicc::option("CalOut").set("value","CalOut").set("selected")) :
	   (cgicc::option("CalOut").set("value","CalOut"))) << std::endl
       << cgicc::select() << std::endl
       << "            	     </div>"      << std::endl
       << "                </div>"        << std::endl;
  	               
  *out << "                <div class=\"controlRegisterCheckBox\">" << std::endl
       << cgicc::label("Set CR #1").set("for","CR1Set")             << std::endl
       << cgicc::input().set("type","checkbox").set("id","CR1Set")
                        .set("name","CR1Set"  ).set("checked")
       << "                </div>"                          << std::endl
       << "              </div>"                            << std::endl
       << cgicc::br().set("style","clear:both;")         << std::endl
       << cgicc::comment() << "ending the CR #1 section" <<cgicc::comment() << std::endl
       << "            </section>"                          << std::endl
       << std::endl;

  *out << "            <section>" << std::endl
       << "              <div class=\"controlRegister\">"                           << std::endl
       << "                <div class=\"controlRegisterBits\">"                     << std::endl
       << "            	     <div class=\"controlRegisterDropdownEntry\">"          << std::endl
       << cgicc::label("DigInSel &lt;7&gt;"  ).set("for","DigInSel" )    << std::endl
       << cgicc::select().set("id","DigInSel").set("name","DigInSel")    << std::endl
       << ((gem::hw::vfat::DigInSelToString.at(params.digInSel).compare("ANIP")) == 0 ?
	   (cgicc::option("Analog").set("value","AnIP").set("selected")) :
	   (cgicc::option("Analog").set("value","AnIP"))) << std::endl
       << ((gem::hw::vfat::DigInSelToString.at(params.digInSel).compare("DIGIP")) == 0 ?
	   (cgicc::option("Digital").set("value","DigIP").set("selected")) :
	   (cgicc::option("Digital").set("value","DigIP"))) << std::endl
       << cgicc::select()                                                << std::endl
       << "            	     </div>"                                     << std::endl;
	               
  *out << "                  <div class=\"controlRegisterDropdownEntry\">"      << std::endl
       << cgicc::label("MSPulseLength &lt;6:4&gt;").set("for","MSPulseLength")  << std::endl
       << cgicc::select().set("id","MSPulseLength").set("name","MSPulseLength") << std::endl
       //<< ((gem::hw::vfat::MSPulseLengthToString.at(params.msPulseLen).compare("CLOCK1")) == 0 ?
       << ((gem::hw::vfat::MSPulseLengthToString.at(params.msPulseLen).compare("1")) == 0 ?
	   (cgicc::option("1 Clock").set("value","1").set("selected")) :
	   (cgicc::option("1 Clock").set("value","1"))) << std::endl
       //<< ((gem::hw::vfat::MSPulseLengthToString.at(params.msPulseLen).compare("CLOCK2")) == 0 ?
       << ((gem::hw::vfat::MSPulseLengthToString.at(params.msPulseLen).compare("2")) == 0 ?
	   (cgicc::option("2 Clocks").set("value","2").set("selected")) :
	   (cgicc::option("2 Clocks").set("value","2"))) << std::endl
       //<< ((gem::hw::vfat::MSPulseLengthToString.at(params.msPulseLen).compare("CLOCK3")) == 0 ?
       << ((gem::hw::vfat::MSPulseLengthToString.at(params.msPulseLen).compare("3")) == 0 ?
	   (cgicc::option("3 Clocks").set("value","3").set("selected")) :
	   (cgicc::option("3 Clocks").set("value","3"))) << std::endl
       //<< ((gem::hw::vfat::MSPulseLengthToString.at(params.msPulseLen).compare("CLOCK4")) == 0 ?
       << ((gem::hw::vfat::MSPulseLengthToString.at(params.msPulseLen).compare("4")) == 0 ?
	   (cgicc::option("4 Clocks").set("value","4").set("selected")) :
	   (cgicc::option("4 Clocks").set("value","4"))) << std::endl
       //<< ((gem::hw::vfat::MSPulseLengthToString.at(params.msPulseLen).compare("CLOCK5")) == 0 ?
       << ((gem::hw::vfat::MSPulseLengthToString.at(params.msPulseLen).compare("5")) == 0 ?
	   (cgicc::option("5 Clocks").set("value","5").set("selected")) :
	   (cgicc::option("5 Clocks").set("value","5"))) << std::endl
       //<< ((gem::hw::vfat::MSPulseLengthToString.at(params.msPulseLen).compare("CLOCK6")) == 0 ?
       << ((gem::hw::vfat::MSPulseLengthToString.at(params.msPulseLen).compare("6")) == 0 ?
	   (cgicc::option("6 Clocks").set("value","6").set("selected")) :
	   (cgicc::option("6 Clocks").set("value","6"))) << std::endl
       //<< ((gem::hw::vfat::MSPulseLengthToString.at(params.msPulseLen).compare("CLOCK7")) == 0 ?
       << ((gem::hw::vfat::MSPulseLengthToString.at(params.msPulseLen).compare("7")) == 0 ?
	   (cgicc::option("7 Clocks").set("value","7").set("selected")) :
	   (cgicc::option("7 Clocks").set("value","7"))) << std::endl
       //<< ((gem::hw::vfat::MSPulseLengthToString.at(params.msPulseLen).compare("CLOCK8")) == 0 ?
       << ((gem::hw::vfat::MSPulseLengthToString.at(params.msPulseLen).compare("8")) == 0 ?
	   (cgicc::option("8 Clocks").set("value","8").set("selected")) :
	   (cgicc::option("8 Clocks").set("value","8"))) << std::endl
       << cgicc::select()                               << std::endl
       << "                  </div>"                    << std::endl;
  	               
  *out << "                  <div class=\"controlRegisterDropdownEntry\">"  << std::endl
       << cgicc::label("HitCountMode &lt;3:0&gt;").set("for","HitCountMode")  << std::endl
       << cgicc::select().set("id","HitCountMode").set("name","HitCountMode") << std::endl
       //<< ((gem::hw::vfat::HitCountModeToString.at(params.hitCountMode).compare("FASTOR128")) == 0 ?
       << ((gem::hw::vfat::HitCountModeToString.at(params.hitCountMode).compare("FASTOR")) == 0 ?
	   (cgicc::option("Fast OR(128)").set("value","FastOR").set("selected")) :
	   (cgicc::option("Fast OR(128)").set("value","FastOR"))) << std::endl
       //<< ((gem::hw::vfat::HitCountModeToString.at(params.hitCountMode).compare("COUNTS1")) == 0 ?
       << ((gem::hw::vfat::HitCountModeToString.at(params.hitCountMode).compare("S1")) == 0 ?
	   (cgicc::option("Count S1").set("value","S1").set("selected")) :
	   (cgicc::option("Count S1").set("value","S1"))) << std::endl
       //<< ((gem::hw::vfat::HitCountModeToString.at(params.hitCountMode).compare("COUNTS2")) == 0 ?
       << ((gem::hw::vfat::HitCountModeToString.at(params.hitCountMode).compare("S2")) == 0 ?
	   (cgicc::option("Count S2").set("value","S2").set("selected")) :
	   (cgicc::option("Count S2").set("value","S2"))) << std::endl
       //<< ((gem::hw::vfat::HitCountModeToString.at(params.hitCountMode).compare("COUNTS3")) == 0 ?
       << ((gem::hw::vfat::HitCountModeToString.at(params.hitCountMode).compare("S3")) == 0 ?
	   (cgicc::option("Count S3").set("value","S3").set("selected")) :
	   (cgicc::option("Count S3").set("value","S3"))) << std::endl
       //<< ((gem::hw::vfat::HitCountModeToString.at(params.hitCountMode).compare("COUNTS4")) == 0 ?
       << ((gem::hw::vfat::HitCountModeToString.at(params.hitCountMode).compare("S4")) == 0 ?
	   (cgicc::option("Count S4").set("value","S4").set("selected")) :
	   (cgicc::option("Count S4").set("value","S4"))) << std::endl
       //<< ((gem::hw::vfat::HitCountModeToString.at(params.hitCountMode).compare("COUNTS5")) == 0 ?
       << ((gem::hw::vfat::HitCountModeToString.at(params.hitCountMode).compare("S5")) == 0 ?
	   (cgicc::option("Count S5").set("value","S5").set("selected")) :
	   (cgicc::option("Count S5").set("value","S5"))) << std::endl
       //<< ((gem::hw::vfat::HitCountModeToString.at(params.hitCountMode).compare("COUNTS6")) == 0 ?
       << ((gem::hw::vfat::HitCountModeToString.at(params.hitCountMode).compare("S6")) == 0 ?
	   (cgicc::option("Count S6").set("value","S6").set("selected")) :
	   (cgicc::option("Count S6").set("value","S6"))) << std::endl
       //<< ((gem::hw::vfat::HitCountModeToString.at(params.hitCountMode).compare("COUNTS7")) == 0 ?
       << ((gem::hw::vfat::HitCountModeToString.at(params.hitCountMode).compare("S7")) == 0 ?
	   (cgicc::option("Count S7").set("value","S7").set("selected")) :
	   (cgicc::option("Count S7").set("value","S7"))) << std::endl
       //<< ((gem::hw::vfat::HitCountModeToString.at(params.hitCountMode).compare("COUNTS8")) == 0 ?
       << ((gem::hw::vfat::HitCountModeToString.at(params.hitCountMode).compare("S8")) == 0 ?
	   (cgicc::option("Count S8").set("value","S8").set("selected")) :
	   (cgicc::option("Count S8").set("value","S8"))) << std::endl
       << cgicc::select()                                 << std::endl
       << "                  </div>"                      << std::endl
       << "                </div>"                        << std::endl;
  	               
  *out << "                <div class=\"controlRegisterCheckBox\">" << std::endl
       << cgicc::label("Set CR #2").set("for","CR2Set")             << std::endl
       << cgicc::input().set("type","checkbox").set("id","CR2Set")
                        .set("name","CR2Set"  ).set("checked")
       << "                </div>"                       << std::endl
       << "              </div>"                         << std::endl
       << cgicc::br().set("style","clear:both;")         << std::endl
       << cgicc::comment() << "ending the CR #2 section" <<cgicc::comment() << std::endl
       << "            </section>"                       << std::endl
       << std::endl;

  *out << "            <section>"                                          << std::endl
       << "              <div class=\"controlRegister\">"                  << std::endl
       << "                <div class=\"controlRegisterBits\">"            << std::endl
       << "            	     <div class=\"controlRegisterDropdownEntry\">" << std::endl
       << cgicc::label("DFTest &lt;4&gt;"  ).set("for","DFTest")    << std::endl
       << cgicc::select().set("id","DFTest").set("name","DFTest")   << std::endl
       //<< ((gem::hw::vfat::DFTestPatternToString.at(params.sendTestPattern).compare("IDLE")) == 0 ?
       << ((gem::hw::vfat::DFTestPatternToString.at(params.sendTestPattern).compare("OFF")) == 0 ?
	   (cgicc::option("Idle").set("value","Off").set("selected")) :
	   (cgicc::option("Idle").set("value","Off"))) << std::endl
       //<< ((gem::hw::vfat::DFTestPatternToString.at(params.sendTestPattern).compare("SEND")) == 0 ?
       << ((gem::hw::vfat::DFTestPatternToString.at(params.sendTestPattern).compare("ON")) == 0 ?
	   (cgicc::option("Send").set("value","On").set("selected")) :
	   (cgicc::option("Send").set("value","On"))) << std::endl
       << cgicc::select()                                           << std::endl
       << "                  </div>"                                << std::endl;
	               
  *out << "                  <div class=\"controlRegisterDropdownEntry\">" << std::endl
       << cgicc::label("PbBG &lt;3&gt;").set("for","PbBG")                 << std::endl
       << cgicc::select().set("id","PbBG").set("name","PbBG")              << std::endl
       //<< ((gem::hw::vfat::PbBGToString.at(params.padBandGap).compare("UNCONNECTED")) == 0 ?
       << ((gem::hw::vfat::PbBGToString.at(params.padBandGap).compare("OFF")) == 0 ?
	   (cgicc::option("Unconnected").set("value","Off").set("selected")) :
	   (cgicc::option("Unconnected").set("value","Off"))) << std::endl
       //<< ((gem::hw::vfat::PbBGToString.at(params.padBandGap).compare("CONNECTED")) == 0 ?
       << ((gem::hw::vfat::PbBGToString.at(params.padBandGap).compare("ON")) == 0 ?
	   (cgicc::option("Connected").set("value","On").set("selected")) :
	   (cgicc::option("Connected").set("value","On"))) << std::endl
       << cgicc::select()            << std::endl
       << "                  </div>" << std::endl;
	               
  *out << "                  <div class=\"controlRegisterDropdownEntry\">"    << std::endl
       << cgicc::label("TrimDACRange &lt;2:0&gt;").set("for","TrimDACRange")  << std::endl
       << cgicc::select().set("id","TrimDACRange").set("name","TrimDACRange") << std::endl
       //<< ((gem::hw::vfat::TrimDACRangeToString.at(params.trimDACRange).compare("DEFAULT")) == 0 ?
       << ((gem::hw::vfat::TrimDACRangeToString.at(params.trimDACRange).compare("0")) == 0 ?
	   (cgicc::option("Default").set("value","0").set("selected")) :
	   (cgicc::option("Default").set("value","0"))) << std::endl
       //<< ((gem::hw::vfat::TrimDACRangeToString.at(params.trimDACRange).compare("VALUE1")) == 0 ?
       << ((gem::hw::vfat::TrimDACRangeToString.at(params.trimDACRange).compare("1")) == 0 ?
	   (cgicc::option("Value1").set("value","1").set("selected")) :
	   (cgicc::option("Value1").set("value","1"))) << std::endl
       //<< ((gem::hw::vfat::TrimDACRangeToString.at(params.trimDACRange).compare("VALUE2")) == 0 ?
       << ((gem::hw::vfat::TrimDACRangeToString.at(params.trimDACRange).compare("2")) == 0 ?
	   (cgicc::option("Value2").set("value","2").set("selected")) :
	   (cgicc::option("Value2").set("value","2"))) << std::endl
       //<< ((gem::hw::vfat::TrimDACRangeToString.at(params.trimDACRange).compare("VALUE3")) == 0 ?
       << ((gem::hw::vfat::TrimDACRangeToString.at(params.trimDACRange).compare("3")) == 0 ?
	   (cgicc::option("Value3").set("value","3").set("selected")) :
	   (cgicc::option("Value3").set("value","3"))) << std::endl
       //<< ((gem::hw::vfat::TrimDACRangeToString.at(params.trimDACRange).compare("VALUE4")) == 0 ?
       << ((gem::hw::vfat::TrimDACRangeToString.at(params.trimDACRange).compare("4")) == 0 ?
	   (cgicc::option("Value4").set("value","4").set("selected")) :
	   (cgicc::option("Value4").set("value","4"))) << std::endl
       //<< ((gem::hw::vfat::TrimDACRangeToString.at(params.trimDACRange).compare("VALUE5")) == 0 ?
       << ((gem::hw::vfat::TrimDACRangeToString.at(params.trimDACRange).compare("5")) == 0 ?
	   (cgicc::option("Value5").set("value","5").set("selected")) :
	   (cgicc::option("Value5").set("value","5"))) << std::endl
       //<< ((gem::hw::vfat::TrimDACRangeToString.at(params.trimDACRange).compare("VALUE6")) == 0 ?
       << ((gem::hw::vfat::TrimDACRangeToString.at(params.trimDACRange).compare("6")) == 0 ?
	   (cgicc::option("Value6").set("value","6").set("selected")) :
	   (cgicc::option("Value6").set("value","6"))) << std::endl
       //<< ((gem::hw::vfat::TrimDACRangeToString.at(params.trimDACRange).compare("VALUE7")) == 0 ?
       << ((gem::hw::vfat::TrimDACRangeToString.at(params.trimDACRange).compare("7")) == 0 ?
	   (cgicc::option("Value7").set("value","7").set("selected")) :
	   (cgicc::option("Value7").set("value","7"))) << std::endl
       << cgicc::select()                                 << std::endl
       << "                  </div>"                      << std::endl
       << "                </div>"                        << std::endl;
	               
  *out << "                <div class=\"controlRegisterCheckBox\">" << std::endl
       << cgicc::label("Set CR #3").set("for","CR3Set")             << std::endl
       << cgicc::input().set("type","checkbox").set("id","CR3Set")
                        .set("name","CR3Set").set("checked")
       << "                </div>"                                     << std::endl
       << "              </div>"                                       << std::endl
       << cgicc::br().set("style","clear:both;")         << std::endl
       << cgicc::comment() << "ending the CR #3 section" <<cgicc::comment() << std::endl
       << "            </section>"                                     << std::endl
       << cgicc::comment() << "ending the Control registers fieldset" << cgicc::comment() << std::endl
       << "          </fieldset>"                                      << std::endl;  
}

//building the settings view
void gem::hw::vfat::VFAT2Manager::VFAT2ControlPanelWeb::createSettingsLayout(xgi::Output *out, const gem::hw::vfat::VFAT2ControlParams params) {
  boost::format inputform("%1%");

  *out << "          <fieldset class=\"vfatBiasing\">"               << std::endl
       << "            <legend>VFAT2 Biasing</legend>"               << std::endl
       << "            <div  class=\"vfatCurrentBiasing\">"          << std::endl
       << cgicc::label("IPreampIn").set("for","IPreampIn") << std::endl
       << cgicc::input().set("class","vfatBiasInput").set("id","IPreampIn").set("name","IPreampIn")
                        .set("type","number").set("min","0").set("max","255")
                        .set("value",boost::str(inputform%static_cast<unsigned>(params.iPreampIn)))
       << std::endl
       << cgicc::input().set("class","vfatBiasCheckbox").set("type","checkbox")
                        .set("id","SetIPreampIn").set("name","SetIPreampIn")
       << std::endl
       << cgicc::br() << std::endl
       << cgicc::label("IPreampFeed").set("for","IPreampFeed") << std::endl
       << cgicc::input().set("class","vfatBiasInput").set("id","IPreampFeed").set("name","IPreampFeed")
                        .set("type","number").set("min","0").set("max","255")
                        .set("value",boost::str(inputform%static_cast<unsigned>(params.iPreampFeed)))
       << std::endl
       << cgicc::input().set("class","vfatBiasCheckbox").set("type","checkbox")
                        .set("id","SetIPreampFeed"  ).set("name","SetIPreampFeed")
       << std::endl
       << cgicc::br() << std::endl
       << cgicc::label("IPreampOut").set("for","IPreampOut") << std::endl
       << cgicc::input().set("class","vfatBiasInput").set("id","IPreampOut").set("name","IPreampOut")
                        .set("type","number").set("min","0").set("max","255")
                        .set("value",boost::str(inputform%static_cast<unsigned>(params.iPreampOut)))
       << std::endl
       << cgicc::input().set("class","vfatBiasCheckbox").set("type","checkbox")
                        .set("id","SetIPreampOut"  ).set("name","SetIPreampOut")
       << std::endl
       << cgicc::br() << std::endl
       << cgicc::label("IShaper").set("for","IShaper") << std::endl
       << cgicc::input().set("class","vfatBiasInput").set("id","IShaper").set("name","IShaper")
                        .set("type","number").set("min","0").set("max","255")
                        .set("value",boost::str(inputform%static_cast<unsigned>(params.iShaper)))
       << std::endl
       << cgicc::input().set("class","vfatBiasCheckbox").set("type","checkbox")
                        .set("id","SetIShaper"  ).set("name","SetIShaper")
       << std::endl
       << cgicc::br() << std::endl
       << cgicc::label("IShaperFeed").set("for","IShaperFeed") << std::endl
       << cgicc::input().set("class","vfatBiasInput").set("id","IShaperFeed").set("name","IShaperFeed")
                        .set("type","number").set("min","0").set("max","255")
                        .set("value",boost::str(inputform%static_cast<unsigned>(params.iShaperFeed)))
       << std::endl
       << cgicc::input().set("class","vfatBiasCheckbox").set("type","checkbox")
                        .set("id","SetIShaperFeed"  ).set("name","SetIShaperFeed")
       << std::endl
       << cgicc::br() << std::endl
       << cgicc::label("IComp)").set("for","IComp") << std::endl
       << cgicc::input().set("class","vfatBiasInput").set("id","IComp").set("name","IComp")
                        .set("type","number").set("min","0").set("max","255")
                        .set("value",boost::str(inputform%static_cast<unsigned>(params.iComp)))
       << std::endl
       << cgicc::input().set("class","vfatBiasCheckbox").set("type","checkbox")
                        .set("id","SetIComp"  ).set("name","SetIComp")
       << std::endl
       << cgicc::br() << std::endl
       << cgicc::comment() << "ending the Current biasing div" << cgicc::comment() << std::endl
       << "            </div>" << std::endl;

  *out << "            <div  class=\"vfatOtherBiasing\">"        << std::endl
       << cgicc::label("Latency").set("for","Latency") << std::endl
       << cgicc::input().set("class","vfatBiasInput").set("id","Latency").set("name","Latency")
                        .set("type","number").set("min","0").set("max","255")
                        .set("value",boost::str(inputform%static_cast<unsigned>(params.latency)))
       << std::endl
       << cgicc::input().set("class","vfatBiasCheckbox").set("type","checkbox")
                        .set("id","SetLatency").set("name","SetLatency")
       << std::endl
       << cgicc::br() << std::endl
       << cgicc::label("VCal").set("for","VCal")                     << std::endl
       << cgicc::input().set("class","vfatBiasInput").set("id","VCal").set("name","VCal")
                        .set("type","number").set("min","0").set("max","255")
                        .set("value",boost::str(inputform%static_cast<unsigned>(params.vCal)))
       << std::endl
       << cgicc::input().set("class","vfatBiasCheckbox").set("type","checkbox")
                   	     .set("id","SetVCal").set("name","SetVCal")          << std::endl
       << cgicc::br() << std::endl
       << cgicc::label("VThreshold1").set("for","VThreshold1")       << std::endl
       << cgicc::input().set("class","vfatBiasInput").set("id","VThreshold1").set("name","VThreshold1")
                        .set("type","number").set("min","0").set("max","255")
                        .set("value",boost::str(inputform%static_cast<unsigned>(params.vThresh1)))
       << std::endl
       << cgicc::input().set("class","vfatBiasCheckbox").set("type","checkbox")
                        .set("id","SetVThreshold1").set("name","SetVThreshold1")
       << std::endl
       << cgicc::br() << std::endl
       << cgicc::label("VThreshold2").set("for","VThreshold2") << std::endl
       << cgicc::input().set("class","vfatBiasInput").set("id","VThreshold2" ).set("name","VThreshold2")
                        .set("type","number").set("min","0").set("max","255")
                        .set("value",boost::str(inputform%static_cast<unsigned>(params.vThresh2)))
       << std::endl
       << cgicc::input().set("class","vfatBiasCheckbox").set("type","checkbox")
                   	.set("id","SetVThreshold2").set("name","SetVThreshold2") << std::endl
       << cgicc::br() << std::endl
       << cgicc::label("CalPhase").set("for","CalPhase") << std::endl
       << cgicc::input().set("class","vfatBiasInput").set("id","CalPhase" ).set("name","CalPhase")
                        .set("type","number").set("min","0").set("max","255")
                        .set("value",boost::str(inputform%static_cast<unsigned>(params.calPhase)))
       << std::endl

       << cgicc::input().set("class","vfatBiasCheckbox").set("type","checkbox")
                        .set("id","SetCalPhase").set("name","SetCalPhase")
       << std::endl
       << cgicc::br() << std::endl
       << cgicc::comment() << "ending the Other biasing div" << cgicc::comment() << std::endl
       << "            </div>" << std::endl
       << cgicc::comment() << "ending the Biasing fieldset" << cgicc::comment() << std::endl
       << "          </fieldset>" << std::endl;
  
}

//building the channel view
void gem::hw::vfat::VFAT2Manager::VFAT2ControlPanelWeb::createChannelRegisterLayout(xgi::Output *out, const gem::hw::vfat::VFAT2ControlParams params) {
  boost::format inputform("%1%");
  
  *out << "        <fieldset class=\"vfatChannelRegisters\">" << std::endl
       << cgicc::legend("VFAT2 Channel registers") << std::endl
       << "          <fieldset class=\"vfatChannelSettings\">" << std::endl
       << cgicc::legend("Ch. 1 (inc. analog ch. 0)") << std::endl
       << "            <div>" << std::endl
       << cgicc::label("Ch0Cal").set("class","vfatCheckboxLabel").set("for","Cal0") << std::endl
       << cgicc::input().set("class","vfatChannelCheckbox").set("type","checkbox")
                        .set("id","Cal0").set("name","Cal0")
                        .set(params.channels[0].calPulse0 == 0 ? "" : "checked")
       << std::endl
       << cgicc::br() << std::endl
       << cgicc::label("Ch1Cal").set("class","vfatCheckboxLabel").set("for","Cal1") << std::endl
       << cgicc::input().set("class","vfatChannelCheckbox").set("type","checkbox")
                        .set("id","Cal1"  ).set("name","Cal1")
                        .set(params.channels[0].calPulse == 0 ? "" : "checked")
       << std::endl
       << cgicc::br() << std::endl
       << cgicc::label("Mask").set("class","vfatCheckboxLabel").set("for","Ch1Mask") << std::endl
       << cgicc::input().set("class","vfatChannelCheckbox").set("type","checkbox")
                        .set("id","Ch1Mask").set("name","Ch1Mask")
                        .set(params.channels[0].mask == 0 ? "" : "checked") << std::endl
       << "            </div>" << std::endl;
  	           
  *out << "            <div>" << std::endl
       << cgicc::label("TrimDAC").set("class","vfatTrimDACLabel").set("for","Ch1TrimDAC") << std::endl
       << cgicc::input().set("class","vfatChannelInput").set("type","number")
                        .set("id","Ch1TrimDAC").set("name","Ch1TrimDAC")
                        .set("min","0").set("max","31")
                        .set("value",boost::str(inputform%static_cast<unsigned>(params.channels[0].trimDAC)))
       << std::endl
       << cgicc::input().set("class","vfatChannelCheckbox").set("type","checkbox")
                        .set("id","SetCh1TrimDAC" ).set("name","SetCh1TrimDAC")
       << std::endl
       << cgicc::br() << std::endl
       << cgicc::input().set("class","vfatChannelButton").set("type","submit")
                        .set("value","Set Channel 0/1").set("name","VFAT2ControlOption")
       << cgicc::br() << std::endl
       << "            </div>" << std::endl
       << "            <!-- ending the Channel 0,1 fieldset -->" << std::endl
       << "          </fieldset>" << std::endl;
  
  int chanID = params.activeChannel;
  /*
  std::cout << "chan = "<< std::dec << (unsigned)chanID
	    << "; activeChannel = " << std::dec<< (unsigned)params.activeChannel
	    << std::endl;
  */
  *out << "          <fieldset class=\"vfatChannelSettings\">" << std::endl
       << cgicc::legend("Ch. # (2-128)")         << std::endl
       << "            <div>"                                  << std::endl
       << cgicc::label("Chan #").set("class","vfatTrimDACLabel").set("for","ChanSel") << std::endl
       << cgicc::input().set("class","vfatChannelInput").set("type","number").set("id","ChanSel")
                        .set("name","ChanSel").set("min","2").set("max","128")
                        .set("value",boost::str(inputform%chanID))
       << std::endl
       << "            </div>" << std::endl
       << "            <div>"  << std::endl
       << cgicc::label("ChCal").set("class","vfatCheckboxLabel").set("for","ChCal")      << std::endl
       << cgicc::input().set("class","vfatChannelCheckbox").set("type","checkbox")
                        .set("id","ChCal").set("name","ChCal")
                        .set(params.channels[chanID-1].calPulse == 0 ? "" : "checked")
       << std::endl
       << cgicc::br() << std::endl
       << cgicc::label("Mask").set("class","vfatCheckboxLabel").set("for","ChMask") << std::endl
       << cgicc::input().set("class","vfatChannelCheckbox").set("type","checkbox")
                        .set("id","ChMask").set("name","ChMask")
                        .set(params.channels[chanID-1].mask == 0 ? "" : "checked")
       << std::endl
       << "            </div>" << std::endl;
  	           
  *out << "            <div>" << std::endl
       << cgicc::label("TrimDAC").set("class","vfatTrimDACLabel").set("for","TrimDAC") << std::endl
       << cgicc::input().set("class","vfatChannelInput").set("type","number")
                        .set("id","TrimDAC").set("name","TrimDAC").set("min","0").set("max","31")
                        .set("value",boost::str(inputform%static_cast<unsigned>(params.channels[chanID-1].trimDAC)))
       << std::endl
       << cgicc::input().set("class","vfatChannelCheckbox").set("type","checkbox")
                        .set("id","SetTrimDAC").set("name","SetTrimDAC")
       << std::endl
       << "            </div>" << std::endl
       << cgicc::br() << std::endl;
	           
  *out << "            <div>" << std::endl
       << "              <div>" << std::endl
       << cgicc::input().set("class","vfatChannelButton").set("type","submit")
                        .set("value","Get This Channel").set("name","VFAT2ControlOption")
       << std::endl
       << "              </div>" << std::endl
       << "              <div>" << std::endl
       << cgicc::input().set("class","vfatChannelButton").set("type","submit")
                        .set("value","Set This Channel").set("name","VFAT2ControlOption")
       << cgicc::br() << std::endl
       << cgicc::input().set("class","vfatChannelButton").set("type","submit")
                        .set("value","Set All Channels").set("name","VFAT2ControlOption")
       << std::endl
       << "              </div>" << std::endl
       << "            </div>" << std::endl
       << cgicc::comment() << "ending the Channel register fieldset" << cgicc::comment() << std::endl
       << "          </fieldset>" << std::endl
       << cgicc::comment() << "ending the Channel registers fieldset" << cgicc::comment() << std::endl
       << "        </fieldset>" << std::endl;
}

//building the command view
void gem::hw::vfat::VFAT2Manager::VFAT2ControlPanelWeb::createCommandLayout(xgi::Output *out, const gem::hw::vfat::VFAT2ControlParams params) {
  *out << cgicc::section().set("class","vfatCommands") << std::endl
       << cgicc::h3() << "Commands" << cgicc::h3()
       << std::endl << std::endl
       << cgicc::input().set("class","vfatButtonInput")//.set("style","width:auto")
                        .set("type","submit").set("value","Scan for connected VFATs")
                        .set("name","ScanForVFATS")
       << std::endl
       << cgicc::br() << std::endl
       << cgicc::input().set("class","vfatButtonInput")//.set("style","width:auto")
                        .set("type","submit").set("value","Initialize VFAT")
                        .set("name","InitializeVFAT")
       << std::endl
       << cgicc::br() << std::endl;

  *out << cgicc::fieldset().set("class","selectedChipControl") << std::endl
       << cgicc::legend("Selected Chip")         << std::endl
       << cgicc::input().set("class","vfatButtonInput")//.set("style","width:auto")
                        .set("type","submit").set("value","Read VFAT").set("name","VFAT2ControlOption")
       << std::endl
       << cgicc::br() << std::endl
       << cgicc::input().set("class","vfatButtonInput")//.set("style","width:auto")
                        .set("type","submit").set("value","Write VFAT").set("name","VFAT2ControlOption")
       << std::endl
       << cgicc::br() << std::endl
       << cgicc::label("Compare written values").set("for","Compare") << std::endl
       << cgicc::input().set("type","checkbox").set("id","Compare").set("value","Compare")
                       .set("name","Compare")
       << std::endl
       << cgicc::br() << std::endl
       << cgicc::br() << std::endl
       //changed from type submit to type button
       << cgicc::input().set("class","vfatButtonInput")////.set("style","width:auto")
                        .set("type","button").set("value","Select All Registers")
                        .set("onClick","toggleVFATCheckboxes(true)")
			.set("name","SelectAll")
       << std::endl
       << cgicc::br() << std::endl
       //changed from type submit to type button
       << cgicc::input().set("class","vfatButtonInput")////.set("style","width:auto")
			.set("type","button").set("value","Select None")
                        .set("onClick","toggleVFATCheckboxes(false)")
                        .set("name","SelectNone")
       << std::endl
       << cgicc::comment() << "ending the Selected chip fieldset" << cgicc::comment()
       << std::endl
       << cgicc::fieldset() << std::endl
       << cgicc::br() << std::endl
       << std::endl;

  *out << cgicc::fieldset().set("class","xmlSettingsUpload") << std::endl
       << cgicc::legend("XML Settings Export/Upload")        << std::endl
       << cgicc::label("File :").set("for","xmlFileName")    << std::endl
       << cgicc::input().set("type","text").set("id","xmlFileName").set("name","xmlFileName")
       << std::endl
       << cgicc::br() << std::endl
       << cgicc::input().set("class","vfatButtonInput")//.set("style","width:auto")
                        .set("type","submit").set("value","Load XML settings").set("name","Load")
       << std::endl
       << cgicc::br() << std::endl
       << cgicc::input().set("class","vfatButtonInput")//.set("style","width:auto")
                        .set("type","submit").set("value","Export current settings").set("name","Export")
       << std::endl
       << cgicc::br() << std::endl
       << cgicc::comment() << "ending the XML file upload fieldset" << cgicc::comment() << std::endl
       << cgicc::fieldset() << std::endl
    
       << cgicc::comment() << "ending the Commands section" << cgicc::comment() << std::endl
       << cgicc::section() << std::endl;
}

void gem::hw::vfat::VFAT2Manager::VFAT2ControlPanelWeb::getCurrentParametersAsXML( ) {
  
}

void gem::hw::vfat::VFAT2Manager::VFAT2ControlPanelWeb::saveCurrentParametersAsXML( ) {
  
}

void gem::hw::vfat::VFAT2Manager::VFAT2ControlPanelWeb::setParametersByXML( ) {
  
}

