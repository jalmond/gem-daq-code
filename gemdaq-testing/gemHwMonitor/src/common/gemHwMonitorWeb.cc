#include "gem/hwMonitor/gemHwMonitorWeb.h"
#include <boost/algorithm/string.hpp>

XDAQ_INSTANTIATOR_IMPL(gem::hwMonitor::gemHwMonitorWeb)

gem::hwMonitor::gemHwMonitorWeb::gemHwMonitorWeb(xdaq::ApplicationStub * s)
    throw (xdaq::exception::Exception):
        xdaq::WebApplication(s)
{
    xgi::framework::deferredbind(this, this, &gemHwMonitorWeb::Default, "Default");
    xgi::framework::deferredbind(this, this, &gemHwMonitorWeb::Dummy, "Dummy");
    xgi::framework::deferredbind(this, this, &gemHwMonitorWeb::controlPanel, "Control Panel");
    xgi::framework::deferredbind(this, this, &gemHwMonitorWeb::setConfFile,"setConfFile");
    xgi::framework::deferredbind(this, this, &gemHwMonitorWeb::uploadConfFile,"uploadConfFile");
    xgi::framework::deferredbind(this, this, &gemHwMonitorWeb::getCratesConfiguration,"getCratesConfiguration");
    xgi::framework::deferredbind(this, this, &gemHwMonitorWeb::pingCrate,"pingCrate");
    xgi::framework::deferredbind(this, this, &gemHwMonitorWeb::expandCrate,"expandCrate");
    xgi::framework::deferredbind(this, this, &gemHwMonitorWeb::cratePanel,"cratePanel");
    xgi::framework::deferredbind(this, this, &gemHwMonitorWeb::expandGLIB,"expandGLIB");
    xgi::framework::deferredbind(this, this, &gemHwMonitorWeb::glibPanel,"glibPanel");
    xgi::framework::deferredbind(this, this, &gemHwMonitorWeb::expandOH,"expandOH");
    xgi::framework::deferredbind(this, this, &gemHwMonitorWeb::ohPanel,"ohPanel");
    xgi::framework::deferredbind(this, this, &gemHwMonitorWeb::expandVFAT,"expandVFAT");
    xgi::framework::deferredbind(this, this, &gemHwMonitorWeb::vfatPanel,"vfatPanel");
    gemHwMonitorSystem_ = new gemHwMonitorSystem();
    gemHwMonitorCrate_ = new gemHwMonitorCrate();
    gemHwMonitorGLIB_ = new gemHwMonitorGLIB();
    gemHwMonitorOH_ = new gemHwMonitorOH();
    gemHwMonitorVFAT_ = new gemHwMonitorVFAT();
    gemSystemHelper_ = new gemHwMonitorHelper(gemHwMonitorSystem_);
    crateCfgAvailable_ = false;
}

gem::hwMonitor::gemHwMonitorWeb::~gemHwMonitorWeb()
{
    delete gemHwMonitorSystem_;
    delete gemHwMonitorCrate_;
    delete gemHwMonitorGLIB_;
    delete gemHwMonitorOH_;
    delete gemHwMonitorVFAT_;
    delete gemSystemHelper_;
}
void gem::hwMonitor::gemHwMonitorWeb::Default(xgi::Input * in, xgi::Output * out )
    throw (xgi::exception::Exception)
{
    this->controlPanel(in,out);
}

void gem::hwMonitor::gemHwMonitorWeb::pingCrate(xgi::Input * in, xgi::Output * out )

    throw (xgi::exception::Exception)
{
    cgicc::Cgicc cgi(in);
    for (unsigned int i = 0; i != gemHwMonitorSystem_->getDevice()->getSubDevicesRefs().size(); i++) 
    {
        if (cgi.queryCheckbox(gemHwMonitorSystem_->getDevice()->getSubDevicesRefs().at(i)->getDeviceId())) 
        {
            //checkedCrates_.push_back(gemHwMonitorSystem_->getDevice()->getSubDevicesRefs().at(i)->getDeviceId());
            //std::cout << "checked crate: "<<checkedCrates_.back() <<std::endl;
            gem::hw::vfat::HwVFAT2* crateDevice_ = new gem::hw::vfat::HwVFAT2(getApplicationLogger(), "VFAT9");
            crateDevice_->setAddressTableFileName("testbeam_registers.xml");
            crateDevice_->setDeviceIPAddress("192.168.0.115");
            crateDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS.VFAT9");
            crateDevice_->connectDevice();
            if (crateDevice_->isHwConnected())
            {
                gemHwMonitorSystem_->setSubDeviceStatus(0,i);
            } else {
                gemHwMonitorSystem_->setSubDeviceStatus(1,i);
            }
            delete crateDevice_;
        }
    }
    this->controlPanel(in,out);
}

