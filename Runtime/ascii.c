///////////////////////////////////////////////////////////////////////////
//
//  NAME
//    ascii.cpp - ASCII interface sample program
//
//  DESCRIPTION
//    This program demonstrates a very simple terminal-like interface to
//    the ASCII interface of the DCX series controllers. It uses the IO
//    functions in DCXIO.CPP to perform the actual I/O.
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


void main( int argc, char** argv )
{
    char  key = 0;
    char  chrin;
    PMEM  CharOut     = reinterpret_cast<PMEM>( 0xd0804 );
    PMEM  CharInp     = reinterpret_cast<PMEM>( 0xd0800 );
    PBYTE Interpreter = reinterpret_cast<PBYTE>( 0xd0808 );

//
//  If DCX command interpreter busy, send ascii Escape as first character to clear
//
    if (*Interpreter & DCXBusy)
        key = ESC;

    cputs( "DCX ASCII Sample Program (^C to exit)\r\n\n" );
    while (true) {

//
//  If we don't have a character to send (key == 0) and there is a character
//  waiting at the keyboard grab it
//
        if (!key && kbhit())
            key = getch();

//
//  If key is nonzero, send it to the motion controller
//
        if (key && PutcAscii( key, CharInp, Interpreter )) {
#ifdef DCXPC
            putch( key );       // echo (dcp-pc only)
#endif
            key = 0;            // success - zero out so we don't send again
        }
//
//  Check for output from the motion controller
//
        if (chrin = GetcAscii( CharOut )) {
            putch( chrin );     // send data to screen for user display

#ifdef DCXAT
            if (chrin == '\r')  // add linefeeds to carriage return
                putch('\n');
#endif
            if (chrin == '\b')  // clear character for back space
                cputs( " \b" );
        }
    }
}

