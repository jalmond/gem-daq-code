///////////////////////////////////////////////
//
// Parse XML File
//
///////////////////////////////////////////////
#include "gem/utils/gemXMLparser.h"
#include "gem/utils/gemComplexDeviceProperties.h"

gem::utils::gemXMLparser::gemXMLparser(const std::string& xmlFile)
{
    xmlFile_ = xmlFile;
    gemSystem_ = new gemSystemProperties();
    gemSystem_->setDeviceId("GEM");
}


gem::utils::gemXMLparser::~gemXMLparser()
{
    delete gemSystem_;
}

void gem::utils::gemXMLparser::parseXMLFile()
{
    //LOG4CPLUS_INFO(this->getApplicationLogger(), "Parsing XML file: " << settingsFile_.toString());

    //
    /// Initialize XML4C system
    try{
        xercesc::XMLPlatformUtils::Initialize();
        //LOG4CPLUS_INFO(this->getApplicationLogger(), "Successfully initialized XML4C system");
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
    std::cout << "[XML PARSER]: Xerces parcer created " << std::endl;
    parser->setValidationScheme(xercesc::XercesDOMParser::Val_Auto);
    parser->setDoNamespaces(false);
    parser->setCreateEntityReferenceNodes(false);
    //parser->setToCreateXMLDeclTypeNode(true);
    std::cout << "[XML PARSER]: Xerces parcer tuned up " << std::endl;


    //  Parse the XML file, catching any XML exceptions that might propogate
    //  out of it.
    //
    bool errorsOccured = false;
    try{
        parser->parse(xmlFile_.c_str());
    }


    catch (const xercesc::XMLException& e){
        std::cerr << "An error occured during parsing\n   Message: "
            << xercesc::XMLString::transcode(e.getMessage()) << std::endl;
        errorsOccured = true;
        //fileError = "An error occured during parsing of selected file. Please select another configuration file.";
    }


    catch (const xercesc::DOMException& e){
        std::cerr << "An error occured during parsing\n   Message: "
            << xercesc::XMLString::transcode(e.msg) << std::endl;
        errorsOccured = true;
        //fileError = "An error occured during parsing of selected file. Please select another configuration file.";
    }

    catch (...){
        std::cerr << "An error occured during parsing" << std::endl;
        errorsOccured = true;
        //fileError = "An error occured during parsing of selected file. Please select another configuration file.";
    }

    // If the parse was successful, output the document data from the DOM tree
    //crateIds.clear();
    //crateNodes.clear();

    if (!errorsOccured){
        std::cout << "[XML PARSER]: DOM tree created succesfully" << std::endl;
        xercesc::DOMNode * pDoc = parser->getDocument();
        std::cout << "[XML PARSER]: Base node (getDocument) obtained" << std::endl;
        xercesc::DOMNode * n = pDoc->getFirstChild();
        std::cout << "[XML PARSER]: First child node obtained" << std::endl;
        while (n) {
            std::cout << "[XML PARSER]: Loop on child nodes" << std::endl;
            if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) 
            {
                std::cout << "[XML PARSER]: Element node found" << std::endl;
                if (strcmp("GEMSystem",xercesc::XMLString::transcode(n->getNodeName()))==0) {
                    std::cout << "[XML PARSER]: GEM system found" << std::endl;
                    parseGEMSystem(n);
                }
            }
            n = n->getNextSibling();
        } 
    } 

    std::cout << "[XML PARSER]: Parser pointer " << parser << std::endl;
    delete parser;
    std::cout << "[XML PARSER]: Xerces parcer deleted " << std::endl;
    xercesc::XMLPlatformUtils::Terminate();
}

void gem::utils::gemXMLparser::parseGEMSystem(xercesc::DOMNode * pNode)
{
    //LOG4CPLUS_INFO(this->getApplicationLogger(), "parseGEMSystem");
    std::cout << "[XML PARSER]: GEM system parsing" << std::endl;
    xercesc::DOMNode * n = pNode->getFirstChild();
    std::cout << "[XML PARSER]: GEM system parsing: get first child" << std::endl;
    while (n) {
        if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE)
        {    
            if (strcmp("uTCACrate",xercesc::XMLString::transcode(n->getNodeName()))==0) {
                //LOG4CPLUS_INFO(this->getApplicationLogger(),xercesc::XMLString::transcode(n->getAttributes()->getNamedItem(xercesc::XMLString::transcode("CrateID"))->getNodeValue()));
                //crateIds.push_back(xercesc::XMLString::transcode(n->getAttributes()->getNamedItem(xercesc::XMLString::transcode("CrateID"))->getNodeValue()));
                //crateNodes.push_back(n);
                //if (crateIds.size() == 1) {
                //currentCrate = 0;
                //currentCrateId = crateIds[0];
                //}
                std::cout << "[XML PARSER]: GEM system parsing: uTCA crate found" << std::endl;
                if (countChildElementNodes(n)) {
                    std::cout << "[XML PARSER]: GEM system parsing: uTCA crate is not empty" << std::endl;
                    gemCrateProperties* crate = new gemCrateProperties();
                    std::cout << "[XML PARSER]: GEM system parsing: new crate properties object created" << std::endl;
                    crate->setDeviceId(xercesc::XMLString::transcode(n->getAttributes()->getNamedItem(xercesc::XMLString::transcode("CrateId"))->getNodeValue()));
                    gemSystem_->addSubDeviceRef(crate);
                    gemSystem_->addSubDeviceId(crate->getDeviceId());
                    std::cout << "[XML PARSER]: GEM system parsing: new crate properties object added to crateRefs" << std::endl;
                    parseCrate(n);
                }
            }
        }    
        n = n->getNextSibling();
    }
}

