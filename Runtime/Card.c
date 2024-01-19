/********************************************************************

                                Card.c

    Interface specific code. 
    This file only should touch the hardware.

*********************************************************************/


#include "stdafx.h"

#include <rt.h>
#include <stdio.h>      // sprintf()
#include <string.h>     // strlen()
#include <stdlib.h>     // min()

#include "vlcport.h"
#include "dcflat.h"     // EROP()
#include "driver.h"     /* SEMAPHORE */
#include "errors.h"     /* IDS_RT_DP_RW_TEST                     */
#include "auxrut.h"     /* StartTimeout(), IsTimeout(), EROP     */
#include "IO_Port.h"   /* DUAL_PORT                             */
#include "card.h"       /* Init()                                */
#include "dcxio.h"

#define CMDBUFFER	0x0900
#define RPYBUFFER	0x0A00

#define CMDASCII_IN	0x0800
#define CMDASCII_OU	0x0804
#define CMDASCII_ST	0x0808
#define CMDASCII_BU	0x0




/******************* Card specific  Functions  *******************************/


/******************* Initialization  *******************************/


static int TestAndFill(UINT8* pc, const int Size, const int test, const int fill)   /* test == 1  --> no test */
{
    int i  = 0;
    for(; i < Size;  *pc++ = fill, i++)
    {
        int c = *pc & 255;
        if(test != 1  &&  test != c)
        {
            EROP("Ram Error.  Address %p, is 0x%02x, and should be 0x%02x", pc, c, test, 0);
            return IDS_IOPORT_HW_TEST;
        }
    }
    return SUCCESS;
}


int  Init( LPDRIVER_INST const pNet, P_ERR_PARAM const lpErrors )
{
    int rc = SUCCESS;

    DUAL_PORT* const   dp = pNet->pDpr;
    UINT16     DprHWTests = pNet->DprHWTests;

    if(DprHWTests == HWTEST_RW)
    {
        UINT8* const pc = (UINT8*)dp;

        TestAndFill(pc, DPR_TOTAL_SIZE, 1, 0xff);                     /* write 0xff overall       */
        rc = TestAndFill(pc, DPR_TOTAL_SIZE, 0xff, 0x55);             /* test 0xff and write 0x55 */
        if(rc == SUCCESS) rc = TestAndFill(pc, DPR_TOTAL_SIZE, 0x55, 0xaa);
        if(rc == SUCCESS) rc = TestAndFill(pc, DPR_TOTAL_SIZE, 0xaa, 0x00);
    }

    if( pNet->ofsConfigFile && pNet->szConfigFile )
    {
        char* pConfigFile = BuildUiotPointer( pNet->ofsConfigFile );
        char* p     = pConfigFile;
        int   nLeft = pNet->szConfigFile;
        char  buf[81]; 

        while( nLeft )
        {
            size_t i = min( sizeof(buf)-1, nLeft );
            memcpy( buf, p, i ); 
            buf[i] = 0;
            printf( "%s", buf );
            nLeft -= i;
            p     += i;
        }

        puts("");
    }

    return rc;
}



/****************************************************************************************
    IN:     pName   --> pointer to the device user name
            Address --> device's network address
            pBuf    --> pointer to the destination buffer
            szBuf   --> size of the destination buffer

    OUT:    *pBuf   <-- "Address xx (usr_name)".  
    Note:   The device user name may be truncated!
*/
static void LoadDeviceName( char* pName, UINT16 Address, char* pBuf, size_t szBuf )
{
    if( szBuf && (pBuf != NULL) )
    {
        char* format = "Address %d";

        *pBuf = '\0';

        if( szBuf > (strlen(format)+3) )    /* Address may need more digits */
        {
            size_t  len;

            sprintf(pBuf, format, Address & 0xffff);

            len = strlen( pBuf ); 

            if( pName && ((szBuf - len) > 10) )     /* if we still have 10 free bytes  */
            {
                strcat(pBuf, " (");
                len += 2;
                strncat( pBuf, pName, szBuf-len-2 );
                *(pBuf + szBuf - 2) = '\0';
                strcat( pBuf, ")" );
            }
        }
    }
}



