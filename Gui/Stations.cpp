/*********************************************************************

                        stations.cpp

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
#include "stations.h"       // interface to stations.cpp


/*********************************************************************************************/
/****************************** Usefull Fourtier classes *************************************/



/*********************************************************************************************/
/******************************* CStation Implementation **************************************/

CStation::CStation( HDriverSym* pDevSym )
    : m_arrModule( pDevSym->GetName() ), 
      m_arrInput(  pDevSym->GetName() ), 
      m_arrOutput( pDevSym->GetName() )
{
    m_pDev  = pDevSym;
    m_pName = pDevSym->GetName();
    memset( &m_StationInst, 0, sizeof(STATION_INST) );
    m_StationInst.DevType = (UINT16)m_pDev->GetId();

    DebugString( "creating station %s\n", m_pName );
}

CStation::~CStation()
{
    DebugString( "removing station  %s\n", m_pName );
    delete m_pDev; 
}


CIOPointArray* CStation::GetPointArray( UINT16 Point, SymAccess Access, UINT16& PointOffset )
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



// makes sure  m_DeviceInst  is can safely be used at run time
int CStation::DevValid( )
{
    int rc = SUCCESS;
    return rc;
}




// load config info into STATION_INST
int CStation::LoadDevice()
{
    int rc = SUCCESS;

    STATIONCONFIG StationCfg;   // where config data is loaded from symdb

    rc = ListConfigGet( StationCfg );

    if( rc == SUCCESS )
    {
        m_StationInst.Address   = StationCfg.Address;
        m_StationInst.bCritical = StationCfg.bCritical;
        m_StationInst.Sentinel  = RT3_SENTINEL;

        m_StationInst.Input.Size  = InputSizer();    // let runtime know how large is the  input image of this device
        m_StationInst.Output.Size = OutputSizer();   // let runtime know how large is the output image of this device

        m_arrInput.m_UiotSize    = m_StationInst.Input.Size;    // let the CIOPointArray know how large is
        m_arrOutput.m_UiotSize   = m_StationInst.Output.Size;   // the area where points can be defined
    }
    else
    {
        rc = IDS_CP_DEVICE_NOT_CONFIGURED;
        Erop( rc, m_pName, "", "", "" );
    }    


    return rc;
}





int CStation::Load()
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

        if( m_arrInput.GetSize() )                  // if there are any input points defined here
            m_StationInst.Input.bUsed   = 1;
        else                                        // else
            m_arrInput.m_UiotSize =  0;             //      do not reserve UIOT space

        if( m_arrOutput.GetSize() )                 // if there are any output points defined here
            m_StationInst.Output.bUsed   = 1;
        else                                        // else
            m_arrOutput.m_UiotSize =  0;            //      do not reserve UIOT space
    
    }

    DebugString( "CStation::Load(). Name=%s, rc=%d\n", m_pName, rc );
    
    return rc;
}




UINT16 CStation::InputSizer()
{
    ASSERT( m_StationInst.DevType == DEVICE_STATION );
    UINT16 Size = sizeof(UINT8)*32;   // was 4
    return Size;
}


UINT16 CStation::OutputSizer()
{
    ASSERT( m_StationInst.DevType == DEVICE_STATION );
    UINT16 Size = 0;
    return Size;
}


// if there are input tags defined --> returns the device input size
// else --> returns 0
UINT16 CStation::GetUiotInputSize()  
{
    return m_arrInput.m_UiotSize;
}


// if there are output tags defined --> returns the device output size
// else --> returns 0
UINT16 CStation::GetUiotOutputSize()
{
    return m_arrOutput.m_UiotSize;
}


UINT16 CStation::GetInputSize()         // Device  input size
{
    return m_StationInst.Input.Size;
}


UINT16 CStation::GetOutputSize()        // Device output size
{
    return m_StationInst.Output.Size;
}


int CStation::Check( CStation* pStation )      // makes sure "this" can live with pDevice
{
    int rc = SUCCESS;

    if( m_StationInst.Address == pStation->m_StationInst.Address )
    {
        rc = IDS_CP_SAME_DEVICE_ADDRESS;
    }

    return rc; 
}


//**************************************************************************************
//	Check to see if pModule is on "this" station.
int CStation::Check( CModule* pModule )
{
	int rc = SUCCESS;

    ASSERT( m_StationInst.Address == pModule->m_ModuleInst.Address );
	
	if( rc == SUCCESS )
        rc = m_arrModule.Check( pModule );

    return rc; 
}


