/****************************************************************************

                            Driver.h

    FOURTIER Driver specfic UIOT structures and adjunct to the network node

*****************************************************************************/



#ifndef __DRIVER_H__
#define __DRIVER_H__



// "FOURTIER" id num. Make sure there is a copy of this line in driver\genname\DriverId.h
#define DriverDCPC100       0x5e7f8d51L	// ID number unique a must


/*
    Version 01.0002
    Bump this version # every time DRIVER_INST, DEVICE_INST or
    DEVICE_IO structs have changed. 
    This will force old projects to be recompiled before execution. 
*/
#define DCPC100VERS         0x00010005L	//	 0x00010002L	



//  Network config struct id
#define NETCONFIG_ID        DriverDCPC100


/* 
    Network config struct version
    Bump this version # every time NETCONFIG got new fields. 
    NetPass1() will force drivers in old projects to be reconfigured. 
    If old fields and NETCONFIG size are preserved, 
    configuration in old fields will be kept. 
*/
#define NETCONFIG_VERS      0x00010006L



// Device  config struct id
#define STATIONCONFIG_ID        0x12345600L    



/*
    Device  config struct version
    Bump this version # every time DEVCONFIG got new fields. 
    NetPass1() will force devices in old projects to be reconfigured. 
    If old fields and DEVCONFIG size are preserved, 
    configuration in old fields will be kept. 
*/
#define STATIONCONFIG_VERS      0x00010005L     // 0x00010001L     


// Device  config struct id
#define MODULECONFIG_ID         0x12345601L    



/*
    Device  config struct version
    Bump this version # every time DEVCONFIG got new fields. 
    NetPass1() will force devices in old projects to be reconfigured. 
    If old fields and DEVCONFIG size are preserved, 
    configuration in old fields will be kept. 
*/
#define MODULECONFIG_VERS       0x00010005L     //0x00010001L     



// load this value in DRIVER_INST for rt checking
#define RT3_SENTINEL        0x55667788L


// max 4 networks can be controlled by 1 PC
#define  MAX_DRV_INSTANCES          4       



#include  "errors.h"

#ifndef _INC_MCCL
#include "MCCL.H"
#endif


#ifndef APSTUDIO_READONLY_SYMBOLS


// pmc include


#ifndef VLCPORT_H__
#include "vlcport.h"
#pragma warning( disable: 4244 )
#endif

#ifndef DATASEG_H__
#include "dataseg.h"
#endif


/*****************************************************************************************

    This file keeps all necessary definitions needed for our fourtier driver.
    This is a driver build for didactical purposes. It can be used as a starting point 
    when we need to create a new real driver.
    The FOURTIER driver assumes we have an IO network. 
    The network is controlled by a card and can have up to MAX_DEVICES devices. 
    The card is accesible by a dual port ram and a port address.
    The DRP is DPR_TOTAL_SIZE bytes large and contains a control space, an input space and
    an output space.
    To keep things simple, our devices have all the same size: 8 bytes. 
    They are mapped in the DPR IO spaces based on their network address: device 0's input
    area can be found at the beginning of the DPR input space, device 1's input area is 
    8 bytes farther...  The same mechanism works for devices' output points. 
    In order to see input tags changing, we have to use an external application 
    that writes in the DPR input space.  We can also use the POKE special function to write there. 
    When generating the driver we can change DPR_OUTPUT_OFF to match DPR_INPUT_OFF. 
    Input space will then overlap the output space, so in the VLC project all input tags 
    will be animated by their correspondent output tags.    

    This network we want to control assume a 4 tier driver:
    Tier 0: The driver itself. To map I/O points here use the 'Status' device
    Tier 1: Stations.  They have I/O points,  and I/O modules.  
                        Stations are configured with a network address. 
    Tier 2: Modules. They live in a slot on a station.  They have I/O points. 
                        Modules are configured with a network address (to identify 
                        the parent station),  and a slot #.  
    Tier 3: I/O points. 

    Configuring the driver:
    1.  Choose a DPR address.
    2.  Choose a port address (didactic purpose only: will not be used)
    3.  Choose an interrupt level (didactic purpose only: the interrupt routine does nothing)
    4.  Skip HW tests. We may want to control the thoroughness of initial hw tests.
    5.  Simulate:  if on, there will be no attempt to touch the hardware.
    6.  Watchdog:  if on, the card's watchdog must be cyclicly kicked at run time.
    7.  Cyclic Input Read:  if on,  we update UIOT input image every Input() call.
                            if off, we have to rely on some hardware features telling us if 
                                    any input changed.
    
    Configuring devices:
    1.  Choose a link address (0 ... 127). This determines allocation in the DPR IO space
    2.  Critical:  if on, this device must be active at Online()    
    

    There are 5 different driver models we are studying:
    Model1:     No special functions at all.    (Simulate)
    Model2:     Only sync s.f. No background task.  (PID, Utility, ...)
    Model3:     Sequential async s.f. processing:  Pend & Done lists. (ex. ABKTX, MTL, ...)
                    DRIVER_INST needs  MarkTime.
    Model4:     Simoultaneous async s.f. processing: Pend, Run, Done lists (ex. DHPLUS, ...)
                    The hw supports commands with reply. 
                    New commands can be launched while others are waiting for their replies.
    Model5:     Paralel sequential s.f. processing. 
                    The hw supports a fixed # of channels that can accept commands.
                    Commands executed on different channels can run in paralel.
                    Commands executed on a channel are performed on a FIFO basis.
                    Pend[], Done lists    (DATALOG)
                    DRIVER_INST needs  MarkTime[].
    

    Here is an example for model 3:    
    
*****************************************************************************************/



