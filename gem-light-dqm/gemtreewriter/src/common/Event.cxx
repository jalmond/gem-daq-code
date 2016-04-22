/**
* GEM Event ROOT class
*/
/*! \file */
/*! 
////////////////////////////////////////////////////////////////////////
//
//                          GEM Event classes
//                       =======================
//
//  The VFATdata class for the GEM detector
//    private:
//        uint8_t b1010;                    // 1010:4 Control bits, shoud be 1010
//        uint16_t BC;                      // Bunch Crossing number, 12 bits
//        uint8_t b1100;                    // 1100:4, Control bits, shoud be 1100
//        uint16_t EC;                      // Event Counter, 8 bits
//        uint8_t Flag;                     // Control Flags: 4 bits, Hamming Error/AFULL/SEUlogic/SUEI2C
//        uint8_t b1110;                    // 1110:4 Control bits, shoud be 1110
//        uint16_t ChipID;                  // Chip ID, 12 bits
//        uint64_t lsData;                  // channels from 1to64 
//        uint64_t msData;                  // channels from 65to128
//        uint16_t crc;                     // Check Sum value, 16 bits
//
//  The GEBdata class represents the data structure coming from a single GEB board.
//
//    private:
//        uint32_t ZSFlag;                // ZeroSuppresion flags, 24 bits
//        uint16_t ChamID;                // Chamber ID, 12 bits
//        uint32_t sumVFAT;               // Rest part of the header, reserved for the moment
//        uint64_t fRunHeader;            // RunType:4 VT1:8 VT2:8 minTH:8 maxTH:8 Step:8 - Threshold Scan Header
//        std::vector<VFATdata> vfats;
//        uint16_t OHcrc;                 // OH Check Sum, 16 bits
//        uint16_t OHwCount;              // OH Counter, 16 bits
//        uint16_t ChamStatus;            // Chamber Status, 16 bits
//        uint16_t GEBres;                // Reserved part of trailer
//
//  The Event class is a naive/simple example of a GEM event structure.
//
//    private:
//        EventHeader    fEvtHdr;
//        
//        //uint64_t header1;             // AmcNo:4      0000:4     LV1ID:24   BXID:12     DataLgth:20 
//        uint8_t AmcNo;
//        uint8_t b0000;
//        uint32_t LV1ID;                    // What is this? Which var format should be used?
//        uint16_t BXID;                     // Is it Bunch crossing ID? Should it be Int_t?
//        uint32_t DataLgth;                 // What is this?
//        //uint64_t header2;             // User:32      OrN:16     BoardID:16
//        uint16_t OrN;                   // What is this?
//        uint16_t BoardID;
//        //uint64_t header3;               // DAVList:24   BufStat:24 DAVCount:5 FormatVer:3 MP7BordStat:8 
//        uint32_t DAVList;
//        uint32_t BufStat;
//        uint8_t DAVCount;
//        uint8_t FormatVer;
//        uint8_t MP7BordStat;
//
//        std::vector<GEBdata> gebs;      // Should we use vector or better have TClonesArray here?
//        //uint64_t trailer2;            // EventStat:32 GEBerrFlag:24  
//        uint32_t EventStat;
//        uint32_t GEBerrFlag;
//        //uint64_t trailer1;            // crc:32       LV1IDT:8   0000:4     DataLgth:20 
//        uint32_t crc;
//        uint8_t LV1IDT;
//        uint8_t b0000T;
//        uint32_t DataLgthT;
//  
//  The EventHeader class has 3 data members (integers):
//     public:
//        Int_t          fEvtNum;
//        Int_t          fRun;
//        Int_t          fDate;
//
////////////////////////////////////////////////////////////////////////
  \author Mykhailo.Dalchenko@cern.ch
*/
#include "RVersion.h"
#include "TRandom.h"
#include "TDirectory.h"
#include "TProcessID.h"

#include "Event.h"

ClassImp(EventHeader)
ClassImp(Event)

Event::Event()
{
   // Create an Event object.
   Clear();
}

//______________________________________________________________________________
Event::~Event()
{
   Clear();
}

//______________________________________________________________________________
void Event::Build(bool isEventGood_)
{
    //Save current Object count
    Int_t ObjectNumber = TProcessID::GetObjectCount();
    Clear();
    fisEventGood = isEventGood_;

    //Restore Object count 
    //To save space in the table keeping track of all referenced objects
    //we assume that our events do not address each other. We reset the 
    //object count to what it was at the beginning of the event.
    TProcessID::SetObjectCount(ObjectNumber);
}  

//______________________________________________________________________________
void Event::SetHeader(Int_t i, Int_t run, Int_t date)
{
   fEvtHdr.Set(i, run, date);
}

//______________________________________________________________________________
void Event::Clear()
{
    famc13s.clear();
    fisEventGood = false;
}