int  TestConfig( LPDRIVER_INST const pNet, P_ERR_PARAM const lpErrors )
{
    int rc = SUCCESS;


    PSTATION_INST pStation;

    for( pStation = pNet->pStationList; pStation->ofsName; pStation++ )
    {
        DUAL_PORT* const dp = (DUAL_PORT*)pNet->pDpr;     /* pointer to the dualport */

        pStation->bPresent = 1;

        /*
        Check pStation. 
        if( the device is not on the network )
            pStation->bPresent = 0;
        */
        
        // let's say for example, station 5 is offline
       // if( pStation->Address == 5 )
        //    pStation->bPresent = 0;
        
        if( !pStation->bPresent && pStation->bCritical)
        {
            LoadDeviceName( pStation->pName, pStation->Address, lpErrors->Param3, sizeof(lpErrors->Param3) );
            rc = IDS_IOPORT_DEVICE_OFFLINE; 
        }
    }

    return rc;
}


/********************* runtime specific card access functions ********************/



void PortInput( SPECIAL_INST_PORT* const pData)
{
    int   rc  = SUCCESS;

    UINT16 const Addr = pData->Address;
    UINT16* pResult   = BuildUiotPointer( pData->Header.ofsResult );
    
    if( Addr >= 0x100 && Addr < 0x1000 )
    {
        UINT16* pInValue = BuildUiotPointer( pData->ofsInValue );

        switch(pData->Length)
        {
            case 1: 
					*pInValue = inbyte( Addr );
					break;

            case 2: 
					*pInValue = inhword( Addr );
					break;

            case 4: 
					// *pInValue = inword( Addr );
					break;
        }
    }
    else 
        rc = IDS_IOPORT_INVALID_ADDERSS;

    *pResult = rc;
}


void PortOutput( SPECIAL_INST_PORT* const pData)
{
    int   rc  = SUCCESS;

    UINT16 const Addr = pData->Address;
    UINT16* pResult   = BuildUiotPointer( pData->Header.ofsResult );
    
    if( Addr >= 0x100 && Addr < 0x1000 )
    {
        const UINT16 OutValue = pData->OutValue;
    
        switch(pData->Length)
        {
            case 1: 
					outbyte( Addr, OutValue );
					break;

            case 2: 
                    outhword( Addr, OutValue );
					break;

            case 4: 
                    // outword( Addr, OutValue );
					break;
        }
    }
    else 
        rc = IDS_IOPORT_INVALID_ADDERSS;

    *pResult = rc;
}





void DoPeekCommand( const LPDRIVER_INST pNet, SPECIAL_INST_COMMAND* const pData )
{
    int     rc       = SUCCESS;
    UINT8*  const dp = (UINT8*)pNet->pDpr; 
    const UINT32     dpSize   = DPR_TOTAL_SIZE; 
    const UINT32     dpOffset = pData->Address; 
    const LPPTBUFFER pRBuffer = &pData->RBuffer;
    const UINT32     Length   = pData->WLength;
    UINT16* pResult = BuildUiotPointer( pData->Header.ofsResult );

    if( !Length )
    {
        rc = IDS_IOPORT_RW_ZERO;
    } 
    else if( Length > pRBuffer->Size )
    {
        rc = IDS_IOPORT_READ_SIZE;
    }
    else if( dpOffset + Length > dpSize )
    {
        rc = IDS_IOPORT_DPR_OUT;
    }
    else
    {



		UINT8* p = BuildUiotPointer( pRBuffer->Offset );
        CardCopy( p, dp+dpOffset, Length);
    }
    
    *pResult = rc;

    {
        /*
        char buffer[100];
        char Param[68];
        LoadDeviceName( "Bambi55", 10, Param, sizeof( Param ) );
        printf( "Param=%s\n", Param );
        printf( "Param=%s\n", Param );
        printf( "Param=%s. Len=%d\n", Param, strlen(Param) );
        sprintf( buffer, "Test1. %s", Param );
        printf( "%s\n", buffer );
        */
    }

}