//************   1 byte alignment typedef structures !!!   ************

#pragma BYTE_ALIGN(_SPECIAL_INST)
typedef  struct _SPECIAL_INST*  LPSPECIAL_INST_;
typedef                 UINT8*  LPUINT8;

typedef  selector   SEMAPHORE;

typedef  UINT32     UIOTREF2CHAR;
typedef  UINT32     UIOTREF2SINT8;
typedef  UINT32     UIOTREF2UINT8;
typedef  UINT32     UIOTREF2SINT16;
typedef  UINT32     UIOTREF2UINT16;
typedef  UINT32     UIOTREF2SINT32;
typedef  UINT32     UIOTREF2UINT32;
typedef  UINT32     UIOTREF2DOUBLE;
typedef  UINT32     UIOTREF2VOID;


#pragma BYTE_ALIGN(_LINKED_LIST)
typedef struct _LINKED_LIST 
{
    LPSPECIAL_INST_     pHead;      // Pointer to the first element in the linked list
    LPSPECIAL_INST_     pTail;      // Pointer to the last element in the linked list
    SEMAPHORE           Semaphore;  // Semaphore that locks the SPECIAL_INST list
    UINT16              uCounter;   // How many items are enqueued here
} LINKED_LIST, *LPLINKED_LIST; 
#pragma BYTE_NORMAL()


#pragma BYTE_ALIGN(_PTBUFFER)
typedef struct _PTBUFFER 
{
    UIOTREF2VOID    Offset;  
    UINT32          Size;           // Use PTBUFFER type for PT_BUFFERs
} PTBUFFER, * LPPTBUFFER;           // Its size is 8 bytes
#pragma BYTE_NORMAL()


#pragma BYTE_ALIGN(_TASK)
typedef struct _TASK 
{
    UINT16      hTask;          // background/interrupt task handle
    SEMAPHORE   Semaphore;      // Where the background task waits
    void*       pStack;         // Pointer to the stack allocated to the task
    UINT16      bBusy;          // True if Special I/O Task is working on packet, used during shutdown
    UINT16      Error;          // error code for the task's init sequence
    void*       IrqThunk;       // pointer to the interrupt routine
    UINT16      level;          // irmx encoded IRQ
    UINT16      align;
} TASK, * LPTASK; 
#pragma BYTE_NORMAL()




#pragma BYTE_ALIGN(_DEVICE_IO)  // 1 byte alignment
typedef struct _DEVICE_IO       // Specifies the UIOT offset and the size for each device
{
    void*        pSrc;          // DPR  offset/pointer for input devices || UIOT offset/pointer for output devices
    void*        pDst;          // UIOT offset/pointer for input devices || DPR  offset/pointer for output devices
    UINT16       Size;          // device input or output size.  Never 0 !!!
    UINT16       bUsed;         // If no I/O tags defined in the UIOT, skip it
    UIOTREF2VOID ofsUiot;       // ofsDst for input devices or ofsSrc for output devices
} DEVICE_IO, *LPDEVICE_IO;            
#pragma BYTE_NORMAL()


#pragma BYTE_ALIGN(_MODULE_INST)         // 1 byte alignment
typedef struct _MODULE_INST
{
    UIOTREF2VOID ofsName;       // UIOT offset to the device name generated at compile time
    char*        pName;         // UIOT pointer o the device name
    UINT16       DevType;       // DEVICE_MODULE_ ...
    UINT16       ProductCode;   // from the device configuration. 
    UINT16       Slot;          // device's network address
    UINT16       Address;       // device's network address
    UINT16       bCritical;     // if 1 --> device must be online when load and go
    UINT16       bSwap;         // if 1 --> swap bytes while copying them: UIOT <--> Dpr
    DEVICE_IO    Input;
    DEVICE_IO    Output;
    UINT32       Sentinel;      // 0x55667788 - check correct mapping

} MODULE_INST, *PMODULE_INST;
#pragma BYTE_NORMAL()


#pragma BYTE_ALIGN(_STATION_INST)         // 1 byte alignment
typedef struct _STATION_INST
{
    UIOTREF2VOID ofsName;       // UIOT offset to the device name generated at compile time
    char*        pName;         // UIOT pointer o the device name
    UINT16       DevType;       // DEVICE_STATION
    UINT16       align;
    UIOTREF2VOID ofsModuleList; // UIOT offset to the list of modules
    PMODULE_INST pModuleList;   // virtual pointer to the list on modules generated based on ofsModuleList.
    UINT16       nModules;      // How many modules should we find in the list
    UINT16       Address;       // device's network address
    UINT16       bCritical;     // if 1 --> device must be online when load and go
    UINT16       bPresent;      // if 1 --> device is online
    UINT16       bInput;        // if 1 --> there is at least 1 module with input image
    UINT16       bOutput;       // if 1 --> there is at least 1 module with output image
    DEVICE_IO    Input;
    DEVICE_IO    Output;
    UINT32       Sentinel;      // 0x55667788 - check correct mapping

} STATION_INST, *PSTATION_INST;
#pragma BYTE_NORMAL()


