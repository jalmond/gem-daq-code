#ifndef gem_utils_gemXMLparser_h
#define gem_utils_gemXMLparser_h

#include <string>
#include <map>

#include <xercesc/dom/DOM.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

#include "gem/utils/GEMLogging.h"

#include "gem/utils/gemComplexDeviceProperties.h"

namespace gem {
  namespace utils {
    class gemXMLparser
    {
      
    public:
      gemXMLparser(const std::string& xmlFile);

      ~gemXMLparser();

      void parseXMLFile();

      /** 
       *   Parse section of XML configuration file describing GEM system
       */
      void parseGEMSystem(xercesc::DOMNode * pNode);

      /** 
       *   Parse section of XML configuration file describing available uTCA crates
       */
      void parseCrate(xercesc::DOMNode * pNode);

      /** 
       *   Parse section of XML configuration file describing available GLIB board(s)
       */
      void parseGLIB(xercesc::DOMNode * pNode);

      /** 
       *   Parse section of XML configuration file describing available OH board(s)
       */
      void parseOH(xercesc::DOMNode * pNode);

      /** 
       *   Parse section of XML configuration file describing VFAT2 registers
       */
      void parseVFAT2Settings(xercesc::DOMNode * pNode);

      /**
       *   Access to gemSystemProperties. A bit unsafe due to non-const pointer  
       */
      gemSystemProperties* getGEMDevice() {return gemSystem_;}

      /**
       *   Count child nodes of element type of the given XML node
       */
      int countChildElementNodes(xercesc::DOMNode * pNode);

    private:
      static void addProperty(const char* key, const xercesc::DOMNode* n, gemGLIBProperties* glib);
      static void addProperty(const char* key, const xercesc::DOMNode* n, gemOHProperties* oh);
      static void addProperty(const char* key, const xercesc::DOMNode* n, gemVFATProperties* vfat);
      std::string xmlFile_;
      gemSystemProperties *gemSystem_;
      log4cplus::Logger gemLogger_;
    };
  } // end namespace gem::utils
} // end namespace gem

#endif