void DoPokeCommand( const LPDRIVER_INST pNet, SPECIAL_INST_COMMAND* const pData )
{
    int          rc  = SUCCESS;
    UINT8* const dp  = (UINT8*)pNet->pDpr; 
    const UINT32     dpSize   = DPR_TOTAL_SIZE; 
    const UINT32     dpOffset = pData->Address; 
    const LPPTBUFFER pWBuffer = &pData->WBuffer;
    const UINT32     Length   = pData->WLength;
    UINT16* pResult = BuildUiotPointer( pData->Header.ofsResult );


    if( !Length )
    {
        rc = IDS_IOPORT_RW_ZERO;
    } 
    else if( Length > pWBuffer->Size )
    {
        rc = IDS_IOPORT_WRITE_SIZE;
    }
    else if( dpOffset + Length > dpSize )
    {
        rc = IDS_IOPORT_DPR_OUT;
    }
    else
    {
        UINT8* p = BuildUiotPointer( pWBuffer->Offset );
        CardCopy( dp+dpOffset, p, Length);
    }
    
    *pResult = rc;
}



void GetDriverStatus( const LPDRIVER_INST pNet, SPECIAL_INST_COMMAND* const pData )
{
    const DUAL_PORT* const dp = (DUAL_PORT*)pNet->pDpr;
    UINT16* pDriverStatus = BuildUiotPointer( pData->ofsDDevStatus );
    *pDriverStatus = 0;	//dp->NetStatus;
}


void GetDeviceStatus( const LPDRIVER_INST pNet, SPECIAL_INST_COMMAND* const pData )
{
    int    rc       = SUCCESS;
    UINT16 Address  = pData->Address;
    UINT16* pResult = BuildUiotPointer( pData->Header.ofsResult );
    
    if( Address < MAX_DEVICES )
    {
        UINT16* pDeviceStatus = BuildUiotPointer( pData->ofsDDevStatus );
        const DUAL_PORT* const dp = (DUAL_PORT*)pNet->pDpr;
        *pDeviceStatus = 0;	//dp->DevStatus[Address];
    }
    else 
        rc = IDS_IOPORT_INVALID_ADDERSS;
        
    *pResult = rc;
}


/*
 *   Long lasting function, asynchronusely processed, called by BackgroundTask().
 *   Copies WriteBuffer into ReadBuffer, one character per second. 
*/
 
void DoCommand( const LPDRIVER_INST pNet, SPECIAL_INST* const pData )
{
    int rc = SUCCESS;

    SPECIAL_INST_COMMAND* const pWork = &pData->Work.paramCommand;

    UINT16* pResult = BuildUiotPointer( pWork->Header.ofsResult );

    const LPPTBUFFER pWBuffer = &pWork->WBuffer;
    const LPPTBUFFER pRBuffer = &pWork->RBuffer;

    UINT8* Src = BuildUiotPointer( pWBuffer->Offset );
    UINT8* Dst = BuildUiotPointer( pRBuffer->Offset );
    UINT16 Length  = pWork->WLength;
    
        UINT16* pRLength = BuildUiotPointer( pWork->ofsRLength );

        int    Timeout  = pWork->Timeout * 1000;        /* how much time we can afford to wait */
        UINT16 n = 0;

        pData->MarkTime = StartTimeout(Timeout);        /* milisecond when it should complete  */

        while( (rc == SUCCESS) && (n < Length) && !pNet->bGoOffLine )
        {
            *Dst++    = *Src++;
            *pRLength = ++n;
    
            if( IsTimeout( pData->MarkTime ) )          /* if not timeout yet, sleeps 1 tick */
                rc = 4;
            else
            {
                //Delay(100);      // sleep 100 ms
                Delay(1000);     // sleep 1 second 
            }
        }
    
    
    *pResult = rc;
}