#pragma BYTE_ALIGN(_STATUS_INST)         // 1 byte alignment
typedef struct _STATUS_INST
{
    UIOTREF2VOID ofsName;       // UIOT offset to the device name generated at compile time
    char*        pName;         // UIOT pointer o the device name
    UINT16       DevType;       // DEVICE_STATUS
    UINT16       align;
    DEVICE_IO    Input;
    DEVICE_IO    Output;

} STATUS_INST, *PSTATUS_INST;
#pragma BYTE_NORMAL()



#pragma BYTE_ALIGN(_DRIVER_INST) 
typedef struct _DRIVER_INST 
{
    NETWORK Net;

        // Compile-time Static variables.  This structure maps to UIOT

    UIOTREF2VOID    ofsStationList;     // Where the STATION_INST list starts.
    UINT32          nStations;          // How many stations should we find in the list

    UINT16          PortAddress;        // 0x250,  0x254,  ...
    UINT16          align;
    UINT32          DualPortAddress;    // 0xd0000, 0xd1000, ...
    UINT16          IrqLevel;           // 0 ... 15
    UINT16          BaudRate;           // BAUDRATE_125, BAUDRATE_250_...
    UINT16          StopState;          // SS_HOLD_LAST_STATE,  SS_ZERO
    UINT16          bSimulate;          // =0 --> interface card must be present
    UINT16          bWatchdog;          // =1 --> kick the watchdog
    UINT16          DprHWTests;         // HWTEST_RW, HWTEST_OFF
    UINT16          InputRead;          // INPUT_READ_CYCLIC, INPUT_READ_COS,
    UINT16          align1;

    UIOTREF2UINT8   ofsConfigFile;      // Where the config file can be found in the UIOT
    UINT32          szConfigFile;       // How many bytes in the config file

    STATUS_INST     Status;             // Status input data and control output data associated to this driver instance
    
        // Run-time Dynamic Variables
    PSTATION_INST   pStationList;       // virtual pointer to the list of stations generated based on ofsStationList.

    void*           pDpr;               // (DUAL_PORT*) - iRmx ptr to where the board is in physical memory
    UINT16          bFirstCycle;        // Set by OnLine(), reset by Output(). Read by Input() and Output()
    UINT16          bGoOffLine;         // Tell all the bkg functions to shutdown

    LINKED_LIST     Pend;               // Pointer to the linked list of pending functions
    LINKED_LIST     Done;               // Pointer to the linked list of done  functions

    TASK            BackgroundTask;     // controls for the background task
    TASK            InterruptTask;      // controls for the interrupt task

    UIOTREF2UINT32  ofsSentinel;        // 0x55667788 - check correct mapping
    UINT32          Sentinel;           // 0x55667788 - check correct mapping

} DRIVER_INST, *LPDRIVER_INST;    
#pragma BYTE_NORMAL() 


#pragma BYTE_ALIGN( _SPECIAL_INST_HEADER )      // Must be first block in all paremeter blocks
typedef struct _SPECIAL_INST_HEADER
{       // Compile-time Static variables.  This structure maps to .rcd descrition
                                    // off, sz, ob.sz
    UINT16          FunctionId;     //  0    2   2L   PT_CONST  --> UINT16, _SIZE 2L
    UINT16          align;          //  2    2
    UIOTREF2UINT16  ofsStatus;      //  4    4   2L   PT_REF    --> tag's offset in the UIOT
    UIOTREF2UINT16  ofsResult;      //  8    4   2L   PT_REF    --> tag's offset in the UIOT
} SPECIAL_INST_HEADER;              //      12 == sizeof( SPECIAL_INST_HEADER )
#pragma BYTE_NORMAL()
/*
    Note: beacuse all functions have an Id field and a return status, we can standardize them 
    at offsets 0 and 4. This is especially helpful when using customized parameter structures 
    to better match function particularities and to save memory. 
*/


#pragma BYTE_ALIGN( _SPECIAL_INST_COMMAND ) 
typedef struct _SPECIAL_INST_COMMAND
{       // Compile-time Static variables.  This structure maps to .rcd descrition
                                        // off, sz, ob.sz
    SPECIAL_INST_HEADER Header;         //  0   12        the header must always be first
    UINT32              Address;        // 12    4   4L   PT_VALUE, PT_DEVICE --> UINT32
    UIOTREF2UINT16      ofsDDevStatus;  // 16    4   2L   PT_REF    --> tag's offset in the UIOT
    PTBUFFER            RBuffer;        // 20    8   8L   PT_BUFFER --> tag's offset in the UIOT
    PTBUFFER            WBuffer;        // 28    8   8L   PT_BUFFER --> tag's offset in the UIOT
    UIOTREF2UINT16      ofsRLength;     // 36    4   2L   PT_REF    --> tag's offset in the UIOT
    UINT16              WLength;        // 40    2   2L   PT_VALUE  --> UINT16, _SIZE 2L
    UINT16              align;          // 42    2
    UINT32              Timeout;        // 44    4   4L   PT_VALUE  --> UINT32, _SIZE 4L
} SPECIAL_INST_COMMAND;                 //      48 == sizeof( SPECIAL_INST_COMMAND )
#pragma BYTE_NORMAL()