void gem::utils::gemXMLparser::parseCrate(xercesc::DOMNode * pNode)
{
    //LOG4CPLUS_INFO(this->getApplicationLogger(), "parseCrate");
    std::cout << "[XML PARSER]: GEM system parsing: starting parseCrate" << std::endl;
    xercesc::DOMNode * n = pNode->getFirstChild();
    std::cout << "[XML PARSER]: crate parsing: look for children" << std::endl;
    while (n) {
        if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE)
        {
            if (strcmp("MCH",xercesc::XMLString::transcode(n->getNodeName()))==0) {
                //LOG4CPLUS_INFO(this->getApplicationLogger(), "parseMCH");
            } 
            if (strcmp("AMC",xercesc::XMLString::transcode(n->getNodeName()))==0) {
                //LOG4CPLUS_INFO(this->getApplicationLogger(), "parseAMC");
            } 
            if (strcmp("GLIB",xercesc::XMLString::transcode(n->getNodeName()))==0) {
                //LOG4CPLUS_INFO(this->getApplicationLogger(), "parseGLIB");
                std::cout << "[XML PARSER]: crate parsing: GLIB found" << std::endl;
                if (countChildElementNodes(n)) {
                    std::cout << "[XML PARSER]: crate parsing: GLIB is not empty" << std::endl;
                    gemGLIBProperties* glib = new gemGLIBProperties();
                    std::cout << "[XML PARSER]: crate parsing: create new GLIBproperties object" << std::endl;
                    glib->setDeviceId(xercesc::XMLString::transcode(n->getAttributes()->getNamedItem(xercesc::XMLString::transcode("GLIBId"))->getNodeValue()));
                    gemSystem_->getSubDevicesRefs().back()->addSubDeviceRef(glib);
                    gemSystem_->getSubDevicesRefs().back()->addSubDeviceId(glib->getDeviceId());
                    std::cout << "[XML PARSER]: crate parsing: Add new GLIBproperties to the subdevices of parent crate" << std::endl;
                    parseGLIB(n);
                }
            } 
        }
        n = n->getNextSibling();
    }
}


void gem::utils::gemXMLparser::parseGLIB(xercesc::DOMNode * pNode)
{
    std::cout << "[XML PARSER]: crate parsing: start GLIB parsing" << std::endl;
    xercesc::DOMNode * n = pNode->getFirstChild();
    gemGLIBProperties* glib_ = gemSystem_->getSubDevicesRefs().back()->getSubDevicesRefs().back();
    std::cout << "[XML PARSER]: GLIB parsing: retrieve GLIB device from the devices parent tree" << std::endl;
    while (n) {
        if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE)
        {
            addProperty("Station", n, glib_);
            addProperty("FW", n, glib_);
            addProperty("IP", n, glib_);
            addProperty("DEPTH", n, glib_);
            addProperty("TDC_SBits", n, glib_);
            if (strcmp("OH",xercesc::XMLString::transcode(n->getNodeName()))==0) {
                std::cout << "[XML PARSER]: GLIB parsing: OH found" << std::endl;
                if (countChildElementNodes(n)) {
                    gemOHProperties* oh = new gemOHProperties();
                    std::cout << "[XML PARSER]: GLIB parsing: create new OHproperties obect" << std::endl;
                    oh->setDeviceId(xercesc::XMLString::transcode(n->getAttributes()->getNamedItem(xercesc::XMLString::transcode("OHId"))->getNodeValue()));
                    gemSystem_->getSubDevicesRefs().back()->getSubDevicesRefs().back()->addSubDeviceRef(oh);
                    gemSystem_->getSubDevicesRefs().back()->getSubDevicesRefs().back()->addSubDeviceId(oh->getDeviceId());
                    std::cout << "[XML PARSER]: GLIB parsing: Add new OHproperties to the subdevices of parent device" << std::endl;
                    parseOH(n);
                }
            }
        }    
        n = n->getNextSibling();
    }    
}

