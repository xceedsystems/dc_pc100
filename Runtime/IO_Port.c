/************************************************************

                Fourtier.c


This file implements all the module entry points: 

int rtIdentify( P_IDENTITY_BLOCK* ppIdentityBlock );
int rtLoad(   UINT32 ScanRate, UINT32* rDirectCalls );
int rtOpen(   LPDRIVER_INST lpNet, P_ERR_PARAM lpErrors);
int rtReload( LPDRIVER_INST lpNet, P_ERR_PARAM lpErrors);
int rtOnLine( LPDRIVER_INST lpNet, P_ERR_PARAM lpErrors);
int rtInput(  LPDRIVER_INST lpNet);
int rtOutput( LPDRIVER_INST lpNet);
int rtSpecial(LPDRIVER_INST lpNet, LPSPECIAL_INST lpData);
int rtOffLine(LPDRIVER_INST lpNet, P_ERR_PARAM  lpErrors);
int rtClose(  LPDRIVER_INST lpNet, P_ERR_PARAM  lpErrors);
int rtUnload( );

**************************************************************/


#include "stdafx.h"


/*********************************************************/
/*                                                       */
/*                Fourtier Sample Program                */
/*                                                       */
/*********************************************************/ 
                 
#include <rt.h>

#include "vlcport.h"
#include "CSFlat.h"     // FCoreSup
#include "DCFlat.h"     // FDriverCore
#include "driver.h"

#include "version.h"
#include "auxrut.h"
#include "Io_port.h"   // DUAL_PORT
#include "task.h"
#include "card.h"
#include "dcxio.h"

#define CMDBUFFER	0x0900
#define RPYBUFFER	0x0A00


/************************************************************/

/*
    This code is provided here as an example on how to use 
    call back functions implemented in the csFlat.lib

*/



int rtIdentify( P_IDENTITY_BLOCK* ppIdentityBlock )
{
    static IDENTITY_BLOCK IdentityBlock; 
    IdentityBlock.DriverId   = DriverDCPC100;
    IdentityBlock.DriverVers = DCPC100VERS;
    IdentityBlock.pName      = PRODUCT_NAME ", " PRODUCT_VERSION;
    *ppIdentityBlock = &IdentityBlock;
    return 0;
}

int rtLoad( UINT32 ScanRate, UINT32* rDirectCalls )
{



    // Executing the LOAD PROJECT command

    #if defined( _DEBUG )
        SetDebuggingFlag( 1 );  // Disable the VLC watchdog, so we can step through our code. 
    #endif  // _DEBUG


    // Use direct calls for very fast applications.  
    // With the appropriate bit set, Input(), Output() and/or Special()
    //  can be directly called from the engine thread, 
    //  saving the delay introduced by a task switch. 
    // Note:  Functions exectuted in the engine thread cannot call 
    //  some C stdlib APIs, like sprintf(), malloc(), ...
    
    // *rDirectCalls = ( DIRECT_INPUT | DIRECT_OUTPUT | DIRECT_SPECIAL );

	



    EROP( "rtLoad() ScanRate=%d, rDirectCalls=%x", ScanRate, *rDirectCalls, 0, 0 );

    return 0;
}