#pragma BYTE_ALIGN(_SPECIAL_INST_PORT)  // we may have substitutes for SPECIAL_INST_PARAM
typedef struct _SPECIAL_INST_PORT
{       // Compile-time Static variables.  This structure maps to .rcd descrition
                                        // off, sz, ob.sz
    SPECIAL_INST_HEADER Header;         //  0   12        the header must always be first
    UINT16              Address;        // 12    2   2L   PT_VALUE  --> UINT16, _SIZE 2L
    UINT16              Length;         // 14    2   2L   PT_VALUE  --> UINT16, _SIZE 2L
    UIOTREF2UINT16      ofsInValue;     // 16    4   2L   PT_REF    --> tag's offset in the UIOT 
    UINT16              OutValue;       // 20    2   2L   PT_VALUE  --> UINT16, _SIZE 2L
    UINT16              align;          // 22    2   2L   PT_VALUE  --> UINT16, _SIZE 2L
} SPECIAL_INST_PORT;                    //      24 == sizeof( SPECIAL_INST_PORT )
#pragma BYTE_NORMAL()


// pmc stuff


#pragma BYTE_ALIGN( _DC2_BIN_TX ) // use with RX_BIN
typedef struct _DC2_BIN_TX
{       // Compile-time Static variables.  This structure maps to .rcd descrition
                                        // off, sz, ob.sz
    SPECIAL_INST_HEADER Header;         //  0   12        the header must always be first
    UINT16				Channel;        // 12    2   2L PT_value
    UINT16				Function;		// 14	 2	 2L	PT_VALUE
    UINT32				Value;	        // 16	 4	 2L	PT_value
	UIOTREF2CHAR		RxString;		// 20	 8	 8L	PT_BUFFER
    UINT16				Stat;			// 28	 2	 2L PT_sREF
    UINT16				Channel2;        //30	 2
    UINT16				Function2;		// 32	 2
    UINT32				Value2;	        // 34	 4
	UINT16				Align;			// 38	 2

} DC2_BIN_TX;			                //  40
#pragma BYTE_NORMAL()


#pragma BYTE_ALIGN( _STATUS_DC2 ) 
typedef struct _STATUS_DC2
{       // Compile-time Static variables.  This structure maps to .rcd descrition
                                        // off, sz, ob.sz
    SPECIAL_INST_HEADER Header;
    UINT16              Stat;
    UINT16              align;

} STATUS_DC2;                 
#pragma BYTE_NORMAL()


///// we have more for the union

typedef union _SPECIAL_INST_PARAM
{       // Compile-time Static variables.  This structure maps to .rcd descrition
                                        // off, sz
    SPECIAL_INST_HEADER				paramHeader;		//  0   12
    SPECIAL_INST_PORT				paramPort;			//  0   24
    SPECIAL_INST_COMMAND			paramCommand;  //  0   48

    DC2_BIN_TX						paramDC2TxBin;
    DC2_BIN_TX						paramDC2RxBin;
} SPECIAL_INST_PARAM;                   //      48 == sizeof(SPECIAL_INST_PARAM)






/* was
typedef union _SPECIAL_INST_PARAM
{       // Compile-time Static variables.  This structure maps to .rcd descrition
                                        // off, sz
    SPECIAL_INST_HEADER  paramHeader;   //  0   12
    SPECIAL_INST_COMMAND paramCommand;  //  0   48
    SPECIAL_INST_PORT    paramPort;     //  0   24
} SPECIAL_INST_PARAM;                   //      48 == sizeof(SPECIAL_INST_PARAM)

*/


typedef struct _SPECIAL_INST
{       // Compile-time Static variables.  This structure maps to .rcd descrition
                                        // off,  sz
    SPECIAL_INST_PARAM  User;           //   0   48
    SPECIAL_INST_PARAM  Work;           //  48   48

        // generic, same for all drivers having asyncronous special functions
    UINT32                MarkTime;     //  96    4  when this s.f. must be complete
    SINT16                Status;       // 100    2
    UINT16                Busy;         // 102    2    
    struct _SPECIAL_INST* pNext;        // 104    4

} SPECIAL_INST, *LPSPECIAL_INST;        //      108 == sizeof( SPECIAL_INST )

