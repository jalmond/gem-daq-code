// VFAT2ManagerWeb.cc

#include "gem/hw/vfat/VFAT2ManagerWeb.h"
#include "gem/hw/vfat/VFAT2Manager.h"
#include "gem/hw/vfat/VFAT2Monitor.h"

#include "gem/hw/vfat/exception/Exception.h"

#include "xcept/tools.h"

gem::hw::vfat::VFAT2ManagerWeb::VFAT2ManagerWeb(gem::hw::vfat::VFAT2Manager* vfatApp) :
  gem::base::GEMWebApplication(vfatApp)
{
}

gem::hw::vfat::VFAT2ManagerWeb::~VFAT2ManagerWeb()
{
  //default destructor
}

void gem::hw::vfat::VFAT2ManagerWeb::webDefault(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  if (p_gemFSMApp)
    DEBUG("current state is" << dynamic_cast<gem::hw::vfat::VFAT2Manager*>(p_gemFSMApp)->getCurrentState());
  *out << cgicc::script().set("type","text/javascript")
    .set("src","/gemdaq/gemhardware/html/scripts/vfat/vfat.js")
       << cgicc::script() << std::endl;
  *out << "<div class=\"xdaq-tab-wrapper\">" << std::endl;
  *out << "<div class=\"xdaq-tab\" title=\"VFAT2Manager Control Panel\" >"  << std::endl;
  controlPanel(in,out);
  *out << "</div>" << std::endl;

  *out << "<div class=\"xdaq-tab\" title=\"Monitoring page\"/>"  << std::endl;
  monitorPage(in,out);
  *out << "</div>" << std::endl;

  std::string expURL = "/" + p_gemApp->getApplicationDescriptor()->getURN() + "/expertPage";
  *out << "<div class=\"xdaq-tab\" title=\"Expert page\"/>"  << std::endl;
  expertPage(in,out);
  *out << "</div>" << std::endl;

  std::string cardURL = "/" + p_gemApp->getApplicationDescriptor()->getURN() + "/cardPage";
  *out << "<div class=\"xdaq-tab\" title=\"Card page\"/>"  << std::endl;
  cardPage(in,out);
  *out << "</div>" << std::endl;

  *out << "</div>" << std::endl;
  std::string updateLink = "/" + p_gemApp->m_urn + "/jsonUpdate";
  *out << "<script type=\"text/javascript\">"            << std::endl
       << "    startUpdate( \"" << updateLink << "\" );" << std::endl
       << "</script>" << std::endl;
}

/*To be filled in with the monitor page code*/
void gem::hw::vfat::VFAT2ManagerWeb::monitorPage(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  INFO("monitorPage");
  *out << "<div class=\"xdaq-tab-wrapper\">" << std::endl;
  *out << "<div class=\"xdaq-tab\" title=\"DAQ Link Monitoring\" >"  << std::endl;
  // all monitored VFATs in one page, or separate tabs?
  //buildDaqLinkMonitoring();
  *out << "</div>" << std::endl;
  *out << "</div>" << std::endl;
}

/*To be filled in with the expert page code*/
void gem::hw::vfat::VFAT2ManagerWeb::expertPage(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  INFO("expertPage");
  //fill this page with the expert views for the VFAT2Manager
  *out << "expertPage</br>" << std::endl;
}

/*To be filled in with the card page code*/
void gem::hw::vfat::VFAT2ManagerWeb::cardPage(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  INFO("cardPage");
  //fill this page with the card views for the VFAT2Manager
  *out << "<div class=\"xdaq-tab-wrapper\">" << std::endl;
  for (unsigned int i = 0; i < gem::base::GEMFSMApplication::MAX_AMCS_PER_CRATE; ++i) {
    auto card = dynamic_cast<gem::hw::vfat::VFAT2Manager*>(p_gemFSMApp)->m_vfat2monitors[i];
    if (card) {
      *out << "<div class=\"xdaq-tab\" title=\"" << card->getDeviceID() << "\" >"  << std::endl;
      card->buildMonitorPage(out);
      *out << "</div>" << std::endl;
    }
  }
  *out << "</div>" << std::endl;
}

void gem::hw::vfat::VFAT2ManagerWeb::jsonUpdate(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  out->getHTTPResponseHeader().addHeader("Content-Type", "application/json");
  *out << " { \n";
  for (unsigned int i = 0; i < gem::base::GEMFSMApplication::MAX_AMCS_PER_CRATE; ++i) {
    auto card = dynamic_cast<gem::hw::vfat::VFAT2Manager*>(p_gemFSMApp)->m_vfat2monitors[i];
    if (card) {
      *out << "\"vfat" << std::setw(2) << std::setfill('0') << (i+1) << "\"  : { \n";
      card->jsonUpdateItemSets(out);
      *out << " }, \n";
    }
  }
  *out << " } \n";
}
