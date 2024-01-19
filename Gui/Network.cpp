/*********************************************************************

                        network.cpp

**********************************************************************/

#include "stdafx.h"



#include <windows.h>
#include <windowsx.h>
#include <io.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>          // sprintf()


#include "drvrsym.h"
#include "drvrio.h"         // HStreamAbs
#include "resource.h"
#include "driver.h"         // Fourtier driver specifics
#include "goodies.h"        // interface to goodies.cpp
#include "network.h"        // interface to network.cpp




/*********************************************************************************************/
/****************************** Usefull Fourtier classes *************************************/




/*********************************************************************************************/
/****************************** CNetwork Implementation **************************************/

CNetwork::CNetwork( LPCSTR pNetName, LPCSTR ProjPath )
    : m_arrStation( pNetName ) 
{
    m_pName             = pNetName;
    m_uConfigTotal      = 0;
    m_uInputTotal       = 0;
    m_uOutputTotal      = 0;
    m_uNameTotal        = 0;
    m_uStationInstTotal = 0;
    m_uModuleInstTotal  = 0;
    m_uStationCnt       = 0;
    m_uModuleCnt        = 0;
    m_pStatus           = NULL;
    m_ConfigFile        = ProjPath;
    memset( &m_NetInst, 0, sizeof(DRIVER_INST) );
    DebugString( "creating network %s\n", m_pName );
}

CNetwork::~CNetwork()
{
    delete m_pStatus;   // remove the status object, if any.
    DebugString( "removing network  %s\n", m_pName );
}



int CNetwork::IsStatus( UINT32 DevType )
{
    return DevType == DEVICE_STATUS;
}

int CNetwork::IsStation( UINT32 DevType )
{

	int bIsStation = FALSE; 

    switch(DevType)
    {
	case DEVICE_STATION:
	case DEVICE_MOTION_STATION:
	case DEVICE_OTHER_MOTION_STATION:

				bIsStation = TRUE;
	}

    return bIsStation;
}

int CNetwork::IsModule( UINT32 DevType )
{
    int bIsModule = FALSE; 

    switch(DevType)
    {
	case DEVICE_MODULE_GENERIC:
	case DEVICE_MODULE_4W_INPUT:
	case DEVICE_MODULE_2W_INPUT:
	case DEVICE_MODULE_1W_INPUT:
	case DEVICE_MODULE_1B_INPUT:
	case DEVICE_MODULE_4W_OUTPUT:
	case DEVICE_MODULE_2W_OUTPUT:
	case DEVICE_MODULE_1W_OUTPUT:
	case DEVICE_MODULE_1B_OUTPUT:
	case DEVICE_MODULEIO_GENERIC:
	case DEVICE_MODULEIO_4W_INPUT:
	case DEVICE_MODULEIO_2W_INPUT:
	case DEVICE_MODULEIO_1W_INPUT:
	case DEVICE_MODULEIO_1B_INPUT:
	case DEVICE_MODULEIO_4W_OUTPUT:
	case DEVICE_MODULEIO_2W_OUTPUT:
	case DEVICE_MODULEIO_1W_OUTPUT:
	case DEVICE_MODULEIO_1B_OUTPUT:
	case DEVICE_MODULEIO_4W_IORO :
	case DEVICE_MODULEIO_2W_IORO :
	case DEVICE_MODULEIO_1W_IORO :
	case DEVICE_MODULEIO_1B_IORO :
	case DEVICE_MODULEIO_4W_IANDO :
	case DEVICE_MODULEIO_2W_IANDO :
	case DEVICE_MODULEIO_1W_IANDO :
	case DEVICE_MODULEIO_1B_IANDO :

            bIsModule = TRUE;
            
    }

    return bIsModule;
}


int CNetwork::IsOKPortAddress()
{
    return IsBetween( m_NetInst.PortAddress, PORT_MIN, PORT_MAX) && !(m_NetInst.PortAddress % PORT_STEP);
}


int CNetwork::IsOKDPR()
{
    return IsBetween(m_NetInst.DualPortAddress, DPADR_MIN, DPADR_MAX) && !(m_NetInst.DualPortAddress % DPADR_STEP);
}



