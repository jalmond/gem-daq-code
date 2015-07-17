#include "gem/hw/vfat/GEMController.h"
#include <typeinfo>
#include <sys/stat.h>
#include <string>

//#include <boost/lexical_cast.hpp>
//#include <boost/format.hpp>

XDAQ_INSTANTIATOR_IMPL(gem::hw::vfat::GEMController)

gem::hw::vfat::GEMController::GEMController(xdaq::ApplicationStub * s)
throw (xdaq::exception::Exception):
xdaq::WebApplication(s),
  gemLogger_(this->getApplicationLogger())
{
  xgi::framework::deferredbind(this, this, &GEMController::Default,       "Default"       );
  xgi::framework::deferredbind(this, this, &GEMController::Dummy,         "Dummy"         );
  xgi::framework::deferredbind(this, this, &GEMController::ControlPanel,  "ControlPanel"  );
  xgi::framework::deferredbind(this, this, &GEMController::controlVFAT2,  "controlVFAT2"  );
  xgi::framework::deferredbind(this, this, &GEMController::CrateSelection,"CrateSelection"); 
  xgi::framework::deferredbind(this, this, &GEMController::setConfFile,   "setConfFile"   ); 
  xgi::framework::deferredbind(this, this, &GEMController::MCHStatus,     "MCHStatus"     ); 
  xgi::framework::deferredbind(this, this, &GEMController::MCHUtils,      "MCHUtils"      ); 
  xgi::framework::deferredbind(this, this, &GEMController::AMCStatus,     "AMCStatus"     ); 
  xgi::framework::deferredbind(this, this, &GEMController::AMCUtils,      "AMCUtils"      ); 
  xgi::framework::deferredbind(this, this, &GEMController::GLIBStatus,    "GLIBStatus"    ); 
  xgi::framework::deferredbind(this, this, &GEMController::GLIBUtils,     "GLIBUtils"     ); 
  xgi::framework::deferredbind(this, this, &GEMController::OHStatus,      "OHStatus"      ); 
  xgi::framework::deferredbind(this, this, &GEMController::OHUtils,       "OHUtils"       ); 
  xgi::framework::deferredbind(this, this, &GEMController::VFAT2Manager,  "VFAT2Manager"  ); 

  //device_ = "CMS_hybrid_J8";
  device_ = "VFAT9";
  settingsFile_ = "";

  // Detect when the setting of default parameters has been performed
  this->getApplicationInfoSpace()->addListener(this, "urn:xdaq-event:setDefaultValues");

  getApplicationInfoSpace()->fireItemAvailable("device", &device_);
  getApplicationInfoSpace()->fireItemAvailable("settingsFile", &settingsFile_);

  getApplicationInfoSpace()->fireItemValueRetrieve("device", &device_);
  getApplicationInfoSpace()->fireItemValueRetrieve("settingsFile", &settingsFile_);

}

gem::hw::vfat::GEMController::~GEMController()
{
  vfatDevice->releaseDevice();
}

///////////////////////////////////////////////
//
// Parse XML File
//
///////////////////////////////////////////////


void gem::hw::vfat::GEMController::parseXMLFile()
{
  LOG4CPLUS_INFO(this->getApplicationLogger(), "Parsing XML file: " << settingsFile_.toString());

  //
  /// Initialize XML4C system
  try{
    xercesc::XMLPlatformUtils::Initialize();
    LOG4CPLUS_INFO(this->getApplicationLogger(), "Successfully initialized XML4C system");
  }
  catch(const xercesc::XMLException& toCatch){
    std::cerr << "Error during Xerces-c Initialization.\n"
	      << "  Exception message:"
	      << xercesc::XMLString::transcode(toCatch.getMessage()) << std::endl;
    return ;
  }


  //  Create our parser, then attach an error handler to the parser.
  //  The parser will call back to methods of the ErrorHandler if it
  //  discovers errors during the course of parsing the XML document.
  //
  xercesc::XercesDOMParser *parser = new xercesc::XercesDOMParser;
  parser->setValidationScheme(xercesc::XercesDOMParser::Val_Auto);
  parser->setDoNamespaces(false);
  parser->setCreateEntityReferenceNodes(false);
  //parser->setToCreateXMLDeclTypeNode(true);


  //  Parse the XML file, catching any XML exceptions that might propogate
  //  out of it.
  //
  bool errorsOccured = false;
  try{
    parser->parse(settingsFile_.toString().c_str());
  }


  catch (const xercesc::XMLException& e){
    std::cerr << "An error occured during parsing\n   Message: "
	      << xercesc::XMLString::transcode(e.getMessage()) << std::endl;
    errorsOccured = true;
    fileError = "An error occured during parsing of selected file. Please select another configuration file.";
  }


  catch (const xercesc::DOMException& e){
    std::cerr << "An error occured during parsing\n   Message: "
	      << xercesc::XMLString::transcode(e.msg) << std::endl;
    errorsOccured = true;
    fileError = "An error occured during parsing of selected file. Please select another configuration file.";
  }

  catch (...){
    std::cerr << "An error occured during parsing" << std::endl;
    errorsOccured = true;
    fileError = "An error occured during parsing of selected file. Please select another configuration file.";
  }

  // If the parse was successful, output the document data from the DOM tree
  crateIds.clear();
  crateNodes.clear();

  if (!errorsOccured){
    xercesc::DOMNode * pDoc = parser->getDocument();
    xercesc::DOMNode * n = pDoc->getFirstChild();
    while (n) {
      if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) 
	{
	  if (strcmp("GEMSystem",xercesc::XMLString::transcode(n->getNodeName()))==0) {
	    parseGEMSystem(n);
	  }
	}
      n = n->getNextSibling();
    } 
  } 

  //delete parser;
  //xercesc::XMLPlatformUtils::Terminate();

  vfatDevice->getAllSettings();
  vfatParams_ = vfatDevice->getVFAT2Params();
}

void gem::hw::vfat::GEMController::parseGEMSystem(xercesc::DOMNode * pNode)
{
  LOG4CPLUS_INFO(this->getApplicationLogger(), "parseGEMSystem");
  xercesc::DOMNode * n = pNode->getFirstChild();
  while (n) {
    if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE)
      {    
	if (strcmp("uTCACrate",xercesc::XMLString::transcode(n->getNodeName()))==0) {
	  LOG4CPLUS_INFO(this->getApplicationLogger(),xercesc::XMLString::transcode(n->getAttributes()->getNamedItem(xercesc::XMLString::transcode("CrateID"))->getNodeValue()));
	  crateIds.push_back(xercesc::XMLString::transcode(n->getAttributes()->getNamedItem(xercesc::XMLString::transcode("CrateID"))->getNodeValue()));
	  crateNodes.push_back(n);
	  if (crateIds.size() == 1) {
	    currentCrate = 0;
	    currentCrateId = crateIds[0];
	  }
	  parseCrate(n);
	}
      }    
    n = n->getNextSibling();
  }
}

void gem::hw::vfat::GEMController::parseCrate(xercesc::DOMNode * pNode)
{
  LOG4CPLUS_INFO(this->getApplicationLogger(), "parseCrate");
  bool cur = false;
  if (xercesc::XMLString::transcode(pNode->getAttributes()->getNamedItem(xercesc::XMLString::transcode("CrateID"))->getNodeValue()) == currentCrateId) {
    cur = true;
  }
  if (cur) {
    MCHIds.clear();
    AMCIds.clear();
    GLIBIds.clear();
  }

  xercesc::DOMNode * n = pNode->getFirstChild();
  while (n) {
    if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE)
      {
	if (strcmp("MCH",xercesc::XMLString::transcode(n->getNodeName()))==0) {
	  LOG4CPLUS_INFO(this->getApplicationLogger(), "parseMCH");
	  if (cur) MCHIds.push_back(xercesc::XMLString::transcode(n->getAttributes()->getNamedItem(xercesc::XMLString::transcode("MCHId"))->getNodeValue()));
	} 
	if (strcmp("AMC",xercesc::XMLString::transcode(n->getNodeName()))==0) {
	  LOG4CPLUS_INFO(this->getApplicationLogger(), "parseAMC");
	  if (cur) AMCIds.push_back(xercesc::XMLString::transcode(n->getAttributes()->getNamedItem(xercesc::XMLString::transcode("AMCId"))->getNodeValue()));
	} 
	if (strcmp("GLIB",xercesc::XMLString::transcode(n->getNodeName()))==0) {
	  LOG4CPLUS_INFO(this->getApplicationLogger(), "parseGLIB");
	  GLIBIds.push_back(xercesc::XMLString::transcode(n->getAttributes()->getNamedItem(xercesc::XMLString::transcode("GLIBId"))->getNodeValue()));
	  if (cur) parseGLIB(n);
	} 
      }
    n = n->getNextSibling();
  }
}


void gem::hw::vfat::GEMController::parseGLIB(xercesc::DOMNode * pNode)
{
  LOG4CPLUS_INFO(this->getApplicationLogger(), "parseGLIB");
  OHIds.clear();
  xercesc::DOMNode * n = pNode->getFirstChild();
  while (n) {
    if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE)
      {    
	if (strcmp("OH",xercesc::XMLString::transcode(n->getNodeName()))==0) {
	  OHIds.push_back(xercesc::XMLString::transcode(n->getAttributes()->getNamedItem(xercesc::XMLString::transcode("OHId"))->getNodeValue()));
	  parseOH(n);
	}
      }    
    n = n->getNextSibling();
  }    
}

