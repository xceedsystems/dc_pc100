/********************************************************************

            Ctoolkit.h             

   This file contains defintions for all of the common structures 
   and type definitions necessary to describe our hardware

**********************************************************************/



#ifndef  __IO_PORT_H__
#define  __IO_PORT_H__



/*============================================================================

 STATUS_BYTE - 

=============================================================================*/



/*=============================================================================

  DP_HEADER   - defines the structure of the dual port memory header.

=============================================================================*/

// #pragma BYTE_ALIGN(_DUAL_PORT)  /* 1 byte alignement  */
// typedef struct _DUAL_PORT
// {
//     UINT16 NetStatus;                                  /* :0000 */
//     UINT16 NetCommand;                                 /* :0002 */
//     UINT16 Watchdog;                                   /* :0004 */
//     UINT16 reserved;                                   /* :0006 */
//     UINT16 DevStatus[MAX_DEVICES];                     /* :0008 */
//     UINT8  filler[DPR_CONTROL_SIZE-MAX_DEVICES*2-8];   /* :0108 */
//     UINT8  Input[DPR_INPUT_SIZE];                      /* :0800 */
//     UINT8  Output[DPR_OUTPUT_SIZE];                    /* :0c00 */
// 	   UINT8  DprPort[DPR_TOTAL_SIZE];
// } DUAL_PORT;
// #pragma BYTE_NORMAL()

#pragma BYTE_ALIGN(_PCDCXAXIS)         // 1 byte alignment
typedef struct _PCDCXAXIS
{
    UINT8       Axis[0xFF];       // AXISxx

} PCDCXAXIS;

#pragma BYTE_NORMAL()

#pragma BYTE_ALIGN(_AXIS_CONT)         // 1 byte alignment
typedef struct _AXIS_CONT
{
    UINT32      MotorStatus;				// 0000
    UINT32      CurrentPosition;			// 0004
    UINT32      TargetPosition;				// 0008
    UINT32      ProgrammedVelocity;			// 000c
    UINT32      empty1;						// 0010
    UINT32      ProgrammedAccelaration;		// 0014
    UINT32      ProgrammedDeceleration;		// 0018
    UINT32      empty2;						// 001c
    UINT32      MoveDelta[4];				// 0024
    UINT32      SlewVelocity;				// 0034
    UINT32      IndexRegister[2];			// 0038
    UINT32      SlewAcceleration;			// 0040
    UINT32      StopOnError[2];				// 0044
    UINT32      EncoderChange;				// 004c
    UINT32      TrajectoryGenerator[4];		// 0050
    UINT16      PositionGain;				// 0060
    UINT16      DerivativeGain;				// 0062
    UINT16      IntegralGain;				// 0064
    UINT16      IntegratingLimit;			// 0066
    UINT16      SamplingFrequency;			// 0068
    UINT32      PositiveTorqueLimit;		// 006a
    UINT16      WaitStopperTimer;			// 006e
    UINT32      MasterAxis;					// 0070
    UINT8       empty3[144];				// 0074 //39

} AXIS_CONT ;

#pragma BYTE_NORMAL()

#pragma BYTE_ALIGN( _STATUSBIT)         // 1 byte alignment
typedef struct _STATUSBIT
{
	UINT8 MotorBusy;
	UINT8 VelocityMode;
	UINT8 TrajectoryCompleted;
	UINT8 Direction;
	UINT8 Phaseing;
	UINT8 MotorHomed;
	UINT8 MotorError;
	UINT8 LookForIndex;
	UINT8 LookEdge;
	UINT8 MovingPositiveDirection;
	UINT8 MovingNegativeDirection;
	UINT8 BreakPointReached;
	UINT8 MotorIsJoging;
	UINT8 AmplifierFaultEnable;
	UINT8 AmplifierFaultTripped;
	UINT8 LimitPositiveEnable;
	UINT8 LimitPositiveTripped;
	UINT8 LimitNegativeEnable;
	UINT8 LimitNegativeTripped;
	UINT8 JogRightEnable;
	UINT8 JogRightActive;
	UINT8 JogLeftEnable;
	UINT8 JogLeftActive;
	UINT8 IndexAuxEncoder;
	UINT8 CourseHome;
	UINT8 AmplifierFault;
	UINT8 Reserved;
	UINT8 LimitPositive;
	UINT8 LimitNegative;
	UINT8 JogRight;
	UINT8 JogLeft;
} STATUSBIT;
#pragma BYTE_NORMAL()


#pragma BYTE_ALIGN(_DUAL_PORT)  /* 1 byte alignement  */
typedef   struct _DUAL_PORT
{

	union

	{
		UINT8  DprPort[DPR_TOTAL_SIZE];

		AXIS_CONT SlotAxis[16];	//000
//		PCDCXAXIS CmdChar;		//800
//		PCDCXAXIS CmdBin;		//900
//		PCDCXAXIS CmdBinReply;	//a00
//		PCDCXAXIS Spare1;		//b00
//		PCDCXAXIS Spare2;		//c00
//		PCDCXAXIS Spare3;		//d00
//		PCDCXAXIS Spare4;		//e00
//		PCDCXAXIS Spare5;		//f00
	};


} DUAL_PORT, *PDUAL_PORT;

#pragma BYTE_NORMAL()



/*****************************************************************************
**
**          NON-DUALPORT STRUCTURE DEFINTIONS
**
*****************************************************************************/



#endif            /* __FOURTIER_H__ */