// make sure  m_NetInst  can safely be used at run time
int CNetwork::NetValid()
{
    int rc = SUCCESS;

    if( !IsOKPortAddress() )
    {
        char buf[32];
        sprintf( buf, "%x", m_NetInst.PortAddress );
        Erop( IDS_CP_INVALID_PORT, m_pName, "", "", buf);
        rc = FAILURE;
    }
        
    if( !IsOKDPR() )
    {
        char buf[32];
        sprintf( buf, "%x", m_NetInst.DualPortAddress );
        Erop( IDS_CP_INVALID_DPR, m_pName, "", "", buf);
        rc = FAILURE;
    }

    if(!IsBetween( m_NetInst.IrqLevel, 0, NO_IRQ))
    {
        Erop( IDS_CP_INVALID_IRQLEVEL, m_pName, "", "", m_NetInst.IrqLevel & 0xff);
        rc = FAILURE;
    }

    if( (m_NetInst.BaudRate != BAUDRATE_125) && 
        (m_NetInst.BaudRate != BAUDRATE_250) && 
        (m_NetInst.BaudRate != BAUDRATE_500) )
    {
        Erop( IDS_CP_INVALID_BAUDRATE, m_pName, "", "", "");
        rc = FAILURE;
    }
    
    if( (m_NetInst.StopState != SS_HOLD_LAST_STATE) && (m_NetInst.StopState != SS_ZERO) )
    {
        Erop( IDS_CP_INVALID_STOPSTATE, m_pName, "", "", "");
        rc = FAILURE;
    }
        
    return rc;
}


// test if the driver is configured and load config info into DRIVER_INST
int CNetwork::LoadNetwork()
{
    int rc = SUCCESS;

    NETCONFIG    NetConfig;     // where config data is loaded from symdb
    UINT16       Size;          // the actual size of the config block as recored in symdb

    memset( &NetConfig, 0, sizeof(NETCONFIG) );
    rc = ListConfigGet( NetConfig, Size );

    if( rc == SUCCESS )
    {
        if( Size                      == sizeof(NETCONFIG)           &&  
            NetConfig.NetcfgId        == NETCONFIG_ID                &&  
            NetConfig.NetcfgVersMajor == ( NETCONFIG_VERS >> 16    ) &&  
            NetConfig.NetcfgVersMinor >= ( NETCONFIG_VERS & 0xffff )
          )
        {
            m_NetInst.DualPortAddress = NetConfig.DualPortAddress;
            m_NetInst.PortAddress     = NetConfig.PortAddress;
            m_NetInst.IrqLevel        = NetConfig.IrqLevel;
            m_NetInst.BaudRate        = NetConfig.BaudRate;
            m_NetInst.DprHWTests      = NetConfig.DprHWTests; 
            m_NetInst.StopState       = NetConfig.StopState;
            m_NetInst.bSimulate       = NetConfig.bSimulate; 
            m_NetInst.bWatchdog       = NetConfig.bWatchdog; 
            m_NetInst.InputRead       = NetConfig.InputRead; 
            m_NetInst.Sentinel        = RT3_SENTINEL;

            char* Fname = (char*)&NetConfig.ConfigFile[0];

            if( strlen( Fname ) )
            {
                m_ConfigFile += Fname;
            
                int file = open( m_ConfigFile, _O_RDONLY | _O_BINARY );
                if( file >= 0 )
                {
                    long filesize = lseek( file, 0L, SEEK_END );
                    if( filesize >= 0 )
                        m_NetInst.szConfigFile = filesize;
                    close( file );

                    if( !m_NetInst.szConfigFile )
                    {
                        rc = IDS_CP_INVALID_CFG_FILE;
                        Erop( rc, m_pName, m_ConfigFile, "", "" );
                    }

                }
                else
                {
                    rc = IDS_CP_CANNOT_OPEN_CFG_FILE;
                    Erop( rc, m_pName, m_ConfigFile, "", "" );
                }

                DebugString( "CNetwork::LoadNetwork(). m_ConfigFile=%s\n Size=%l\n", 
                    m_ConfigFile, m_NetInst.szConfigFile );
            }
            else
                m_ConfigFile = "";

        }
        else
        {
            rc = IDS_CP_DRIVER_NOT_CONFIGURED;
            Erop( rc, m_pName, "", "", "" );
        }    
    }
    else
    {
        rc = IDS_CP_CANNOT_GET_NETCONFIG;
        Erop( rc, m_pName, "", "", "" );
    }


    return rc;
}