void gem::hw::vfat::GEMController::parseOH(xercesc::DOMNode * pNode)
{
  LOG4CPLUS_INFO(this->getApplicationLogger(), "parseOH");
  VFAT2Ids.clear();
  xercesc::DOMNode * n = pNode->getFirstChild();
  while (n) {
    if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE)
      {    
	if (strcmp("VFATSettings",xercesc::XMLString::transcode(n->getNodeName()))==0) parseVFAT2Settings(n);
      }    
    n = n->getNextSibling();
  }    
}

void gem::hw::vfat::GEMController::parseVFAT2Settings(xercesc::DOMNode * pNode)
{
  LOG4CPLUS_INFO(this->getApplicationLogger(), "parseVFAT2Settings");
  xercesc::DOMNode * n = pNode->getFirstChild();
  while (n) {
    if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE)
      {
	if (strcmp("ChipID",xercesc::XMLString::transcode(n->getNodeName()))==0) VFAT2Ids.push_back(xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue()));
	if (strcmp("ControlRegisters",xercesc::XMLString::transcode(n->getNodeName()))==0) parseControlRegisters(n);
	if (strcmp("BiasSettings",xercesc::XMLString::transcode(n->getNodeName()))==0) parseBiasSettings(n);
      }
    n = n->getNextSibling();
  }
}

void gem::hw::vfat::GEMController::parseControlRegisters(xercesc::DOMNode * pNode)
{
  LOG4CPLUS_INFO(this->getApplicationLogger(), "parseControlRegisters");
  xercesc::DOMNode * n = pNode->getFirstChild();
  while (n) {
    if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE)
      {   
	if (strcmp("ControlRegister0",xercesc::XMLString::transcode(n->getNodeName()))==0) parseControlRegister0(n);
	if (strcmp("ControlRegister1",xercesc::XMLString::transcode(n->getNodeName()))==0) parseControlRegister1(n);
	if (strcmp("ControlRegister2",xercesc::XMLString::transcode(n->getNodeName()))==0) parseControlRegister2(n);
	if (strcmp("ControlRegister3",xercesc::XMLString::transcode(n->getNodeName()))==0) parseControlRegister3(n);
      }   
    n = n->getNextSibling();
  }
}

void gem::hw::vfat::GEMController::parseControlRegister0(xercesc::DOMNode * pNode)
{
  LOG4CPLUS_INFO(this->getApplicationLogger(), "parseControlRegister0");
  xercesc::DOMNode * n = pNode->getFirstChild();
  while (n) {
    if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE)
      {    
	if (strcmp("CalMode",xercesc::XMLString::transcode(n->getNodeName()))==0)
	  {
	    LOG4CPLUS_INFO(this->getApplicationLogger(), "CalMode: " << xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue()));
	    vfatDevice->setCalibrationMode((gem::hw::vfat::StringToCalibrationMode.at(boost::to_upper_copy((std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())))));
	  }
	if (strcmp("CalPolarity",xercesc::XMLString::transcode(n->getNodeName()))==0)
	  {
	    LOG4CPLUS_INFO(this->getApplicationLogger(), "CalPolarity: " << xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue()));
	    vfatDevice->setCalPolarity((gem::hw::vfat::StringToCalPolarity.at(boost::to_upper_copy((std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())))));
	  }
	if (strcmp("MSPolarity",xercesc::XMLString::transcode(n->getNodeName()))==0)
	  {
	    LOG4CPLUS_INFO(this->getApplicationLogger(), "MSPolarity: " << xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue()));
	    vfatDevice->setMSPolarity((gem::hw::vfat::StringToMSPolarity.at(boost::to_upper_copy((std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())))));
	  }
	if (strcmp("TriggerMode",xercesc::XMLString::transcode(n->getNodeName()))==0)
	  {
	    LOG4CPLUS_INFO(this->getApplicationLogger(), "TriggerMode: " << xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue()));
	    vfatDevice->setTriggerMode((gem::hw::vfat::StringToTriggerMode.at(boost::to_upper_copy((std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())))));
	  }
	if (strcmp("RunMode",xercesc::XMLString::transcode(n->getNodeName()))==0)
	  {
	    LOG4CPLUS_INFO(this->getApplicationLogger(), "RunMode: " << xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue()));
	    vfatDevice->setRunMode((gem::hw::vfat::StringToRunMode.at(boost::to_upper_copy((std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())))));
	  }
      }    
    n = n->getNextSibling();
  }
}

void gem::hw::vfat::GEMController::parseControlRegister1(xercesc::DOMNode * pNode)
{
  LOG4CPLUS_INFO(this->getApplicationLogger(), "parseControlRegister1");
  xercesc::DOMNode * n = pNode->getFirstChild();
  while (n) {
    if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE)
      {    
	if (strcmp("ReHitCT",xercesc::XMLString::transcode(n->getNodeName()))==0)
	  {
	    LOG4CPLUS_INFO(this->getApplicationLogger(), "ReHitCT: " << xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue()));
	    vfatDevice->setHitCountCycleTime((gem::hw::vfat::StringToReHitCT.at(boost::to_upper_copy((std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())))));
	  }
	if (strcmp("LVDSPowerSave",xercesc::XMLString::transcode(n->getNodeName()))==0)
	  {
	    LOG4CPLUS_INFO(this->getApplicationLogger(), "LVDSPowerSave: " << xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue()));
	    vfatDevice->setLVDSMode((gem::hw::vfat::StringToLVDSPowerSave.at(boost::to_upper_copy((std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())))));
	  }
	if (strcmp("ProbeMode",xercesc::XMLString::transcode(n->getNodeName()))==0)
	  {
	    LOG4CPLUS_INFO(this->getApplicationLogger(), "ProbeMode: " << xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue()));
	    vfatDevice->setProbeMode((gem::hw::vfat::StringToProbeMode.at(boost::to_upper_copy((std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())))));
	  }
	if (strcmp("DACSel",xercesc::XMLString::transcode(n->getNodeName()))==0)
	  {
	    LOG4CPLUS_INFO(this->getApplicationLogger(), "DACSel: " << xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue()));
	    vfatDevice->setDACMode((gem::hw::vfat::StringToDACMode.at(boost::to_upper_copy((std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())))));
	  }
      }    
    n = n->getNextSibling();
  }
}

void gem::hw::vfat::GEMController::parseControlRegister2(xercesc::DOMNode * pNode)
{
  LOG4CPLUS_INFO(this->getApplicationLogger(), "parseControlRegister2");
  xercesc::DOMNode * n = pNode->getFirstChild();
  while (n) {
    if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE)
      {    
	if (strcmp("DigInSel",xercesc::XMLString::transcode(n->getNodeName()))==0)
	  {
	    LOG4CPLUS_INFO(this->getApplicationLogger(), "DigInSel: " << xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue()));
	    vfatDevice->setInputPadMode((gem::hw::vfat::StringToDigInSel.at(boost::to_upper_copy((std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())))));
	  }
	if (strcmp("MSPulseLength",xercesc::XMLString::transcode(n->getNodeName()))==0)
	  {
	    LOG4CPLUS_INFO(this->getApplicationLogger(), "MSPulseLength: " << xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue()));
	    vfatDevice->setMSPulseLength((gem::hw::vfat::StringToMSPulseLength.at(boost::to_upper_copy((std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())))));
	  }
	if (strcmp("HitCountSel",xercesc::XMLString::transcode(n->getNodeName()))==0)
	  {
	    LOG4CPLUS_INFO(this->getApplicationLogger(), "HitCountSel: " << xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue()));
	    vfatDevice->setHitCountMode((gem::hw::vfat::StringToHitCountMode.at(boost::to_upper_copy((std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())))));
	  }
      }    
    n = n->getNextSibling();
  }
}

void gem::hw::vfat::GEMController::parseControlRegister3(xercesc::DOMNode * pNode)
{
  LOG4CPLUS_INFO(this->getApplicationLogger(), "parseControlRegister3");
  xercesc::DOMNode * n = pNode->getFirstChild();
  while (n) {
    if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE)
      {    
	if (strcmp("DFTest",xercesc::XMLString::transcode(n->getNodeName()))==0)
	  {
	    LOG4CPLUS_INFO(this->getApplicationLogger(), "DFTest: " << xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue()));
	    vfatDevice->sendTestPattern((gem::hw::vfat::StringToDFTestPattern.at(boost::to_upper_copy((std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())))));
	  }
	if (strcmp("PbBG",xercesc::XMLString::transcode(n->getNodeName()))==0)
	  {
	    LOG4CPLUS_INFO(this->getApplicationLogger(), "PbBG: " << xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue()));
	    vfatDevice->setBandgapPad((gem::hw::vfat::StringToPbBG.at(boost::to_upper_copy((std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())))));
	  }
	if (strcmp("TrimDACRange",xercesc::XMLString::transcode(n->getNodeName()))==0)
	  {
	    LOG4CPLUS_INFO(this->getApplicationLogger(), "TrimDACRange: " << xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue()));
	    vfatDevice->setTrimDACRange((gem::hw::vfat::StringToTrimDACRange.at(boost::to_upper_copy((std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())))));
	  }
      }    
    n = n->getNextSibling();
  }
}

