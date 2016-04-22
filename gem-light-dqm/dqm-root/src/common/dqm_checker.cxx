class GEMDQMchecker
{
  public:
    GEMDQMchecker(){m_errorCode = 0;}
    virtual ~GEMDQMchecker(){}
    bool check(){return m_check;}
    int getErrorCode(){return m_errorCode;}
  protected:
    int m_errorCode;
    bool m_check;
};