void	PMCPutBinary(  const LPDRIVER_INST pNet, DC2_BIN_TX* const pData)
{
    int   rc  = SUCCESS;

   UINT16* pResult = BuildUiotPointer( pData->Header.ofsResult);

   UINT8* const CmdBuf  = (UINT8*)pNet->pDpr;


	
	// WAIT for cmd buffer clear to 0
	// this loop cause 14mSec approx !!
	// not acceptable
	// while (!((UINT8)CmdBuf[CMDBUFFER] == 0)) pData->Stat = -2; 
    // therefore that end of special function block
	// needs to have a discision block to ccheck for -1
    // and retries


	if (pData->Channel ==0) 
	{
	pData->Value =0;
	}

	if (pData->Function ==PMC_RT)
	{	(UINT8 volatile) CmdBuf[0x0800]= 27;
		Delay(1);
		(UINT8 volatile) CmdBuf[0x0800] = 27;
		*pResult = rc;
	}

	else
	{
	if ( (UINT8 volatile)CmdBuf[CMDBUFFER] == 0) 
	{   // is DCX ready?	
		(UINT8 volatile)CmdBuf[CMDBUFFER+1]= (UINT8) pData->Function;
		(UINT8 volatile)CmdBuf[CMDBUFFER+2]=  (UINT8) pData->Channel;
//		*(UINT32 volatile*) &CmdBuf[CMDBUFFER+3] = (UINT32) 0  ;		// Value
		*(UINT32 volatile*) &CmdBuf[CMDBUFFER+3] = (UINT32)pData->Value  ;		// Value

		(UINT8 volatile)CmdBuf[CMDBUFFER] = 6;
		*pResult = rc;

	}

	else
	{
		*pResult = -1;
	}

	}

}


void	PMCGetBinary( const LPDRIVER_INST pNet, DC2_BIN_TX* const pData)
{
    int   rc  = SUCCESS;
	int bCount, CntSize;

	UINT16* pResult = BuildUiotPointer( pData->Header.ofsResult);
    UINT8* const CmdBuf  = (UINT8*)pNet->pDpr; 
	UINT8*	pRxString	= BuildUiotPointer( pData->RxString);	
	CntSize = (UINT8)pData->Value; 
	bCount = -1;

	if ( (UINT8) CmdBuf[RPYBUFFER]>= CntSize) {   // Data ?
        

		bCount = (UINT8) CmdBuf[RPYBUFFER];
		if (bCount > 255) bCount = 255;

        CardCopy(  pRxString, &CmdBuf[RPYBUFFER], bCount);
		CmdBuf[RPYBUFFER]= 0;


	}

	else
	{
		pRxString[0] = 0;	// not possible for 7 zero's
		pRxString[1] = 0;
		pRxString[2] = 0;
		pRxString[3] = 0;
		pRxString[4] = 0;
		pRxString[5] = 0;
		pRxString[6] = 0;
			*pResult = bCount;		//no reply

	}


	*pResult = bCount; // char read in rec. buffer

}

void	PMCPutAscii( const LPDRIVER_INST pNet, SPECIAL_INST_COMMAND* const pWork)
{


   int rc = SUCCESS;
   int StrCnt =0;
	
   
   // SPECIAL_INST_COMMAND* const pWork = &pData->Work.paramCommand  ;

	//Result 
    UINT16* pResult = BuildUiotPointer( pWork->Header.ofsResult );
    UINT8* const CmdBuf  = (UINT8*)pNet->pDpr; 

    const LPPTBUFFER pWBuffer = &pWork->RBuffer;

    UINT8* Src = BuildUiotPointer( pWBuffer->Offset );


	int StrLen = strlen (Src);
	
	if (StrLen <1)
		rc=-2;

	if  (!(Src[StrLen-1] == 0x0D))
		Src[StrLen++]=0x0D;

	StrCnt=-1;
	while (rc == SUCCESS && StrCnt<StrLen   && !(pNet->bGoOffLine) )



       {
			
			
			if ((CmdBuf[CMDASCII_IN]==0x00) && !(CmdBuf[CMDASCII_ST] ==0xFF))
			{

			StrCnt++;
			CmdBuf[CMDASCII_IN]= Src[StrCnt];
			}

			
            if ( Src[StrLen] == 0x0d) 
			{ 
				rc = StrCnt;			//exit !

				while (CmdBuf[CMDASCII_IN] && (CmdBuf[CMDASCII_ST] ==0xFF)) {}

			}

			
        }


    *pResult = rc;

}