void gem::hw::vfat::GEMController::parseBiasSettings(xercesc::DOMNode * pNode)
{
  LOG4CPLUS_INFO(this->getApplicationLogger(), "parseBiasSettings");
  xercesc::DOMNode * n = pNode->getFirstChild();
  while (n) {
    if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE)
      {    
	if (strcmp("CurrentBias",xercesc::XMLString::transcode(n->getNodeName()))==0) parseCurrentBias(n);
	if (strcmp("Latency",xercesc::XMLString::transcode(n->getNodeName()))==0) vfatDevice->writeVFATReg("Latency",atoi(xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())));
	if (strcmp("VCal",xercesc::XMLString::transcode(n->getNodeName()))==0) vfatDevice->writeVFATReg("VCal",atoi(xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())));
	if (strcmp("VThreshold1",xercesc::XMLString::transcode(n->getNodeName()))==0) vfatDevice->writeVFATReg("VThreshold1",atoi(xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())));
	if (strcmp("VThreshold2",xercesc::XMLString::transcode(n->getNodeName()))==0) vfatDevice->writeVFATReg("VThreshold2",atoi(xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())));
	if (strcmp("CalPhase",xercesc::XMLString::transcode(n->getNodeName()))==0) vfatDevice->writeVFATReg("CalPhase",atoi(xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())));
      }    
    n = n->getNextSibling();
  }
}

void gem::hw::vfat::GEMController::parseCurrentBias(xercesc::DOMNode * pNode)
{
  LOG4CPLUS_INFO(this->getApplicationLogger(), "parseCurrentBias");
  xercesc::DOMNode * n = pNode->getFirstChild();
  while (n) {
    if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE)
      {    
	if (strcmp("IPreampIn",xercesc::XMLString::transcode(n->getNodeName()))==0) vfatDevice->writeVFATReg("IPreampIn",atoi(xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())));
	if (strcmp("IPreampFeed",xercesc::XMLString::transcode(n->getNodeName()))==0) vfatDevice->writeVFATReg("IPreampFeed",atoi(xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())));
	if (strcmp("IPreampOut",xercesc::XMLString::transcode(n->getNodeName()))==0) vfatDevice->writeVFATReg("IPreampOut",atoi(xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())));
	if (strcmp("IShaper",xercesc::XMLString::transcode(n->getNodeName()))==0) vfatDevice->writeVFATReg("IShaper",atoi(xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())));
	if (strcmp("IShaperFeed",xercesc::XMLString::transcode(n->getNodeName()))==0) vfatDevice->writeVFATReg("IShaperFeed",atoi(xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())));
	if (strcmp("IComp",xercesc::XMLString::transcode(n->getNodeName()))==0) vfatDevice->writeVFATReg("IComp",atoi(xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())));
      }    
    n = n->getNextSibling();
  }
}

///////////////////////////////////////////////
//
//  Set Conf File
//
///////////////////////////////////////////////

void gem::hw::vfat::GEMController::setConfFile(xgi::Input * in, xgi::Output * out )
  throw (xgi::exception::Exception)
{
  cgicc::Cgicc cgi(in);
  std::string oldFile = settingsFile_;
  std::string newFile = cgi.getElement("xmlFilename")->getValue();
  struct stat buffer;
  if (stat(newFile.c_str(), &buffer) == 0) {
    settingsFile_ = newFile;
    //parseXMLFile();
    if (fileError.size()) fileWarning = fileError;
    else {
      if (crateIds.size() == 0) {
	fileWarning = "Selected file doesn't have any crate information. Please select another configuration file.";
	settingsFile_ = oldFile;
      }
      else {
	fileWarning = "";
	settingsFile_ = newFile;
      }
    }
  }
  else {
    fileWarning = "Selected file doesn't exist. Please select another configuration file.";
  }
  this->ControlPanel(in,out);
}

///////////////////////////////////////////////
//
// Crate Selection
//
///////////////////////////////////////////////

void gem::hw::vfat::GEMController::CrateSelection(xgi::Input * in, xgi::Output * out )
  throw (xgi::exception::Exception)
{
  cgicc::Cgicc cgi(in);

  std::string value = cgi.getElement("chosenCrateId")->getValue();
  LOG4CPLUS_INFO(this->getApplicationLogger(),"select crate: " << value << std::endl);
  if(!value.empty())
    {
      //int k=in_value.find(" ",0);
      //std::string value = (k) ? in_value.substr(0,k):in_value;
      currentCrateId = value;
      for(unsigned i=0; i< crateIds.size(); i++)
        {
	  if(value==crateIds[i]) {
	    currentCrate=i;
	    parseCrate(crateNodes[i]);
	  }
        }
    }
  this->ControlPanel(in,out);
}

///////////////////////////////////////////////
//
// MCH Status
//
///////////////////////////////////////////////

void gem::hw::vfat::GEMController::MCHStatus(xgi::Input * in, xgi::Output * out )
  throw (xgi::exception::Exception)
{
  std::string ControlPanel = toolbox::toString("/%s/ControlPanel",getApplicationDescriptor()->getURN().c_str());
  *out << cgicc::a("[Go Back]").set("href",ControlPanel) << std::endl;
  *out << cgicc::br();
  *out << cgicc::h1("MCH Status")<< std::endl;
}

///////////////////////////////////////////////
//
// MCH Utils
//
///////////////////////////////////////////////

void gem::hw::vfat::GEMController::MCHUtils(xgi::Input * in, xgi::Output * out )
  throw (xgi::exception::Exception)
{
  std::string ControlPanel = toolbox::toString("/%s/ControlPanel",getApplicationDescriptor()->getURN().c_str());
  *out << cgicc::a("[Go Back]").set("href",ControlPanel) << std::endl;
  *out << cgicc::br();
  *out << cgicc::h1("MCH Utils")<< std::endl;

}

///////////////////////////////////////////////
//
// AMC Status
//
///////////////////////////////////////////////

void gem::hw::vfat::GEMController::AMCStatus(xgi::Input * in, xgi::Output * out )
  throw (xgi::exception::Exception)
{
  std::string ControlPanel = toolbox::toString("/%s/ControlPanel",getApplicationDescriptor()->getURN().c_str());
  *out << cgicc::a("[Go Back]").set("href",ControlPanel) << std::endl;
  *out << cgicc::br();
  *out << cgicc::h1("AMC13 Status")<< std::endl;
}

///////////////////////////////////////////////
//
// AMC Utils
//
///////////////////////////////////////////////

void gem::hw::vfat::GEMController::AMCUtils(xgi::Input * in, xgi::Output * out )
  throw (xgi::exception::Exception)
{
  std::string ControlPanel = toolbox::toString("/%s/ControlPanel",getApplicationDescriptor()->getURN().c_str());
  *out << cgicc::a("[Go Back]").set("href",ControlPanel) << std::endl;
  *out << cgicc::br();
  *out << cgicc::h1("AMC13 Utils")<< std::endl;
}

///////////////////////////////////////////////
//
// GLIB Status
//
///////////////////////////////////////////////

void gem::hw::vfat::GEMController::GLIBStatus(xgi::Input * in, xgi::Output * out )
  throw (xgi::exception::Exception)
{
  std::string ControlPanel = toolbox::toString("/%s/ControlPanel",getApplicationDescriptor()->getURN().c_str());
  *out << cgicc::a("[Go Back]").set("href",ControlPanel) << std::endl;
  *out << cgicc::br();
  *out << cgicc::h1("GLIB Status")<< std::endl;

  *out << cgicc::h2("OH Boards");
  if (OHIds.size()) {
    for (unsigned int i = 0; i < OHIds.size(); i++) {
      *out << "OH Board: " << OHIds[i] << cgicc::br();

      *out << cgicc::table().set("border","0");

      *out << cgicc::td();
      std::string f1 = toolbox::toString("/%s/OHStatus",getApplicationDescriptor()->getURN().c_str());
      *out << cgicc::form().set("method","GET").set("action",f1) << std::endl ;
      *out << cgicc::input().set("type","submit").set("value","OH Status") << std::endl ;
      *out << cgicc::form();
      *out << cgicc::td();

      *out << cgicc::td();
      std::string f2 = toolbox::toString("/%s/Dummy",getApplicationDescriptor()->getURN().c_str());
      *out << cgicc::form().set("method","GET").set("action",f2) << std::endl ;
      *out << cgicc::input().set("type","submit").set("value","OH Tests") << std::endl ;
      *out << cgicc::form();
      *out << cgicc::td();

      *out << cgicc::td();
      std::string f3 = toolbox::toString("/%s/OHUtils",getApplicationDescriptor()->getURN().c_str());
      *out << cgicc::form().set("method","GET").set("action",f3) << std::endl ;
      *out << cgicc::input().set("type","submit").set("value","OH Utils") << std::endl ;
      *out << cgicc::form();
      *out << cgicc::td();

      *out << cgicc::table();
    }
  }
  else *out << "There are no OH boards" << cgicc::br();
  *out << cgicc::br();
}

///////////////////////////////////////////////
//
// GLIB Utils
//
///////////////////////////////////////////////