/*
Note1: This struct is declared 1 byte aligned on top of file. The struct description is 
       evaluated by the "Runtime" sub-project only.  
       The 'Gui' subproject evaluates the SPECIAL_INST parameter block as presented 
       by the FNC_... definitions. 

Note2: For a very simple function module,  SPECIAL_INST is sufficient.  
       Parameter fields can be described directly in SPECIAL_INST.  
       SPECIAL_INST_PARAM, SPECIAL_INST_PORT, SPECIAL_INST_COMMAND and SPECIAL_INST_HEADER 
       are optional.  They have been defined here only to show a more complex example. 

Note3: In order to save memory SPECIAL_INST can be used only for asynchronous special functions. 
       SPECIAL_INST_COMMAND, SPECIAL_INST_PORT, or even SPECIAL_INST_HEADER 
       will do the job for synchronous special functions. 
       Make sure the correct param block size is declared NET(DEV)_FUNC_TYPE paragraph (p#2).

Note4: Because asynchronous functions are executed concurenlty with the flowchart code, 
       it is safer to provide a copy of the parameter block, to be used by the background thread. 
       This is why we have introduced the 'User' and 'Work' areas. 
       'User' is the area marked by the compiler to be filled in every time a function 
       is called. When the function is posted for execution, 'User' is copied into 'Work' 
       and 'Work' is what the background sees.
       Make sure the fields in 'User' and 'Header' match the FNC_... definitions. 
       It is a good idea to have them both mapped at offset 0.

Note5: The Runtime Special() entry point offers a pointer to the associated SPECIAL_INST. 
       Depending on the FunctionId, the right parameter layout will be selected. 
       This can be implemented in 3 ways: 
       a. Define 1 layout only large enough to encompass all parameters needed by any function. 
       b. Define 1 layout for every function, and cast to the right one based on the FunctionId. 
       c. Define 1 layout for every function, store them into a union and select the right 
          union branch based on the FunctionId. 
       Our current implementation is a mixture of a. and c. and should be optimal 
       for consumed memory and code complexity. 
*/


#ifdef WINVER          // This is for MSVC compiler


#ifndef DRVRUTIL_H__
#include "drvrutil.h"   // SS_ZERO
#endif

// What we put into the database for network config

#pragma BYTE_ALIGN(_NETCONFIG)     // 1 byte alignment
typedef struct _NETCONFIG
{
    UINT32           NetcfgId;          //  0  NETCONFIG_ID
    UINT16           NetcfgVersMinor;   //  4  LOW(  NETCONFIG_VERS )
    UINT16           NetcfgVersMajor;   //  6  HIGH( NETCONFIG_VERS )
    UINT32           DualPortAddress;   //  8  0xd0000, 0xd1000, ...
    UINT16           PortAddress;       // 12  0x250,  0x254,  ...
    UINT16           IrqLevel;          // 14  0 ... 15
    UINT16           BaudRate;          // 16  BAUDRATE_125, BAUDRATE_250, ...
    STOP_STATE_TYPES StopState;         // 18  SS_HOLD_LAST_STATE,  SS_ZERO
    //UINT16           StopState;       // 18  0 --> keep scanning, 1 --> stop scanning

    UINT16           bSimulate;         // 20  =0 --> interface card must be present
    UINT16           bWatchdog;         // 22  =1 --> kick the watchdog
    UINT16           DprHWTests;        // 24  HWTEST_RW, HWTEST_OFF
    UINT16           InputRead;         // 26  INPUT_READ_CYCLIC, INPUT_READ_COS,

    UINT8            ConfigFile[128];   // 28  File name for the configuration file
    UINT8            reserved[64];      //156  add new fields without changing NETCONFIG size
    
} NETCONFIG;                            //220  == NET_CONFIG_SIZE == sizeof(NETCONFIG)
#pragma BYTE_NORMAL()


#pragma BYTE_ALIGN(_DEVCFG_HD)  // 1 byte alignment
typedef struct _DEVCFG_HD
{                               //  Byte Offset
    UINT32  NetcfgId;           //  0 NETCONFIG_ID
    UINT32  DevcfgId;           //  4 DEVCONFIG_ID
    UINT16  DevcfgVersMinor;    //  8 LOW(  DEVCONFIG_VERS )
    UINT16  DevcfgVersMajor;    // 10 HIGH( DEVCONFIG_VERS )
} DEVCFG_HD;                    // 12 == DEVCFG_HD_SIZE == sizeof(DEVCFG_HD)
#pragma BYTE_NORMAL()


#pragma BYTE_ALIGN(_STATIONCONFIG)  // 1 byte alignment
typedef struct _STATIONCONFIG
{                                   //  Byte Offset
    DEVCFG_HD   Header;             //  0 DEVCONFIG header
    UINT32      Address;            // 12 device's address on the link
    UINT16      bCritical;          // 16 =1 --> this device must be present on the link
    UINT16      reserved;           // 18 add new fields without changing DEVCONFIG size
} STATIONCONFIG;                    // 20 == STATIONCONFIG_SIZE == sizeof(STATIONCONFIG)
#pragma BYTE_NORMAL()

