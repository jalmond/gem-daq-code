#include "gem/utils/soap/GEMSOAPToolBox.h"

#include "gem/utils/exception/Exception.h"
#include "log4cplus/logger.h"
#include "log4cplus/loggingmacros.h"
#include "gem/utils/GEMLogging.h"

#include "toolbox/string.h"
#include "xdaq/NamespaceURI.h"

#include "xoap/MessageFactory.h"
#include "xoap/domutils.h"
#include "xoap/SOAPBody.h"
#include "xoap/SOAPEnvelope.h"
#include "xoap/SOAPPart.h"
#include "xoap/SOAPSerializer.h"

xoap::MessageReference gem::utils::soap::GEMSOAPToolBox::makeSoapReply(std::string const& command,
								       std::string const& response)
{
  xoap::MessageReference reply        = xoap::createMessage();
  xoap::SOAPEnvelope     envelope     = reply->getSOAPPart().getEnvelope();
  xoap::SOAPName         responseName = envelope.createName(command, "xdaq", XDAQ_NS_URI);
  xoap::SOAPElement      bodyElement  = envelope.getBody().addBodyElement(responseName);
  bodyElement.addTextNode(response);
  return reply;
}

xoap::MessageReference gem::utils::soap::GEMSOAPToolBox::makeSoapFaultReply(std::string const& faultString,
									    std::string const& faultCode,
									    std::string const& detail,
									    std::string const& faultActor)
{
  xoap::MessageReference reply       = xoap::createMessage();
  xoap::SOAPEnvelope     envelope    = reply->getSOAPPart().getEnvelope();
  xoap::SOAPName         faultName   = envelope.createName("Fault", "xdaq", XDAQ_NS_URI);
  xoap::SOAPElement      bodyElement = envelope.getBody().addBodyElement(faultName);

  xoap::SOAPName    faultStringName    = envelope.createName("faultstring", "xdaq", XDAQ_NS_URI);
  xoap::SOAPElement faultStringElement = bodyElement.addChildElement(faultStringName);
  faultStringElement.addTextNode(faultString);

  xoap::SOAPName    faultCodeName    = envelope.createName("faultcode", "xdaq", XDAQ_NS_URI);
  xoap::SOAPElement faultCodeElement = bodyElement.addChildElement(faultCodeName);
  faultCodeElement.addTextNode(faultCode);
  
  if (detail.size() > 0) {
    xoap::SOAPName    detailName    = envelope.createName("detail", "xdaq", XDAQ_NS_URI);
    xoap::SOAPElement detailElement = bodyElement.addChildElement(detailName);
    detailElement.addTextNode(detail);
  }
  
  if (faultActor.size() > 0) {
    xoap::SOAPName faultActorName = envelope.createName("faultactor", "xdaq", XDAQ_NS_URI);
    xoap::SOAPElement faultActorElement = bodyElement.addChildElement(faultActorName);
    faultActorElement.addTextNode(faultActor);
  }
  
  return reply;
}

xoap::MessageReference gem::utils::soap::GEMSOAPToolBox::makeFSMSoapReply(std::string const& event,
									  std::string const& state)
{
  xoap::MessageReference reply         = xoap::createMessage();
  xoap::SOAPEnvelope     envelope      = reply->getSOAPPart().getEnvelope();
  xoap::SOAPBody         body          = envelope.getBody();
  std::string            replyString   = event + " Response";
  xoap::SOAPName         replyName     = envelope.createName(replyString, "xdaq", XDAQ_NS_URI);
  xoap::SOAPBodyElement  replyElement  = body.addBodyElement(replyName);
  xoap::SOAPName         stateName     = envelope.createName("state", "xdaq", XDAQ_NS_URI);
  xoap::SOAPElement      stateElement  = replyElement.addChildElement(stateName);
  xoap::SOAPName         attributeName = envelope.createName("stateName", "xdaq", XDAQ_NS_URI);
  stateElement.addAttribute(attributeName, state);
  return reply;
}


std::string gem::utils::soap::GEMSOAPToolBox::extractFSMCommandName(xoap::MessageReference const& msg)
{
  xoap::SOAPPart     part = msg->getSOAPPart();
  xoap::SOAPEnvelope env  = part.getEnvelope();
  xoap::SOAPBody     body = env.getBody();
  
  DOMNode*     node     = body.getDOMNode();
  DOMNodeList* bodyList = node->getChildNodes();

  /* what is this method doing differntly? when would this not work vs the other method?
  for (unsigned int i = 0; i < bodyList->getLength(); i++) 
    {
      DOMNode* command = bodyList->item(i);
      if (command->getNodeType() == DOMNode::ELEMENT_NODE)
	return xoap::XMLCh2String(command->getLocalName());
    }
  */
  // The body should contain a single node with the name of the FSM
  // command to execute.
  if (bodyList->getLength() != 1) {
    XCEPT_RAISE(xoap::exception::Exception,
		toolbox::toString("Expected exactly one element "
				  "in FSM command SOAP message, "
				  "but found %d.", bodyList->getLength()));
  }
  return xoap::XMLCh2String((bodyList->item(0))->getLocalName());  
}

bool gem::utils::soap::GEMSOAPToolBox::sendCommand(std::string const& cmd,
						   xdaq::ApplicationContext* appCxt,
						   xdaq::ApplicationDescriptor* srcDsc,
						   xdaq::ApplicationDescriptor* destDsc
						   //log4cplus::Logger* logger,
						   //std::string const& param
						   )
  throw (gem::utils::exception::Exception)
{
  try {
    xoap::MessageReference msg = xoap::createMessage();
    
    xoap::SOAPEnvelope env = msg->getSOAPPart().getEnvelope();
    xoap::SOAPName soapcmd = env.createName(cmd,"xdaq", XDAQ_NS_URI);
    env.getBody().addBodyElement(soapcmd);
    
    xoap::MessageReference response = appCxt->postSOAP(msg,*srcDsc,*destDsc);
    std::string  tool;
    xoap::dumpTree(response->getSOAPPart().getEnvelope().getDOMNode(),tool);
    
    //LOG4CPLUS_INFO(logger, "sendCommand(" + cmd + ") received response: " + tool);
    
  } catch (xcept::Exception& e) {
    XCEPT_RETHROW(gem::utils::exception::SOAPException,
		  ::toolbox::toString("Command %s failed [%s]",cmd.c_str(),e.what()),e);
  } catch (std::exception& e) {
    XCEPT_RAISE(gem::utils::exception::SOAPException,
		::toolbox::toString("Command %s failed [%s]",cmd.c_str(),e.what()));
  } catch(...) {
    XCEPT_RAISE(gem::utils::exception::SOAPException,
		::toolbox::toString("Command %s failed",cmd.c_str()));
  }
  return true;
}