void gem::hw::vfat::GEMController::GLIBUtils(xgi::Input * in, xgi::Output * out )
  throw (xgi::exception::Exception)
{
  std::string ControlPanel = toolbox::toString("/%s/ControlPanel",getApplicationDescriptor()->getURN().c_str());
  *out << cgicc::a("[Go Back]").set("href",ControlPanel) << std::endl;
  *out << cgicc::br();
  *out << cgicc::h1("GLIB Utils")<< std::endl;
}

///////////////////////////////////////////////
//
// OH Status
//
///////////////////////////////////////////////

void gem::hw::vfat::GEMController::OHStatus(xgi::Input * in, xgi::Output * out )
  throw (xgi::exception::Exception)
{
  std::string ControlPanel = toolbox::toString("/%s/GLIBStatus",getApplicationDescriptor()->getURN().c_str());
  *out << cgicc::a("[Go Back]").set("href",ControlPanel) << std::endl;
  *out << cgicc::br();
  *out << cgicc::h1("OH Status")<< std::endl;

  *out << cgicc::h2("VFAT2 Chips");
  if (VFAT2Ids.size()) {
    for (unsigned int i = 0; i < VFAT2Ids.size(); i++) {
      *out << "VFAT2 Chip: " << VFAT2Ids[i] << cgicc::br();

      *out << cgicc::table().set("border","0");

      *out << cgicc::td();
      std::string f1 = toolbox::toString("/%s/VFAT2Manager",getApplicationDescriptor()->getURN().c_str());
      *out << cgicc::form().set("method","GET").set("action",f1) << std::endl ;
      *out << cgicc::input().set("type","submit").set("value","VFAT2Manager") << std::endl ;
      *out << cgicc::form();
      *out << cgicc::td();

      *out << cgicc::td();
      std::string f2 = toolbox::toString("/%s/Dummy",getApplicationDescriptor()->getURN().c_str());
      *out << cgicc::form().set("method","GET").set("action",f2) << std::endl ;
      *out << cgicc::input().set("type","submit").set("value","VFAT2 Tests") << std::endl ;
      *out << cgicc::form();
      *out << cgicc::td();

      *out << cgicc::td();
      std::string f3 = toolbox::toString("/%s/OHUtils",getApplicationDescriptor()->getURN().c_str());
      *out << cgicc::form().set("method","GET").set("action",f3) << std::endl ;
      *out << cgicc::input().set("type","submit").set("value","VFAT2 Utils") << std::endl ;
      *out << cgicc::form();
      *out << cgicc::td();

      *out << cgicc::table();
    }
  }
  else *out << "There are no VFAT2 Chips" << cgicc::br();
  *out << cgicc::br();
}

///////////////////////////////////////////////
//
// OH Utils
//
///////////////////////////////////////////////

void gem::hw::vfat::GEMController::OHUtils(xgi::Input * in, xgi::Output * out )
  throw (xgi::exception::Exception)
{
  std::string ControlPanel = toolbox::toString("/%s/ControlPanel",getApplicationDescriptor()->getURN().c_str());
  *out << cgicc::a("[Go Back]").set("href",ControlPanel) << std::endl;
  *out << cgicc::br();
  *out << cgicc::h1("OH Utils")<< std::endl;
}

///////////////////////////////////////////////
//
// VFAT2 Manager
//
///////////////////////////////////////////////