#pragma BYTE_ALIGN(_MODULECONFIG)   // 1 byte alignment
typedef struct _MODULECONFIG
{                                   //  Byte Offset
    DEVCFG_HD   Header;             //  0 DEVCONFIG header
    UINT16      Address;            // 12 device's address on the link
    UINT16      Slot;               // 14 device's address on the link
    UINT16      InputSize;          // 16 device's address on the link
    UINT16      OutputSize;         // 18 device's address on the link
    UINT16      bCritical;          // 20 =1 --> this device must be present on the link
    UINT16      bSwapBytes;         // 22 =1 --> swap bytes when copying data: UIOT <--> DPR
    UINT8       ProductCode[8];     // 24 edit field. 16#ffff
} MODULECONFIG;                     // 32 == MODULECONFIG_SIZE == sizeof(MODULECONFIG)
#pragma BYTE_NORMAL()



#pragma BYTE_ALIGN(_DEVCONFIG)      // 1 byte alignment
typedef struct _DEVCONFIG
{                                   // Byte Offset
    union
    {
        DEVCFG_HD     Header;       // 12
        STATIONCONFIG Station;      // 20
        MODULECONFIG  Module;       // 32
        UINT32        reserved[16]; // 64 add new fields without changing NETCONFIG size
    };

} DEVCONFIG;                        // 64 == DEVCONFIG_SIZE == sizeof(DEVCONFIG)
#pragma BYTE_NORMAL()
/*
    Note: The reserved fields will be used for future developpment. 
    They ensure compatibility with projects generated by older versions of this driver.
*/


#endif      // WINVER


#endif      // ! APSTUDIO_READONLY_SYMBOLS

/* 
    Defines for .rcd file 
    Arithmetic expressions are allowed to define RC and RCD constants, 
    when  ONLY using + and -.  
    It is a good idea to have them encapsulated in ( ).
    Never use * and /.  The RC compiler silently ignores them.
*/


// SPECIAL_INST offsets & sizes
#define FNC_HD_FUNCTIONID           0L 
#define FNC_HD_FUNCTIONID_SIZE          2L      // PT_CONST  --> size 2L    
#define FNC_HD_STATUS               4L 
#define FNC_HD_STATUS_SIZE              2L      // PT_REF --> size of the object pointed to
#define FNC_HD_RESULT               8L 
#define FNC_HD_RESULT_SIZE              2L      // PT_REF --> size of the object pointed to

#define FNC_CM_ADDRESS              12L    
#define FNC_CM_ADDRESS_SIZE             4L      // PT_VALUE, PT_DEVICE  --> size 4L    
#define FNC_CM_DDSTATUS             16L    
#define FNC_CM_DDSTATUS_SIZE            2L      // PT_REF --> size of the object pointed to
#define FNC_CM_RBUFFER              20L 
#define FNC_CM_RBUFFER_SIZE             8L      // PT_BUFFER --> size 8L
#define FNC_CM_WBUFFER              28L 
#define FNC_CM_WBUFFER_SIZE             8L      // PT_BUFFER --> size 8L
#define FNC_CM_RLENGTH              36L 
#define FNC_CM_RLENGTH_SIZE             2L      // PT_REF --> size of the object pointed to
#define FNC_CM_WLENGTH              40L 
#define FNC_CM_WLENGTH_SIZE             2L      // PT_VALUE --> 2L
#define FNC_CM_TIMEOUT              44L    
#define FNC_CM_TIMEOUT_SIZE             4L      // PT_VALUE  --> size 4L    

#define FNC_PO_ADDRESS              12L    
#define FNC_PO_ADDRESS_SIZE             2L      // PT_VALUE --> 2L
#define FNC_PO_LENGTH               14L    
#define FNC_PO_LENGTH_SIZE              2L      // PT_VALUE --> 2L
#define FNC_PO_IN_VALUE             16L 
#define FNC_PO_IN_VALUE_SIZE            2L      // PT_REF --> size of the object pointed to
#define FNC_PO_OUT_VALUE            20L    
#define FNC_PO_OUT_VALUE_SIZE           2L      // PT_VALUE --> 2L

#define FNC_BIN_AXIS				12L		// pt ref
#define FNC_BIN_AXIS_SIZE			2L		//2
#define FNC_BIN_FUNCTION			14L		// pt ref correct 14
#define FNC_BIN_FUNCTION_SIZE		2L
#define FNC_BIN_VALUE				16L		// pt ref
#define FNC_BIN_VALUE_SIZE			4L
#define FNC_BIN_STRING				20L
#define FNC_BIN_STRING_SIZE			8L
#define FNC_BIN_STAT				24L
#define FNC_BIN_STAT_SIZE			2L

#define FNC_BIN_AXIS2				26L		// 2
#define FNC_BIN_FUNCTION2			28L		// 2
#define FNC_BIN_VALUE2				30L		// 4



#define FNC_BIN_INST_SIZE			40

#define FNC_HD_SPECIAL_INST_SIZE        12
#define FNC_CM_SPECIAL_INST_SIZE        48
#define FNC_PO_SPECIAL_INST_SIZE        24
#define FNC_SPECIAL_INST_SIZE           108


// pmc

// #define FNC_TXRX_ASCII_TXSTRING				12L
// #define FNC_TXRX_ASCII_TXSTRING_SIZE		 8L
// #define FNC_TXRX_ASCII_STAT				20L
// #define FNC_TXRX_ASCII_STAT_SIZE		 2L