int CNetwork::LoadStation( CStation* pNewStation )
{
    int rc = pNewStation->Load();

	// Verify that each device is 'unique'
	if( rc == SUCCESS )
		rc = m_arrStation.Check( pNewStation );

	if( rc == SUCCESS )
    {
		// Add the Station so that later tests don't mysteriously fail.
        Add( pNewStation );     //InsertInOrder( pNewStation );
		m_uNameTotal += strlen( pNewStation->m_pName ) + 1;
		m_uStationCnt++;

		/* Reserve UIOT input space only if there are defined tags	*/
		m_uInputTotal  += pNewStation->GetUiotInputSize(); 

		/* Reserve UIOT output space only if there are defined tags	*/
		m_uOutputTotal += pNewStation->GetUiotOutputSize();  
    }

    return rc;
}


int CNetwork::LoadModule( CModule* pNewModule )
{
	int rc = pNewModule->Load();

	if( rc == SUCCESS )
    {
        // Make sure pNewModule has a place in the tree of stations/modules
        // If there is such a place return the parent station. 
        // If not,  print the error message and return NULL.
        CStation* pStation = m_arrStation.Check( pNewModule );

        if( pStation )
        {
            // We have a parent station for this module
            pStation->Add( pNewModule );    // pStation->InsertInOrder( pNewModule );
			m_uNameTotal += strlen( pNewModule->m_pName ) + 1;
			m_uModuleCnt++;

			/* Reserve UIOT input space only if there are defined tags	*/
            m_uInputTotal  += pNewModule->GetUiotInputSize(); 
            if( pNewModule->GetInputSize() )
                pStation->m_StationInst.bInput = 1;    // Tell the runtime there is a least 1 input module here

			/* Reserve UIOT output space only if there are defined tags	*/
			m_uOutputTotal += pNewModule->GetUiotOutputSize();  
            if( pNewModule->GetOutputSize() )
                pStation->m_StationInst.bOutput = 1;    // Tell the runtime there is a least 1 output module here
        }
    }

    return rc;
}

int CNetwork::LoadStatus( CStatus* pNewStatus )
{
    int rc = SUCCESS;

    // Verify that there isn't another status device already defined.
    if( m_pStatus )
    {
        Erop( IDS_CP_TWO_STATUS_DEVICES, m_pName, m_pStatus->m_pName, pNewStatus->m_pName, "" );
        rc = FAILURE;
    }

	if( rc == SUCCESS )
        rc = pNewStatus->Load();

	if( rc == SUCCESS )
    {
		// Add the Station so that later tests don't mysteriously fail.
        m_pStatus = pNewStatus;
		m_uNameTotal += strlen( pNewStatus->m_pName ) + 1;

		/* Reserve UIOT input space only if there are defined tags	*/
		m_uInputTotal  += pNewStatus->GetUiotInputSize(); 

		/* Reserve UIOT output space only if there are defined tags	*/
		m_uOutputTotal += pNewStatus->GetUiotOutputSize();  
    }

    return rc;
}



int CNetwork::Load()
{
    int rc = LoadNetwork();

    if( rc == SUCCESS )
        rc = NetValid();

    if( ListStart() )    /* scan all devices */
    {
        CModuleArray arrModule( "" );   // temporary store here orphan modules; 

        do 
        {
            HDriverSym* pDev = ListGet();
			UINT16 DevType   = pDev->GetId();

			// Stations are 'master' devices which can have 1 - n slots (aka Modules)
			if( IsStation( DevType ) )
			{
                // This device is a station
                CStation* pNewStation = new CStation( pDev );
                rc = LoadStation( pNewStation );
				if( rc != SUCCESS )
                    delete pNewStation;     // also deletes  *pDev
            }
            else if( IsModule( DevType ) )
			{
                // This device is a module
				arrModule.Add( new CModule( pDev ) );   // store it here for now
			}
            else if( IsStatus( DevType ) )
			{
                // This device is the singleton status which brings support for driver dedicated I/O.
                CStatus* pNewStatus = new CStatus( pDev, &m_NetInst.Status );
                rc =LoadStatus( pNewStatus );
				if( rc != SUCCESS )
                    delete pNewStatus;      // also deletes  *pDev
			}
            else
            {
                Erop( IDS_CP_UNKNOWN_DEVICE, m_pName, "", "", DevType );
                rc = FAILURE;
            }

        } while( ListNext());


        if( rc == SUCCESS )
        {
            for( ; arrModule.GetSize() ; arrModule.RemoveAt(0) )
            {
			    CModule* pNewModule = arrModule[0];
                rc = LoadModule( pNewModule );
			    if( rc != SUCCESS )
					delete pNewModule;  // also deletes  *pDev
            }
        }
    }
    ListEnd();

    DebugString( "CNetwork::Load(). Name=%s, rc=%d\n", m_pName, rc );
    
    return rc;
}