void gem::hwMonitor::gemHwMonitorWeb::Dummy(xgi::Input * in, xgi::Output * out )

    throw (xgi::exception::Exception)
{
    *out << "Dummy" <<std::endl; 
}

/* Generates the main page interface. Allows to choose the configuration file, then shows the availability of crates corresponding to this configuration.
 * Allows to launch the test utility to check the crates state.
 * */
void gem::hwMonitor::gemHwMonitorWeb::controlPanel(xgi::Input * in, xgi::Output * out )
    throw (xgi::exception::Exception)
{
    try {

        *out << "<link rel=\"stylesheet\" type=\"text/css\" href=\"/gemdaq/gemHwMonitor/html/css/bootstrap.css\">" << std::endl
        << "<link rel=\"stylesheet\" type=\"text/css\" href=\"/gemdaq/gemHwMonitor/html/css/bootstrap-theme.css\">" << std::endl;
        //*out << "<link rel=\"stylesheet\" type=\"text/css\" href=\"/gemdaq/gemHwMonitor/html/css/bootstrap.min.css\">" << std::endl
        //<< "<link rel=\"stylesheet\" type=\"text/css\" href=\"/gemdaq/gemHwMonitor/html/css/bootstrap-theme.min.css\">" << std::endl;

        ///////////////////////////////////////////////
        //
        // GEM System Configuration
        //
        ///////////////////////////////////////////////
        *out << "<div class=\"panel panel-primary\">" << std::endl;
        *out << "<div class=\"panel-heading\">" << std::endl;
        *out << "<h1><div align=\"center\">GEM System Configuration</div></h1>" << std::endl;
        *out << "</div>" << std::endl;
        *out << "<div class=\"panel-body\">" << std::endl;

        *out << "<h3><div class=\"alert alert-info\" role=\"alert\" align=\"center\">Current configuration file : "<< gemSystemHelper_->getXMLconfigFile() <<  "</div></h3>" << std::endl;
        *out << cgicc::br()<< std::endl;

        std::string methodText = toolbox::toString("/%s/setConfFile",getApplicationDescriptor()->getURN().c_str());
        *out << cgicc::form().set("method","POST").set("action",methodText) << std::endl ;
        *out << cgicc::input().set("type","text").set("name","xmlFilename").set("size","120").set("ENCTYPE","multipart/form-data").set("value",gemSystemHelper_->getXMLconfigFile()) << std::endl;
        *out << "<button type=\"submit\" class=\"btn btn-primary\">Set configuration file</button>" << std::endl;
        *out << cgicc::form() << std::endl ;

        std::string methodUpload = toolbox::toString("/%s/uploadConfFile",getApplicationDescriptor()->getURN().c_str());
        *out << cgicc::form().set("method","POST").set("enctype","multipart/form-data").set("action",methodUpload) << std::endl ;
        //*out << cgicc::input().set("type","file").set("name","xmlFilenameUpload").set("size","80") << std::endl;
        *out << "<span class=\"btn btn-primary btn-file\">Browse <input type=\"file\" name=\"xmlFilenameUpload\"></span>" << std::endl;
        *out << "<button type=\"submit\" class=\"btn btn-primary\">Submit</button>" << std::endl;
        *out << cgicc::form() << std::endl ;

        *out << cgicc::hr()<< std::endl;

        *out << "<h2><div align=\"center\">Connected Crates</div></h2>" << std::endl;
	    this->showCratesAvailability(in,out);
        *out << cgicc::hr()<< std::endl;
        *out << cgicc::br();
    }

    catch (const xgi::exception::Exception& e) {
        LOG4CPLUS_INFO(this->getApplicationLogger(),"Something went wrong displaying ControlPanel xgi: " << e.what());
        XCEPT_RAISE(xgi::exception::Exception, e.what());
    }
    catch (const std::exception& e) {
        LOG4CPLUS_INFO(this->getApplicationLogger(),"Something went wrong displaying the ControlPanel: " << e.what());
        XCEPT_RAISE(xgi::exception::Exception, e.what());
    }
}
void gem::hwMonitor::gemHwMonitorWeb::showCratesAvailability(xgi::Input * in, xgi::Output * out )
throw (xgi::exception::Exception)
{
    // If crates config is not available yet prompt to get it
    if (!crateCfgAvailable_) {
        *out << "<h3><div class=\"alert alert-warning\" role=\"alert\" align=\"center\">Crates configuration isn't available. Please, get the crates configuration</div></h3>" << std::endl;
        std::string methodGetCratesCfg = toolbox::toString("/%s/getCratesConfiguration", getApplicationDescriptor()->getURN().c_str());
        *out << "<div align=\"center\">" << std::endl;
        *out << cgicc::form().set("method","POST").set("action", methodGetCratesCfg) << std::endl ;
        *out << "<button type=\"submit\" class=\"btn btn-primary\">Get crates configuration</button>" << std::endl;
        *out << cgicc::form() << std::endl ;
        *out << "</div>" << std::endl;
    } else {
        //*out << "Crates configuration is taken from XML. In order to check their availability please select needed crates and press 'Check selected crates availability' button. " << 
        //"To have more information about their state press 'Test selected crates' button" << 
        //    cgicc::br() << std::endl;
        std::string methodExpandCrate = toolbox::toString("/%s/expandCrate", getApplicationDescriptor()->getURN().c_str());
        std::string methodSelectCrate = toolbox::toString("/%s/selectCrate", getApplicationDescriptor()->getURN().c_str());
        std::string methodPingCrate = toolbox::toString("/%s/pingCrate",getApplicationDescriptor()->getURN().c_str());
        *out << "<div align=\"center\">" << std::endl;
        *out << cgicc::table().set("class","table-condensed");
        *out << cgicc::tr();
        for (int i=0; i<nCrates_; i++) {
            std::string currentCrateID;
            currentCrateID += gemHwMonitorSystem_->getCurrentSubDeviceId(i);
            *out << cgicc::td();
                *out << cgicc::form().set("method","POST").set("action", methodExpandCrate) << std::endl ;
                if (gemHwMonitorSystem_->getSubDeviceStatus(i) == 0)
                {
                    *out << "<button type=\"submit\" class=\"btn btn-success\" name=\"crateButton\" value=\"" << currentCrateID << "\">" << currentCrateID<< "</button>" << std::endl;
                } else if (gemHwMonitorSystem_->getSubDeviceStatus(i) == 1)
                {
                    *out << "<button type=\"submit\" class=\"btn btn-warning\" name=\"crateButton\" value=\"" << currentCrateID << "\">" << currentCrateID<< "</button>" << std::endl;
                } else if (gemHwMonitorSystem_->getSubDeviceStatus(i) == 2)
                {
                    *out << "<button type=\"submit\" class=\"btn btn-disabled\" name=\"crateButton\" value=\"" << currentCrateID << "\" disabled>" << currentCrateID<< "</button>" << std::endl;
                }
                *out << cgicc::form() << std::endl ;
            *out << cgicc::td();
        }
        *out << cgicc::tr();
        *out << cgicc::tr();
        *out << cgicc::form().set("method","GET").set("action",methodPingCrate) << std::endl ;
        for (int i=0; i<nCrates_; i++) {
            std::string currentCrateID;
            currentCrateID += gemHwMonitorSystem_->getCurrentSubDeviceId(i);
            *out << cgicc::td();
                *out << "<div align=\"center\">"<< cgicc::input().set("type","checkbox").set("name",currentCrateID) << "</div>" << std::endl;
            *out << cgicc::td();
        }
        *out << cgicc::tr();
        *out << cgicc::table();
            *out << "<button type=\"submit\" class=\"btn btn-primary\">Check availability of selected crates</button>" << std::endl;
        *out << cgicc::form();
        *out << "</div>" << std::endl;
        *out << cgicc::br();
    }
}