int rtOpen( LPDRIVER_INST pNet, P_ERR_PARAM pErrors)
{
    // Executing the LOAD PROJECT command

        PSTATUS_INST  pStatus  = &pNet->Status;
        PSTATION_INST pStation = pNet->pStationList;
		UINT8* const CmdBuf  = (UINT8*)pNet->pDpr;


    int rc = SUCCESS;



// 		(UINT8)CmdBuf[CMDBUFFER] = 0;

 //		(UINT8)CmdBuf[CMDBUFFER+1] = PMC_RT;  //function motor off
//		(UINT8)CmdBuf[CMDBUFFER+2] = 0;		// axis
//		(UINT8)CmdBuf[CMDBUFFER+3] = 0;		// data
//		(UINT8)CmdBuf[CMDBUFFER+4] = 0;		// data
//		(UINT8)CmdBuf[CMDBUFFER+5] = 0;		// data
//		(UINT8)CmdBuf[CMDBUFFER+6] = 0;		// data
//		(UINT8)CmdBuf[CMDBUFFER+7] = 0;		// data
 //		(UINT8)CmdBuf[CMDBUFFER] = 6;
    // rc = test();  // Example on how to use the call back functions implemented in the csFlat.lib

    if( pNet->Sentinel != RT3_SENTINEL )
        rc = IDS_VLCRTERR_ALIGNMENT;

    if( rc == SUCCESS )
    {
        UINT32* pSentinel = BuildUiotPointer( pNet->ofsSentinel );
        if( *pSentinel != RT3_SENTINEL )
            rc = IDS_VLCRTERR_ALIGNMENT;
    }

    EROP( "rtOpen() pNet=%p, pErrors=%p, PortAddress=%x", pNet, pErrors, pNet->PortAddress, 0 );


    if( !pNet->bSimulate )
	{
        if( rc == SUCCESS )
        {
            pNet->pDpr = AllocateDpr( pNet->DualPortAddress, DPR_TOTAL_SIZE );
            if( pNet->pDpr == NULL )
                rc = IDS_VLCRTERR_CREATE_DESCRIPTOR;
        }

        if( rc == SUCCESS )
        {
            DUAL_PORT*    pDpr    = pNet->pDpr;
            PSTATUS_INST  pStatus = &pNet->Status;
            PSTATION_INST pStation;

            pStatus->pName      = BuildUiotPointer( pStatus->ofsName );
            pStatus->Input.pDst = BuildUiotPointer( pStatus->Input.ofsUiot );


            // pStatus->Input.pSrc = &pDpr->Input[ 0 ];    // TO DO. Set here the correct pointer to the dpr;

            // pStatus->Output.pDst = &pDpr->Input[ 0 ];  // TO DO. Set here the correct pointer to the dpr;
            

//was 			pStatus->Output.pSrc = BuildUiotPointer( pStatus->Output.ofsUiot );
    
            pNet->pStationList = BuildUiotPointer( pNet->ofsStationList );

            for( pStation = pNet->pStationList; pStation->ofsName && ( rc == SUCCESS ); pStation++ )
            {
        
                if( pStation->Sentinel == RT3_SENTINEL )
                {
                    PMODULE_INST pModule;

                    pStation->pName       = BuildUiotPointer( pStation->ofsName );
                    pStation->pModuleList = BuildUiotPointer( pStation->ofsModuleList );

                    // pStation->Input.pSrc = &pDpr->Input[ 0 ];       // TO DO. Set here the correct pointer to the dpr;

                    // pStation->Output.pDst = &pDpr->Input[ 0 ];     // TO DO. Set here the correct pointer to the dpr;
                    pStation->Output.pSrc = BuildUiotPointer( pStation->Output.ofsUiot );


	pStation->Input.pDst = BuildUiotPointer( pStation->Input.ofsUiot );

	if (pStation->Address < 9 )
	pStation->Input.pSrc = (void*)&pDpr->SlotAxis[pStation->Address -1 ].MotorStatus;	
	if (pStation->Address ==9 )
	pStation->Input.pSrc = (void*)&pDpr->DprPort[0x81a];	




                    for( pModule = pStation->pModuleList; pModule->ofsName && ( rc == SUCCESS ); pModule++ )
                    {
                        if( pModule->Sentinel == RT3_SENTINEL )
                        {
                            pModule->pName      = BuildUiotPointer( pModule->ofsName );
                            pModule->Input.pDst = BuildUiotPointer( pModule->Input.ofsUiot );
                     //     pModule->Input.pSrc = &pDpr->Input[ 0 ];     // TO DO. Set here the correct pointer to the dpr;
                     //     pModule->Output.pDst = &pDpr->Input[ 0 ];   // TO DO. Set here the correct pointer to the dpr;
                            pModule->Output.pSrc = BuildUiotPointer( pModule->Output.ofsUiot );

						switch(pModule->DevType)
						{

						case DEVICE_MODULE_4W_INPUT:
						case DEVICE_MODULE_2W_INPUT:
						case DEVICE_MODULE_1W_INPUT:
						case DEVICE_MODULE_1B_INPUT:
							pModule->Input.pSrc = (void*)&pDpr->DprPort[ pModule->Slot ];
							break;

						case DEVICE_MODULE_4W_OUTPUT:
						case DEVICE_MODULE_2W_OUTPUT:
						case DEVICE_MODULE_1W_OUTPUT:
						case DEVICE_MODULE_1B_OUTPUT:
							pModule->Output.pDst = (void*)&pDpr->DprPort[ pModule->Slot ];
							break;
						
						case DEVICE_MODULEIO_GENERIC:
						case DEVICE_MODULEIO_4W_INPUT:
						case DEVICE_MODULEIO_2W_INPUT:
						case DEVICE_MODULEIO_1W_INPUT:
						case DEVICE_MODULEIO_1B_INPUT:
							pModule->Input.pSrc = (UINT32 *)pModule->Slot ;
							break;
						case DEVICE_MODULEIO_4W_OUTPUT:
						case DEVICE_MODULEIO_2W_OUTPUT:
						case DEVICE_MODULEIO_1W_OUTPUT:
						case DEVICE_MODULEIO_1B_OUTPUT:
							pModule->Output.pDst=(UINT32 *)pModule->Slot ;
							break;
						case DEVICE_MODULEIO_4W_IORO :
						case DEVICE_MODULEIO_2W_IORO :
						case DEVICE_MODULEIO_1W_IORO :
						case DEVICE_MODULEIO_1B_IORO :
						case DEVICE_MODULEIO_4W_IANDO:
						case DEVICE_MODULEIO_2W_IANDO :
						case DEVICE_MODULEIO_1W_IANDO :
						case DEVICE_MODULEIO_1B_IANDO :
							{pModule->Input.pSrc = (UINT32 *)pModule->Slot ;
							pModule->Output.pDst= (UINT32 *)pModule->Slot ;
							break;

						
							}

						}
							 

                        }
                        else
                            rc = IDS_VLCRTERR_ALIGNMENT;
                    }
                }
                else
                    rc = IDS_VLCRTERR_ALIGNMENT;
            }
        }

        if( rc == SUCCESS )
            rc = Init( pNet, pErrors);

        if( rc == SUCCESS )
            rc = CreateBackgroundTask(pNet);
	}

    return rc;
}

