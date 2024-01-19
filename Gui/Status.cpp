/*********************************************************************

                        status.cpp

**********************************************************************/


#include "stdafx.h"


#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <ctype.h>

#include "drvrsym.h"
#include "resource.h"       // IDS_ERR_...
#include "driver.h"         // Fourtier driver specifics
#include "goodies.h"        // interface to goodies.cpp
#include "status.h"         // interface to status.cpp


/*********************************************************************************************/
/****************************** Usefull Fourtier classes *************************************/



/*********************************************************************************************/
/******************************* CStatus Implementation **************************************/

CStatus::CStatus( HDriverSym* pDevSym, PSTATUS_INST pStatusInst )
    : m_arrInput(  pDevSym->GetName() ), 
      m_arrOutput( pDevSym->GetName() )
{
    m_pDev  = pDevSym;
    m_pName = pDevSym->GetName();
    m_pStatusInst = pStatusInst; 
    m_pStatusInst->DevType = (UINT16)m_pDev->GetId();

    DebugString( "creating status device %s\n", m_pName );
}

CStatus::~CStatus()
{
    DebugString( "removing status device  %s\n", m_pName );
    delete m_pDev; 
}


CIOPointArray* CStatus::GetPointArray( UINT16 Point, SymAccess Access, UINT16& PointOffset )
{
    // Point        is the point id as defined in the RCD file. 
    // PointOffset  is the point offset (in bits) in the device space. 
    // PointOffset  is calculated using Point:  PointOffset = function( Point ); 
    // In our simple case: PointOffset = Point;
    
    CIOPointArray* pPointArray = NULL; 

    if( Access == saInput)
    {
        PointOffset = Point;
        pPointArray = &m_arrInput;
    }

    if( Access == saOutput)
    {
        PointOffset = Point;
        pPointArray = &m_arrOutput;
    }

    return  pPointArray;
}




// load config info into STATUS_INST
int CStatus::LoadDevice()
{
    int rc = SUCCESS;

    m_pStatusInst->Input.Size  = InputSizer();  // let runtime know how large is the  input image of this device
    m_pStatusInst->Output.Size = OutputSizer(); // let runtime know how large is the output image of this device

    m_arrInput.m_UiotSize  = m_pStatusInst->Input.Size;     // let the CIOPointArray know how large is
    m_arrOutput.m_UiotSize = m_pStatusInst->Output.Size;    // the area where points can be defined

    return rc;
}





int CStatus::Load()
{
    int rc = SUCCESS;

    if( rc == SUCCESS )
        rc = LoadDevice();

    if( rc == SUCCESS )
    {
        if( ListStart())    /* scan all points */
        {
            do
            {
                int rc1 = SUCCESS;
                
                HDriverSym* pPnt     = ListGet();
                SymAccess   Access   = pPnt->GetAccess();     // saInput, saOutput
                UINT16      Point    = pPnt->GetPoint();      // point offset in bits   (0xffff for strobe out)
                LPCSTR      pPntName = pPnt->GetName();       // user's name for that I/O tag
        
                UINT16         PointOffset = 0;
                CIOPointArray* pPointArray = GetPointArray( Point, Access, PointOffset );

                if( pPointArray != NULL )
                {
                    CIOPoint* pNewPoint = new CIOPoint( pPnt, PointOffset );

                    rc1 = pPointArray->Check( pNewPoint );

                    if( rc1 == SUCCESS )
                        pPointArray->Add( pNewPoint );
                    else
                        delete pNewPoint;               // also deletes *pPnt 
                }
                else
                {
                    rc1 = IDS_CP_INVALID_POINT;
                    Erop( rc1, pPntName, m_pName, "", "");
                    delete pPnt;
                }

                if( rc1 != SUCCESS )
                    rc = FAILURE;

            } while( ListNext());
        }
        ListEnd();

		//  If there are any input points defined here
        if( m_arrInput.GetSize() )
            m_pStatusInst->Input.bUsed = 1;     // set the 'used' flag
        else  
            m_arrInput.m_UiotSize =  0;         // otherwise do not reserve UIOT space

		// If there are any output points defined here
        if( m_arrOutput.GetSize() )
            m_pStatusInst->Output.bUsed = 1;    // set the 'used' flag
        else 
            m_arrOutput.m_UiotSize =  0;        // otherwise do not reserve UIOT space
    }

    DebugString( "CStatus::Load(). Name=%s, rc=%d\n", m_pName, rc );
    
    return rc;
}




