/*********************************************************************

                        modules.cpp

**********************************************************************/




#include "stdafx.h"


#include <windows.h>
#include <windowsx.h>
#include <ctype.h>

#include "drvrsym.h"
#include "resource.h"       // IDS_ERR_...
#include "driver.h"         // Fourtier driver specifics
#include "goodies.h"        // interface to goodies.cpp
#include "modules.h"        // interface to modules.cpp



/*********************************************************************************************/
/****************************** Usefull Fourtier classes *************************************/



/*********************************************************************************************/
/******************************* CModule Implementation **************************************/

CModule::CModule( HDriverSym* pDevSym )
    : m_arrInput( pDevSym->GetName() ), m_arrOutput( pDevSym->GetName() )
{
    m_pDev  = pDevSym;
    m_pName = pDevSym->GetName();
    memset( &m_ModuleInst, 0, sizeof(MODULE_INST) );
    m_ModuleInst.DevType = (UINT16)m_pDev->GetId();

    DebugString( "creating module %s\n", m_pName );
}

CModule::~CModule()
{
    DebugString( "removing module  %s\n", m_pName );
    delete m_pDev; 
}


CIOPointArray* CModule::GetPointArray( UINT16 Point, SymAccess Access, UINT16& PointOffset )
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



// makes sure  m_ModuleInst  is can safely be used at run time
int CModule::DevValid( )
{
    int rc = SUCCESS;
    return rc;
}




// load config info into MODULE_INST
int CModule::LoadDevice()
{
    int rc = SUCCESS;

    MODULECONFIG ModuleCfg;   // where config data is loaded from symdb

    rc = ListConfigGet( ModuleCfg );

    if( rc == SUCCESS )
    {

        m_ModuleInst.Address    = ModuleCfg.Address;
        m_ModuleInst.Slot       = ModuleCfg.Slot;
        m_ModuleInst.bCritical  = ModuleCfg.bCritical;
        m_ModuleInst.Sentinel   = RT3_SENTINEL;

        int DevType = m_ModuleInst.DevType;

        if( DevType == DEVICE_MODULE_GENERIC )
        {
            UINT32 ProductCode;
            int rc1 = ConvertAtoI( (LPCSTR)ModuleCfg.ProductCode, ProductCode );

            if( rc1 == SUCCESS && ProductCode <= 0xffff )
                m_ModuleInst.ProductCode = (UINT16)ProductCode;
            else
            {
                rc = IDS_CP_DEVCONFIG_INVALID_PCODE;
                Erop( rc, m_pName, (LPCSTR)ModuleCfg.ProductCode, 0, 0);
            }

            m_ModuleInst.bSwap  = ModuleCfg.bSwapBytes;
        }
        else if( DevType == DEVICE_MODULE_4W_INPUT )
        {
            m_ModuleInst.ProductCode = DevType;   // for ex.
            m_ModuleInst.bSwap       = 0;   // 
        }
        else if( DevType == DEVICE_MODULE_4W_OUTPUT )
        {
            m_ModuleInst.ProductCode = DevType;   // for ex.
            m_ModuleInst.bSwap       = 0;   // 
        }
        else if( DevType == DEVICE_MODULE_4W_IORO )
        {
            m_ModuleInst.ProductCode = DevType ;   // for ex.
            m_ModuleInst.bSwap       = 0;   // 
        }
        else if( DevType == DEVICE_MODULE_4W_IANDO )
        {
            m_ModuleInst.ProductCode = DevType;   // for ex.
            m_ModuleInst.bSwap       = 0;   // 
        }
//
		else if( DevType == DEVICE_MODULE_2W_IANDO )
        {
            m_ModuleInst.ProductCode = DevType;   // for ex.
            m_ModuleInst.bSwap       = 0;   // 
        }
		else if( DevType == DEVICE_MODULE_1W_IANDO )
        {
            m_ModuleInst.ProductCode = DevType;   // for ex.
            m_ModuleInst.bSwap       = 0;   // 
        }
		else if( DevType == DEVICE_MODULE_1B_IANDO )
        {
            m_ModuleInst.ProductCode = DevType;   // for ex.
            m_ModuleInst.bSwap       = 0;   // 
        }
		else if( DevType == DEVICE_MODULE_2W_IORO )
        {
            m_ModuleInst.ProductCode = DevType;   // for ex.
            m_ModuleInst.bSwap       = 0;   // 
        }
		else if( DevType == DEVICE_MODULE_1W_IORO )
        {
            m_ModuleInst.ProductCode = DevType;   // for ex.
            m_ModuleInst.bSwap       = 0;   // 
        }
		else if( DevType == DEVICE_MODULE_1B_IORO )
        {
            m_ModuleInst.ProductCode = DevType;   // for ex.
            m_ModuleInst.bSwap       = 0;   // 
        }

		else 
        {
            m_ModuleInst.ProductCode = DevType;   // for ex.
            m_ModuleInst.bSwap       = 0;   // 
        }

//
        m_ModuleInst.Input.Size  = InputSizer( ModuleCfg );     // let runtime know how large is the  input image of this device
        m_ModuleInst.Output.Size = OutputSizer( ModuleCfg );    // let runtime know how large is the output image of this device

        m_arrInput.m_UiotSize    = m_ModuleInst.Input.Size;     // let the CIOPointArray know how large is
        m_arrOutput.m_UiotSize   = m_ModuleInst.Output.Size;    // the area where points can be defined
    }
    else
    {
        rc = IDS_CP_DEVICE_NOT_CONFIGURED;
        Erop( rc, m_pName, "", "", "" );
    }    


    return rc;
}




