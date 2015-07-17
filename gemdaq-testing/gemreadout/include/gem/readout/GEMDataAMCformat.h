#ifndef gem_readout_GEMDataAMCformat_h
#define gem_readout_GEMDataAMCformat_h

#include <iostream>
#include <iomanip> 
#include <fstream>
#include <string>
#include <vector>

using namespace std;

namespace gem {
  namespace readout {

     /*
      uint32_t bxExp;   // :28
      uint16_t bxNum;   // :8,     Sbit:8
      */

      struct VFATData {
        uint16_t BC;          // 1010:4,   BC:12 
        uint16_t EC;          // 1100:4,   EC:8,      Flags:4
        uint16_t ChipID;      // 1110,     ChipID:12
        uint64_t lsData;      // channels from 1to64
        uint64_t msData;      // channels from 65to128
        uint16_t crc;         // :16
      };    
    
      struct GEBData {
        uint64_t header;      // ZSFlag:24 ChamID:12 
        std::vector<VFATData> vfats;
        uint64_t trailer;     // OHcrc: 16 OHwCount:16  ChamStatus:16
      };

      struct GEMData {
        uint64_t header1;      // AmcNo:4      0000:4     LV1ID:24   BXID:12     DataLgth:20 
        uint64_t header2;      // User:32      OrN:16     BoardID:16
        uint64_t header3;      // DAVList:24   BufStat:24 DAVCount:5 FormatVer:3 MP7BordStat:8 
        std::vector<GEBData> gebs;
        uint64_t trailer2;     // EventStat:32 GEBerrFlag:24  
        uint64_t trailer1;     // crc:32       LV1IDT:8   0000:4     DataLgth:20 
      };

      /*
       *  GEB Data Format
       *    geb.header
       *    vfat vector
       *    geb.trailer
       *
       */

      bool writeGEBheader(string file, int event, const GEBData& geb){
        ofstream outf(file.c_str(), ios_base::app );
        if( event<0) return(false);
        if(!outf.is_open()) return(false);
          outf << hex << geb.header << dec << endl;
          outf.close();
        return(true);
      };	  

      bool printGEBheader(int event, const GEBData& geb){
        if( event<0) return(false);
 	  cout << "Received tracking data word:" << endl;
	  cout << " 0x" << std::setw(8) << hex << geb.header << " ChamID " << ((0x000000fff0000000 & geb.header) >> 28) 
               << dec << " sumVFAT " << (0x000000000fffffff & geb.header) << endl;
        return(true);
      };	  

      bool writeZEROline(string file){
        ofstream outf(file.c_str(), ios_base::app );
        if(!outf.is_open()) return(false);
          outf << "\n" << endl;
          outf.close();
        return(true);
      };	  

      bool writeGEBtrailer(string file, int event, const GEBData& geb){
        ofstream outf(file.c_str(), ios_base::app );
        if( event<0) return(false);
        if(!outf.is_open()) return(false);
          outf << hex << geb.trailer << dec << endl;
          outf.close();
        return(true);
      };	  

      bool writeVFATdata(string file, int event, const VFATData& vfat){
        ofstream outf(file.c_str(), ios_base::app );
        if( event<0) return(false);
        if(!outf.is_open()) return(false);
          outf << hex << vfat.BC << dec << endl;
          outf << hex << vfat.EC << dec << endl;
          outf << hex << vfat.ChipID << dec << endl;
          outf << hex << vfat.lsData << dec << endl;
          outf << hex << vfat.msData << dec << endl;
          outf << hex << vfat.crc << dec << endl;
          //writeZEROline(file);
          outf.close();
        return(true);
      };	  

      bool printVFATdata(int event, const VFATData& vfat){
        if( event<0) return(false);
 	  cout << "Received tracking data word:" << endl;
	  cout << "BC      :: 0x" << std::setfill('0') << std::setw(4) << hex << vfat.BC     << dec << endl;
	  cout << "EC      :: 0x" << std::setfill('0') << std::setw(4) << hex << vfat.EC     << dec << endl;
	  cout << "ChipID  :: 0x" << std::setfill('0') << std::setw(4) << hex << vfat.ChipID << dec << endl;
          cout << "<127:64>:: 0x" << std::setfill('0') << std::setw(8) << hex << vfat.msData << dec << endl;
          cout << "<63:0>  :: 0x" << std::setfill('0') << std::setw(8) << hex << vfat.lsData << dec << endl;
	  cout << "crc     :: 0x" << std::setfill('0') << std::setw(4) << hex << vfat.crc    << dec << endl;
        return(true);
      };

      bool writeGEBheaderBinary(string file, int event, const GEBData& geb){
        ofstream outf(file.c_str(), ios_base::app | ios::binary );
        if( event<0) return(false);
        if(!outf.is_open()) return(false);
  	  outf.write( (char*)&geb.header, sizeof(geb.header));
          outf.close();
        return(true);
      };
	  