// #define FNC_TXRX_ASCII_INST_SIZE			24

#define FNC_DC2_STAT						12L
#define FNC_DC2_STAT_SIZE					2L









// NETCONFIG offsets & sizes
#define NET_ID                      0 
#define NET_ID_SIZE                     32 
#define NET_VERS                    4 
#define NET_VERS_SIZE                   32 
#define NET_DPADR                   8 
#define NET_DPADR_SIZE                  32 
#define NET_PORT                    12
#define NET_PORT_SIZE                   16 
#define NET_IRQ                     14
#define NET_IRQ_SIZE                    16 
#define NET_BAUDRATE                16 
#define NET_BAUDRATE_SIZE               16 
#define NET_STOPSTATE               18 
#define NET_STOPSTATE_SIZE              16 
#define NET_SIMULATE                20 
#define NET_SIMULATE_SIZE               16 
#define NET_WATCHDOG                22 
#define NET_WATCHDOG_SIZE               16 
#define NET_HWTEST                  24 
#define NET_HWTEST_SIZE                 16 
#define NET_INPUTREAD               26 
#define NET_INPUTREAD_SIZE              16 
#define NET_CONFIG_FILE             28 
#define NET_CONFIG_FILE_SIZE            1024
#define NETCONFIG_SIZE              220 

// DEVCONFIG offsets & sizes
// DEVCFG_HD offsets & sizes
#define DEV_HD_DRVID                0
#define DEV_HD_DRVID_SIZE               32
#define DEV_HD_ID                   4
#define DEV_HD_ID_SIZE                  32
#define DEV_HD_VERS                 8
#define DEV_HD_VERS_SIZE                32

// STATIONCONFIG offsets & sizes
#define DEV_ST_ADDRESS              12 
#define DEV_ST_ADDRESS_SIZE             32 
#define DEV_ST_CRITICAL             16 
#define DEV_ST_CRITICAL_SIZE            16 

// MODULECONFIG offsets & sizes
#define DEV_MD_ADDRESS              12 
#define DEV_MD_ADDRESS_SIZE             16 
#define DEV_MD_SLOT                 14 
#define DEV_MD_SLOT_SIZE                16 
#define DEV_MD_INPUTSZ              16 
#define DEV_MD_INPUTSZ_SIZE             16 
#define DEV_MD_OUTPUTSZ             18
#define DEV_MD_OUTPUTSZ_SIZE            16 
#define DEV_MD_CRITICAL             20 
#define DEV_MD_CRITICAL_SIZE            16 
#define DEV_MD_SWAP                 22 
#define DEV_MD_SWAP_SIZE                16 
#define DEV_MD_PCODE                24 
#define DEV_MD_PCODE_SIZE               64 


#define DEVCONFIG_SIZE              64 

// Dual port ram layout
// #define  DPR_CONTROL_OFF        0
// #define  DPR_CONTROL_SIZE       2048
// #define  DPR_INPUT_OFF          2048    // where the input image can be found in the dpr
// #define  DPR_INPUT_SIZE         1024    // 1kbyte =  MAX_DEVICES * 8bytes input devices
// #define  DPR_OUTPUT_OFF         3072    // where the output image can be found in the dpr
// #define  DPR_OUTPUT_OFF       2048    // for didactic purposes use 2048 --> outputs will be looped back in inputs
// #define  DPR_OUTPUT_SIZE        1024    // 1kbyte =  MAX_DEVICES * 8bytes input devices

#define  DPR_TOTAL_SIZE         4096    // 4 kbytes
#define  MAX_DEVICES            64    // max 128 devices allowed by our didactical network


#define  DPADR_MIN              0x80000L	// was a0000, for testing put to 80000
#define  DPADR_MAX              0xef000L
#define  DPADR_STEP             0x01000L    // 4 kbytes increments
#define  DPADR_DEFAULT          0xd0000L

#define  DPROFF_MIN               0x0000
#define  DPROFF_MAX               0x0fff
#define  DPROFF_STEP              1           
#define  DPROFF_DEFAULT           0x000

#define  PORT_MIN               0x000
#define  PORT_MAX               0x3fc
#define  PORT_STEP              1          
#define  PORT_DEFAULT           0x250

#define  PORT_MINL               0x0000L
#define  PORT_MAXL              0x3ffL
#define  PORT_STEPL              1L      
#define  PORT_DEFAULTL           0x250L

#define	 IO_STATION_DEF			0x10L
#define	 IO_STATION_MIN			0x10L
#define	 MOTION_STATION_DEF		0x01L
#define	 MOTION_STATION_MIN		0x01L
#define	 MOTION_STATION_MAX		0x08L
#define	 OTHER_MOTION_STATION_DEF		0x09L
#define	 OTHER_MOTION_STATION_MIN		0x09L
#define	 OTHER_MOTION_STATION_MAX		0x09L



#define  NO_IRQ                 0L
#define  HWTEST_RW              1
#define  HWTEST_OFF             0
#define  INPUT_READ_COS         1
#define  INPUT_READ_CYCLIC      0