void gem::hwMonitor::gemHwMonitorWeb::setConfFile(xgi::Input * in, xgi::Output * out )
throw (xgi::exception::Exception)
{
    cgicc::Cgicc cgi(in);
    std::string newFile = cgi.getElement("xmlFilename")->getValue();
    struct stat buffer;
    if (stat(newFile.c_str(), &buffer) == 0) {
        gemSystemHelper_->setXMLconfigFile(newFile.c_str());
        crateCfgAvailable_ = false;
    }
    else {
        XCEPT_RAISE(xgi::exception::Exception, "File not found");
    }
    this->controlPanel(in,out);
}

void gem::hwMonitor::gemHwMonitorWeb::uploadConfFile(xgi::Input * in, xgi::Output * out )
throw (xgi::exception::Exception)
{
    cgicc::Cgicc cgi(in);
    std::string newFile = cgi.getElement("xmlFilenameUpload")->getValue();
    struct stat buffer;
    if (stat(newFile.c_str(), &buffer) == 0) {
        gemSystemHelper_->setXMLconfigFile(newFile.c_str());
        crateCfgAvailable_ = false;
    }
    else {
        XCEPT_RAISE(xgi::exception::Exception, "File not found");
    }
    this->controlPanel(in,out);
}
void gem::hwMonitor::gemHwMonitorWeb::getCratesConfiguration(xgi::Input * in, xgi::Output * out )
throw (xgi::exception::Exception)
{
    gemSystemHelper_->configure();
    crateCfgAvailable_ = true;
    nCrates_ = gemHwMonitorSystem_->getNumberOfSubDevices();
    for (int i=0; i<nCrates_; i++) {
        gemHwMonitorSystem_->addSubDeviceStatus(2);
    }
    this->controlPanel(in,out);
}
void gem::hwMonitor::gemHwMonitorWeb::selectCrate(xgi::Input * in, xgi::Output * out )
throw (xgi::exception::Exception)
{
}
void gem::hwMonitor::gemHwMonitorWeb::expandCrate(xgi::Input * in, xgi::Output * out )
throw (xgi::exception::Exception)
{
    cgicc::Cgicc cgi(in);
    crateToShow_ = cgi.getElement("crateButton")->getValue();
//for (auto i = gemHwMonitorSystem_->getDevice()->getSubDevicesRefs().begin(); i != gemHwMonitorSystem_->getDevice()->getSubDevicesRefs().end(); i++) {
    //if (i->getDeviceId() == crateToShow_) {gemHwMonitorCrate_->setDeviceConfiguration(*i);}
    // Auto-pointer doesn't work for some reason. Improve this later.
    for (unsigned int i = 0; i != gemHwMonitorSystem_->getDevice()->getSubDevicesRefs().size(); i++) 
    {
        if (gemHwMonitorSystem_->getDevice()->getSubDevicesRefs().at(i)->getDeviceId() == crateToShow_) 
        {
            gemHwMonitorCrate_->setDeviceConfiguration(*gemHwMonitorSystem_->getDevice()->getSubDevicesRefs().at(i));
            for (int i=0; i<gemHwMonitorCrate_->getNumberOfSubDevices(); i++) {
                if (i) 
                {
                    gemHwMonitorCrate_->addSubDeviceStatus(2);
                } else {
                    gemHwMonitorCrate_->addSubDeviceStatus(0);
                }
            }
        }
    }
    this->cratePanel(in,out);
}
void gem::hwMonitor::gemHwMonitorWeb::cratePanel(xgi::Input * in, xgi::Output * out )
throw (xgi::exception::Exception)
{
    *out << "<link rel=\"stylesheet\" type=\"text/css\" href=\"/gemdaq/gemHwMonitor/html/css/bootstrap.css\">" << std::endl
    << "<link rel=\"stylesheet\" type=\"text/css\" href=\"/gemdaq/gemHwMonitor/html/css/bootstrap-theme.css\">" << std::endl;
    *out << "<div class=\"panel panel-primary\">" << std::endl;
    *out << "<div class=\"panel-heading\">" << std::endl;
    *out << "<h1><div align=\"center\">Chip Id : "<< crateToShow_ << "</div></h1>" << std::endl;
    *out << "</div>" << std::endl;
    *out << "<div class=\"panel-body\">" << std::endl;
    *out << "<h3><div class=\"alert alert-info\" role=\"alert\" align=\"center\">Device base node : "<< "ENDCAP 1"
    << "<br> PLACEHOLDER <br>"<< "</div></h3>" << std::endl;
    *out << cgicc::hr()<< std::endl;
    *out << "<h2><div align=\"center\">Connected AMC13 boards</div></h2>" << std::endl;
    *out << cgicc::br()<< std::endl;
    *out << "<h3><div align=\"center\">There're no AMC13 boards</div></h3>" << std::endl;
    *out << cgicc::hr()<< std::endl;
    std::string methodExpandGLIB = toolbox::toString("/%s/expandGLIB", getApplicationDescriptor()->getURN().c_str());
    *out << cgicc::table().set("class","table");
    *out << "<tr><h2><div align=\"center\">Connected Optohybrids</div></h2></tr>" << std::endl;
    *out << "<tr>" << std::endl;
    for (int i=0; i<gemHwMonitorCrate_->getNumberOfSubDevices(); i++) {
        std::string currentGLIBId;
        currentGLIBId += gemHwMonitorCrate_->getCurrentSubDeviceId(i);
        *out << cgicc::td();
            *out << cgicc::form().set("method","POST").set("action", methodExpandGLIB) << std::endl ;
            if (gemHwMonitorCrate_->getSubDeviceStatus(i) == 0)
            {
                *out << "<button type=\"submit\" class=\"btn btn-success\" name=\"glibButton\" value=\"" << currentGLIBId << "\">" << currentGLIBId<< "</button>" << std::endl;
            } else if (gemHwMonitorCrate_->getSubDeviceStatus(i) == 1)
            {
                *out << "<button type=\"submit\" class=\"btn btn-warning\" name=\"glibButton\" value=\"" << currentGLIBId << "\">" << currentGLIBId<< "</button>" << std::endl;
            } else if (gemHwMonitorCrate_->getSubDeviceStatus(i) == 2)
            {
                *out << "<button type=\"submit\" class=\"btn btn-disabled\" name=\"glibButton\" value=\"" << currentGLIBId << "\" disabled>" << currentGLIBId<< "</button>" << std::endl;
            }
            *out << cgicc::form() << std::endl ;
        *out << cgicc::td();
    }
    *out << "</tr>" << std::endl;
    *out << cgicc::table() <<std::endl;;
    *out << "</div>" << std::endl;
    *out << cgicc::br()<< std::endl;
    *out << cgicc::hr()<< std::endl;
}