//******************************************************************
// Insert a station into the list sorted by Address
void CNetwork::InsertInOrder( CStation *pStation )
{
	int	Index;
	int	MaxStation = m_arrStation.GetSize();
	CStation* pStationScan;

	// Find the location to insert this Station
	for (Index = 0; Index < MaxStation && MaxStation; Index++)
	{
		pStationScan = m_arrStation.GetAt(Index);
		if (*pStation < *pStationScan)
			break;
	}

	// Put the Station into a sorted spot
    m_arrStation.InsertAt( Index, pStation );
}


//******************************************************************
// Add a station to the list
void CNetwork::Add( CStation *pStation )
{
	// Add it to the list
    m_arrStation.Add( pStation );
}



// makes sure "this" can live with pNetwork
int CNetwork::Check( CNetwork* pNetwork )
{
    int rc = SUCCESS;

//    if( m_NetInst.PortAddress == pNetwork->m_NetInst.PortAddress )
//    {
//        char b[32];
//        sprintf(b, "%x", m_NetInst.PortAddress );
//        Erop( IDS_CP_SAME_PORT, m_pName, pNetwork->m_pName, b, 0);
//        rc = FAILURE;
//    }
    if( m_NetInst.DualPortAddress == pNetwork->m_NetInst.DualPortAddress )
    {
        char b[32];
        sprintf(b, "%x", m_NetInst.DualPortAddress );
        Erop( IDS_CP_SAME_DPRADR, m_pName, pNetwork->m_pName, b, 0);
        rc = FAILURE;
    }
//    if( m_NetInst.IrqLevel == pNetwork->m_NetInst.IrqLevel )
//    {
//        Erop( IDS_CP_SAME_IRQ, m_pName, pNetwork->m_pName, m_NetInst.IrqLevel & 0xff, 0);
//        rc = FAILURE;
//    }

    return rc; 
}




//******************************************************************
//	The config block passed to the runtime will contain:
//	1.  our DRIVER_INST struct
//	2.  a STATION_INST array with an element for each defined station 
//		device plus a terminator element
//	3.  a MODULE_INST array with an element for each defined device plus
//	    a terminator element
//	4.  the zero terminated names of all defined devices (the name of the status device is first)
//  5.  The configuration file
//  6.  sentinel, for sanity check
void CNetwork::ReportSizes(UINT32& rConfigTotal,UINT32& rInputTotal,UINT32& rOutputTotal)
{
    // Space to store all STATION_INST
    m_uStationInstTotal = (m_uStationCnt + 1) * sizeof( STATION_INST );

    // Space to store all MODULE_INST
    m_uModuleInstTotal  = (m_uModuleCnt + m_uStationCnt) * sizeof( MODULE_INST );

    m_uConfigTotal = sizeof(DRIVER_INST) - sizeof( NETWORK) + 
                    m_uStationInstTotal +
                    m_uModuleInstTotal  +
                    m_uNameTotal        + 
                    m_NetInst.szConfigFile  +
                    sizeof( UINT32 );   // RT3_SENTINEL

    rConfigTotal = m_uConfigTotal;
    rInputTotal  = m_uInputTotal;
    rOutputTotal = m_uOutputTotal;
}



