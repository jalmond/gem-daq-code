#ifndef gem_utils_soap_GEMSOAPToolBox_h
#define gem_utils_soap_GEMSOAPToolBox_h

//using the SOAP toolbox defined in the TCDS code base with extra functionality from the EMU codebase
#include <string>

#include "xoap/MessageReference.h"
#include "xercesc/util/XercesDefs.hpp"

#include "xdaq/ApplicationDescriptor.h"
#include "xdaq/ApplicationContext.h"

#include "gem/utils/exception/Exception.h"

namespace gem {
  namespace utils {
    namespace soap {
      
      class GEMSOAPToolBox
      {
      public:
	//methods copied from tcds soap helper
	static xoap::MessageReference makeSoapReply(std::string const& command,
						    std::string const& response);
	static xoap::MessageReference makeSoapFaultReply(std::string const& faultString,
							 std::string const& faultCode="Server",
							 std::string const& faultActor="",
							 std::string const& detail="");
	static xoap::MessageReference makeFSMSoapReply(std::string const& event,
						       std::string const& state);
	
	static std::string extractFSMCommandName(xoap::MessageReference const& msg);
	
	/**
	 * @param cmd command to send to the application
	 * @param appCxt context in which the source/receiver applications are running
	 * @param srcDsc source application descriptor
	 * @param destDsc destination application descriptor
	 * @param logger logger object
	 * @param param parameters to append to the SOAP message
	 * returns true if successful/completed
	 */
	static bool sendCommand(std::string const& cmd,
				xdaq::ApplicationContext* appCxt,
				xdaq::ApplicationDescriptor* srcDsc,
				xdaq::ApplicationDescriptor* destDsc
				//log4cplus::Logger* logger,
				//std::string const& param
				)
	  throw (gem::utils::exception::Exception);
	
	//methods copied from emu/soap/toolbox
	/*
	xoap::MessageReference createMessage( const gem::utils::soap::QualifiedName &command, 
					      const gem::utils::soap::Parameters &parameters = gem::utils::soap::Parameters::none,
					      const gem::utils::soap::Attributes &attributes = gem::utils::soap::Attributes::none,
					      const vector<gem::utils::soap::Attachment> &attachments = gem::utils::soap::Attachment::none );
	
	void addAttachments(           xoap::MessageReference message,
				       const vector<gem::utils::soap::Attachment> &attachments );
	void addAttributes(            xoap::MessageReference message,
				       xoap::SOAPElement* element,
				       const gem::utils::soap::Attributes &attributes );
	void includeParameters(        xoap::MessageReference message,
				       xoap::SOAPElement* parent,
				       gem::utils::soap::Parameters &parameters );
	void includeParameters(        xoap::MessageReference message,
				       xoap::SOAPElement* parent,
				       const gem::utils::soap::Parameters &parameters );
	void extractCommandAttributes( xoap::MessageReference message,
				       gem::utils::soap::Attributes &attributes );
	void extractParameters(        xoap::MessageReference message,
				       gem::utils::soap::Parameters &parameters );
	void setResponseTimeout(       xoap::MessageReference message,
				       const uint64_t timeoutInSec );
	*/
      private:
	
      protected:
	GEMSOAPToolBox();
      };
      
    }//end namespace gem::utils::soap

  }//end namespace gem::base

}//end namespace gem

#endif 