void gem::hwMonitor::gemHwMonitorWeb::expandGLIB(xgi::Input * in, xgi::Output * out )
throw (xgi::exception::Exception)
{
    cgicc::Cgicc cgi(in);
    glibToShow_ = cgi.getElement("glibButton")->getValue();
    // Auto-pointer doesn't work for some reason. Improve this later.
    for (unsigned int i = 0; i != gemHwMonitorCrate_->getDevice()->getSubDevicesRefs().size(); i++) 
    {
        if (gemHwMonitorCrate_->getDevice()->getSubDevicesRefs().at(i)->getDeviceId() == glibToShow_) 
        {
            gemHwMonitorGLIB_->setDeviceConfiguration(*gemHwMonitorCrate_->getDevice()->getSubDevicesRefs().at(i));
            for (int i=0; i<gemHwMonitorGLIB_->getNumberOfSubDevices(); i++) {
                if (i) 
                {
                    gemHwMonitorGLIB_->addSubDeviceStatus(2);
                } else {
                    gemHwMonitorGLIB_->addSubDeviceStatus(0);
                }
            }
        }
    }
    this->glibPanel(in,out);
}
void gem::hwMonitor::gemHwMonitorWeb::glibPanel(xgi::Input * in, xgi::Output * out )
throw (xgi::exception::Exception)
{
    *out << "<link rel=\"stylesheet\" type=\"text/css\" href=\"/gemdaq/gemHwMonitor/html/css/bootstrap.css\">" << std::endl
    << "<link rel=\"stylesheet\" type=\"text/css\" href=\"/gemdaq/gemHwMonitor/html/css/bootstrap-theme.css\">" << std::endl;
    *out << "<div class=\"panel panel-primary\">" << std::endl;
    *out << "<div class=\"panel-heading\">" << std::endl;
    *out << "<h1><div align=\"center\">Chip Id : "<< glibToShow_ << "</div></h1>" << std::endl;
    *out << "</div>" << std::endl;
    *out << "<div class=\"panel-body\">" << std::endl;
    *out << "<h3><div class=\"alert alert-info\" role=\"alert\" align=\"center\">Device base node : "<< crateToShow_ 
    << "<br> PLACEHOLDER <br>"<< "</div></h3>" << std::endl;
    std::string methodExpandOH = toolbox::toString("/%s/expandOH", getApplicationDescriptor()->getURN().c_str());
    *out << cgicc::table().set("class","table");
    *out << "<tr><h2><div align=\"center\">Connected Optohybrids</div></h2></tr>" << std::endl;
    *out << "<tr>" << std::endl;
    for (int i=0; i<gemHwMonitorGLIB_->getNumberOfSubDevices(); i++) {
        std::string currentOHId;
        currentOHId += gemHwMonitorGLIB_->getCurrentSubDeviceId(i);
        *out << cgicc::td();
            *out << cgicc::form().set("method","POST").set("action", methodExpandOH) << std::endl ;
            if (gemHwMonitorGLIB_->getSubDeviceStatus(i) == 0)
            {
                *out << "<button type=\"submit\" class=\"btn btn-success\" name=\"ohButton\" value=\"" << currentOHId << "\">" << currentOHId<< "</button>" << std::endl;
            } else if (gemHwMonitorGLIB_->getSubDeviceStatus(i) == 1)
            {
                *out << "<button type=\"submit\" class=\"btn btn-warning\" name=\"ohButton\" value=\"" << currentOHId << "\">" << currentOHId<< "</button>" << std::endl;
            } else if (gemHwMonitorGLIB_->getSubDeviceStatus(i) == 2)
            {
                *out << "<button type=\"submit\" class=\"btn btn-disabled\" name=\"ohButton\" value=\"" << currentOHId << "\" disabled>" << currentOHId<< "</button>" << std::endl;
            }
            *out << cgicc::form() << std::endl;
        *out << cgicc::td();
    }
    *out << "</tr>" << std::endl;
    *out << cgicc::table() <<std::endl;;
    *out << "</div>" << std::endl;
    *out << cgicc::br()<< std::endl;
    *out << cgicc::hr()<< std::endl;
}