//*************************************************************************************
// Called in NetPass2 when we have real UIOT offsets for Config, Input and Output areas
// Also increment the Name, ModuleInst, PrgInput and PrgOutput global offsets. 
void CStation::UpdateOffsets( UINT32& ofsModuleInst, 
                              UINT32& ofsNames, 
                              UINT32& PrgInputOfs, 
                              UINT32& PrgOutputOfs, 
                              UINT32  IODelta )
{
    m_StationInst.nModules      = m_arrModule.GetSize();    // how many modules on this station;
    m_StationInst.ofsModuleList = ofsModuleInst;
    m_StationInst.ofsName       = ofsNames;
    
    // increment the 'name' offset
    ofsNames += strlen( m_pName ) + 1;
    m_arrModule.UpdateOffsets( ofsNames, PrgInputOfs, PrgOutputOfs, IODelta );

    // increment the 'ModuleInst' offset
    ofsModuleInst += ( m_StationInst.nModules + 1 ) * sizeof( MODULE_INST );


	UINT32 PrgOfs;

    // Set the offset for the input image
	{
		PrgOfs = 0;

		// If there are any input tags defined here
		if( m_arrInput.m_UiotSize )
		{
			// store the associated offset in the drv image
			m_StationInst.Input.ofsUiot = PrgInputOfs + IODelta;
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
			m_StationInst.Output.ofsUiot = PrgOutputOfs + IODelta;
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



int  CStation::ListConfigGet( STATIONCONFIG& Stationconfig )
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
            pHeader->DevcfgId        == STATIONCONFIG_ID               &&  
            pHeader->DevcfgVersMajor == ( STATIONCONFIG_VERS >> 16 )   &&  
            pHeader->DevcfgVersMinor >= ( STATIONCONFIG_VERS & 0xffff )
          )
        {
            Stationconfig = Devconfig.Station;
        }
        else
            rc = FAILURE;
    }

    return rc;
}


BOOL CStation::ListStart()
{
    return ListDeviceStart( m_pName );
}


//******************************************************************
// Insert a CModule into the list sorted by and Slot #
void CStation::InsertInOrder( CModule* pModule )
{
	int	Index;
	int	MaxModule = m_arrModule.GetSize();
	CModule* pModuleScan;

	// Find the location to insert this Device
	for (Index = 0; Index < MaxModule && MaxModule; Index++)
	{
		pModuleScan = m_arrModule.GetAt(Index);
		if (*pModule < *pModuleScan)
			break;
	}

	// Put the module into a sorted spot
    m_arrModule.InsertAt( Index, pModule );
}


//******************************************************************
// Add a CModule to the list
void CStation::Add( CModule* pModule )
{
	// Put the module into the list
    m_arrModule.Add( pModule );
}



bool CStation::operator<(const CStation& rStation)
{
	return m_StationInst.Address < rStation.m_StationInst.Address;
}


/*************************************************************************/

CStationArray::CStationArray( LPCSTR pNetworkName ) 
{
    SetSize( 0, MAX_DEVICES );
    m_pNetName = pNetworkName;
    DebugString( "creating CStationArray for %s\n", m_pNetName );
}

CStationArray::~CStationArray()  
{ 
    DebugString( "removing CStationArray\n", "" );
    for(int i = 0; i < GetSize(); i++ )
        delete GetAt( i );
    RemoveAll(); 
}


// Makes sure pStation can live with all previousely defined Stations. 
// Verify that there aren't any collisions.
int CStationArray::Check( CStation* pStation )
{
	int rc = SUCCESS;

	for(int StationIndex = 0; StationIndex < GetSize() && (rc == SUCCESS) ;StationIndex++)
	{
		CStation* pStationList = GetAt( StationIndex );
		rc  = pStation->Check( pStationList );

		if( rc != SUCCESS )
			Erop( rc, pStation->m_pName, pStationList->m_pName, m_pNetName, pStation->m_StationInst.Address );
	}

	return rc; 
}   // End of CStationArray::Check(Station*)



// Find the CStation with the specified Address
CStation*  CStationArray::FindStation( CModule* pModule ) const
{
	// Check all Stations until one is found which matchs
	for (int StationIndex = 0; StationIndex < GetSize() ; StationIndex++)
	{
		CStation* pStation = GetAt( StationIndex );
        if( pStation->m_StationInst.Address == pModule->m_ModuleInst.Address )
            return pStation;
	}

    return NULL;
}


// makes sure there is a Station for pModule
CStation* CStationArray::Check( CModule* pModule )
{
    CStation* pStation = FindStation( pModule );

    if( pStation )
    {
		int rc  = pStation->Check( pModule );
	    if( rc != SUCCESS )
            pStation = NULL;
    }
    else
		Erop( IDS_CP_NO_STATION, pModule->m_pName, m_pNetName, "", pModule->m_ModuleInst.Address );

	return pStation; 
}   // End of CStationArray::Check(CModule*)


//*************************************************************************************
// Called in NetPass2 when we have real UIOT offsets for Config, Input and Output areas
// Also increment the Name, ModuleInst, PrgInput and PrgOutput global offsets. 
void CStationArray::UpdateOffsets(  UINT32& ofsModuleInst, 
                                    UINT32& ofsNames, 
                                    UINT32& PrgInputOfs, 
                                    UINT32& PrgOutputOfs, 
                                    UINT32  IODelta )
{
    for( int StationIndex = 0; StationIndex < GetSize() ; StationIndex++ )
    {
        CStation* pStation = GetAt( StationIndex );
        pStation->UpdateOffsets( ofsModuleInst, ofsNames, PrgInputOfs, PrgOutputOfs, IODelta );
    }
}


/***************************** End of CStation Implementation *********************************/
/*********************************************************************************************/




/************************** End of Usefull Fourtier classes **********************************/
/*********************************************************************************************/


