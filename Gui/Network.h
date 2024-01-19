/*********************************************************************

                        network.h

**********************************************************************/




#ifndef __NETWORK_H__
#define __NETWORK_H__




#include "status.h"
#include "stations.h"


/****************************************************************************************/
/**************************** Usefull Fourtier classes **********************************/





class CNetwork : public CObject, public CDrvIntf
{
    private:
        UINT32 m_uConfigTotal;      // necessary space for our run time work data
        UINT32 m_uInputTotal;       // necessary space for all device input  images
        UINT32 m_uOutputTotal;      // necessary space for all device output images
        UINT32 m_uNameTotal;        // Space to store all device names

        UINT32 m_uStationInstTotal; // Space to store all STATION_INST
        UINT32 m_uModuleInstTotal;  // Space to store all MODULE_INST
        UINT32 m_uStationCnt;       // # of defined Stations
        UINT32 m_uModuleCnt;        // # of defined Modules

        CStatus*      m_pStatus;    // pointer to the associated status device
        CStationArray m_arrStation; // stationss belonging to this network
        CString       m_ConfigFile; // CString needed to store the names of recorded networks


    public:
        CString      m_pName;       // CString needed to store the names of recorded networks
        DRIVER_INST  m_NetInst;     // Network runtime data

    private:
        int  IsStatus(  UINT32 DevType );
        int  IsStation( UINT32 DevType );
        int  IsModule(  UINT32 DevType );
        int  IsOKPortAddress();
        int  IsOKDPR();

        int  LoadStation( CStation* pNewStation );
        int  LoadModule(  CModule*  pNewModule  );
        int  LoadStatus(  CStatus*  pNewStatus  );

        int  LoadNetwork();         // copies data from NETCONFIG to DRIVER_INST
        int  NetValid( );           // makes sure DRIVER_INST can be safely used at run time
	    void InsertInOrder( CStation* pStation );
        void Add(           CStation *pStation );

        int  ListConfigGet( NETCONFIG& Netconfig, UINT16& rBytesRet );
        int  ListConfigPut( NETCONFIG& Netconfig );
        BOOL ListStart();

    public:
        int  Load();                            // loads from symdb all info related to this network
        int  Check( CNetwork* pNetwork );       // makes sure "this" can live with pNetwork
        void ReportSizes( UINT32& rConfigTotal, UINT32& rInputTotal, UINT32& rOutputTotal );
        void UpdateOffsets( UINT32 CfgOfs, UINT32 InputOfs, UINT32 OutputOfs, UINT32 IODelta );
        void WriteConfigData( HStreamAbs& rFileHandle );
        
        // Accessing elements

        CNetwork( LPCSTR NetName, LPCSTR ProjPath );
        virtual ~CNetwork();
        
        void AssertValid() const { return; }
};


class CNetworkArray : public CPtrArray
{   
    private:
        DRIVER_KEY  m_NetKey;

    public:
        CString     m_ProjPath;

        int Check( CNetwork* pNetwork );

        // Accessing elements
        CNetwork*  GetAt(int nIndex) const  {return (CNetwork* )CPtrArray::GetAt(nIndex);}
        CNetwork*& ElementAt(int nIndex)    {return (CNetwork*&)CPtrArray::ElementAt(nIndex);}

        CNetwork*  GetNetwork( LPCSTR lpNetName );

        // overloaded operator helpers
        CNetwork*  operator[](int nIndex) const {return GetAt(nIndex);}
        CNetwork*& operator[](int nIndex)       {return ElementAt(nIndex);}

        CNetworkArray( DRIVER_KEY NetKey, LPCSTR ProjPath );
        virtual ~CNetworkArray();
};



/************************** End of Usefull Fourtier classes **********************************/
/*********************************************************************************************/


#endif      /* __NETWORK_H__  */