void gem::hwMonitor::gemHwMonitorWeb::expandOH(xgi::Input * in, xgi::Output * out )
throw (xgi::exception::Exception)
{
    cgicc::Cgicc cgi(in);
    ohToShow_ = cgi.getElement("ohButton")->getValue();
    // Auto-pointer doesn't work for some reason. Improve this later.
    for (unsigned int i = 0; i != gemHwMonitorGLIB_->getDevice()->getSubDevicesRefs().size(); i++) 
    {
        if (gemHwMonitorGLIB_->getDevice()->getSubDevicesRefs().at(i)->getDeviceId() == ohToShow_) 
        {
            gemHwMonitorOH_->setDeviceConfiguration(*gemHwMonitorGLIB_->getDevice()->getSubDevicesRefs().at(i));
            for (int i=0; i<gemHwMonitorOH_->getNumberOfSubDevices(); i++) {
                if (i) 
                {
                    gemHwMonitorOH_->addSubDeviceStatus(2);
                } else {
                    gemHwMonitorOH_->addSubDeviceStatus(0);
                }
            }
        }
    }
    this->ohPanel(in,out);
}
void gem::hwMonitor::gemHwMonitorWeb::ohPanel(xgi::Input * in, xgi::Output * out )
throw (xgi::exception::Exception)
{
    *out << "<link rel=\"stylesheet\" type=\"text/css\" href=\"/gemdaq/gemHwMonitor/html/css/bootstrap.css\">" << std::endl
    << "<link rel=\"stylesheet\" type=\"text/css\" href=\"/gemdaq/gemHwMonitor/html/css/bootstrap-theme.css\">" << std::endl;
    *out << "<div class=\"panel panel-primary\">" << std::endl;
    *out << "<div class=\"panel-heading\">" << std::endl;
    *out << "<h1><div align=\"center\">Chip Id : "<< ohToShow_ << "</div></h1>" << std::endl;
    *out << "</div>" << std::endl;
    *out << "<div class=\"panel-body\">" << std::endl;
    *out << "<h3><div class=\"alert alert-info\" role=\"alert\" align=\"center\">Device base node : "<< crateToShow_ << "::" << glibToShow_ 
    << "<br> PLACEHOLDER <br>"<< "</div></h3>" << std::endl;
    std::string methodExpandVFAT = toolbox::toString("/%s/expandVFAT", getApplicationDescriptor()->getURN().c_str());
    *out << cgicc::table().set("class","table");
    *out << "<tr><h2><div align=\"center\">Connected VFAT's</div></h2></tr>" << std::endl;
    *out << "<tr>" << std::endl;
    for (int i=0; i<gemHwMonitorOH_->getNumberOfSubDevices(); i++) {
        std::string currentVFATId;
        currentVFATId += gemHwMonitorOH_->getCurrentSubDeviceId(i);
        *out << cgicc::td();
            *out << cgicc::form().set("method","POST").set("action", methodExpandVFAT) << std::endl ;
            if (gemHwMonitorOH_->getSubDeviceStatus(i) == 0)
            {
                *out << "<button type=\"submit\" class=\"btn btn-success\" name=\"vfatButton\" value=\"" << currentVFATId << "\">" << currentVFATId<< "</button>" << std::endl;
            } else if (gemHwMonitorOH_->getSubDeviceStatus(i) == 1)
            {
                *out << "<button type=\"submit\" class=\"btn btn-warning\" name=\"vfatButton\" value=\"" << currentVFATId << "\">" << currentVFATId<< "</button>" << std::endl;
            } else if (gemHwMonitorOH_->getSubDeviceStatus(i) == 2)
            {
                *out << "<button type=\"submit\" class=\"btn btn-disabled\" name=\"vfatButton\" value=\"" << currentVFATId << "\" disabled>" << currentVFATId<< "</button>" << std::endl;
            }
            *out << cgicc::form() << std::endl;
        *out << cgicc::td();
    }
    *out << "</tr>" << std::endl;
    *out << cgicc::table() <<std::endl;;

    *out << "</div>" << std::endl;
    *out << cgicc::br()<< std::endl;
    *out << cgicc::hr()<< std::endl;
}

