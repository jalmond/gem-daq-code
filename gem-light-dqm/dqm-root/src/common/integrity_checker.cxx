#include "dqm_checker.cxx"
#include <Event.h>
class GEMDQMIntegrityChecker: public GEMDQMchecker
{
  public:
    GEMDQMIntegrityChecker(VFATdata & vfat_)
    {
      m_vfat = &vfat_;
    }
    ~GEMDQMIntegrityChecker(){}
    bool check()
    {
      if (m_vfat->b1010() != 0xa) {
        m_errorCode = 1010;
        m_check = false;
      } 
      if (m_vfat->b1100() != 0xc){
        m_errorCode += 1100;
        m_check = false;
      } 
      if (m_vfat->b1110() != 0xe){
        m_errorCode += 1110;
        m_check = false;
      }
      return m_check;
    }
  private:
    VFATdata *m_vfat;
};