      bool writeGEBtrailerBinary(string file, int event, const GEBData& geb){
        ofstream outf(file.c_str(), ios_base::app | ios::binary );
        if( event<0) return(false);
        if(!outf.is_open()) return(false);
  	  outf.write( (char*)&geb.trailer, sizeof(geb.trailer));
          outf.close();
        return(true);
      };

      bool writeVFATdataBinary(string file, int event, const VFATData& vfat){
        ofstream outf(file.c_str(), ios_base::app | ios::binary );
        if( event<0) return(false);
        if(!outf.is_open()) return(false);
  	  outf.write( (char*)&vfat.BC, sizeof(vfat.BC));
  	  outf.write( (char*)&vfat.EC, sizeof(vfat.EC));
  	  outf.write( (char*)&vfat.ChipID, sizeof(vfat.ChipID));
  	  outf.write( (char*)&vfat.lsData, sizeof(vfat.lsData));  
  	  outf.write( (char*)&vfat.msData, sizeof(vfat.msData));
  	  outf.write( (char*)&vfat.crc, sizeof(vfat.crc));
          outf.close();
        return(true);
      };	  

      bool readVFATDataBinary(string file, int event, const VFATData& vfat){
        ifstream inpf(file.c_str(), ios_base::app | ios::binary );
        if( event<0) return(false);
        if(!inpf.is_open()) return(false);
  	  inpf.read( (char*)&vfat.BC, sizeof(vfat.BC));
  	  inpf.read( (char*)&vfat.EC, sizeof(vfat.EC));
  	  inpf.read( (char*)&vfat.ChipID, sizeof(vfat.ChipID));
          inpf.read( (char*)&vfat.lsData, sizeof(vfat.lsData));
          inpf.read( (char*)&vfat.msData, sizeof(vfat.msData));
  	  inpf.read( (char*)&vfat.crc, sizeof(vfat.crc));
          inpf.seekg (0, inpf.cur);
        return(true);
      };	  

      //
      // Useful printouts 
      //
      void show4bits(uint8_t x) {
        int i;
        const unsigned long unit = 1;
        for(i=(sizeof(uint8_t)*4)-1; i>=0; i--)
          (x & ((unit)<<i))?putchar('1'):putchar('0');
     	//printf("\n");
      }

      void show16bits(uint16_t x) {
        int i;
        const unsigned long unit = 1;
        for(i=(sizeof(uint16_t)*8)-1; i>=0; i--)
          (x & ((unit)<<i))?putchar('1'):putchar('0');
     	printf("\n");
      }

      void show24bits(uint32_t x) {
        int i;
        const unsigned long unit = 1;
        for(i=(sizeof(uint32_t)*8)-8-1; i>=0; i--)
         (x & ((unit)<<i))?putchar('1'):putchar('0');
     	printf("\n");
      }

      void show32bits(uint32_t x) {
        int i;
        const unsigned long unit = 1;
        for(i=(sizeof(uint32_t)*8)-1; i>=0; i--)
         (x & ((unit)<<i))?putchar('1'):putchar('0');
     	printf("\n");
      }

      void show64bits(uint64_t x) {
        int i;
        const unsigned long unit = 1;
        for(i=(sizeof(uint64_t)*8)-1; i>=0; i--)
          (x & ((unit)<<i))?putchar('1'):putchar('0');
     	printf("\n");
      }

      bool printVFATdataBits(int event, const VFATData& vfat){
        if( event<0) return(false);
 	  cout << "\nReceived VFAT data word:" << endl;

          uint8_t   b1010 = (0xf000 & vfat.BC) >> 12;
          show4bits(b1010); cout << " BC     0x" << hex << (0x0fff & vfat.BC) << dec << endl;

          uint8_t   b1100 = (0xf000 & vfat.EC) >> 12;
          uint16_t   EC   = (0x0ff0 & vfat.EC) >> 4;
          uint8_t   Flag  = (0x000f & vfat.EC);
          show4bits(b1100); cout << " EC     0x" << hex << EC << dec << endl; 
          show4bits(Flag);  cout << " Flags " << endl;

          uint8_t   b1110 = (0xf000 & vfat.ChipID) >> 12;
          uint16_t ChipID = (0x0fff & vfat.ChipID);
          show4bits(b1110); cout << " ChipID 0x" << hex << ChipID << dec << " " << endl;

          /* cout << "     bxExp  0x" << hex << vfat.bxExp << dec << " " << endl;
	     cout << "     bxNum  0x" << hex << ((0xff00 & vfat.bxNum) >> 8) << "        SBit " << (0x00ff & vfat.bxNum) << endl;
           */
          cout << " <127:64>:: 0x" << std::setfill('0') << std::setw(8) << hex << vfat.msData << dec << endl;
          cout << " <63:0>  :: 0x" << std::setfill('0') << std::setw(8) << hex << vfat.lsData << dec << endl;
          cout << "     crc    0x" << hex << vfat.crc << dec << endl;

          //cout << " " << endl; show16bits(vfat.EC);

        return(true);
      };

  } //end namespace gem::readout
} //end namespace gem
#endif
