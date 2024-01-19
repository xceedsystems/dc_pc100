/*********************************************************************

                        status.h

**********************************************************************/


#ifndef __STATUS_H__
#define __STATUS_H__



#include "points.h"



/****************************************************************************************/
/**************************** Usefull Fourtier classes **********************************/



class CStatus : public CObject, public CDrvIntf
{
    private:
        CIOPointArray m_arrInput;
        CIOPointArray m_arrOutput;
        HDriverSym*   m_pDev;

    public:
        PSTATUS_INST  m_pStatusInst;    // pointer to DRIVER_INST where the unique STATUS_INST live
        LPCSTR        m_pName;          // this name is stored on the heap in the *m_pDev block
                        
    private:
        int             LoadDevice();
        UINT16          InputSizer();
        UINT16          OutputSizer();
        CIOPointArray*  GetPointArray( UINT16 Point, SymAccess Access, UINT16& PointOffset );

        BOOL ListStart();

    public:
        int     Load();
        void    UpdateOffsets( UINT32& ofsNames, 
                               UINT32& PrgInputOfs, 
                               UINT32& PrgOutputOfs, 
                               UINT32  IODelta );


        // Accessing elements
        UINT16  GetInputSize();         // Device  input size
        UINT16  GetOutputSize();        // Device output size
        UINT16  GetUiotInputSize();     // Device  input size or 0 if no  input tags defined
        UINT16  GetUiotOutputSize();    // Device output size or 0 if no output tags defined

        CStatus( HDriverSym* pDevSym, PSTATUS_INST pStatusInst );
        virtual ~CStatus();

        void AssertValid() const { return; }
};



/************************** End of Usefull Fourtier classes **********************************/
/*********************************************************************************************/


#endif      /* __STATUS_H__  */


