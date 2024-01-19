/*********************************************************************

                        modules.h

**********************************************************************/



#ifndef __MODULES_H__
#define __MODULES_H__




#include "points.h"



/****************************************************************************************/
/**************************** Usefull Fourtier classes **********************************/



class CModule : public CObject, public CDrvIntf
{
    private:
        CIOPointArray m_arrInput;
        CIOPointArray m_arrOutput;
        HDriverSym*   m_pDev;

    public:
        LPCSTR        m_pName;        // this name is stored on the heap in the *m_pDev block
        MODULE_INST   m_ModuleInst;
                        
    private:
        int             DevValid();
        int             LoadDevice();
        UINT16          InputSizer(  const MODULECONFIG& ModuleCfg ) const;
        UINT16          OutputSizer( const MODULECONFIG& ModuleCfg ) const;
        CIOPointArray*  GetPointArray( UINT16 Point, SymAccess Access, UINT16& PointOffset );

        int  ListConfigGet( MODULECONFIG& Moduleconfig );
        BOOL ListStart();

    public:
        int     Load();
        int     Check( CModule* pModule );
        void    UpdateOffsets(  UINT32& ofsNames, 
                                UINT32& PrgInputOfs, 
                                UINT32& PrgOutputOfs, 
                                UINT32  IODelta );

        // Accessing elements
        UINT16  GetInputSize();         // Device  input size
        UINT16  GetOutputSize();        // Device output size
        UINT16  GetUiotInputSize();     // Device  input size or 0 if no  input tags defined
        UINT16  GetUiotOutputSize();    // Device output size or 0 if no output tags defined
		bool	operator<(const CModule& rModule);

        CModule( HDriverSym* pDevSym );
        virtual ~CModule();

        void AssertValid() const { return; }
};


/******************************************************************************/

class CModuleArray : public CPtrArray, public CDrvIntf
{   
    private:
        LPCSTR m_pStationName;  // For error messages only

    public:
        int  Check( CModule* pModule );
        void UpdateOffsets( UINT32& ofsNames, 
                            UINT32& PrgInputOfs, 
                            UINT32& PrgOutputOfs, 
                            UINT32  IODelta );
        
        // Accessing elements
        CModule*  GetAt(int nIndex) const  {return (CModule* )CPtrArray::GetAt(nIndex);}
        CModule*& ElementAt(int nIndex)    {return (CModule*&)CPtrArray::ElementAt(nIndex);}

        // overloaded operator helpers
        CModule*  operator[](int nIndex) const {return GetAt(nIndex);}
        CModule*& operator[](int nIndex)       {return ElementAt(nIndex);}

        CModuleArray( LPCSTR pStationName );
        virtual ~CModuleArray();
};



/************************** End of Usefull Fourtier classes **********************************/
/*********************************************************************************************/


#endif      /* __MODULES_H__  */