int CModule::Load()
{
    int rc = SUCCESS;

    if( rc == SUCCESS )
        rc = LoadDevice();

    if(rc == SUCCESS)
        rc = DevValid();

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
        
                UINT16       PointOffset = 0;
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
            m_ModuleInst.Input.bUsed = 1;   // set the 'used' flag
        else  
            m_arrInput.m_UiotSize =  0;     // otherwise do not reserve UIOT space

		// If there are any output points defined here
        if( m_arrOutput.GetSize() )
            m_ModuleInst.Output.bUsed = 1;  // set the 'used' flag
        else 
            m_arrOutput.m_UiotSize =  0;    // otherwise do not reserve UIOT space

    }

    DebugString( "CModule::Load(). Name=%s, rc=%d\n", m_pName, rc );
    
    return rc;
}






UINT16 CModule::InputSizer( const MODULECONFIG& ModuleCfg ) const
{
    UINT16 Size = 0;

    switch( m_ModuleInst.DevType )
    {
        case DEVICE_MODULE_GENERIC:
		case DEVICE_MODULEIO_GENERIC:
                    Size = ModuleCfg.InputSize;  
                    break;

        case DEVICE_MODULE_4W_INPUT:
        case DEVICE_MODULE_4W_IORO:
        case DEVICE_MODULE_4W_IANDO:
                    Size = sizeof(UINT16)*4;
					break;

		case DEVICE_MODULE_2W_INPUT:
		case DEVICE_MODULEIO_2W_INPUT:
		case DEVICE_MODULEIO_2W_IORO :
		case DEVICE_MODULEIO_2W_IANDO :
                    Size = sizeof(UINT16)*2;
					break;


		case DEVICE_MODULE_1W_INPUT:
		case DEVICE_MODULEIO_1W_INPUT:
		case DEVICE_MODULEIO_1W_IORO :		
		case DEVICE_MODULEIO_1W_IANDO :
                    Size = sizeof(UINT16);
					break;

		case DEVICE_MODULE_1B_INPUT:
		case DEVICE_MODULEIO_1B_INPUT:
		case DEVICE_MODULEIO_1B_IORO :
		case DEVICE_MODULEIO_1B_IANDO :
                    Size = sizeof(UINT8);
					break;





    }

    return Size;
}


UINT16 CModule::OutputSizer(  const MODULECONFIG& ModuleCfg ) const
{
    UINT16 Size = 0;

    switch( m_ModuleInst.DevType )
    {
        case DEVICE_MODULE_GENERIC:
		case DEVICE_MODULEIO_GENERIC:

                    Size = ModuleCfg.OutputSize;  
                    break;

        case DEVICE_MODULE_4W_OUTPUT:
        case DEVICE_MODULE_4W_IORO:
        case DEVICE_MODULE_4W_IANDO:
					Size = sizeof(UINT16)*4;
					break;

		case DEVICE_MODULEIO_2W_OUTPUT:
		case DEVICE_MODULE_2W_OUTPUT:
		case DEVICE_MODULEIO_2W_IORO :
		case DEVICE_MODULEIO_2W_IANDO :
					Size = sizeof(UINT16)*2;
					break;


		case DEVICE_MODULE_1W_OUTPUT:
		case DEVICE_MODULEIO_1W_OUTPUT:
		case DEVICE_MODULEIO_1W_IORO :		
		case DEVICE_MODULEIO_1W_IANDO :
					Size = sizeof(UINT16);
					break;



		case DEVICE_MODULEIO_1B_OUTPUT:
		case DEVICE_MODULE_1B_OUTPUT:
		case DEVICE_MODULEIO_1B_IORO :
		case DEVICE_MODULEIO_1B_IANDO :
                    Size = sizeof(UINT8);
					break;


    }
    return Size;
}


// if there are input tags defined --> returns the device input size
// else --> returns 0
UINT16 CModule::GetUiotInputSize()  
{
    return m_arrInput.m_UiotSize;
}


// if there are output tags defined --> returns the device output size
// else --> returns 0
UINT16 CModule::GetUiotOutputSize()
{
    return m_arrOutput.m_UiotSize;
}


UINT16 CModule::GetInputSize()         // Device  input size
{
    return m_ModuleInst.Input.Size;
}


