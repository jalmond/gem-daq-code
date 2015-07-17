#include "gem/hwMonitor/gemVFATMonitor.h"
#include <boost/algorithm/string.hpp>

void gem::hwMonitor::gemVFATMonitor::vfatPanel(xgi::Output * out )
throw (xgi::exception::Exception)
{
    *out << "<link rel=\"stylesheet\" type=\"text/css\" href=\"/gemdaq/gemHwMonitor/html/css/bootstrap.css\">" << std::endl
    << "<link rel=\"stylesheet\" type=\"text/css\" href=\"/gemdaq/gemHwMonitor/html/css/bootstrap-theme.css\">" << std::endl;
    vfatDevice_ = new gem::hw::vfat::HwVFAT2(getApplicationLogger(), "VFAT9");
    vfatDevice_->setAddressTableFileName("testbeam_registers.xml");
    vfatDevice_->setDeviceIPAddress("192.168.0.175");
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
