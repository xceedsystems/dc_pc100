///////////////////////////////////////////////////////////////////////////
//
//  NAME
//    binary.cpp - Binary interface sample program
//
//  DESCRIPTION
//    This program demonstrates usage of the binary interface of the
//    DCX series controllers. It uses the IO functions in DCXIO.CPP to
//    perform the actual I/O. It issues a tell position comand to axis #1
//    and displays the results, looping until terminated with a keypress.
//
//    This sample has been built and tested with the Microsoft Visual C/C++
//    v5 as a 32-bit console application for Windows 95/98. The pointer
//    intialization done at the top of main will need to be modified to 
//    work with other operating systems (NT, UNIX, etc.).
//
//  RELEASE HISTORY
//    Copyright (c) 1999-2000 by Precision Micro Control Corp. All rights
//    reserved.
//      
///////////////////////////////////////////////////////////////////////////

#define DCXPC  1        // un-comment one of these to select DCX-PC or
//#define DCXAT  1      //    DCX-AT compatibility

#include <stdio.h>
#include <conio.h>
#include "dcxio.h"
#include "mccl.h"       // mccl command mnemonics


void main( int argc, char** argv )
{
    PBYTE CmdBuf = reinterpret_cast<PBYTE>( 0xd0900 );
    BYTE  Command[256];
    PBYTE RpyBuf = reinterpret_cast<PBYTE>( 0xd0a00 );
    BYTE  Reply[256];

    cputs( "DCX Binary Sample Program (press any key to exit)\r\n\n" );

//
//  build up command
//
    Command[0] = 6;     // 6 bytes long
    Command[2] = 1;     // axis number
    Command[1] = TP;    // command
    *(reinterpret_cast<int*>( &Command[3] )) = 0;


    while (!kbhit()) {
        if (PutBinary( Command, CmdBuf ) == 6) {
            while (GetBinary( Reply, RpyBuf) == 0)                  // wait for reply
                ;
            long val = *(reinterpret_cast<int*>( &Reply[3] ));      // get position value 
            cprintf( "\rAxis one reported position %d      ", val ); 
        }
    }
}