int rtReload( LPDRIVER_INST pNet, P_ERR_PARAM pErrors)
{
    // Executing the LOAD PROJECT command
    EROP( "rtReload() pNet=%p, pErrors=%p", pNet, pErrors, 0, 0);
    if( !pNet->bSimulate )
    {
        InitLinkedList(&pNet->Pend);
        InitLinkedList(&pNet->Done);
    }

    // make sure pNet is in the same state as after rtOpen(). 
    return 0;
}

int rtOnLine( LPDRIVER_INST pNet, P_ERR_PARAM pErrors)
{

    
	// Executing the START PROJECT command
    int rc = SUCCESS;

	UINT8* volatile CmdBuf  = (UINT8*)pNet->pDpr;


    EROP( "rtOnLine() pNet=%p, pErrors=%p", pNet, pErrors, 0, 0 );
    pNet->bFirstCycle = 1;
    pNet->bGoOffLine  = 0;


 
		(UINT8)CmdBuf[0x0800] = 27;
		Delay(1);
		(UINT8)CmdBuf[0x0800] = 27;
//		Delay(1);
		(UINT8)CmdBuf[0x0800] = 27;
	
	
	if( !pNet->bSimulate )
    {
        /* Check all devices. If critical devices are offline,  rc = IDS_FOURTIER_DEVICE_OFFLINE */

        rc = TestConfig( pNet, pErrors);
        
        /* 
            If we have a watchdog with a time consuming start procedure, start it here. 
            If it takes much shorter than a scan cycle to start it, start it in the first input cycle        
         */
    }

    return rc;

}



int rtInput( LPDRIVER_INST pNet )
{
    // This is the beginning of the VLC scan cycle
    if( !pNet->bSimulate )
    {
        // Copy new input data from the hw to the driver input image in the UIOT. 

        PSTATUS_INST  pStatus  = &pNet->Status;
        PSTATION_INST pStation = pNet->pStationList;

        for( ; pStation->ofsName; pStation++ )
        {
            if( pStation->bInput )
            {
                PMODULE_INST pModule = pStation->pModuleList;
                for( ; pModule->ofsName; pModule++ )
                {
                    if( pModule->Input.bUsed )

						InCardCopy( pModule->Input.pDst, pModule->Input.pSrc,pModule->Input.Size,
						pModule->DevType, pModule->bSwap);

               }
            }
		   // Station Status
            if( pStation->Input.bUsed )
                CardCopy( pStation->Input.pDst, pStation->Input.pSrc, pStation->Input.Size);
        }
		// not use
        // if( pStatus->Input.bUsed )
        //    CardCopy( pStatus->Input.pDst, pStatus->Input.pSrc, pStatus->Input.Size);

        /* 
        if( pNet->bFirstCycle )
            Start the watchdog.
        else
            KickWD(dp);     kick watchdog, if any
        */

        VerifyDoneList(&pNet->Done);    // Flush the completed background functions
    }

    // EROP( "rtInput() pNet=%p", pNet, 0, 0, 0 );

    return SUCCESS;
}