UINT16 CModule::GetOutputSize()        // Device output size
{
    return m_ModuleInst.Output.Size;
}


// makes sure "this" can live with pModule
int CModule::Check( CModule* pModule )
{
    int rc = SUCCESS;

    if( m_ModuleInst.Address != pModule->m_ModuleInst.Address )
    {
        rc = FAILURE;
		Erop( IDS_CP_SOFTWARE_FAULT, "CModule::Check", "", "", "" );
    }

    if( m_ModuleInst.Slot == pModule->m_ModuleInst.Slot )
    {
        rc = FAILURE;
        Erop( IDS_CP_SAME_SLOT, pModule->m_pName, m_pName, m_ModuleInst.Slot, m_ModuleInst.Address );
    }

    return rc; 
}


//*************************************************************************************
// Called in NetPass2 when we have real UIOT offsets for Config, Input and Output areas
// Also increment the Name, PrgInput and PrgOutput global offsets. 
void CModule::UpdateOffsets( UINT32& ofsNames, UINT32& PrgInputOfs, UINT32& PrgOutputOfs, UINT32 IODelta )
{
	UINT32 PrgOfs;

	m_ModuleInst.ofsName = ofsNames;
    
    // increment the 'name' offset
    ofsNames += strlen( m_pName ) + 1;

	// Set the offset for the input image
	{
		PrgOfs = 0;

		// If there are any input tags defined here
		if( m_arrInput.m_UiotSize )
		{
			// store the associated offset in the drv image
			m_ModuleInst.Input.ofsUiot = PrgInputOfs + IODelta;
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
			m_ModuleInst.Output.ofsUiot = PrgOutputOfs + IODelta;
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




int  CModule::ListConfigGet( MODULECONFIG& Moduleconfig )
{
    int       rc = SUCCESS;
    UINT16    Size;
    DEVCONFIG Devconfig;

    memset( &Devconfig, 0, sizeof(DEVCONFIG) );
    
    rc = ListDeviceConfigGet( m_pName, &Devconfig, sizeof(DEVCONFIG), Size );

    if( rc == SUCCESS )
    {
        if( Size != sizeof(DEVCONFIG) )
            rc = FAILURE;
    }

    if( rc == SUCCESS )
    {
        DEVCFG_HD* pHeader = &Devconfig.Header;

        if( pHeader->NetcfgId        == NETCONFIG_ID               &&  
            pHeader->DevcfgId        == MODULECONFIG_ID               &&  
            pHeader->DevcfgVersMajor == ( MODULECONFIG_VERS >> 16 )   &&  
            pHeader->DevcfgVersMinor >= ( MODULECONFIG_VERS & 0xffff )
          )
        {
            Moduleconfig = Devconfig.Module;
        }
        else
            rc = FAILURE;
    }

    return rc;
}


BOOL CModule::ListStart()
{
    return ListDeviceStart( m_pName );
}


bool CModule::operator<(const CModule& rModule)
{
	if( m_ModuleInst.Address == rModule.m_ModuleInst.Address )
        return m_ModuleInst.Slot < rModule.m_ModuleInst.Slot;
	else
        return m_ModuleInst.Address < rModule.m_ModuleInst.Address;
}


/*************************************************************************/

CModuleArray::CModuleArray( LPCSTR pStationName ) 
{
    SetSize( 0, MAX_DEVICES );
    m_pStationName = pStationName;
    DebugString( "creating CModuleArray for station %s\n", m_pStationName );
}

CModuleArray::~CModuleArray()  
{ 
    DebugString( "removing CModuleArray\n", "" );
    for(int i = 0; i < GetSize(); i++ )
        delete GetAt( i );
    RemoveAll(); 
}


// makes sure  pModule can live with all previousely defined modules
int CModuleArray::Check( CModule* pModule )
{
    int    rc    = SUCCESS;

    for( int ModuleIndex = 0; ModuleIndex < GetSize() && (rc == SUCCESS) ; ModuleIndex++ )
    {
        CModule* pMod1 = GetAt( ModuleIndex );
        rc = pMod1->Check( pModule );
    }

    return rc; 
}


//*************************************************************************************
// Called in NetPass2 when we have real UIOT offsets for Config, Input and Output areas
// Also increment the Name, PrgInput and PrgOutput global offsets. 
void CModuleArray::UpdateOffsets( UINT32& ofsNames, UINT32& PrgInputOfs, UINT32& PrgOutputOfs, UINT32 IODelta )
{
    for( int ModuleIndex = 0; ModuleIndex < GetSize() ; ModuleIndex++ )
    {
        CModule* pModule = GetAt( ModuleIndex );
        pModule->UpdateOffsets( ofsNames, PrgInputOfs, PrgOutputOfs, IODelta );
    }
}


/***************************** End of CModule Implementation *********************************/
/*********************************************************************************************/




/************************** End of Usefull Fourtier classes **********************************/
/*********************************************************************************************/