void	PMCGetAscii( const LPDRIVER_INST pNet, SPECIAL_INST* const pData)
{


   int rc = SUCCESS;
   int StrCnt =0;

    SPECIAL_INST_COMMAND* const pWork = &pData->Work.paramCommand  ;

	//Result 
    UINT16* pResult = BuildUiotPointer( pWork->Header.ofsResult );
    UINT8* const CmdBuf  = (UINT8*)pNet->pDpr; 


    const LPPTBUFFER pWBuffer = &pWork->WBuffer  ;

    UINT8* Src = BuildUiotPointer( pWBuffer->Offset );



        while( (rc == SUCCESS) && !(CmdBuf[CMDASCII_ST]==0) && !pNet->bGoOffLine && !(StrCnt<255))
        {
			
			*Src = CmdBuf[CMDASCII_OU];
			*Src++;			
			StrCnt++;
		
//			*pResult=StrCnt;
        }
    
    *pResult = rc;

}

void	PMCPutBinary2Axis(  const LPDRIVER_INST pNet, DC2_BIN_TX* const pData)
{
    int   rc  = SUCCESS;

   UINT16* pResult = BuildUiotPointer( pData->Header.ofsResult);

    UINT8* const CmdBuf  = (UINT8*)pNet->pDpr;


	if ( (UINT8 volatile)CmdBuf[CMDBUFFER] == 0) 
	{   // is DCX ready?	
		(UINT8 volatile)CmdBuf[CMDBUFFER+1]= (UINT8) pData->Function;
		(UINT8 volatile)CmdBuf[CMDBUFFER+2]=  (UINT8) pData->Channel;
//		*(UINT32 volatile*) &CmdBuf[CMDBUFFER+3] = (UINT32) 0  ;	
		*(UINT32 volatile*) &CmdBuf[CMDBUFFER+3] = (UINT32)pData->Value ;

		(UINT8 volatile)CmdBuf[CMDBUFFER+7]= (UINT8) pData->Function2;
		(UINT8 volatile)CmdBuf[CMDBUFFER+8]=  (UINT8) pData->Channel2;
//		*(UINT32 volatile*) &CmdBuf[CMDBUFFER+9] = (UINT32) 0  ;		
		*(UINT32 volatile*) &CmdBuf[CMDBUFFER+9] = (UINT32)pData->Value2  ;   //12




		(UINT8 volatile)CmdBuf[CMDBUFFER] = 12;

		*pResult = rc;

	}

	else
	{
		*pResult = -1;
	}
}

void	PMCPutBinary2Axis2(  const LPDRIVER_INST pNet, DC2_BIN_TX* const pData)
{
    int   rc  = SUCCESS;

   UINT16* pResult = BuildUiotPointer( pData->Header.ofsResult);

    UINT8* const CmdBuf  = (UINT8*)pNet->pDpr;





	if ( (UINT8 volatile)CmdBuf[CMDBUFFER] == 0) 
	{   // is DCX ready?	
	(UINT8 volatile)CmdBuf[CMDBUFFER+1]= (UINT8) pData->Function;
	(UINT8 volatile)CmdBuf[CMDBUFFER+2]=  (UINT8) pData->Channel;
//	*(UINT32 volatile* ) &CmdBuf[CMDBUFFER+3] = (UINT32) 0  ;	
	*(UINT32 volatile*) &CmdBuf[CMDBUFFER+3] = (UINT32)pData->Value ;

	*(UINT8 volatile*) &CmdBuf[CMDBUFFER+7]= (UINT8) pData->Function2;
	*(UINT8 volatile*) &CmdBuf[CMDBUFFER+8]=  (UINT8) pData->Channel2;
	// *(UINT32 *) &CmdBuf[CMDBUFFER+9] = (UINT32) 0  ;		
	*(UINT32 volatile*) &CmdBuf[CMDBUFFER+9] = (UINT32)pData->Value2  ;   //12



		*(UINT8 volatile*) &CmdBuf[CMDBUFFER] = 12;

		*pResult = rc;

	}

	else
	{
		*pResult = -1;
	}
}