void gem::utils::gemXMLparser::parseOH(xercesc::DOMNode * pNode)
{
    std::cout << "[XML PARSER]: GLIB parsing: start OH parsing" << std::endl;
    xercesc::DOMNode * n = pNode->getFirstChild();
    gemOHProperties* oh_ = gemSystem_->getSubDevicesRefs().back()->getSubDevicesRefs().back()->getSubDevicesRefs().back();
    std::cout << "[XML PARSER]: OH parsing: retrieve OH device from the devices parent tree" << std::endl;
    while (n) {
        if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE)
        {   
            addProperty("TrigSource", n, oh_);
            addProperty("TDC_SBits", n, oh_);
            addProperty("VFATClock", n, oh_);
            addProperty("VFATFallback", n, oh_);
            addProperty("CDCEClock", n, oh_);
            addProperty("CDCEFallback", n, oh_);
            addProperty("FPGAPLLLock", n, oh_);
            addProperty("CDCELock", n, oh_);
            addProperty("GTPLock", n, oh_);
            addProperty("FW", n, oh_);
            if (strcmp("VFATSettings",xercesc::XMLString::transcode(n->getNodeName()))==0) {
                std::cout << "[XML PARSER]: OH parsing: VFATSettings tag found" << std::endl;
                if (countChildElementNodes(n)) {
                    gemVFATProperties* vfat = new gemVFATProperties();
                    std::cout << "[XML PARSER]: OH parsing: create new VFATproperties object" << std::endl;
                    vfat->setDeviceId(xercesc::XMLString::transcode(n->getAttributes()->getNamedItem(xercesc::XMLString::transcode("VFATId"))->getNodeValue()));
                    std::cout << "[XML PARSER]: OH parsing: retrieve VFAT device ID" << std::endl;
                    gemSystem_->getSubDevicesRefs().back()->getSubDevicesRefs().back()->getSubDevicesRefs().back()->addSubDeviceRef(vfat);
                    std::cout << "[XML PARSER]: OH parsing: add new VFATproperties to the subdevices of the parent device" << std::endl;
                    gemSystem_->getSubDevicesRefs().back()->getSubDevicesRefs().back()->getSubDevicesRefs().back()->addSubDeviceId(vfat->getDeviceId());
                    std::cout << "[XML PARSER]: OH parsing: add VFAT device ID to the subdevices of the parent device" << std::endl;
                    parseVFAT2Settings(n);
                }
            }
        }    
        n = n->getNextSibling();
    }    
}

void gem::utils::gemXMLparser::parseVFAT2Settings(xercesc::DOMNode * pNode)
{
    std::cout << "[XML PARSER]: OH parsing: start VFAT parsing" << std::endl;
    xercesc::DOMNode * n = pNode->getFirstChild();
    gemVFATProperties* vfat_ = gemSystem_->getSubDevicesRefs().back()->getSubDevicesRefs().back()->getSubDevicesRefs().back()->getSubDevicesRefs().back();
    std::cout << "[XML PARSER]: VFAT parsing: retrieve VFAT device from the devices parent tree" << std::endl;
    while (n) {
        if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE)
        {    
            addProperty("CalMode", n, vfat_);
            addProperty("CalPolarity", n, vfat_);
            addProperty("MSPolarity", n, vfat_);
            addProperty("TriggerMode", n, vfat_);
            addProperty("RunMode", n, vfat_);
            addProperty("ReHitCT", n, vfat_);
            addProperty("LVDSPowerSave", n, vfat_);
            addProperty("ProbeMode", n, vfat_);
            addProperty("DACSel", n, vfat_);
            addProperty("DigInSel", n, vfat_);
            addProperty("MSPulseLength", n, vfat_);
            addProperty("HitCountSel", n, vfat_);
            addProperty("DFTest", n, vfat_);
            addProperty("PbBG", n, vfat_);
            addProperty("TrimDACRange", n, vfat_);
            addProperty("IPreampIn", n, vfat_);
            addProperty("IPreampFeed", n, vfat_);
            addProperty("IPreampOut", n, vfat_);
            addProperty("IShaper", n, vfat_);
            addProperty("IShaperFeed", n, vfat_);
            addProperty("IComp", n, vfat_);
            addProperty("Latency", n, vfat_);
            addProperty("VCal", n, vfat_);
            addProperty("VThreshold1", n, vfat_);
            addProperty("VThreshold2", n, vfat_);
            addProperty("CalPhase", n, vfat_);
        }    
        n = n->getNextSibling();
    }
}

void gem::utils::gemXMLparser::addProperty(const char* key, const xercesc::DOMNode* n, gemGLIBProperties* glib)
{
    if (strcmp(key,xercesc::XMLString::transcode(n->getNodeName()))==0)
    {
        std::string value = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
        glib->addDeviceProperty(key,value);
    }
}

void gem::utils::gemXMLparser::addProperty(const char* key, const xercesc::DOMNode* n, gemOHProperties* oh)
{
    if (strcmp(key,xercesc::XMLString::transcode(n->getNodeName()))==0)
    {
        std::string value = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
        oh->addDeviceProperty(key,value);
    }
}

void gem::utils::gemXMLparser::addProperty(const char* key, const xercesc::DOMNode* n, gemVFATProperties* vfat)
{
    if (strcmp(key,xercesc::XMLString::transcode(n->getNodeName()))==0)
    {
        std::string value = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
        vfat->addDeviceProperty(key,value);
    }
}

int gem::utils::gemXMLparser::countChildElementNodes(xercesc::DOMNode * pNode) {
    int count = 0;
    if (pNode->hasChildNodes()) {
        xercesc::DOMNode * n = pNode->getFirstChild();
        while (n) {
            if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) count++;
            n = n->getNextSibling();
        }
    }
    return count;
}