int rtOutput( LPDRIVER_INST pNet)
{
    // This is the end of the VLC scan cycle
    if( !pNet->bSimulate )
    {
        // Copy new output data from the UIOT driver output image to the hw.

        PSTATUS_INST  pStatus  = &pNet->Status;
        PSTATION_INST pStation = pNet->pStationList;
        
        for( ; pStation->ofsName; pStation++ )
        {
            if( pStation->bOutput )
            {
                PMODULE_INST pModule = pStation->pModuleList;
                for( ; pModule->ofsName; pModule++ )
                {
                    if( pModule->Output.bUsed )

               OutCardCopy( pModule->Output.pDst , pModule->Output.pSrc, 
			   pModule->Output.Size, pModule->DevType,pModule->bSwap);

//             OutCardCopy( pOutput->pDst, pOutput->pSrc, pOutput->Size,pOutput->Type,pOutput->IOSwap);

                }
            }
			// Not use
            //if( pStation->Output.bUsed )
           //     CardCopy( pStation->Output.pDst, pStation->Output.pSrc, pStation->Output.Size);

        }
		// not use
        //if( pStatus->Output.bUsed )
        //    CardCopy( pStatus->Output.pDst, pStatus->Output.pSrc, pStatus->Output.Size);

        if( pNet->bFirstCycle )     /* first Output() ? */
        {
            /*  Only now we have a valid output image in the DPR. 
                EnableOutputs(dp);  enable outputs (if our hardware lets us) 
             */
            pNet->bFirstCycle = 0;
        }       
    }

    // EROP( "rtOutput() pNet=%p", pNet, 0, 0, 0 );

    return SUCCESS;
}



int rtSpecial( LPDRIVER_INST pNet, LPSPECIAL_INST pData)
{
    // A trapeziodal block has been hit.

    UINT16  Result = 0;
    UINT16  Status = VLCFNCSTAT_OK;

    PDUAL_PORT    pDpr    = pNet->pDpr;

    
    EROP( "rtSpecial() pNet=%p, pData=%p", pNet, pData, 0, 0 );

    if( !pNet->bSimulate )
    {
        int  FunctionId = pData->User.paramHeader.FunctionId;
        switch( FunctionId ) 
        {
 //       case DC2_RX_ASCII:
 //                   Status = Pend( pNet, pData );   /* *pResult is set by the bkg task */
 //                   break;

		case DC2_TX_ASCII:

			 PMCPutAscii( pNet,&pData->User.paramCommand ) ;


//			Status = Pend( pNet, pData );   /* *pResult is set by the bkg task */
            break;
            
            case DRVF_PORT_INPUT:
                    PortInput( &pData->User.paramPort );
                    break;

			case  DC2_TX_BIN1 :
			case  DC2_TX_BIN2 :
			case  DC2_TX_BIN4 :
			case  DC2_TX_BIN5 :
				 PMCPutBinary( pNet,&pData->User.paramDC2TxBin) ;
				 break;
			case  DC2_TX_2AXIS :
				 PMCPutBinary2Axis( pNet,&pData->User.paramDC2TxBin) ;
				 break;
			case  DC2_TX_2AXIS2 :
				 PMCPutBinary2Axis2( pNet,&pData->User.paramDC2TxBin) ;
				 break;

 			case  DC2_RX_BIN  :

				 PMCGetBinary( pNet, &pData->User.paramDC2RxBin) ;
					 break;


            case DRVF_PORT_OUTPUT:
                    PortOutput( &pData->User.paramPort );
                    break;

            case DRVF_PEEK:
                    DoPeekCommand( pNet, &pData->User.paramCommand );
                    break;

            case DRVF_POKE:
                    DoPokeCommand( pNet, &pData->User.paramCommand );
                    break;
             
            default:
                    Status = VLCFNCSTAT_WRONGPARAM;
                    break;
        }
    
        // EROP("Special();  FunId= %d, Status= %d, pData= %p", FunctionId, Status, pData, 0);
    }
    else
    {
		UINT16* pResult = BuildUiotPointer( pData->User.paramHeader.ofsResult );
        if( pResult )   // some functions may not have the Result param implemented
		    *pResult = (UINT32) SUCCESS;

        Status = VLCFNCSTAT_SIMULATE;
    }

    if( pData->User.paramHeader.ofsStatus )   // some functions may not have the status param implemented
	{
		UINT16* pStatus = BuildUiotPointer( pData->User.paramHeader.ofsStatus );
		*pStatus = Status;
	}
    
    return SUCCESS;
}