void gem::hwMonitor::gemHwMonitorWeb::expandVFAT(xgi::Input * in, xgi::Output * out )
throw (xgi::exception::Exception)
{
    cgicc::Cgicc cgi(in);
    vfatToShow_ = cgi.getElement("vfatButton")->getValue();
    // Auto-pointer doesn't work for some reason. Improve this later.
    for (unsigned int i = 0; i != gemHwMonitorOH_->getDevice()->getSubDevicesRefs().size(); i++) 
    {
        if (gemHwMonitorOH_->getDevice()->getSubDevicesRefs().at(i)->getDeviceId() == vfatToShow_) 
        {
            gemHwMonitorVFAT_->setDeviceConfiguration(*gemHwMonitorOH_->getDevice()->getSubDevicesRefs().at(i));
        }
    }
    this->vfatPanel(in,out);
}
void gem::hwMonitor::gemHwMonitorWeb::vfatPanel(xgi::Input * in, xgi::Output * out )
throw (xgi::exception::Exception)
{
    *out << "<link rel=\"stylesheet\" type=\"text/css\" href=\"/gemdaq/gemHwMonitor/html/css/bootstrap.css\">" << std::endl
    << "<link rel=\"stylesheet\" type=\"text/css\" href=\"/gemdaq/gemHwMonitor/html/css/bootstrap-theme.css\">" << std::endl;
    vfatDevice_ = new gem::hw::vfat::HwVFAT2(getApplicationLogger(), "VFAT9");
    vfatDevice_->setAddressTableFileName("testbeam_registers.xml");
    vfatDevice_->setDeviceIPAddress("192.168.0.115");
    vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+vfatToShow_);
    vfatDevice_->connectDevice();
    vfatDevice_->readVFAT2Counters();
    vfatDevice_->getAllSettings();
    std::cout << vfatDevice_->getVFAT2Params()<<std::endl; 
    *out << "<div class=\"panel panel-primary\">" << std::endl;
    *out << "<div class=\"panel-heading\">" << std::endl;
    *out << "<h1><div align=\"center\">Chip Id : "<< vfatToShow_ << "</div></h1>" << std::endl;
    *out << "</div>" << std::endl;
    *out << "<div class=\"panel-body\">" << std::endl;
    *out << "<h3><div class=\"alert alert-info\" role=\"alert\" align=\"center\">Device base node : "<< crateToShow_ << "::" << glibToShow_ << "::" << ohToShow_ <<  "</div></h3>" << std::endl;
    std::map <std::string, std::string> vfatProperties_;
    vfatProperties_ = gemHwMonitorVFAT_->getDevice()->getDeviceProperties();

    *out << cgicc::table().set("class","table");
    *out << cgicc::tr()<< std::endl;
        *out << cgicc::td();
            *out << cgicc::h3("Parameter");
        *out << cgicc::td()<< std::endl;
        *out << cgicc::td();
            *out << cgicc::h3("XML value");
        *out << cgicc::td()<< std::endl;
        *out << cgicc::td();
            *out << cgicc::h3("Hardware value");
        *out << cgicc::td()<< std::endl;
    *out << cgicc::tr() << std::endl;
    auto it = vfatProperties_.begin();
                   printVFAThwParameters("CalMode", (it->second).c_str(), (gem::hw::vfat::CalibrationModeToString.at(vfatDevice_->getVFAT2Params().calibMode)).c_str(), out); it++;
                   printVFAThwParameters("CalPhase", (it->second).c_str(), (vfatDevice_->getVFAT2Params().calPhase), out); it++;
                   printVFAThwParameters("CalPolarity", (it->second).c_str(), (gem::hw::vfat::CalPolarityToString.at(vfatDevice_->getVFAT2Params().calPol)).c_str(), out); it++;
                   printVFAThwParameters("DACSel", (it->second).c_str(), (gem::hw::vfat::DACModeToString.at(vfatDevice_->getVFAT2Params().dacMode)).c_str(), out); it++;
                   printVFAThwParameters("DFTest", (it->second).c_str(), (gem::hw::vfat::DFTestPatternToString.at(vfatDevice_->getVFAT2Params().sendTestPattern)).c_str(), out); it++;
                   printVFAThwParameters("DigInSel", (it->second).c_str(), (gem::hw::vfat::DigInSelToString.at(vfatDevice_->getVFAT2Params().digInSel)).c_str(), out); it++;
                   printVFAThwParameters("HitCountSel", (it->second).c_str(), (gem::hw::vfat::HitCountModeToString.at(vfatDevice_->getVFAT2Params().hitCountMode)).c_str(), out); it++;
                   printVFAThwParameters("IComp", (it->second).c_str(), vfatDevice_->getVFAT2Params().iComp, out); it++;
                   printVFAThwParameters("IPreampFeed", (it->second).c_str(), vfatDevice_->getVFAT2Params().iPreampFeed, out); it++;
                   printVFAThwParameters("IPreampIn", (it->second).c_str(), vfatDevice_->getVFAT2Params().iPreampIn, out); it++;
                   printVFAThwParameters("IPreampOut", (it->second).c_str(), vfatDevice_->getVFAT2Params().iPreampOut, out); it++;
                   printVFAThwParameters("IShaper", (it->second).c_str(), vfatDevice_->getVFAT2Params().iShaper, out); it++;
                   printVFAThwParameters("IShaperFeed", (it->second).c_str(), vfatDevice_->getVFAT2Params().iShaperFeed, out); it++;
                   printVFAThwParameters("LVDSPowerSave", (it->second).c_str(), (gem::hw::vfat::LVDSPowerSaveToString.at(vfatDevice_->getVFAT2Params().lvdsMode)).c_str(), out); it++;
                   printVFAThwParameters("Latency", (it->second).c_str(), vfatDevice_->getVFAT2Params().latency, out); it++;
                   printVFAThwParameters("MSPolarity", (it->second).c_str(), (gem::hw::vfat::MSPolarityToString.at(vfatDevice_->getVFAT2Params().msPol)).c_str(), out); it++;
                   printVFAThwParameters("MSPulseLength", (it->second).c_str(), (gem::hw::vfat::MSPulseLengthToString.at(vfatDevice_->getVFAT2Params().msPulseLen)).c_str(), out); it++;
                   printVFAThwParameters("PbBG", (it->second).c_str(), (gem::hw::vfat::PbBGToString.at(vfatDevice_->getVFAT2Params().padBandGap)).c_str(), out); it++;
                   printVFAThwParameters("ProbeMode", (it->second).c_str(), (gem::hw::vfat::ProbeModeToString.at(vfatDevice_->getVFAT2Params().probeMode)).c_str(), out); it++;
                   printVFAThwParameters("RecHitCT", (it->second).c_str(), (gem::hw::vfat::ReHitCTToString.at(vfatDevice_->getVFAT2Params().reHitCT)).c_str(), out); it++;
                   printVFAThwParameters("RunMode", (it->second).c_str(), (gem::hw::vfat::RunModeToString.at(vfatDevice_->getVFAT2Params().runMode)).c_str(), out); it++;
                   printVFAThwParameters("TriggerMode", (it->second).c_str(), (gem::hw::vfat::TriggerModeToString.at(vfatDevice_->getVFAT2Params().trigMode)).c_str(), out); it++;
                   printVFAThwParameters("TrimDACRange", (it->second).c_str(), (gem::hw::vfat::TrimDACRangeToString.at(vfatDevice_->getVFAT2Params().trimDACRange)).c_str(), out); it++;
                   printVFAThwParameters("VCal", (it->second).c_str(), vfatDevice_->getVFAT2Params().vCal, out); it++;
                   printVFAThwParameters("VThreshold1", (it->second).c_str(), vfatDevice_->getVFAT2Params().vThresh1, out); it++;
                   printVFAThwParameters("VThreshold2", (it->second).c_str(), vfatDevice_->getVFAT2Params().vThresh2, out); it++;
    *out << cgicc::tr();
    *out << cgicc::table();
    *out << "</div>" << std::endl;

    *out << cgicc::br()<< std::endl;
    *out << cgicc::hr()<< std::endl;

    delete vfatDevice_;
}

