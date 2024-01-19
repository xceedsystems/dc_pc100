///////////////////////////////////////////////////////////////////////////
//
//  NAME
//    dcxio.h - DCX Interface Code header file
//
///////////////////////////////////////////////////////////////////////////

#ifndef _INC_DCXIO                  // include only once
#define _INC_DCXIO


//
//  some useful data types
//
// typedef char* volatile PMEM;
// typedef unsigned char  BYTE;
// typedef BYTE* PBYTE;

//
//  Constants
//
// #ifdef DCXPC
#define DCXBusy     0xFF;     // ASCII busy for the DCX-PC controllers
// #else
//   const BYTE DCXBusy    = 0x10;     // ASCII busy for the DCX-AT controllers
// #endif


#define ESC            0x1B;    // ASCII escape characer


//
//  function prototypes
//
// int  GetAscii( char* Reply, PMEM CharInp, PMEM CharOut, char* Interpreter, char PromptChar );
// char GetcAscii( PMEM CharOut );
// int  PutAscii( char* Command, PMEM CharInp, PMEM CharOut,  char* Interpreter );
// char PutcAscii( char ch, PMEM CharInp, char* Interpreter );

int  GetBinary(  char* Reply,  char* RpyBuf );
int  PutBinary(  char* Command,  char* RpyBuf  );





#endif