int rtOffLine( LPDRIVER_INST pNet, P_ERR_PARAM pErrors)
{
    // Executing the STOP PROJECT command
    int rc = SUCCESS;

        PSTATUS_INST  pStatus  = &pNet->Status;
        PSTATION_INST pStation = pNet->pStationList;
		UINT8* const CmdBuf  = (UINT8*)pNet->pDpr;

    EROP( "rtOffLine() pNet=%p, pErrors=%p", pNet, pErrors, 0, 0 );

    pNet->bGoOffLine = 1;

    if( !pNet->bSimulate )
    {
        rc = WaitForAllFunctionCompletion(pNet);  /* wait for the backgroung task to calm down */
        
        if( rc == SUCCESS )
        {
            /*
            DUAL_PORT far *  dp  = (DUAL_PORT far *)pNet->pDpr;
            if( pNet->StopState == 1 )
                rc = stop scanning;
    
            DisableOutputs(dp, &pNet->trans_count);
            DisableWD(dp); 
            */
        }
		// zero all output


        if( pNet->StopState == 1 )
		{
        for( ; pStation->ofsName; pStation++ )
        
            if( pStation->bOutput )
            {
                PMODULE_INST pModule = pStation->pModuleList;

                for( ; pModule->ofsName; pModule++ )
                
                    if( pModule->Output.bUsed )
					ZeroCardCopy( pModule->Output.pDst , pModule->Output.pSrc,pModule->Output.Size, pModule->DevType);
				
			}
	
		}
 
    }    

// turn off all motors

//locks up when exit without a card !
// while (!((UINT8)CmdBuf[CMDBUFFER] == 0)) ;

 		(UINT8)CmdBuf[0x0800] = 27;
		Delay(1);
		(UINT8)CmdBuf[0x0800] = 27;

 		(UINT8)CmdBuf[CMDBUFFER] = 0xFF;
		Delay(1);
 		(UINT8)CmdBuf[CMDBUFFER] = 0;

 		(UINT8)CmdBuf[CMDBUFFER+1] = PMC_MF;  //function motor off
		(UINT8)CmdBuf[CMDBUFFER+2] = 0;		// axis
		(UINT8)CmdBuf[CMDBUFFER+3] = 0;		// data
		(UINT8)CmdBuf[CMDBUFFER+4] = 0;		// data
		(UINT8)CmdBuf[CMDBUFFER+5] = 0;		// data
		(UINT8)CmdBuf[CMDBUFFER+6] = 0;		// data
		(UINT8)CmdBuf[CMDBUFFER+7] = 0;		// data
 		(UINT8)CmdBuf[CMDBUFFER] = 6;

		Delay(10);
 		(UINT8)CmdBuf[CMDBUFFER] = 0xFF;

 		(UINT8)CmdBuf[CMDBUFFER+1] = PMC_AB;  //function motor off
		(UINT8)CmdBuf[CMDBUFFER+2] = 0;		// axis
		(UINT8)CmdBuf[CMDBUFFER+3] = 0;		// data
		(UINT8)CmdBuf[CMDBUFFER+4] = 0;		// data
		(UINT8)CmdBuf[CMDBUFFER+5] = 0;		// data
		(UINT8)CmdBuf[CMDBUFFER+6] = 0;		// data
		(UINT8)CmdBuf[CMDBUFFER+7] = 0;		// data
 		(UINT8)CmdBuf[CMDBUFFER] = 6;


    EROP("rtOffLine(). exit  rc= %d", rc, 0, 0, 0);

    return rc;
}

/*   if Open() fails, Close() is not automatically called for this instance.
     if lpErrors == NULL, do not report any error, keep the Open()'s error code and params.  
 */ 
int rtClose( LPDRIVER_INST pNet, P_ERR_PARAM pErrors)
{
    // Executing the UNLOAD PROJECT command
    if( !pNet->bSimulate )
    {
        EROP("rtClose(). start. pNet= %p", pNet, 0, 0, 0);
        /*
        {
            DUAL_PORT far* const dp = (DUAL_PORT far *)pNet->pDpr;     / * pointer to the dualport * /
            Reset the board;
        }
        */
        
        //DeleteInterruptTask( pNet );
        DeleteBackgroundTask( pNet );
    
        if( pNet->pDpr )
        {
            FreeDpr( pNet->pDpr );
            pNet->pDpr = NULL;
        }
    }

    EROP( "rtClose() pNet=%p, pErrors=%p", pNet, pErrors, 0, 0 );
    return SUCCESS;
}

int rtUnload( LPUIOT lpUiot )
{
    // Executing the UNLOAD PROJECT command
    EROP( "rtUnload()", 0,0,0,0 );
    return 0;
}