UINT16 CStatus::InputSizer()
{
    ASSERT( m_pStatusInst->DevType == DEVICE_STATUS );
    UINT16 Size = sizeof(UINT8)*4;
    return Size;
}


UINT16 CStatus::OutputSizer()
{
    ASSERT( m_pStatusInst->DevType == DEVICE_STATUS );
    UINT16 Size = 0;
    return Size;
}


// if there are input tags defined --> returns the device input size
// else --> returns 0
UINT16 CStatus::GetUiotInputSize()  
{
    return m_arrInput.m_UiotSize;
}


// if there are output tags defined --> returns the device output size
// else --> returns 0
UINT16 CStatus::GetUiotOutputSize()
{
    return m_arrOutput.m_UiotSize;
}


UINT16 CStatus::GetInputSize()         // Device  input size
{
    return m_pStatusInst->Input.Size;
}


UINT16 CStatus::GetOutputSize()        // Device output size
{
    return m_pStatusInst->Output.Size;
}




//*************************************************************************************
// Called in NetPass2 when we have real UIOT offsets for Config, Input and Output areas
// Also increment the Name, PrgInput and PrgOutput global offsets. 
void CStatus::UpdateOffsets(  UINT32& ofsNames, 
                              UINT32& PrgInputOfs, 
                              UINT32& PrgOutputOfs, 
                              UINT32  IODelta )
{
	m_pStatusInst->ofsName = ofsNames;
    
    // increment the 'name' offset
    ofsNames += strlen( m_pName ) + 1;


	UINT32 PrgOfs;

    // Set the offset for the input image
	{
		PrgOfs = 0;

		// If there are any input tags defined here
		if( m_arrInput.m_UiotSize )
		{
			// store the associated offset in the drv image
            m_pStatusInst->Input.ofsUiot = PrgInputOfs + IODelta;
            if( !PrgOfs )
                PrgOfs = PrgInputOfs;
            // map all input points here, and increment.
			m_arrInput.UpdateOffsets( PrgInputOfs );
		}

	    // The following is for diagnostic purposes only. 
	    // Record in the map file where the input image of this device 
        // is mapped in the prg area
        m_pDev->PutInputOffset( PrgOfs );
	}

	// Set the offset for the output image
	{
		PrgOfs = 0;

		// If there are any output tags defined here
		if( m_arrOutput.m_UiotSize ) 
		{
			// store the associated offset in the drv image
            m_pStatusInst->Output.ofsUiot = PrgOutputOfs + IODelta;
            if( !PrgOfs )
                PrgOfs = PrgOutputOfs;
            // map all output points here, and increment.
			m_arrOutput.UpdateOffsets( PrgOutputOfs );
		}

	    // The following is for diagnostic purposes only. 
	    // Record in the map file where the input image of this device 
        // is mapped in the prg area
	    m_pDev->PutOutputOffset( PrgOfs );
	}

    // The following is a validity check only. 
    // If used, the compiler ensures the values reported by PutInputOffset and PutOutputOffset
    // are inside the progarm images of the driver. 
	ListUpdate( m_pDev );
}




BOOL CStatus::ListStart()
{
    return ListDeviceStart( m_pName );
}




/*************************************************************************/



/***************************** End of CStatus Implementation *********************************/
/*********************************************************************************************/




/************************** End of Usefull Fourtier classes **********************************/
/*********************************************************************************************/


