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
    //gemHwMonitorCrate_ = new gemHwMonitorCrate();
    //gemHwMonitorGLIB_ = new gemHwMonitorGLIB();
    //gemHwMonitorOH_ = new gemHwMonitorOH();
    //gemHwMonitorVFAT_ = new gemHwMonitorVFAT();
    gemSystemHelper_ = new gemHwMonitorHelper(gemHwMonitorSystem_);
    crateCfgAvailable_ = false;
}

gem::hwMonitor::gemHwMonitorWeb::~gemHwMonitorWeb()
{
    delete gemHwMonitorSystem_;
    //delete gemHwMonitorCrate_;
    for_each(gemHwMonitorCrate_.begin(), gemHwMonitorCrate_.end(), free);
    for_each(gemHwMonitorGLIB_.begin(), gemHwMonitorGLIB_.end(), free);
    for_each(gemHwMonitorOH_.begin(), gemHwMonitorOH_.end(), free);
    for_each(gemHwMonitorVFAT_.begin(), gemHwMonitorVFAT_.end(), free);
    //delete gemHwMonitorGLIB_;
    //delete gemHwMonitorOH_;
    //delete gemHwMonitorVFAT_;
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
//            //gem::hw::GEMHwDevice* crateDevice_ = new gem::hw::GEMHwDevice();
            gem::hw::vfat::HwVFAT2* crateDevice_ = new gem::hw::vfat::HwVFAT2();
            crateDevice_->setDeviceIPAddress("192.168.0.162");
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
        for (unsigned int i = 0; i != gemHwMonitorSystem_->getDevice()->getSubDevicesRefs().size(); i++) 
        {
            gemHwMonitorCrate_.push_back(new  gemHwMonitorCrate());
            gemHwMonitorCrate_.back()->setDeviceConfiguration(*gemHwMonitorSystem_->getDevice()->getSubDevicesRefs().at(i));
            for (unsigned int i = 0; i != gemHwMonitorCrate_.back()->getDevice()->getSubDevicesRefs().size(); i++) 
            {
                gemHwMonitorGLIB_.push_back(new gemHwMonitorGLIB());
                gemHwMonitorGLIB_.back()->setDeviceConfiguration(*gemHwMonitorCrate_.back()->getDevice()->getSubDevicesRefs().at(i));
                gemHwMonitorCrate_.back()->addSubDeviceStatus(0);
                std::map <std::string, std::string> glibProperties_;
                glibProperties_ = gemHwMonitorGLIB_.back()->getDevice()->getDeviceProperties();
                for (auto it = glibProperties_.begin(); it != glibProperties_.end(); it++)
                {
                    if (it->first == "IP") glibIP = it->second; 
                }
                for (unsigned int i = 0; i != gemHwMonitorGLIB_.back()->getDevice()->getSubDevicesRefs().size(); i++) 
                {
                    gemHwMonitorOH_.push_back(new gemHwMonitorOH());
                    gemHwMonitorOH_.back()->setDeviceConfiguration(*gemHwMonitorGLIB_.back()->getDevice()->getSubDevicesRefs().at(i));
                    gemHwMonitorGLIB_.back()->addSubDeviceStatus(0);
                    for (unsigned int i = 0; i != gemHwMonitorOH_.back()->getDevice()->getSubDevicesRefs().size(); i++) 
                    {
                        gemHwMonitorVFAT_.push_back(new gemHwMonitorVFAT());
                        gemHwMonitorVFAT_.back()->setDeviceConfiguration(*gemHwMonitorOH_.back()->getDevice()->getSubDevicesRefs().at(i));
                        vfatDevice_ = new gem::hw::vfat::HwVFAT2(gemHwMonitorVFAT_.back()->getDevice()->getDeviceId());
                        //vfatDevice_ = new gem::hw::vfat::HwVFAT2(getApplicationLogger(), gemHwMonitorVFAT_.back()->getDevice()->getDeviceId());
                        vfatDevice_->setDeviceIPAddress(glibIP);
                        std::cout << "vfat ID from XML: " << gemHwMonitorVFAT_.back()->getDevice()->getDeviceId() << std::endl;
                        vfatDevice_->setDeviceBaseNode("VFATS."+gemHwMonitorVFAT_.back()->getDevice()->getDeviceId());
                        vfatDevice_->connectDevice();
                            if (vfatDevice_->isHwConnected()) 
                            {
                                gemHwMonitorVFAT_.back()->setDeviceStatus(0);
                                gemHwMonitorOH_.back()->addSubDeviceStatus(0);
                            } else {
                                gemHwMonitorVFAT_.back()->setDeviceStatus(2);
                                gemHwMonitorOH_.back()->addSubDeviceStatus(2);
                            }
                        delete vfatDevice_;
 
                        //gemHwMonitorVFAT_.back()->setDeviceStatus(0);
                        //gemHwMonitorOH_.back()->addSubDeviceStatus(0);
                    }
                }
            }
        }
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
    //for (auto i = gemHwMonitorSystem_->getDevice()->getSubDevicesRefs().begin(); i != gemHwMonitorSystem_->getDevice()->getSubDevicesRefs().end(); i++) 
    //{
        //if (i->getDeviceId() == crateToShow_) {gemHwMonitorCrate_->setDeviceConfiguration(*i);}
    // Auto-pointer doesn't work for some reason. Improve this later.
    for (unsigned int i = 0; i != gemHwMonitorSystem_->getDevice()->getSubDevicesRefs().size(); i++) 
    {
        if (gemHwMonitorSystem_->getDevice()->getSubDevicesRefs().at(i)->getDeviceId() == crateToShow_) 
        {
            gemHwMonitorCrate_.at(i)->setDeviceConfiguration(*gemHwMonitorSystem_->getDevice()->getSubDevicesRefs().at(i));
            indexCrate_ = i;
            for (int i=0; i<gemHwMonitorCrate_.at(indexCrate_)->getNumberOfSubDevices(); i++) {
                gemHwMonitorGLIB_.at(i)->setDeviceConfiguration(*gemHwMonitorCrate_.at(indexCrate_)->getDevice()->getSubDevicesRefs().at(i));
                std::map <std::string, std::string> glibProperties_;
                glibProperties_ = gemHwMonitorGLIB_.at(i)->getDevice()->getDeviceProperties();
                //std::string glibIP = "192.168.0.164";
                for (auto it = glibProperties_.begin(); it != glibProperties_.end(); it++)
                {
                    if (it->first == "IP") glibIP = it->second; 
                    std::cout << "GLIB IP is "<<glibIP << std::endl;
                }
                gem::hw::glib::HwGLIB* glibDevice_ = new gem::hw::glib::HwGLIB();
                glibDevice_->setDeviceIPAddress(glibIP);
                glibDevice_->connectDevice();
                //if (glibDevice_->isHwConnected())
                //{
                //    gemHwMonitorCrate_.at(indexCrate_)->addSubDeviceStatus(0);
                //} else {
                //    gemHwMonitorCrate_.at(indexCrate_)->addSubDeviceStatus(2);
                //}
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
    *out << "<tr><h2><div align=\"center\">Connected GLIBs</div></h2></tr>" << std::endl;
    *out << "<tr>" << std::endl;
    for (int i=0; i<gemHwMonitorCrate_.at(indexCrate_)->getNumberOfSubDevices(); i++) {
        std::string currentGLIBId;
        currentGLIBId += gemHwMonitorCrate_.at(indexCrate_)->getCurrentSubDeviceId(i);
        *out << cgicc::td();
            *out << cgicc::form().set("method","POST").set("action", methodExpandGLIB) << std::endl ;
            if (gemHwMonitorCrate_.at(indexCrate_)->getSubDeviceStatus(i) == 0)
            {
                *out << "<button type=\"submit\" class=\"btn btn-success\" name=\"glibButton\" value=\"" << currentGLIBId << "\">" << currentGLIBId<< "</button>" << std::endl;
            } else if (gemHwMonitorCrate_.at(indexCrate_)->getSubDeviceStatus(i) == 1)
            {
                *out << "<button type=\"submit\" class=\"btn btn-warning\" name=\"glibButton\" value=\"" << currentGLIBId << "\">" << currentGLIBId<< "</button>" << std::endl;
            } else if (gemHwMonitorCrate_.at(indexCrate_)->getSubDeviceStatus(i) == 2)
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
    for (unsigned int i = 0; i != gemHwMonitorCrate_.at(indexCrate_)->getDevice()->getSubDevicesRefs().size(); i++) 
    {
        if (gemHwMonitorCrate_.at(indexCrate_)->getDevice()->getSubDevicesRefs().at(i)->getDeviceId() == glibToShow_) 
        {
            indexGLIB_ = i;            
            for (int i=0; i<gemHwMonitorGLIB_.at(indexGLIB_)->getNumberOfSubDevices(); i++) {
                std::map <std::string, std::string> glibProperties_;
                glibProperties_ = gemHwMonitorGLIB_.at(indexGLIB_)->getDevice()->getDeviceProperties();
                std::string ohIP = "192.168.0.164";
                for (auto it = glibProperties_.begin(); it != glibProperties_.end(); it++)
                {
                    if (it->first == "IP") ohIP = it->second; 
                    std::cout << "OH IP is "<<ohIP << std::endl;
                }
                gem::hw::optohybrid::HwOptoHybrid* ohDevice_ = new gem::hw::optohybrid::HwOptoHybrid();
                ohDevice_->setDeviceIPAddress(ohIP);
                ohDevice_->connectDevice();
                if (ohDevice_->isHwConnected())
                {
                    gemHwMonitorGLIB_.at(indexGLIB_)->addSubDeviceStatus(0);
                } else {
                    gemHwMonitorGLIB_.at(indexGLIB_)->addSubDeviceStatus(2);
                }
                delete ohDevice_;
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
    std::string methodExpandCrate = toolbox::toString("/%s/expandCrate", getApplicationDescriptor()->getURN().c_str());
    *out << cgicc::table().set("class","table");
    *out << "</tr>" << std::endl;
        *out << cgicc::td();
            *out << cgicc::form().set("method","POST").set("action", methodExpandCrate) << std::endl ;
            *out << "<button type=\"submit\" class=\"btn btn-info\" name=\"crateButton\" value=\"" << crateToShow_ << "\">" << crateToShow_<< "</button>" << std::endl;
            *out << cgicc::form() << std::endl ;
        *out << cgicc::td();
    *out << "</tr>" << std::endl;
    *out << cgicc::table() <<std::endl;;
    glibDevice_ = new gem::hw::glib::HwGLIB();
    glibDevice_->setDeviceIPAddress(glibIP);
    glibDevice_->connectDevice();

    *out << "<div class=\"panel panel-primary\">" << std::endl;
    *out << "<div class=\"panel-heading\">" << std::endl;
    *out << "<h1><div align=\"center\">Chip Id : "<< glibToShow_ << "<br> Firmware version : " << glibDevice_->getUserFirmware() << "</div></h1>" << std::endl;
    *out << "</div>" << std::endl;
    *out << "<div class=\"panel-body\">" << std::endl;
    *out << "<h3><div class=\"alert alert-info\" role=\"alert\" align=\"center\">Device base node : "<< crateToShow_ << "</div></h3>" << std::endl;
    std::string methodExpandOH = toolbox::toString("/%s/expandOH", getApplicationDescriptor()->getURN().c_str());
    *out << cgicc::table().set("class","table");
    *out << "<tr><h2><div align=\"center\">Connected Optohybrids</div></h2></tr>" << std::endl;
    *out << "<tr>" << std::endl;
    for (int i=0; i<gemHwMonitorGLIB_.at(indexGLIB_)->getNumberOfSubDevices(); i++) {
        std::string currentOHId;
        currentOHId += gemHwMonitorGLIB_.at(indexGLIB_)->getCurrentSubDeviceId(i);
        *out << cgicc::td();
            *out << cgicc::form().set("method","POST").set("action", methodExpandOH) << std::endl ;
            if (gemHwMonitorGLIB_.at(indexGLIB_)->getSubDeviceStatus(i) == 0)
            {
                *out << "<button type=\"submit\" class=\"btn btn-success\" name=\"ohButton\" value=\"" << currentOHId << "\">" << currentOHId<< "</button>" << std::endl;
            } else if (gemHwMonitorGLIB_.at(indexGLIB_)->getSubDeviceStatus(i) == 1)
            {
                *out << "<button type=\"submit\" class=\"btn btn-warning\" name=\"ohButton\" value=\"" << currentOHId << "\">" << currentOHId<< "</button>" << std::endl;
            } else if (gemHwMonitorGLIB_.at(indexGLIB_)->getSubDeviceStatus(i) == 2)
            {
                *out << "<button type=\"submit\" class=\"btn btn-disabled\" name=\"ohButton\" value=\"" << currentOHId << "\" disabled>" << currentOHId<< "</button>" << std::endl;
            }
            *out << cgicc::form() << std::endl;
        *out << cgicc::td();
    }
    *out << "</tr>" << std::endl;
    *out << cgicc::table() <<std::endl;

    gem::hw::GEMHwDevice::OpticalLinkStatus linkStatus_;
    for (uint8_t i=1; i<2; i++) //For the moment only link 1 is available for OHv1. The app crashes if link is not available.
    {
        linkStatus_ = glibDevice_->LinkStatus(i);
        *out << cgicc::table().set("class","table");
            *out << "<tr>" << std::endl;
            *out << "<td>" << std::endl;
                *out << "Link N" << std::endl;
            *out << "</td>" << std::endl;
            *out << "<td>" << std::endl;
                *out << "Link Err" << std::endl;
            *out << "</td>" << std::endl;
            *out << "<td>" << std::endl;
                *out << "Received VFAT2 I2C requests" << std::endl;
            *out << "</td>" << std::endl;
            *out << "<td>" << std::endl;
                *out << "Sent VFAT2 I2C requests" << std::endl;
            *out << "</td>" << std::endl;
            *out << "<td>" << std::endl;
                *out << "Received register requests" << std::endl;
            *out << "</td>" << std::endl;
            *out << "<td>" << std::endl;
                *out << "Sent register requests" << std::endl;
            *out << "</td>" << std::endl;
            *out << "</tr>" << std::endl;
            *out << "<tr>" << std::endl;
            *out << "<td>" << std::endl;
                *out << (int)i << std::endl;
            *out << "</td>" << std::endl;
            *out << "<td>" << std::endl;
                *out << linkStatus_.Errors << std::endl;
            *out << "</td>" << std::endl;
            *out << "<td>" << std::endl;
                *out << linkStatus_.I2CReceived << std::endl;
            *out << "</td>" << std::endl;
            *out << "<td>" << std::endl;
                *out << linkStatus_.I2CSent << std::endl;
            *out << "</td>" << std::endl;
            *out << "<td>" << std::endl;
                *out << linkStatus_.RegisterReceived << std::endl;
            *out << "</td>" << std::endl;
            *out << "<td>" << std::endl;
                *out << linkStatus_.RegisterSent << std::endl;
            *out << "</td>" << std::endl;
            *out << "</tr>" << std::endl;
        *out << cgicc::table() <<std::endl;
    }
    *out << cgicc::table().set("class","table");
        *out << "<tr>" << std::endl;
        *out << "<td>" << std::endl;
            *out << "Device IP" << std::endl;
        *out << "</td>" << std::endl;
        *out << "<td>" << std::endl;
            *out << glibDevice_->getIPAddress() << std::endl;
        *out << "</td>" << std::endl;
        *out << "</td>" << std::endl;
        *out << "</tr>" << std::endl;
        *out << "<tr>" << std::endl;
        *out << "<td>" << std::endl;
            *out << "Device MAC address" << std::endl;
        *out << "</td>" << std::endl;
        *out << "<td>" << std::endl;
            *out << glibDevice_->getMACAddress() << std::endl;
        *out << "</td>" << std::endl;
        *out << "</td>" << std::endl;
        *out << "</tr>" << std::endl;
        *out << "<tr>" << std::endl;
        *out << "<td>" << std::endl;
            *out << "PCIe clock multiplier" << std::endl;
        *out << "</td>" << std::endl;
        *out << "<td>" << std::endl;
            *out << (int)glibDevice_->PCIeClkFSel() << std::endl;
        *out << "</td>" << std::endl;
        *out << "</td>" << std::endl;
        *out << "</tr>" << std::endl;
        *out << "<tr>" << std::endl;
        *out << "<td>" << std::endl;
            *out << "PCIe clock reset state" << std::endl;
        *out << "</td>" << std::endl;
        *out << "<td>" << std::endl;
            *out << (int)glibDevice_->PCIeClkMaster() << std::endl;
        *out << "</td>" << std::endl;
        *out << "</td>" << std::endl;
        *out << "</tr>" << std::endl;
        *out << "<tr>" << std::endl;
        *out << "<td>" << std::endl;
            *out << "PCIe clock output status" << std::endl;
        *out << "</td>" << std::endl;
        *out << "<td>" << std::endl;
            *out << (int)glibDevice_->PCIeClkOutput() << std::endl;
        *out << "</td>" << std::endl;
        *out << "</td>" << std::endl;
        *out << "</tr>" << std::endl;
        *out << "<tr>" << std::endl;
        *out << "<td>" << std::endl;
            *out << "CDCE clock output status" << std::endl;
        *out << "</td>" << std::endl;
        *out << "<td>" << std::endl;
            *out << (int)glibDevice_->CDCEPower() << std::endl;
        *out << "</td>" << std::endl;
        *out << "</td>" << std::endl;
        *out << "</tr>" << std::endl;
        *out << "<tr>" << std::endl;
        *out << "<td>" << std::endl;
            *out << "CDCE reference clock" << std::endl;
        *out << "</td>" << std::endl;
        *out << "<td>" << std::endl;
            *out << (int)glibDevice_->CDCEReference() << std::endl;
        *out << "</td>" << std::endl;
        *out << "</td>" << std::endl;
        *out << "</tr>" << std::endl;
        *out << "<tr>" << std::endl;
        *out << "<td>" << std::endl;
            *out << "CDCE syncronization status" << std::endl;
        *out << "</td>" << std::endl;
        *out << "<td>" << std::endl;
            *out << (int)glibDevice_->CDCESync() << std::endl;
        *out << "</td>" << std::endl;
        *out << "</td>" << std::endl;
        *out << "</tr>" << std::endl;
        *out << "<tr>" << std::endl;
        *out << "<td>" << std::endl;
            *out << "CDCE control output status" << std::endl;
        *out << "</td>" << std::endl;
        *out << "<td>" << std::endl;
            *out << (int)glibDevice_->CDCEControl() << std::endl;
        *out << "</td>" << std::endl;
        *out << "</td>" << std::endl;
        *out << "</tr>" << std::endl;
        *out << "<td>" << std::endl;
            *out << "TClkB output to the backplane status" << std::endl;
        *out << "</td>" << std::endl;
        *out << "<td>" << std::endl;
            *out << (int)glibDevice_->TClkBOutput() << std::endl;
        *out << "</td>" << std::endl;
        *out << "</td>" << std::endl;
        *out << "</tr>" << std::endl;
 
      *out << cgicc::table() <<std::endl;


    *out << "</div>" << std::endl;
    *out << cgicc::br()<< std::endl;
    *out << cgicc::hr()<< std::endl;
    delete glibDevice_;
}

void gem::hwMonitor::gemHwMonitorWeb::expandOH(xgi::Input * in, xgi::Output * out )
throw (xgi::exception::Exception)
{
    cgicc::Cgicc cgi(in);
    ohToShow_ = cgi.getElement("ohButton")->getValue();
    // Auto-pointer doesn't work for some reason. Improve this later.
    for (unsigned int i = 0; i != gemHwMonitorGLIB_.at(indexGLIB_)->getDevice()->getSubDevicesRefs().size(); i++) 
    {
        if ((gemHwMonitorGLIB_.at(indexGLIB_)->getDevice()->getSubDevicesRefs().at(i)->getDeviceId() == ohToShow_) && (!(gemHwMonitorOH_.at(i)->isConfigured())))
        {
            indexOH_ = i;
            gemHwMonitorOH_.at(indexOH_)->setIsConfigured(true);
            vfatDevice_ = new gem::hw::vfat::HwVFAT2(vfatToShow_);
            vfatDevice_->setDeviceIPAddress(glibIP);
 
            for (int i=0; i<gemHwMonitorOH_.at(indexOH_)->getNumberOfSubDevices(); i++) {
            std::string vfatID_ = gemHwMonitorOH_.at(indexOH_)->getDevice()->getSubDevicesRefs().at(i)->getDeviceId();
            std::cout << "vfat ID from XML" << vfatID_ << std::endl;
            vfatDevice_->setDeviceBaseNode("VFATS."+vfatID_);
            vfatDevice_->connectDevice();
                if (vfatDevice_->isHwConnected()) 
                {
                    gemHwMonitorOH_.at(indexOH_)->addSubDeviceStatus(0);
                } else {
                    gemHwMonitorOH_.at(indexOH_)->addSubDeviceStatus(2);
                }
            delete vfatDevice_;
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
    std::string methodExpandCrate = toolbox::toString("/%s/expandCrate", getApplicationDescriptor()->getURN().c_str());
    std::string methodExpandGLIB = toolbox::toString("/%s/expandGLIB", getApplicationDescriptor()->getURN().c_str());
    *out << cgicc::table().set("class","table");
    *out << "</tr>" << std::endl;
        *out << cgicc::td();
            *out << cgicc::form().set("method","POST").set("action", methodExpandCrate) << std::endl ;
            *out << "<button type=\"submit\" class=\"btn btn-info\" name=\"crateButton\" value=\"" << crateToShow_ << "\">" << crateToShow_<< "</button>" << std::endl;
            *out << cgicc::form() << std::endl ;
        *out << cgicc::td();
        *out << cgicc::td();
            *out << cgicc::form().set("method","POST").set("action", methodExpandGLIB) << std::endl ;
            *out << "<button type=\"submit\" class=\"btn btn-info\" name=\"glibButton\" value=\"" << glibToShow_ << "\">" << glibToShow_<< "</button>" << std::endl;
            *out << cgicc::form() << std::endl ;
        *out << cgicc::td();
    *out << "</tr>" << std::endl;
    *out << cgicc::table() <<std::endl;;
    ohDevice_ = new gem::hw::optohybrid::HwOptoHybrid();
    ohDevice_->setDeviceIPAddress(glibIP);
    ohDevice_->connectDevice();

    *out << "<div class=\"panel panel-primary\">" << std::endl;
    *out << "<div class=\"panel-heading\">" << std::endl;
    uint8_t link=2;
    //*out << "<h1><div align=\"center\">Chip Id : "<< ohToShow_ << "<br> Firmware version : " << "XXX" << "</div></h1>" << std::endl;
    *out << "<h1><div align=\"center\">Chip Id : "<< ohToShow_ << "<br> Firmware version : " << ohDevice_->getFirmware(link) << "</div></h1>" << std::endl;
    *out << "</div>" << std::endl;
    *out << "<div class=\"panel-body\">" << std::endl;
    *out << "<h3><div class=\"alert alert-info\" role=\"alert\" align=\"center\">Device base node : "<< crateToShow_ << "::" << glibToShow_ << "</div></h3>" << std::endl;
    std::string methodExpandVFAT = toolbox::toString("/%s/expandVFAT", getApplicationDescriptor()->getURN().c_str());
    *out << cgicc::table().set("class","table");
    *out << "<tr><h2><div align=\"center\">Connected VFAT's</div></h2></tr>" << std::endl;
    *out << "<tr>" << std::endl;
    for (int i=0; i<gemHwMonitorOH_.at(indexOH_)->getNumberOfSubDevices(); i++) {
        std::string currentVFATId;
        currentVFATId += gemHwMonitorOH_.at(indexOH_)->getCurrentSubDeviceId(i);
        *out << cgicc::td();
            *out << cgicc::form().set("method","POST").set("action", methodExpandVFAT) << std::endl ;
            if (gemHwMonitorOH_.at(indexOH_)->getSubDeviceStatus(i) == 0)
            {
                *out << "<button type=\"submit\" class=\"btn btn-success\" name=\"vfatButton\" value=\"" << currentVFATId << "\">" << currentVFATId<< "</button>" << std::endl;
            } else if (gemHwMonitorOH_.at(indexOH_)->getSubDeviceStatus(i) == 1)
            {
                *out << "<button type=\"submit\" class=\"btn btn-warning\" name=\"vfatButton\" value=\"" << currentVFATId << "\">" << currentVFATId<< "</button>" << std::endl;
            } else if (gemHwMonitorOH_.at(indexOH_)->getSubDeviceStatus(i) == 2)
            {
                *out << "<button type=\"submit\" class=\"btn btn-danger\" name=\"vfatButton\" value=\"" << currentVFATId << "\">" << currentVFATId<< "</button>" << std::endl;
                //*out << "<button type=\"submit\" class=\"btn btn-disabled\" name=\"vfatButton\" value=\"" << currentVFATId << "\" disabled>" << currentVFATId<< "</button>" << std::endl;
            }
            *out << cgicc::form() << std::endl;
        *out << cgicc::td();
    }
    *out << "</tr>" << std::endl;
    *out << cgicc::table() <<std::endl;

    gem::hw::GEMHwDevice::OpticalLinkStatus linkStatus_;
    for (uint8_t i=0; i<3; i++) //For the moment only link 1 is available for OHv1. The app crashes if link is not available.
    {
        linkStatus_ = ohDevice_->LinkStatus(i);
        *out << cgicc::table().set("class","table");
            *out << "<tr>" << std::endl;
            *out << "<td>" << std::endl;
                *out << "Link N" << std::endl;
            *out << "</td>" << std::endl;
            *out << "<td>" << std::endl;
                *out << "Link Err" << std::endl;
            *out << "</td>" << std::endl;
            *out << "<td>" << std::endl;
                *out << "Received VFAT2 I2C requests" << std::endl;
            *out << "</td>" << std::endl;
            *out << "<td>" << std::endl;
                *out << "Sent VFAT2 I2C requests" << std::endl;
            *out << "</td>" << std::endl;
            *out << "<td>" << std::endl;
                *out << "Received register requests" << std::endl;
            *out << "</td>" << std::endl;
            *out << "<td>" << std::endl;
                *out << "Sent register requests" << std::endl;
            *out << "</td>" << std::endl;
            *out << "</tr>" << std::endl;
            *out << "<tr>" << std::endl;
            *out << "<td>" << std::endl;
                *out << (int)i << std::endl;
            *out << "</td>" << std::endl;
            *out << "<td>" << std::endl;
                *out << linkStatus_.Errors << std::endl;
            *out << "</td>" << std::endl;
            *out << "<td>" << std::endl;
                *out << linkStatus_.I2CReceived << std::endl;
            *out << "</td>" << std::endl;
            *out << "<td>" << std::endl;
                *out << linkStatus_.I2CSent << std::endl;
            *out << "</td>" << std::endl;
            *out << "<td>" << std::endl;
                *out << linkStatus_.RegisterReceived << std::endl;
            *out << "</td>" << std::endl;
            *out << "<td>" << std::endl;
                *out << linkStatus_.RegisterSent << std::endl;
            *out << "</td>" << std::endl;
            *out << "</tr>" << std::endl;
        *out << cgicc::table() <<std::endl;
    }
    std::pair<bool,bool> statusVFATClock_;
    statusVFATClock_ = ohDevice_->StatusVFATClock(link);
    *out << cgicc::table().set("class","table");
        *out << "<tr>" << std::endl;
        *out << "<td>" << std::endl;
            *out << "VFAT Clock Source" << std::endl;
        *out << "</td>" << std::endl;
        *out << "<td>" << std::endl;
            *out << statusVFATClock_.first << std::endl;
        *out << "</td>" << std::endl;
        *out << "</tr>" << std::endl;
        *out << "<tr>" << std::endl;
        *out << "<td>" << std::endl;
            *out << "VFAT Clock Fallback" << std::endl;
        *out << "</td>" << std::endl;
        *out << "<td>" << std::endl;
            *out << statusVFATClock_.second << std::endl;
        *out << "</td>" << std::endl;
        *out << "</tr>" << std::endl;
    //*out << cgicc::table() <<std::endl;

    std::pair<bool,bool> statusCDCEClock_;
    statusCDCEClock_ = ohDevice_->StatusCDCEClock(link);
    //*out << cgicc::table().set("class","table");
        *out << "<tr>" << std::endl;
        *out << "<td>" << std::endl;
            *out << "CDCE Clock Source" << std::endl;
        *out << "</td>" << std::endl;
        *out << "<td>" << std::endl;
            *out << statusCDCEClock_.first << std::endl;
        *out << "</td>" << std::endl;
        *out << "</tr>" << std::endl;
        *out << "<tr>" << std::endl;
        *out << "<td>" << std::endl;
            *out << "CDCE Clock Fallback" << std::endl;
        *out << "</td>" << std::endl;
        *out << "<td>" << std::endl;
            *out << statusCDCEClock_.second << std::endl;
        *out << "</td>" << std::endl;
        *out << "</tr>" << std::endl;
    //*out << cgicc::table() <<std::endl;

    //*out << cgicc::table().set("class","table");
        *out << "<tr>" << std::endl;
        *out << "<td>" << std::endl;
            *out << "Trigger Source" << std::endl;
        *out << "</td>" << std::endl;
        *out << "<td>" << std::endl;
            *out << (int)ohDevice_->getTrigSource(link) << std::endl;
        *out << "</td>" << std::endl;
        *out << "</tr>" << std::endl;
        *out << "<tr>" << std::endl;
        *out << "<td>" << std::endl;
            *out << "S-bit Source" << std::endl;
        *out << "</td>" << std::endl;
        *out << "<td>" << std::endl;
            *out << (int)ohDevice_->getSBitSource(link) << std::endl;
        *out << "</td>" << std::endl;
        *out << "</tr>" << std::endl;
    //*out << cgicc::table() <<std::endl;
 
    const char *l1CountNames[] = {"External L1 Counter", "Internal L1 Counter", "Delayed L1 Counter","Total L1 Counter"};
    //*out << cgicc::table().set("class","table");
        for (uint8_t i = 0; i<4; i++)
        {
            *out << "<tr>" << std::endl;
            *out << "<td>" << std::endl;
                *out << l1CountNames[i] << std::endl;
            *out << "</td>" << std::endl;
            *out << "<td>" << std::endl;
                *out << ohDevice_->GetL1ACount(i,link) << std::endl;
            *out << "</td>" << std::endl;
            *out << "</tr>" << std::endl;
        }
    //*out << cgicc::table() <<std::endl;

    const char *calPulseCountNames[] = {"Internal CalPulse Counter", "Delayed CalPulse Counter","Total CalPulse Counter"};
    //*out << cgicc::table().set("class","table");
        for (uint8_t i = 0; i<3; i++)
        {
            *out << "<tr>" << std::endl;
            *out << "<td>" << std::endl;
                *out << calPulseCountNames[i] << std::endl;
            *out << "</td>" << std::endl;
            *out << "<td>" << std::endl;
                *out << ohDevice_->GetCalPulseCount(i,link) << std::endl;
            *out << "</td>" << std::endl;
            *out << "</tr>" << std::endl;
        }
    *out << cgicc::table() <<std::endl;

    *out << "</div>" << std::endl;
    *out << cgicc::br()<< std::endl;
    *out << cgicc::hr()<< std::endl;
    delete ohDevice_;
}

void gem::hwMonitor::gemHwMonitorWeb::expandVFAT(xgi::Input * in, xgi::Output * out )
throw (xgi::exception::Exception)
{
    cgicc::Cgicc cgi(in);
    vfatToShow_ = cgi.getElement("vfatButton")->getValue();
    // Auto-pointer doesn't work for some reason. Improve this later.
    for (unsigned int i = 0; i != gemHwMonitorOH_.at(indexOH_)->getDevice()->getSubDevicesRefs().size(); i++) 
    {
        if (gemHwMonitorOH_.at(indexOH_)->getDevice()->getSubDevicesRefs().at(i)->getDeviceId() == vfatToShow_) 
        {
            indexVFAT_ = i;
        }
    }
    this->vfatPanel(in,out);
}

void gem::hwMonitor::gemHwMonitorWeb::vfatPanel(xgi::Input * in, xgi::Output * out )
throw (xgi::exception::Exception)
{
    *out << "<link rel=\"stylesheet\" type=\"text/css\" href=\"/gemdaq/gemHwMonitor/html/css/bootstrap.css\">" << std::endl
    << "<link rel=\"stylesheet\" type=\"text/css\" href=\"/gemdaq/gemHwMonitor/html/css/bootstrap-theme.css\">" << std::endl;
    vfatDevice_ = new gem::hw::vfat::HwVFAT2(vfatToShow_);
    vfatDevice_->setDeviceIPAddress(glibIP);
    vfatDevice_->setDeviceBaseNode("VFATS."+vfatToShow_);
    vfatDevice_->connectDevice();
    vfatDevice_->readVFAT2Counters();
    vfatDevice_->getAllSettings();
    std::string methodExpandCrate = toolbox::toString("/%s/expandCrate", getApplicationDescriptor()->getURN().c_str());
    std::string methodExpandGLIB = toolbox::toString("/%s/expandGLIB", getApplicationDescriptor()->getURN().c_str());
    std::string methodExpandOH = toolbox::toString("/%s/expandOH", getApplicationDescriptor()->getURN().c_str());
    *out << cgicc::table().set("class","table");
    *out << "<tr>" << std::endl;
        *out << cgicc::td();
            *out << cgicc::form().set("method","POST").set("action", methodExpandCrate) << std::endl ;
            *out << "<button type=\"submit\" class=\"btn btn-info\" name=\"crateButton\" value=\"" << crateToShow_ << "\">" << crateToShow_<< "</button>" << std::endl;
            *out << cgicc::form() << std::endl ;
        *out << cgicc::td();
        *out << cgicc::td();
            *out << cgicc::form().set("method","POST").set("action", methodExpandGLIB) << std::endl ;
            *out << "<button type=\"submit\" class=\"btn btn-info\" name=\"glibButton\" value=\"" << glibToShow_ << "\">" << glibToShow_<< "</button>" << std::endl;
            *out << cgicc::form() << std::endl ;
        *out << cgicc::td();
        *out << cgicc::td();
            *out << cgicc::form().set("method","POST").set("action", methodExpandOH) << std::endl ;
            *out << "<button type=\"submit\" class=\"btn btn-info\" name=\"ohButton\" value=\"" << ohToShow_ << "\">" << ohToShow_<< "</button>" << std::endl;
            *out << cgicc::form() << std::endl ;
        *out << cgicc::td();
    *out << "</tr>" << std::endl;
    *out << cgicc::table() <<std::endl;;
    *out << "<div class=\"panel panel-primary\">" << std::endl;
    *out << "<div class=\"panel-heading\">" << std::endl;
    *out << "<h1><div align=\"center\">Chip Id : "<< vfatToShow_ << "</div></h1>" << std::endl;
    *out << "</div>" << std::endl;
    *out << "<div class=\"panel-body\">" << std::endl;
    *out << "<h3><div class=\"alert alert-info\" role=\"alert\" align=\"center\">Device base node : "<< crateToShow_ << "::" << glibToShow_ << "::" << ohToShow_ <<  "</div></h3>" << std::endl;
    std::map <std::string, std::string> vfatProperties_;
    vfatProperties_ = gemHwMonitorVFAT_.at(indexVFAT_)->getDevice()->getDeviceProperties();

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
                   printVFAThwParameters("CalMode", (vfatProperties_.find("CalMode")->second).c_str(), (gem::hw::vfat::CalibrationModeToString.at(vfatDevice_->getVFAT2Params().calibMode)).c_str(), out);
                   printVFAThwParameters("CalPolarity", (vfatProperties_.find("CalPolarity")->second).c_str(), (gem::hw::vfat::CalPolarityToString.at(vfatDevice_->getVFAT2Params().calPol)).c_str(), out);
                   printVFAThwParameters("MSPolarity", (vfatProperties_.find("MSPolarity")->second).c_str(), (gem::hw::vfat::MSPolarityToString.at(vfatDevice_->getVFAT2Params().msPol)).c_str(), out);
                   printVFAThwParameters("TriggerMode", (vfatProperties_.find("TriggerMode")->second).c_str(), (gem::hw::vfat::TriggerModeToString.at(vfatDevice_->getVFAT2Params().trigMode)).c_str(), out);
                   printVFAThwParameters("RunMode", (vfatProperties_.find("RunMode")->second).c_str(), (gem::hw::vfat::RunModeToString.at(vfatDevice_->getVFAT2Params().runMode)).c_str(), out);
                   printVFAThwParameters("ReHitCT", (vfatProperties_.find("ReHitCT")->second).c_str(), (gem::hw::vfat::ReHitCTToString.at(vfatDevice_->getVFAT2Params().reHitCT)).c_str(), out);
                   printVFAThwParameters("LVDSPowerSave", (vfatProperties_.find("LVDSPowerSave")->second).c_str(), (gem::hw::vfat::LVDSPowerSaveToString.at(vfatDevice_->getVFAT2Params().lvdsMode)).c_str(), out);
                   printVFAThwParameters("DACMode", (vfatProperties_.find("DACMode")->second).c_str(), (gem::hw::vfat::DACModeToString.at(vfatDevice_->getVFAT2Params().dacMode)).c_str(), out);
                   printVFAThwParameters("DigInSel", (vfatProperties_.find("DigInSel")->second).c_str(), (gem::hw::vfat::DigInSelToString.at(vfatDevice_->getVFAT2Params().digInSel)).c_str(), out);
                   printVFAThwParameters("MSPulseLength", (vfatProperties_.find("MSPulseLength")->second).c_str(), (gem::hw::vfat::MSPulseLengthToString.at(vfatDevice_->getVFAT2Params().msPulseLen)).c_str(), out);
                   printVFAThwParameters("HitCountMode", (vfatProperties_.find("HitCountMode")->second).c_str(), (gem::hw::vfat::HitCountModeToString.at(vfatDevice_->getVFAT2Params().hitCountMode)).c_str(), out);
                   printVFAThwParameters("PbBG", (vfatProperties_.find("PbBG")->second).c_str(), (gem::hw::vfat::PbBGToString.at(vfatDevice_->getVFAT2Params().padBandGap)).c_str(), out);
                   printVFAThwParameters("TrimDACRange", (vfatProperties_.find("TrimDACRange")->second).c_str(), (gem::hw::vfat::TrimDACRangeToString.at(vfatDevice_->getVFAT2Params().trimDACRange)).c_str(), out);
                   printVFAThwParameters("IPreampIn", (vfatProperties_.find("IPreampIn")->second).c_str(), vfatDevice_->getVFAT2Params().iPreampIn, out);
                   printVFAThwParameters("IPreampFeed", (vfatProperties_.find("IPreampFeed")->second).c_str(), vfatDevice_->getVFAT2Params().iPreampFeed, out);
                   printVFAThwParameters("IPreampOut", (vfatProperties_.find("IPreampOut")->second).c_str(), vfatDevice_->getVFAT2Params().iPreampOut, out);
                   printVFAThwParameters("IShaper", (vfatProperties_.find("IShaper")->second).c_str(), vfatDevice_->getVFAT2Params().iShaper, out);
                   printVFAThwParameters("IShaperFeed", (vfatProperties_.find("IShaperFeed")->second).c_str(), vfatDevice_->getVFAT2Params().iShaperFeed, out);
                   printVFAThwParameters("IComp", (vfatProperties_.find("IComp")->second).c_str(), vfatDevice_->getVFAT2Params().iComp, out);
                   printVFAThwParameters("Latency", (vfatProperties_.find("Latency")->second).c_str(), vfatDevice_->getVFAT2Params().latency, out);
                   printVFAThwParameters("VCal", (vfatProperties_.find("VCal")->second).c_str(), vfatDevice_->getVFAT2Params().vCal, out);
                   printVFAThwParameters("VThreshold1", (vfatProperties_.find("VThreshold1")->second).c_str(), vfatDevice_->getVFAT2Params().vThresh1, out);
                   printVFAThwParameters("VThreshold2", (vfatProperties_.find("VThreshold2")->second).c_str(), vfatDevice_->getVFAT2Params().vThresh2, out);
                   printVFAThwParameters("CalPhase", (vfatProperties_.find("CalPhase")->second).c_str(), (vfatDevice_->getVFAT2Params().calPhase), out);
                   //printVFAThwParameters("DFTest", (vfatProperties_.find("DFTest")->second).c_str(), (gem::hw::vfat::DFTestPatternToString.at(vfatDevice_->getVFAT2Params().sendTestPattern)).c_str(), out);
                   //printVFAThwParameters("ProbeMode", (vfatProperties_.find("ProbeMode")->second).c_str(), (gem::hw::vfat::ProbeModeToString.at(vfatDevice_->getVFAT2Params().probeMode)).c_str(), out);
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
        std::cout << vfatToShow_ << " status : " << gemHwMonitorVFAT_.at(indexVFAT_)->getDeviceStatus() << std::endl;
    if (!(gemHwMonitorVFAT_.at(indexVFAT_)->getDeviceStatus()))
    {
        std::cout << vfatToShow_ << " status : " << gemHwMonitorVFAT_.at(indexVFAT_)->getDeviceStatus() << std::endl;
        (boost::iequals(value1, value2)) ? gemHwMonitorVFAT_.at(indexVFAT_)->setDeviceStatus(0):gemHwMonitorVFAT_.at(indexVFAT_)->setDeviceStatus(1);
        for (unsigned int i = 0; i != gemHwMonitorOH_.at(indexOH_)->getDevice()->getSubDevicesRefs().size(); i++) 
        {
            if (gemHwMonitorOH_.at(indexOH_)->getDevice()->getSubDevicesRefs().at(i)->getDeviceId() == vfatToShow_) 
            {
                gemHwMonitorOH_.at(indexOH_)->setSubDeviceStatus(1,i);
            }
        }
        for (unsigned int i = 0; i != gemHwMonitorGLIB_.at(indexGLIB_)->getDevice()->getSubDevicesRefs().size(); i++) 
        {
            if (gemHwMonitorGLIB_.at(indexGLIB_)->getDevice()->getSubDevicesRefs().at(i)->getDeviceId() == ohToShow_) 
            {
                gemHwMonitorGLIB_.at(indexGLIB_)->setSubDeviceStatus(1,i);
            }
        }
        for (unsigned int i = 0; i != gemHwMonitorCrate_.at(indexCrate_)->getDevice()->getSubDevicesRefs().size(); i++) 
        {
            if (gemHwMonitorCrate_.at(indexCrate_)->getDevice()->getSubDevicesRefs().at(i)->getDeviceId() == glibToShow_) 
            {
                gemHwMonitorCrate_.at(indexGLIB_)->setSubDeviceStatus(1,i);
            }
        }

    }
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