// called in NetPass2 when we have real UIOT offsets for Config, Input and Output areas
void CNetwork::UpdateOffsets( UINT32 CfgOfs, UINT32 PrgInputOfs, UINT32 PrgOutputOfs, UINT32 IODelta )
{
    UINT32 ofsStationInst = CfgOfs         + sizeof(DRIVER_INST) - sizeof( NETWORK);
    UINT32 ofsModuleInst  = ofsStationInst + m_uStationInstTotal;
    UINT32 ofsNames       = ofsModuleInst  + m_uModuleInstTotal;

    UINT32 ConfigFileOfs = ofsNames      + m_uNameTotal;
    UINT32 SentinelOfs   = ConfigFileOfs + m_NetInst.szConfigFile;

    m_NetInst.ofsStationList = ofsStationInst;
    m_NetInst.nStations      = m_uStationCnt;   // tell the runtime how may Stations are in the list

    if( m_pStatus )
        m_pStatus->UpdateOffsets( ofsNames, PrgInputOfs, PrgOutputOfs, IODelta );

    m_arrStation.UpdateOffsets( ofsModuleInst, ofsNames, PrgInputOfs, PrgOutputOfs, IODelta );

    m_NetInst.ofsConfigFile = ConfigFileOfs;
    m_NetInst.ofsSentinel   = SentinelOfs;       // offset to RT3_SENTINEL
}



//******************************************************************
//	Called in NetPass2 to write config data in the config area
//	The # of written bytes must match the ConfigTotal value declared in NetPass1
void CNetwork::WriteConfigData( HStreamAbs& rFileHandle )
{
    // Write the config block as defined by 'ReportSizes'
    // The config block will be read by the runtime

	int       StationIndex, ModuleIndex;
    CStation* pStation;
    UINT32    uConfigTotal;

    {
        char*  pCfg  = (char*) & m_NetInst + sizeof(NETWORK);
        UINT16 szCfg = sizeof(DRIVER_INST) - sizeof(NETWORK);
        rFileHandle.Write( pCfg, szCfg );
        uConfigTotal = szCfg;
    }

	// Write the STATION_INST structure
    for( StationIndex = 0; StationIndex < m_arrStation.GetSize() ; StationIndex++ )
    {
        pStation = m_arrStation[ StationIndex ];

        STATION_INST* pStationInst = &pStation->m_StationInst;
		
        rFileHandle.Write( pStationInst, sizeof( STATION_INST ) );
        uConfigTotal += sizeof( STATION_INST );
    }
    {
        STATION_INST Terminator;
        memset( &Terminator, 0, sizeof(STATION_INST) );
        rFileHandle.Write( &Terminator, sizeof( STATION_INST ) );
        uConfigTotal += sizeof( STATION_INST );
    }

	// Write all of the MODULE_INST structures
    for( StationIndex = 0; StationIndex < m_arrStation.GetSize() ; StationIndex++ )
    {
        pStation = m_arrStation[ StationIndex ];

		// Write a MODULE_INST for each io device belonging to pStation and leave a blank at the end
        for( ModuleIndex = 0; ModuleIndex < pStation->m_arrModule.GetSize() ; ModuleIndex++ )
        {
            MODULE_INST* pModuleInst = &pStation->m_arrModule[ ModuleIndex ]->m_ModuleInst;
            rFileHandle.Write( pModuleInst, sizeof( MODULE_INST ) );
            uConfigTotal += sizeof( MODULE_INST );
        }
        {
            MODULE_INST Terminator;
            memset( &Terminator, 0, sizeof(MODULE_INST) );
            rFileHandle.Write( &Terminator, sizeof( MODULE_INST ) );
            uConfigTotal += sizeof( MODULE_INST );
        }
	}


    {
	    LPCSTR pName;
	    size_t szName;
	    const char cTerminator = 0;

        if( m_pStatus )
        {
	        // Write the name of the status device
            pName  = m_pStatus->m_pName;
            szName = strlen( pName );

            rFileHandle.Write( pName, (UINT16)szName );
            rFileHandle.Write( &cTerminator, sizeof( char ) );
            uConfigTotal += szName + 1;
        }

	    // Write Device Names
        for( StationIndex = 0; StationIndex < m_arrStation.GetSize() ; StationIndex++ )
        {
            pStation = m_arrStation[ StationIndex ];
	        // Write the Station Name
            pName  = pStation->m_pName;
            szName = strlen( pName );

            rFileHandle.Write( pName, (UINT16)szName );
            rFileHandle.Write( &cTerminator, sizeof( char ) );
            uConfigTotal += szName + 1;

	        // Write the name of devices belonging to pNode.
            for( ModuleIndex = 0; ModuleIndex < pStation->m_arrModule.GetSize() ; ModuleIndex++ )
            {
                pName  = pStation->m_arrModule[ ModuleIndex ]->m_pName;
                szName = strlen( pName );

                rFileHandle.Write( pName, (UINT16)szName );
                rFileHandle.Write( &cTerminator, sizeof( char ) );
                uConfigTotal += szName + 1;
            }
        }

    }

    if( m_NetInst.szConfigFile )
    {
        int  file = open( m_ConfigFile, _O_RDONLY | _O_BINARY );
        if( file >= 0 )
        {
            char   buffer[ 2048 ];
            UINT16 nBytes = 0;
            
            do
            {
                nBytes = read( file, buffer, sizeof( buffer ) );
                if( nBytes )
                {
                    rFileHandle.Write( buffer, nBytes );
                    uConfigTotal += nBytes;
                }
            } while( nBytes );

            close( file );
        }
        else
            Erop( IDS_CP_CANNOT_OPEN_CFG_FILE, m_pName, m_ConfigFile, "", "" );
    }


    rFileHandle.Write( &m_NetInst.Sentinel, sizeof(m_NetInst.Sentinel) );
    uConfigTotal += sizeof(m_NetInst.Sentinel);


    ASSERT( uConfigTotal == m_uConfigTotal );
	if (uConfigTotal != m_uConfigTotal)
	{
		LPSTR ErrMsg = "NetPass1, NetPass2 configuration size missmatch";
		Erop( IDS_CP_SOFTWARE_FAULT, m_pName, ErrMsg, "", "" );
	}
}								// CNetwork::WriteConfigData