#define  BAUDRATE_125           1
#define  BAUDRATE_250           2
#define  BAUDRATE_500           3

#define  DEVICE_STATUS              10      // Tier 0 singleton providing I/O support to the driver.
#define  DEVICE_STATION             20      // Tier 1 station (adapter, node, rack, drop, ...) on the network
#define  DEVICE_MOTION_STATION      70      // Tier 1 station (adapter, node, rack, drop, ...) on the network
#define  DEVICE_OTHER_MOTION_STATION      71      // Tier 1 station (adapter, node, rack, drop, ...) on the network


// DPR Type 
#define  DEVICE_MODULE_GENERIC      21      // Tier 2 generic I/O module
#define  DEVICE_MODULE_4W_INPUT     22      // Tier 2 64bits input module
#define  DEVICE_MODULE_4W_OUTPUT    23      // Tier 2 64bits output module
#define  DEVICE_MODULE_2W_INPUT     24      // 32bits input
#define  DEVICE_MODULE_2W_OUTPUT    25      // 32bits output
#define  DEVICE_MODULE_1W_INPUT     26      // 16bits input
#define  DEVICE_MODULE_1W_OUTPUT    27      // 16bits output
#define  DEVICE_MODULE_1B_INPUT     28      // 8bits input
#define  DEVICE_MODULE_1B_OUTPUT    29      // 8bits output


#define  DEVICE_MODULE_4W_IORO      30      // Tier 2 64bits input or  64bits output module
#define  DEVICE_MODULE_2W_IORO      31
#define  DEVICE_MODULE_1W_IORO      32
#define  DEVICE_MODULE_1B_IORO      33

#define  DEVICE_MODULE_4W_IANDO     34      // Tier 2 64bits input and 64bits output module
#define  DEVICE_MODULE_2W_IANDO     35      // Tier 2 16bits input and 16bits output module
#define  DEVICE_MODULE_1W_IANDO     36
#define  DEVICE_MODULE_1B_IANDO     37
// DPR Type 
// IO Type
#define  DEVICE_MODULEIO_GENERIC      41      // Tier 2 generic I/O module
#define  DEVICE_MODULEIO_4W_INPUT     42      // Tier 2 64bits input module
#define  DEVICE_MODULEIO_4W_OUTPUT    43      // Tier 2 64bits output module
#define  DEVICE_MODULEIO_2W_INPUT     44      // 32bits input
#define  DEVICE_MODULEIO_2W_OUTPUT    45      // 32bits output
#define  DEVICE_MODULEIO_1W_INPUT     46      // 16bits input
#define  DEVICE_MODULEIO_1W_OUTPUT    47      // 16bits output
#define  DEVICE_MODULEIO_1B_INPUT     48      // 8bits input
#define  DEVICE_MODULEIO_1B_OUTPUT    49      // 8bits output

#define  DEVICE_MODULEIO_4W_IORO      50      // Tier 2 64bits input or  64bits output module
#define  DEVICE_MODULEIO_2W_IORO      51
#define  DEVICE_MODULEIO_1W_IORO      52
#define  DEVICE_MODULEIO_1B_IORO      53

#define  DEVICE_MODULEIO_4W_IANDO     54      // Tier 2 64bits input and 64bits output module
#define  DEVICE_MODULEIO_2W_IANDO     55      // Tier 2 16bits input and 16bits output module
#define  DEVICE_MODULEIO_1W_IANDO     56
#define  DEVICE_MODULEIO_1B_IANDO     57
// IO Type


#define  DRIVER_FUNC                2000    // special driver functions ids
#define  DRVF_GET_DRVSTAT           2100    // functions at driver level
#define  DRVF_GET_DEVSTAT           2101    
// #define  DRVF_COMMAND              2102    
#define  DRVF_PORT_INPUT            2103    
#define  DRVF_PORT_OUTPUT           2104    
#define  DRVF_PEEK                  2105    
#define  DRVF_POKE                  2106    

#define  DEVICE_FUNC                2010    // special device functions ids
#define  DEVF_GET_DEVSTAT           2200    // functions at device level

// pmc

//#define	DC2_TXRX_ASCII				2300
#define DC2_RX_BIN					2320
#define DC2_STAT					2330

#define DC2_TX_BIN1					2311
#define DC2_TX_BIN2					2312
#define DC2_TX_BIN3					2313
#define DC2_TX_BIN4					2314
#define DC2_TX_BIN5					2315
#define DC2_TX_2AXIS				2316
#define DC2_TX_2AXIS2				2317
#define	DC2_TX_ASCII				2300
#define	DC2_RX_ASCII				2319

#define LIST_FUNCTION1					3001
#define LIST_FUNCTION2					3002
#define LIST_FUNCTION3					3003
#define LIST_FUNCTION4					3004
#define LIST_FUNCTION5					3005
#define LIST_FUNCTION6					3010

#define MOTION_STATION					3006  
#define OTHER_MOTION_STATION			3007 
#define AXIS_E_CHANNEL					3008




#define  MAX_LENGTH                 400


#endif       // __DRIVER_H__ 