void gem::hwMonitor::gemHwMonitorWeb::printVFAThwParameters(const char* key, const char* value1, const char* value2, xgi::Output * out)
throw (xgi::exception::Exception)
{
    std::string alertColor;
    (boost::iequals(value1, value2)) ? alertColor="success" : alertColor="danger";
    *out << "<tr class=\"" << alertColor << "\">" << std::endl;
    *out << "<td>";
    *out << "<strong>" << key << ":" << "</strong>";
    *out << "</td>" << std::endl;
    *out << "<td>";
    *out << value1;
    *out << "</td>" << std::endl;
    *out << "<td>";
    *out << value2;
    *out << "</td>" << std::endl;
    *out << "</tr>" << std::endl;
}

void gem::hwMonitor::gemHwMonitorWeb::printVFAThwParameters(const char* key, const char* value, xgi::Output * out)
throw (xgi::exception::Exception)
{
    *out << "<tr>";
    *out << "<td>";
    *out << key << ":" << std::endl;
    *out << "</td>";
    *out << "<td>";
    *out << value << std::endl;
    *out << "</td>";
    *out << "</tr>";
 
}
void gem::hwMonitor::gemHwMonitorWeb::printVFAThwParameters(const char* key, const char* value1, uint8_t value2, xgi::Output * out)
throw (xgi::exception::Exception)
{
    std::stringstream ss;
    ss << std::dec << (unsigned) value2;
    //ss << std::hex << (unsigned) value;
    std::string value_string = "";
    //std::string value_string = "0x";
    value_string.append(ss.str());
    printVFAThwParameters(key, value1, value_string.c_str(), out);
}
void gem::hwMonitor::gemHwMonitorWeb::printVFAThwParameters(const char* key, uint8_t value, xgi::Output * out)
throw (xgi::exception::Exception)
{
    std::stringstream ss;
    ss << std::dec << (unsigned) value;
    //ss << std::hex << (unsigned) value;
    std::string value_string = "";
    //std::string value_string = "0x";
    value_string.append(ss.str());
    printVFAThwParameters(key, value_string.c_str(), out);
}
