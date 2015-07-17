// GLIBManagerWeb.cc

#include "gem/hw/glib/GLIBManagerWeb.h"
#include "gem/hw/glib/GLIBManager.h"

#include "gem/hw/glib/exception/Exception.h"

#include "xcept/tools.h"

gem::hw::glib::GLIBManagerWeb::GLIBManagerWeb(gem::hw::glib::GLIBManager* glibApp) :
  gem::base::GEMWebApplication(glibApp)
{
  
}

gem::hw::glib::GLIBManagerWeb::~GLIBManagerWeb()
{
  //default destructor
}

/*To be filled in with the monitor page code*/
void gem::hw::glib::GLIBManagerWeb::monitorPage(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  INFO("monitorPage");
  //fill this page with the generic views for the GLIBManager
  //different tabs for certain functions
  *out << "monitorPage</br>" << std::endl;
}

/*To be filled in with the expert page code*/
void gem::hw::glib::GLIBManagerWeb::expertPage(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  INFO("expertPage");
  //fill this page with the expert views for the GLIBManager
  *out << "expertPage</br>" << std::endl;
}

/*To be filled in with the card page code*/
void gem::hw::glib::GLIBManagerWeb::cardPage(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  INFO("cardPage");
  //fill this page with the card views for the GLIBManager
  *out << "cardPage</br>" << std::endl;
}

