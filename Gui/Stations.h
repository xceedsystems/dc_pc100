/*********************************************************************

                        stations.h

**********************************************************************/


#ifndef __STATIONS_H__
#define __STATIONS_H__



#include "modules.h"



/****************************************************************************************/
/**************************** Usefull Fourtier classes **********************************/



class CStation : public CObject, public CDrvIntf
{
    private:
        CIOPointArray m_arrInput;
        CIOPointArray m_arrOutput;
        HDriverSym*   m_pDev;

    public:
        LPCSTR        m_pName;          // this name is stored on the heap in the *m_pDev block
        STATION_INST  m_StationInst;
        CModuleArray  m_arrModule;      // modules belonging to this station
                        
    private:
        int             DevValid();
        int             LoadDevice();
        UINT16          InputSizer();
        UINT16          OutputSizer();
        CIOPointArray*  GetPointArray( UINT16 Point, SymAccess Access, UINT16& PointOffset );

        int  ListConfigGet( STATIONCONFIG& Stationconfig );
        BOOL ListStart();

    public:
        int     Load( );
        int     Check( CStation* pStation );
        int     Check( CModule*  pModule );
	    void    InsertInOrder( CModule* pModule );
	    void    Add(           CModule* pModule );
        void    UpdateOffsets( UINT32& ofsModuleInst, 
                               UINT32& ofsNames, 
                               UINT32& PrgInputOfs, 
                               UINT32& PrgOutputOfs, 
                               UINT32  IODelta );


		bool	operator<(const CStation& rStation);

        // Accessing elements
        UINT16  GetInputSize();         // Device  input size
        UINT16  GetOutputSize();        // Device output size
        UINT16  GetUiotInputSize();     // Device  input size or 0 if no  input tags defined
        UINT16  GetUiotOutputSize();    // Device output size or 0 if no output tags defined

        CStation( HDriverSym* pDevSym );
        virtual ~CStation();

        void AssertValid() const { return; }
};


/******************************************************************************/

class CStationArray : public CPtrArray, public CDrvIntf
{   
    private:
        LPCSTR    m_pNetName;                               // For error messages only
        CStation* FindStation( CModule* pModule ) const;    // Find the Station with the specified Address

    public:
        int       Check( CStation* pStation );
        CStation* Check( CModule*  pModule );

        void UpdateOffsets( UINT32& ofsModuleInst, 
                            UINT32& ofsNames, 
                            UINT32& PrgInputOfs, 
                            UINT32& PrgOutputOfs, 
                            UINT32  IODelta );
        
        // Accessing elements
        CStation*  GetAt(int nIndex) const  {return (CStation* )CPtrArray::GetAt(nIndex);}
        CStation*& ElementAt(int nIndex)    {return (CStation*&)CPtrArray::ElementAt(nIndex);}

        // overloaded operator helpers
        CStation*  operator[](int nIndex) const {return GetAt(nIndex);}
        CStation*& operator[](int nIndex)       {return ElementAt(nIndex);}

        CStationArray( LPCSTR pNetName );
        virtual ~CStationArray();
};



/************************** End of Usefull Fourtier classes **********************************/
/*********************************************************************************************/


#endif      /* __STATIONS_H__  */