int  CNetwork::ListConfigGet( NETCONFIG& Netconfig, UINT16& rBytesRet )
{
    return ListNetworkConfigGet( m_pName, &Netconfig, sizeof(NETCONFIG), rBytesRet );
}

int  CNetwork::ListConfigPut( NETCONFIG& Netconfig )
{
    return ListNetworkConfigPut( m_pName, &Netconfig, sizeof(NETCONFIG) );
}

BOOL CNetwork::ListStart()
{
    return ListNetworkStart( m_pName );
}



/************************************************************************************/

CNetworkArray::CNetworkArray( DRIVER_KEY NetKey, LPCSTR ProjPath ) 
{
    SetSize( 0, MAX_DRV_INSTANCES );
    DebugString( "Creating CNetworkArray. Path=%s\n", ProjPath);
    m_NetKey   = NetKey;    // DriverFOURTIER may be useful when more than 1 driver described in the rcd file
    m_ProjPath = ProjPath;  // full path to the project directory

    // Remove the <proj_file_name>.vlc from projpath. 
    // This is for VLC 3.x only!!!
    char* p = strrchr( m_ProjPath, '.' ); 
    if( p )
    {
        // if the last 4 chars in ProjPath are ".vlc"
        if( !stricmp( p, ".vlc" ) )
        {
            // then the file name is there, and it needs to be removed
            p = strrchr( m_ProjPath, '\\' ); 
            *(p+1) = '\0';
        }
    }
}


CNetworkArray::~CNetworkArray()  
{ 
    DebugString( "removing CNetworkArray\n", "");
    for(int i = 0; i < GetSize(); i++ )
        delete GetAt( i );
    RemoveAll(); 
}


int CNetworkArray::Check( CNetwork* pNetwork )
{
    int rc = SUCCESS;

    for(int i = 0; i < GetSize(); i++ )
    {
        CNetwork* pNet = GetAt( i );
        int       rc1  = pNetwork->Check( pNet );

        if( rc1 != SUCCESS )
            rc = FAILURE;
    }

    return rc; 
}


CNetwork*  CNetworkArray::GetNetwork( LPCSTR lpNetName )
{
    for(int i = 0; i < GetSize(); i++ )
    {
        CNetwork* pNetwork = GetAt( i );
        if( pNetwork->m_pName == lpNetName )
            return pNetwork;
    }

    return NULL;
}

/**************************** End of CNetwork Implementation *********************************/
/*********************************************************************************************/



/************************** End of Usefull Fourtier classes **********************************/
/*********************************************************************************************/