void gem::hw::vfat::GEMController::VFAT2Manager(xgi::Input * in, xgi::Output * out )
  throw (xgi::exception::Exception)
{
  std::string ControlPanel = toolbox::toString("/%s/OHStatus",getApplicationDescriptor()->getURN().c_str());
  *out << cgicc::a("[Go Back]").set("href",ControlPanel) << std::endl;
  *out << cgicc::br();
  *out << cgicc::h1("VFAT2 Manager")<< std::endl;

  try {
    *out << " <link rel=\"stylesheet\" href=\"/gemdaq/gemhardware/html/css/vfat/vfatmanager.css\"/>" << std::endl
	 << " <link rel=\"stylesheet\" href=\"/gemdaq/gemhardware/html/css/vfat/vfatcontrolregisters.css\"/>" << std::endl
	 << " <link rel=\"stylesheet\" href=\"/gemdaq/gemhardware/html/css/vfat/vfatglobalsettings.css\"/>" << std::endl
	 << " <link rel=\"stylesheet\" href=\"/gemdaq/gemhardware/html/css/vfat/vfatchannelregister.css\"/>" << std::endl
	 << " <link rel=\"stylesheet\" href=\"/gemdaq/gemhardware/html/css/vfat/vfatcommands.css\"/>" << std::endl;
    LOG4CPLUS_INFO(this->getApplicationLogger(),"building the control panel vfatParams_:" << vfatParams_);
    LOG4CPLUS_INFO(this->getApplicationLogger(),"vfatDevice->getVFAT2Params():" << vfatDevice->getVFAT2Params());
    std::string method = toolbox::toString("/%s/controlVFAT2",getApplicationDescriptor()->getURN().c_str());
    *out << cgicc::form().set("method","POST")
      .set("action",method)
	 << std::endl;

    *out << cgicc::section().set("style","display:inline-block;float:left") << std::endl
	 << cgicc::fieldset().set("class","vfatParameters") << std::endl
	 << cgicc::legend("VFAT2 Parameters") << std::endl
	 << cgicc::span().set("style","display:block;float:left") << std::endl;
    LOG4CPLUS_INFO(this->getApplicationLogger(),"building the VFATInfoLayout");
    gem::hw::vfat::GEMController::GEMControllerPanelWeb::createVFATInfoLayout(out, vfatParams_);
    *out << cgicc::span() << std::endl;
    *out << std::endl;
    *out << cgicc::span().set("style","display:block;float:right") << std::endl;
    LOG4CPLUS_INFO(this->getApplicationLogger(),"building the CounterLayout");
    gem::hw::vfat::GEMController::GEMControllerPanelWeb::createCounterLayout(out, vfatParams_);
    *out << cgicc::span() << std::endl;
    *out << cgicc::comment() << "ending the VFAT2 Parameters fieldset" << cgicc::comment() << std::endl
	 << cgicc::fieldset() << std::endl;
    *out << std::endl;
    *out << " <section class=\"vfatSettings\">" << std::endl;
    *out << " <fieldset class=\"vfatGlobalSettings\">" << std::endl
	 << cgicc::legend("Global settings") << std::endl;
    LOG4CPLUS_INFO(this->getApplicationLogger(),"building the ControlRegisterLayout");
    gem::hw::vfat::GEMController::GEMControllerPanelWeb::createControlRegisterLayout(out, vfatParams_);
    *out << cgicc::br() << std::endl
	 << std::endl;
    LOG4CPLUS_INFO(this->getApplicationLogger(),"building the SettingsLayout");
    gem::hw::vfat::GEMController::GEMControllerPanelWeb::createSettingsLayout(out, vfatParams_);
    *out << cgicc::comment() << "ending the Global settings fieldset" << cgicc::comment() << std::endl
	 << "	</fieldset>" << std::endl;
    LOG4CPLUS_INFO(this->getApplicationLogger(),"building the ChannelRegisterLayout");
    gem::hw::vfat::GEMController::GEMControllerPanelWeb::createChannelRegisterLayout(out, vfatParams_);
    *out << cgicc::comment() << "ending the vfatSettings section" << cgicc::comment() << std::endl
	 << " </section>" << std::endl
	 << cgicc::comment() << "ending the left side section" << cgicc::comment() << std::endl
	 << cgicc::section() << std::endl;
    LOG4CPLUS_INFO(this->getApplicationLogger(),"building the CommandLayout");
    gem::hw::vfat::GEMController::GEMControllerPanelWeb::createCommandLayout(out, vfatParams_);
    *out << cgicc::form() << cgicc::br() << std::endl;
    *out << cgicc::script().set("type","text/javascript")
      .set("src","http://ajax.googleapis.com/ajax/libs/jquery/1/jquery.min.js")
	 << cgicc::script() << std::endl;
    *out << cgicc::script().set("type","text/javascript")
      .set("src","http://ajax.googleapis.com/ajax/libs/jqueryui/1/jquery-ui.min.js")
	 << cgicc::script() << std::endl;
    *out << cgicc::script().set("type","text/javascript")
      .set("src","/gemdaq/gemhardware/html/scripts/toggleVFATCheckboxes.js")
	 << cgicc::script() << cgicc::br()
	 << std::endl;
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

///////////////////////////////////////////////
//
// Default 
//
///////////////////////////////////////////////

void gem::hw::vfat::GEMController::Default(xgi::Input * in, xgi::Output * out )
  throw (xgi::exception::Exception)
{
  //parseXMLFile();

  this->ControlPanel(in,out);
}

///////////////////////////////////////////////
//
// Dummy
//
///////////////////////////////////////////////

void gem::hw::vfat::GEMController::Dummy(xgi::Input * in, xgi::Output * out )
  throw (xgi::exception::Exception)
{
  this->ControlPanel(in,out);
}

///////////////////////////////////////////////
//
// Control Panel
//
///////////////////////////////////////////////

void gem::hw::vfat::GEMController::ControlPanel(xgi::Input * in, xgi::Output * out )
  throw (xgi::exception::Exception)
{

  try {

    //need to grab the page header and modify it here somehow...
    //gem::hw::vfat::GEMController::GEMControllerPanelWeb::createHeader(out);
    //*out << "  <body>" << std::endl;
    //cgicc::HTTPResponseHeader& head = out->getHTTPResponseHeader();
    //head->addHeader();

    //head = out["head"]
    *out << "    <link rel=\"stylesheet\" href=\"/gemdaq/gemhardware/html/css/vfat/vfatmanager.css\"/>"          << std::endl
	 << "    <link rel=\"stylesheet\" href=\"/gemdaq/gemhardware/html/css/vfat/vfatcontrolregisters.css\"/>" << std::endl
	 << "    <link rel=\"stylesheet\" href=\"/gemdaq/gemhardware/html/css/vfat/vfatglobalsettings.css\"/>"   << std::endl
	 << "    <link rel=\"stylesheet\" href=\"/gemdaq/gemhardware/html/css/vfat/vfatchannelregister.css\"/>"  << std::endl
	 << "    <link rel=\"stylesheet\" href=\"/gemdaq/gemhardware/html/css/vfat/vfatcommands.css\"/>"         << std::endl;

    ///////////////////////////////////////////////
    //
    // GEM System Configuration
    //
    ///////////////////////////////////////////////

    *out << cgicc::h1("GEM System Configuration")<< std::endl;
    //
    *out << cgicc::span().set("style","color:blue");
    *out << cgicc::b(cgicc::i("Current configuration file: ")) ;
    *out << settingsFile_.toString() << cgicc::span() << std::endl ;
    if (fileWarning.size()) {
      *out << cgicc::br()<< std::endl;
      *out << cgicc::span().set("style","color:red");
      *out << cgicc::b(cgicc::i(fileWarning)) << cgicc::span() << std::endl ;
    }
    *out << cgicc::br()<< std::endl;

    std::string metho = toolbox::toString("/%s/setConfFile",getApplicationDescriptor()->getURN().c_str());
    *out << cgicc::form().set("method","POST").set("action",metho) << std::endl ;
    *out << cgicc::input().set("type","text").set("name","xmlFilename").set("size","80").set("ENCTYPE","multipart/form-data").set("value",settingsFile_.toString()) << std::endl;
    *out << cgicc::input().set("type","submit").set("value","Set configuration file") << std::endl ;
    *out << cgicc::form() << std::endl ;

    std::string methodUpload = toolbox::toString("/%s/UploadConfFile",getApplicationDescriptor()->getURN().c_str());
    *out << cgicc::form().set("method","POST").set("enctype","multipart/form-data").set("action",methodUpload) << std::endl ;
    *out << cgicc::input().set("type","file").set("name","xmlFilenameUpload").set("size","80") << std::endl;
    *out << cgicc::input().set("type","submit").set("value","Submit") << std::endl ;
    *out << cgicc::form() << std::endl ;

    *out << cgicc::hr()<< std::endl;
    *out << cgicc::br();

    ///////////////////////////////////////////////
    //
    // System Utilities
    //
    ///////////////////////////////////////////////

    *out << cgicc::h1("System Utilities")<< std::endl;

    *out << cgicc::table().set("border","0");

    *out << cgicc::td();
    std::string b11 = toolbox::toString("/%s/Dummy",getApplicationDescriptor()->getURN().c_str());
    *out << cgicc::form().set("method","GET").set("action",b11) << std::endl ;
    *out << cgicc::input().set("type","submit").set("value","Crates Power-up Init") << std::endl ;
    *out << cgicc::form();
    *out << cgicc::td();

    *out << cgicc::td();
    std::string b12 = toolbox::toString("/%s/Dummy",getApplicationDescriptor()->getURN().c_str());
    *out << cgicc::form().set("method","GET").set("action",b12) << std::endl ;
    *out << cgicc::input().set("type","submit").set("value","Configuration Validation") << std::endl ;
    *out << cgicc::form();
    *out << cgicc::td();

    *out << cgicc::td();
    std::string b13 = toolbox::toString("/%s/Dummy",getApplicationDescriptor()->getURN().c_str());
    *out << cgicc::form().set("method","GET").set("action",b13) << std::endl ;
    *out << cgicc::input().set("type","submit").set("value","Firmware Manager") << std::endl ;
    *out << cgicc::form();
    *out << cgicc::td();

    *out << cgicc::table();
    *out << cgicc::br();
    *out << cgicc::hr()<< std::endl;
    *out << cgicc::br();

    ///////////////////////////////////////////////
    //
    // Crate Utilities
    //
    ///////////////////////////////////////////////

    *out << cgicc::h1("Crate Utilities")<< std::endl;
    //
    *out << cgicc::span().set("style","color:blue");
    *out << cgicc::b(cgicc::i("Current crate : ")) ;
    *out << currentCrateId << cgicc::span() << std::endl ;
    //
    *out << cgicc::br();

    // Begin select crate
    // Config listbox
    *out << cgicc::form().set("action",
			      "/" + getApplicationDescriptor()->getURN() + "/CrateSelection") << std::endl;

    int n_keys = crateIds.size();

    *out << "Choose crate: " << std::endl;
    *out << cgicc::select().set("name", "chosenCrateId") << std::endl;

    int selected_index = currentCrate;
    std::string CrateName;
    for (int i = 0; i < n_keys; ++i) {
      CrateName = crateIds[i];
      if (i == selected_index) {
	*out << cgicc::option()
	  .set("value", CrateName)
	  .set("selected", "");
      } else {
	*out << cgicc::option()
	  .set("value", CrateName);
      }
      *out << CrateName << cgicc::option() << std::endl;
    }

    *out << cgicc::select() << std::endl;

    *out << cgicc::input().set("type", "submit")
      .set("name", "command")
      .set("value", "Select crate") << std::endl;
    *out << cgicc::form() << std::endl;
    //End select crate

    *out << cgicc::br();
    *out << cgicc::table().set("border","0");

    *out << cgicc::td();
    std::string b21 = toolbox::toString("/%s/Dummy",getApplicationDescriptor()->getURN().c_str());
    *out << cgicc::form().set("method","GET").set("action",b21) << std::endl ;
    *out << cgicc::input().set("type","submit").set("value","Check availability of crates") << std::endl ;
    *out << cgicc::form();
    *out << cgicc::td();

    *out << cgicc::td();
    std::string b22 = toolbox::toString("/%s/Dummy",getApplicationDescriptor()->getURN().c_str());
    *out << cgicc::form().set("method","GET").set("action",b22) << std::endl ;
    *out << cgicc::input().set("type","submit").set("value","Check availability of selected crate") << std::endl ;
    *out << cgicc::form();
    *out << cgicc::td();

    *out << cgicc::td();
    std::string b23 = toolbox::toString("/%s/Dummy",getApplicationDescriptor()->getURN().c_str());
    *out << cgicc::form().set("method","GET").set("action",b23) << std::endl ;
    *out << cgicc::input().set("type","submit").set("value","Crate Tests") << std::endl ;
    *out << cgicc::form();
    *out << cgicc::td();

    *out << cgicc::table();
    *out << cgicc::br();
    *out << cgicc::hr()<< std::endl;
    *out << cgicc::br();

    ///////////////////////////////////////////////
    //
    // Crate Configuration
    //
    ///////////////////////////////////////////////

    *out << cgicc::h1("Current Crate Configuration");

    *out << cgicc::h2("MCH Boards");
    if (MCHIds.size()) {
      for (unsigned int i = 0; i < MCHIds.size(); i++) {
	*out << "MCH Board: " << MCHIds[i] << cgicc::br();

	*out << cgicc::table().set("border","0");

	*out << cgicc::td();
	std::string f1 = toolbox::toString("/%s/MCHStatus",getApplicationDescriptor()->getURN().c_str());
	*out << cgicc::form().set("method","GET").set("action",f1) << std::endl ;
	*out << cgicc::input().set("type","submit").set("value","MCH Status") << std::endl ;
	*out << cgicc::form();
	*out << cgicc::td();

	*out << cgicc::td();
	std::string f2 = toolbox::toString("/%s/Dummy",getApplicationDescriptor()->getURN().c_str());
	*out << cgicc::form().set("method","GET").set("action",f2) << std::endl ;
	*out << cgicc::input().set("type","submit").set("value","MCH Tests") << std::endl ;
	*out << cgicc::form();
	*out << cgicc::td();

	*out << cgicc::td();
	std::string f3 = toolbox::toString("/%s/MCHUtils",getApplicationDescriptor()->getURN().c_str());
	*out << cgicc::form().set("method","GET").set("action",f3) << std::endl ;
	*out << cgicc::input().set("type","submit").set("value","MCH Utils") << std::endl ;
	*out << cgicc::form();
	*out << cgicc::td();

	*out << cgicc::table();
      }
    }
    else *out << "There are no MCH boards" << cgicc::br();
    *out << cgicc::br();

    *out << cgicc::h2("AMC13 Boards");
    if (AMCIds.size()) {
      for (unsigned int i = 0; i < AMCIds.size(); i++) {
	*out << "AMC13 Board: " << AMCIds[i] << cgicc::br();

	*out << cgicc::table().set("border","0");

	*out << cgicc::td();
	std::string f1 = toolbox::toString("/%s/AMCStatus",getApplicationDescriptor()->getURN().c_str());
	*out << cgicc::form().set("method","GET").set("action",f1) << std::endl ;
	*out << cgicc::input().set("type","submit").set("value","AMC13 Status") << std::endl ;
	*out << cgicc::form();
	*out << cgicc::td();

	*out << cgicc::td();
	std::string f2 = toolbox::toString("/%s/Dummy",getApplicationDescriptor()->getURN().c_str());
	*out << cgicc::form().set("method","GET").set("action",f2) << std::endl ;
	*out << cgicc::input().set("type","submit").set("value","AMC13 Tests") << std::endl ;
	*out << cgicc::form();
	*out << cgicc::td();

	*out << cgicc::td();
	std::string f3 = toolbox::toString("/%s/AMCUtils",getApplicationDescriptor()->getURN().c_str());
	*out << cgicc::form().set("method","GET").set("action",f3) << std::endl ;
	*out << cgicc::input().set("type","submit").set("value","AMC13 Utils") << std::endl ;
	*out << cgicc::form();
	*out << cgicc::td();

	*out << cgicc::table();
      }
    }
    else *out << "There are no AMC13 boards" << cgicc::br();
    *out << cgicc::br();

    *out << cgicc::h2("GLIB Boards");
    if (GLIBIds.size()) {
      for (unsigned int i = 0; i < GLIBIds.size(); i++) {
	*out << "GLIB Board: " << GLIBIds[i] << cgicc::br();

	*out << cgicc::table().set("border","0");

	*out << cgicc::td();
	std::string f1 = toolbox::toString("/%s/GLIBStatus",getApplicationDescriptor()->getURN().c_str());
	*out << cgicc::form().set("method","GET").set("action",f1) << std::endl ;
	*out << cgicc::input().set("type","submit").set("value","GLIB Status") << std::endl ;
	*out << cgicc::form();
	*out << cgicc::td();

	*out << cgicc::td();
	std::string f2 = toolbox::toString("/%s/Dummy",getApplicationDescriptor()->getURN().c_str());
	*out << cgicc::form().set("method","GET").set("action",f2) << std::endl ;
	*out << cgicc::input().set("type","submit").set("value","GLIB Tests") << std::endl ;
	*out << cgicc::form();
	*out << cgicc::td();

	*out << cgicc::td();
	std::string f3 = toolbox::toString("/%s/GLIBUtils",getApplicationDescriptor()->getURN().c_str());
	*out << cgicc::form().set("method","GET").set("action",f3) << std::endl ;
	*out << cgicc::input().set("type","submit").set("value","GLIB Utils") << std::endl ;
	*out << cgicc::form();
	*out << cgicc::td();

	*out << cgicc::table();
      }
    }
    else *out << "There are no GLIB boards" << cgicc::br();
    *out << cgicc::br();

    std::string method = toolbox::toString("/%s/controlVFAT2",getApplicationDescriptor()->getURN().c_str());
    *out << cgicc::form().set("method","POST")
      .set("action",method)
	 << std::endl;
    *out << cgicc::script().set("type","text/javascript")
      .set("src","/gemdaq/gemhardware/html/scripts/toggleVFATCheckboxes.js")
	 << cgicc::script() << cgicc::br()
	 << std::endl;
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

void gem::hw::vfat::GEMController::controlVFAT2(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  try
    {
      cgicc::Cgicc cgi(in);
      std::vector<cgicc::FormEntry> myElements = cgi.getElements();

      // read a list of the checked checkboxes
      std::vector<std::pair<std::string,uint8_t> > regValsToSet;
      getCheckedRegisters(cgi, regValsToSet);

      //now process the form based on the button clicked
      performAction(cgi, regValsToSet);
      this->VFAT2Manager(in,out);
    }
  catch (const xgi::exception::Exception & e)
    {
      XCEPT_RAISE(xgi::exception::Exception, e.what());
    }
  catch (const std::exception & e)
    {
      XCEPT_RAISE(xgi::exception::Exception, e.what());
    }
}

void gem::hw::vfat::GEMController::getCheckedRegisters(cgicc::Cgicc cgi, std::vector<std::pair<std::string,uint8_t> > &regValsToSet)
  throw (xgi::exception::Exception)
{
  /****
   ***get a list of the following registers to set***

   *Address table register - checkbox name - corresponding form inputs to grab

   ContReg0 - CR0Set - CalMode:CalPolarity:MSPolarity:TriggerMode:RunMode
   ContReg1 - CR1Set - ReHitCT:LVDSPowerSave:ProbeMode:DACMode
   ContReg2 - CR2Set - DigInSel:MSPulseLength:HitCountMode
   ContReg3 - CR3Set - DFTest:PbBG:TrimDACRange

   IPreampIn   - SetIPreampIn    - IPreampIn   
   IPreampFeed - SetIPreampFeed  - IPreampFeed 
   IPreampOut  - SetIPreampOut   - IPreampOut  
   IShaper     - SetIShaper      - IShaper     
   IShpaerFeed - SetIShpaerFeed  - IShpaerFeed 
   IComp       - SetIComp        - IComp       

   Latency     - SetLatency      - Latency     
   VCal        - SetVCal         - VCal        
   VThreshold1 - SetVThreshold1  - VThreshold1 
   VThreshold2 - SetVThreshold2  - VThreshold2 

   CalPhase - SetCalPhase - CalPhase

   //if all selected, pass the params struct to the setter?

   ***Readonly***
   ChipID0
   ChipID1

   UpsetReg

   HitCount0
   HitCount1
   HitCount2
  **/

  //std::vector<std::pair<std::string,uint8_t> > regValsToSet;
  std::pair<std::string,uint8_t> regPair;
  LOG4CPLUS_INFO(this->getApplicationLogger(), "getCheckedRegisters::Getting list of checked registers and values to read/write");
  try
    {
      //cgicc::Cgicc cgi(in);
      std::vector<cgicc::FormEntry> vfat2FormEntries = cgi.getElements();

      if (cgi.queryCheckbox("CR0Set") ) {
	uint8_t regToSet = vfatDevice->getVFAT2Params().control0;
	LOG4CPLUS_INFO(getApplicationLogger(), "Setting the run mode to :"+
		       boost::to_upper_copy(cgi["RunMode"      ]->getValue()));
	vfatDevice->setRunMode(        (gem::hw::vfat::StringToRunMode        .at(boost::to_upper_copy(cgi["RunMode"    ]->getValue()))),regToSet);
	LOG4CPLUS_INFO(getApplicationLogger(), "Setting the trigger mode to :"+
		       boost::to_upper_copy(cgi["TriggerMode"      ]->getValue()));
	vfatDevice->setTriggerMode(    (gem::hw::vfat::StringToTriggerMode    .at(boost::to_upper_copy(cgi["TriggerMode"]->getValue()))),regToSet);
	LOG4CPLUS_INFO(getApplicationLogger(), "Setting the monostable pulse polarity to :"+
		       boost::to_upper_copy(cgi["MSPolarity"      ]->getValue()));
	vfatDevice->setMSPolarity(     (gem::hw::vfat::StringToMSPolarity     .at(boost::to_upper_copy(cgi["MSPolarity" ]->getValue()))),regToSet);
	LOG4CPLUS_INFO(getApplicationLogger(), "Setting the calibration pulse polarity to :"+
		       boost::to_upper_copy(cgi["CalPolarity"      ]->getValue()));
	vfatDevice->setCalPolarity(    (gem::hw::vfat::StringToCalPolarity    .at(boost::to_upper_copy(cgi["CalPolarity"]->getValue()))),regToSet);
	LOG4CPLUS_INFO(getApplicationLogger(), "Setting calibration mode to :"+
		       boost::to_upper_copy(cgi["CalMode"      ]->getValue()));
	vfatDevice->setCalibrationMode((gem::hw::vfat::StringToCalibrationMode.at(boost::to_upper_copy(cgi["CalMode"    ]->getValue()))),regToSet);

	LOG4CPLUS_INFO(getApplicationLogger(), boost::str(boost::format("with value 0x%02x")%(unsigned)regToSet));
	regValsToSet.push_back(std::make_pair("ContReg0",regToSet));
      }

      if (cgi.queryCheckbox("CR1Set") ) {
	LOG4CPLUS_INFO(getApplicationLogger(), "Setting control register 1");
	uint8_t regToSet = vfatDevice->getVFAT2Params().control1;
	LOG4CPLUS_INFO(getApplicationLogger(), "Setting the probe mode to :"+
		       boost::to_upper_copy(cgi["ProbeMode"      ]->getValue()));
	vfatDevice->setProbeMode(        (gem::hw::vfat::StringToProbeMode    .at(boost::to_upper_copy(cgi["ProbeMode"    ]->getValue()))),regToSet);
	LOG4CPLUS_INFO(getApplicationLogger(), "Setting the lvds power save state to :"+
		       boost::to_upper_copy(cgi["LVDSPowerSave"      ]->getValue()));
	vfatDevice->setLVDSMode(         (gem::hw::vfat::StringToLVDSPowerSave.at(boost::to_upper_copy(cgi["LVDSPowerSave"]->getValue()))),regToSet);
	LOG4CPLUS_INFO(getApplicationLogger(), "Setting the hit count cycle time to :"+
		       boost::to_upper_copy(cgi["ReHitCT"      ]->getValue()));
	vfatDevice->setHitCountCycleTime((gem::hw::vfat::StringToReHitCT      .at(boost::to_upper_copy(cgi["ReHitCT"      ]->getValue()))),regToSet);
	LOG4CPLUS_INFO(getApplicationLogger(), "Setting the DAC mode to :"+
		       boost::to_upper_copy(cgi["DACMode"      ]->getValue()));
	LOG4CPLUS_INFO(getApplicationLogger(), boost::to_upper_copy(cgi["DACMode"      ]->getValue()));
	vfatDevice->setDACMode(          (gem::hw::vfat::StringToDACMode      .at(boost::to_upper_copy(cgi["DACMode"      ]->getValue()))),regToSet);
	LOG4CPLUS_INFO(getApplicationLogger(), boost::str(boost::format("with value 0x%02x")%(unsigned)regToSet));
	regValsToSet.push_back(std::make_pair("ContReg1",regToSet));
      }

      if (cgi.queryCheckbox("CR2Set") ) {
	LOG4CPLUS_INFO(getApplicationLogger(), "Setting control register 2");
	uint8_t regToSet = vfatDevice->getVFAT2Params().control2;
	LOG4CPLUS_INFO(getApplicationLogger(), "Setting the monostable pulse length to :"+
		       boost::to_upper_copy(cgi["MSPulseLength"      ]->getValue()));
	vfatDevice->setMSPulseLength((gem::hw::vfat::StringToMSPulseLength.at(boost::to_upper_copy(cgi["MSPulseLength"]->getValue()))),regToSet);
	LOG4CPLUS_INFO(getApplicationLogger(), "Setting the digital input pad to :"+
		       boost::to_upper_copy(cgi["DigInSel"      ]->getValue()));
	vfatDevice->setInputPadMode( (gem::hw::vfat::StringToDigInSel     .at(boost::to_upper_copy(cgi["DigInSel"     ]->getValue()))),regToSet);
	LOG4CPLUS_INFO(getApplicationLogger(), "Setting the hit count mode to :"+
		       boost::to_upper_copy(cgi["HitCountMode"      ]->getValue()));
	vfatDevice->setHitCountMode( (gem::hw::vfat::StringToHitCountMode .at(boost::to_upper_copy(cgi["HitCountMode" ]->getValue()))),regToSet);
	LOG4CPLUS_INFO(getApplicationLogger(), boost::str(boost::format("with value 0x%02x")%(unsigned)regToSet));
	regValsToSet.push_back(std::make_pair("ContReg2",regToSet));
      }

      if (cgi.queryCheckbox("CR3Set") ) {
	LOG4CPLUS_INFO(getApplicationLogger(), "Setting control register 3");
	uint8_t regToSet = vfatDevice->getVFAT2Params().control3;
	LOG4CPLUS_INFO(getApplicationLogger(), "Setting the trim dac range to :"+
		       boost::to_upper_copy(cgi["TrimDACRange"      ]->getValue()));
	vfatDevice->setTrimDACRange((gem::hw::vfat::StringToTrimDACRange .at(boost::to_upper_copy(cgi["TrimDACRange"]->getValue()))),regToSet);
	LOG4CPLUS_INFO(getApplicationLogger(), "Setting the pad band gap to :"+
		       boost::to_upper_copy(cgi["PbBG"      ]->getValue()));
	vfatDevice->setBandgapPad(  (gem::hw::vfat::StringToPbBG         .at(boost::to_upper_copy(cgi["PbBG"        ]->getValue()))),regToSet);
	LOG4CPLUS_INFO(getApplicationLogger(), "Setting the test pattern mode to :"+
		       boost::to_upper_copy(cgi["DFTest"      ]->getValue()));
	vfatDevice->sendTestPattern((gem::hw::vfat::StringToDFTestPattern.at(boost::to_upper_copy(cgi["DFTest"      ]->getValue()))),regToSet);
	LOG4CPLUS_INFO(getApplicationLogger(), boost::str(boost::format("with value 0x%02x")%(unsigned)regToSet));
	regValsToSet.push_back(std::make_pair("ContReg3",regToSet));
      }

      if (cgi.queryCheckbox("SetIPreampIn") )
	regValsToSet.push_back(std::make_pair("IPreampIn"   , cgi["IPreampIn"   ]->getIntegerValue()));
      if (cgi.queryCheckbox("SetIPreampFeed") )
	regValsToSet.push_back(std::make_pair("IPreampFeed" , cgi["IPreampFeed" ]->getIntegerValue()));
      if (cgi.queryCheckbox("SetIPreampOut") )
	regValsToSet.push_back(std::make_pair("IPreampOut"  , cgi["IPreampOut"  ]->getIntegerValue()));
      if (cgi.queryCheckbox("SetIShaper") )
	regValsToSet.push_back(std::make_pair("IShaper"     , cgi["IShaper"     ]->getIntegerValue()));
      if (cgi.queryCheckbox("SetIShaperFeed") )
	regValsToSet.push_back(std::make_pair("IShaperFeed" , cgi["IShaperFeed" ]->getIntegerValue()));
      if (cgi.queryCheckbox("SetIComp") )
	regValsToSet.push_back(std::make_pair("IComp"       , cgi["IComp"       ]->getIntegerValue()));
      if (cgi.queryCheckbox("SetLatency") )
	regValsToSet.push_back(std::make_pair("Latency"     , cgi["Latency"     ]->getIntegerValue()));
      if (cgi.queryCheckbox("SetVCal") )
	regValsToSet.push_back(std::make_pair("VCal"        , cgi["VCal"        ]->getIntegerValue()));
      if (cgi.queryCheckbox("SetVThreshold1") )
	regValsToSet.push_back(std::make_pair("VThreshold1" , cgi["VThreshold1" ]->getIntegerValue()));
      if (cgi.queryCheckbox("SetVThreshold2") )
	regValsToSet.push_back(std::make_pair("VThreshold2" , cgi["VThreshold2" ]->getIntegerValue()));
      if (cgi.queryCheckbox("SetCalPhase") )
	regValsToSet.push_back(std::make_pair("CalPhase"    , cgi["CalPhase"    ]->getIntegerValue()));
    }
  /*
    catch (const boost::exception & e)
    {
    XCEPT_RAISE(boost::exception, e);
    }
  */
  catch (const xgi::exception::Exception & e)
    {
      XCEPT_RAISE(xgi::exception::Exception, e.what());
    }
  catch (const std::exception & e)
    {
      XCEPT_RAISE(xgi::exception::Exception, e.what());
    }  
  LOG4CPLUS_INFO(this->getApplicationLogger(), "getCheckedRegisters::list obtained");
  //return regValsToSet;
}

void gem::hw::vfat::GEMController::performAction(cgicc::Cgicc cgi, std::vector<std::pair<std::string,uint8_t> > regValsToSet)
  throw (xgi::exception::Exception)
{
  /** possible actions
   *Read Counters (name=ReadCounters
   *
   *Get selected channel (name=GetChannel)
   *Set selected channel (name=SetChannel)
   *Set all channels (name=SetAllChannels)
   *
   *Read all(selected?) registers (name=Read)
   *Write selected registers      (name=Write)
   *Compare written values of selected registers      (name=Compare)
   ** change the font colour if the written value
   ** does not equal the read back value?
   *
   *Load settings from XML file (server or upload?)
   *Write current settings to XML file (server or download?)
   **/
  bool makeComparison = cgi.queryCheckbox("Compare");

  if (makeComparison) {
    /**
     *should we just compare what we want to write with what is currently there,
     *or should we write, and compare that what we wrote matches what we said to write?
     */
  }

  LOG4CPLUS_INFO(this->getApplicationLogger(), "performAction::reading options");
  std::string controlOption = "";
  std::string channelOption = "";

  controlOption = cgi["VFAT2ControlOption"]->getValue();
  LOG4CPLUS_INFO(this->getApplicationLogger(),"performAction::Control option " << controlOption);
  LOG4CPLUS_INFO(this->getApplicationLogger(),"performAction::Channel option " << channelOption);
  if (strcmp(controlOption.c_str(),"Read counters") == 0) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Read counters button pressed");
    //vfatDevice->readVFAT2Counters(vfatDevice->getVFAT2Params());
    vfatDevice->readVFAT2Counters();
  }

  else if (strcmp(controlOption.c_str(),"Set Channel 0/1") == 0) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Set channel 1 button pressed");

    if (cgi.queryCheckbox("Cal0") )
      vfatDevice->enableCalPulseToChannel(0);
    else
      vfatDevice->enableCalPulseToChannel(0,false);
    if (cgi.queryCheckbox("Cal1") )
      vfatDevice->enableCalPulseToChannel(1);
    else
      vfatDevice->enableCalPulseToChannel(1,false);
    if (cgi.queryCheckbox("Ch1Mask") )
      vfatDevice->maskChannel(1);
    else
      vfatDevice->maskChannel(1,false);
    if (cgi.queryCheckbox("SetCh1TrimDAC")) 
      vfatDevice->setChannelTrimDAC(1,cgi["Ch1TrimDAC"]->getIntegerValue());

    vfatDevice->readVFAT2Channel(1);
    vfatParams_ = vfatDevice->getVFAT2Params();
    LOG4CPLUS_INFO(this->getApplicationLogger(),"set channel 0/1 - 0x"
		   <<std::hex<<static_cast<unsigned>(vfatParams_.channels[0].fullChannelReg)<<std::dec<<"::<"
		   <<std::hex<<static_cast<unsigned>(vfatParams_.channels[0].calPulse0     )<<std::dec<<":"
		   <<std::hex<<static_cast<unsigned>(vfatParams_.channels[0].calPulse      )<<std::dec<<":"
		   <<std::hex<<static_cast<unsigned>(vfatParams_.channels[0].mask          )<<std::dec<<":"
		   <<std::hex<<static_cast<unsigned>(vfatParams_.channels[0].trimDAC       )<<std::dec<<">"
		   << std::endl);
  }

  else if (strcmp(controlOption.c_str(),"Get This Channel") == 0) {
    //shouldn't need to query the device here, just read the properties from the 
    //stored values, and ensure the web page displays that channel
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Get channel button pressed");
    uint8_t chan = cgi["ChanSel"]->getIntegerValue();
    //uint8_t chanSettings = vfatDevice->getChannelSettings(chan);

    vfatParams_ = vfatDevice->getVFAT2Params();
    vfatParams_.activeChannel = chan;
    //vfatDevice->setActiveChannelWeb(chan);
  }

  else if (strcmp(controlOption.c_str(),"Set This Channel") == 0) {
    //shouldn't need protection here
    uint8_t chan = cgi["ChanSel"]->getIntegerValue();
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Set this channel button pressed for channel "
		   << (unsigned)chan);
    //vfatDevice->getVFAT2Params().channels[chan-1].calPulse  = (cgi.queryCheckbox("ChCal") );
    //vfatDevice->getVFAT2Params().channels[chan-1].mask      = (cgi.queryCheckbox("ChMask") );

    if (cgi.queryCheckbox("ChCal") )
      vfatDevice->enableCalPulseToChannel(chan);
    else
      vfatDevice->enableCalPulseToChannel(chan,false);
    if (cgi.queryCheckbox("ChMask") )
      vfatDevice->maskChannel(chan);
    else
      vfatDevice->maskChannel(chan,false);
    if (cgi.queryCheckbox("SetTrimDAC")) 
      //vfatDevice->getVFAT2Params().channels[chan-1].trimDAC = cgi["TrimDAC"]->getIntegerValue();
      vfatDevice->setChannelTrimDAC(chan,cgi["TrimDAC"]->getIntegerValue());

    //vfatDevice->getVFAT2Params().activeChannel = chan;
    //vfatDevice->readVFAT2Channel(vfatDevice->getVFAT2Params(),chan);
    vfatDevice->readVFAT2Channel(chan);
    vfatParams_ = vfatDevice->getVFAT2Params();
    vfatParams_.activeChannel = chan;
    LOG4CPLUS_INFO(this->getApplicationLogger(),
		   "chan = "<< (unsigned)chan << "; activeChannel = " <<(unsigned)vfatParams_.activeChannel << std::endl);
    LOG4CPLUS_INFO(this->getApplicationLogger(),"set this channel " << (unsigned)chan << " - 0x"
		   <<std::hex<<static_cast<unsigned>(vfatParams_.channels[chan-1].fullChannelReg)<<std::dec<<"::<"
		   <<std::hex<<static_cast<unsigned>(vfatParams_.channels[chan-1].calPulse0     )<<std::dec<<":"
		   <<std::hex<<static_cast<unsigned>(vfatParams_.channels[chan-1].calPulse      )<<std::dec<<":"
		   <<std::hex<<static_cast<unsigned>(vfatParams_.channels[chan-1].mask          )<<std::dec<<":"
		   <<std::hex<<static_cast<unsigned>(vfatParams_.channels[chan-1].trimDAC       )<<std::dec<<">"
		   << std::endl);
  }

  else if (strcmp(controlOption.c_str(),"Set All Channels") == 0) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Set all channels button pressed");
    //apply provided settings to all channels (2-128 or 1-128?)
    uint8_t chan = cgi["ChanSel"]->getIntegerValue();
    int min_chan = 2;
    if (cgi.queryCheckbox("ChCal") )
      for (int chan = min_chan; chan < 129; ++chan) {
	if (chan == 1)
	  vfatDevice->enableCalPulseToChannel(chan-1);
	vfatDevice->enableCalPulseToChannel(chan); }
    if (cgi.queryCheckbox("ChMask") )
      for (int chan = min_chan; chan < 129; ++chan)
	vfatDevice->maskChannel(chan);
    if (cgi.queryCheckbox("SetTrimDAC")) 
      for (int chan = min_chan; chan < 129; ++chan)
	vfatDevice->setChannelTrimDAC(chan,cgi["TrimDAC"]->getIntegerValue());

    //vfatDevice->readVFAT2Channels(vfatDevice->getVFAT2Params());
    vfatDevice->readVFAT2Channels();
    vfatParams_ = vfatDevice->getVFAT2Params();
    vfatParams_.activeChannel = chan;
    LOG4CPLUS_INFO(this->getApplicationLogger(),"set all channels " << (unsigned)chan << " - 0x"
		   <<std::hex<<static_cast<unsigned>(vfatParams_.channels[chan-1].fullChannelReg)<<std::dec<<"::<"
		   <<std::hex<<static_cast<unsigned>(vfatParams_.channels[chan-1].calPulse0     )<<std::dec<<":"
		   <<std::hex<<static_cast<unsigned>(vfatParams_.channels[chan-1].calPulse      )<<std::dec<<":"
		   <<std::hex<<static_cast<unsigned>(vfatParams_.channels[chan-1].mask          )<<std::dec<<":"
		   <<std::hex<<static_cast<unsigned>(vfatParams_.channels[chan-1].trimDAC       )<<std::dec<<">"
		   << std::endl);
  }

  else if (strcmp(controlOption.c_str(),"Read VFAT") == 0) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Read VFAT button pressed with following registers");

    std::vector<std::pair<std::string,uint8_t> >::const_iterator curReg = regValsToSet.begin();
    for (; curReg != regValsToSet.end(); ++curReg) {
      //std::string msg = toolbox::toString((curReg->first).c_str());
      LOG4CPLUS_INFO(this->getApplicationLogger(),curReg->first);
    }
    vfatDevice->readVFATRegs(regValsToSet);
    //readVFAT2Registers(vfatDevice->getVFAT2Params());
    vfatDevice->getAllSettings();
    vfatParams_ = vfatDevice->getVFAT2Params();
  }
  else if (strcmp(controlOption.c_str(),"Write VFAT") == 0) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Write VFAT button pressed with following registers");

    std::vector<std::pair<std::string,uint8_t> >::const_iterator curReg = regValsToSet.begin();
    for (; curReg != regValsToSet.end(); ++curReg) {
      std::string msg =
	toolbox::toString("%s - 0x%02x",(curReg->first).c_str(),static_cast<unsigned>(curReg->second));
      LOG4CPLUS_INFO(this->getApplicationLogger(),msg);
    }

    //do a single transaction
    //vfatDevice->writeVFATRegs(regValsToSet);

    curReg = regValsToSet.begin();
    for (; curReg != regValsToSet.end(); ++curReg)
      vfatDevice->writeVFATReg(curReg->first,curReg->second);

    //readVFAT2Registers(vfatDevice->getVFAT2Params());
    vfatDevice->getAllSettings();
    vfatParams_ = vfatDevice->getVFAT2Params();
  }
  //vfatParams_ = vfatDevice->getVFAT2Params();
}

void gem::hw::vfat::GEMController::actionPerformed(xdata::Event& event)
{
  // This is called after all default configuration values have been
  // loaded (from the XDAQ configuration file).
  if (event.type() == "urn:xdaq-event:setDefaultValues") {
    std::stringstream ss;
    ss << "device_=[" << device_.toString() << "]" << std::endl;
    ss << "settingsFile_=[" << settingsFile_.toString() << "]" << std::endl;
    LOG4CPLUS_DEBUG(getApplicationLogger(), "GEMController::actionPerformed() Default configuration values have been loaded");
    LOG4CPLUS_INFO(this->getApplicationLogger(), ss.str());
    //LOG4CPLUS_DEBUG(getApplicationLogger(), "GEMController::actionPerformed()   --> starting monitoring");
    //monitorP_->startMonitoring();
  }    
  //Initialize the HW device, should have picked up the device string from the xml file by now
  LOG4CPLUS_INFO(this->getApplicationLogger(),"GEMController::GEMController::4 device_ = " << device_.toString() << std::endl);
  vfatDevice = new HwVFAT2(device_.toString());
  vfatDevice->connectDevice();
  setLogLevelTo(uhal::Error());  // Maximise uHAL logging
  LOG4CPLUS_INFO(this->getApplicationLogger(),"GEMController::GEMController::5 device_ = " << device_.toString() << std::endl);

  //initialize the vfatParameters struct
  //readVFAT2Registers(vfatParams);
  vfatDevice->getAllSettings();
  //  LOG4CPLUS_INFO(this->getApplicationLogger(),"vfatParams:" << std::endl
  //       << vfatDevice->getVFAT2Params() << std::endl);
  //readVFAT2Registers();
  vfatParams_ = vfatDevice->getVFAT2Params();
  LOG4CPLUS_INFO(this->getApplicationLogger(),"GEMController::GEMController::6 device_ = " << device_.toString() << std::endl);

}

