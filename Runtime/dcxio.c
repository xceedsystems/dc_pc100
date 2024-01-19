///////////////////////////////////////////////////////////////////////////
//
//  NAME
//    dcx_io.cpp - dcx sample ASCII interface routins
//
//  DESCRIPTION
//    This module implements simple
//    These samples demonstrate basic access to the ASCII interface on the
//    AT200/VM200 series of controllers. Time outs and other high-level
//    features have been left out for clarity. These functions assume that
//    valid pointers to the AT200/VM200 shared memory (identified as PMEM
//    arguments) have been setup by another part of the program. CharInp
//    is the AT200/VM200 character input mailbox (to the AT200/VM200),
//    CharOut it the output mailbox (from the AT200/VM200).
//
//  RELEASE HISTORY
//    Copyright (c) 1999 by Precision Micro Control Corp. All rights
//    reserved.
//      
///////////////////////////////////////////////////////////////////////////


#define DCXPC  1        // un-comment one of these to select DCX-PC or
//#define DCXAT  1      //    DCX-AT compatibility



// #include <stdio.h>
#include <string.h>


#include "dcxio.h"


///////////////////////////////////////////////////////////////////////////
//
//  NAME
//    GetAscii - read a string from the DCX ascii interface
//    
//  DESCRIPTION
//    Read a string from the DCX. Terminates the string following a
//    carriage return or the prompt character.
//
//  ARGUMENTS
//    char* Reply       - buffer to hold reply string, should be 256 bytes
//                        long
//    PMEM  CharInp     - pointer to DCX character input mailbox in dual 
//                        ported memory (offset 0x0800)
//    PMEM  CharOut     - pointer to DCX character output mailbox in dual 
//                        ported memory (offset 0x0804)
//    PBYTE Interpreter - pointer to DCX command interpreter in dual 
//                        ported memory (offset 0x0808)
//    char  PromptChar  - prompt character, should be null (0) for DCX-PC,
//                        '>' for DCX-AT (DCX-AT defaults to this)
//
//  RETURNS
//    The return value is the number of bytes read.
//
///////////////////////////////////////////////////////////////////////////
/*
int GetAscii( char* Reply, PMEM CharInp, PMEM CharOut, PBYTE Interpreter, char PromptChar )
{
    int i = 0,
        ch = 0xff;

    while (i < 255 && ch != 0 && ch != '<' && ch != PromptChar  && ch != '\r') {

        while (!(ch = *CharOut) && *Interpreter & DCXBusy)
            ;                                                   // wait for character

        if ((Reply[i] = static_cast<char>( ch )) != '\0') {     // did we get a character  
            *CharOut = '\0';                                    // yes, clear mailbox
            i++;                                                // count character
        }
    }
    Reply[i] = '\0';                                            // terminate string

   return i;
}

*/
///////////////////////////////////////////////////////////////////////////
//
//  NAME
//    GetcAscii - read character from the DCX ascii interface
//    
//  DESCRIPTION
//    Reads a single character from DCX and clears the output mailbox.
//
//  ARGUMENTS
//    PMEM  CharOut     - pointer to DCX character output mailbox in dual 
//                        ported memory (offset 0x0804)
//
//  RETURNS
//    Return value is the character read or null if no character is
//    waiting.
//
///////////////////////////////////////////////////////////////////////////
/*
char GetcAscii( PMEM CharOut )
{
    char ch = 0;

    if ((ch = *CharOut) != 0)                       // character waiting?
        *CharOut = '\0';                            // yes, clear mailbox
    return ch;
}

*/
///////////////////////////////////////////////////////////////////////////
//
//  NAME
//    PutAscii - write a string to the DCX ascii interface
//    
//  DESCRIPTION
//    Write a null terminated string to the ascii interface of a DCX.
//
//  ARGUMENTS
//    char* Command     - pointer to buffer holding string with commands
//                        for DCX (must be null terminated)
//    PMEM  CharInp     - pointer to DCX character input mailbox in dual 
//                        ported memory (offset 0x0800)
//    PMEM  CharOut     - pointer to DCX character output mailbox in dual 
//                        ported memory (offset 0x0804)
//    PBYTE Interpreter - pointer to DCX command interpreter in dual 
//                        ported memory (offset 0x0808)
//
//  RETURNS
//    The return value is the number of bytes writen.
//
///////////////////////////////////////////////////////////////////////////
/*
int PutAscii( char* Command, PMEM CharInp, PMEM CharOut, PBYTE Interpreter )
{
    char* bptr = Command;
    while (*bptr)                                       // while characters are left
        if (!PutcAscii( *bptr, CharInp, Interpreter ))  // try to send next character
            GetcAscii( CharOut );                       // didn't go, flush any output & try again
        else
            bptr++;                                     // character sent, inc pointer

     return static_cast<int>( bptr - Command );          // return length of command sent

}

*/
///////////////////////////////////////////////////////////////////////////
//
//  NAME
//    PutcAscii - write a character to the ASCII interface
//    
//  DESCRIPTION
//    Write a single ascii character to a DCX. Special processing for the
//    escape and space characters stuffs them in the mailbox regardless
//    of status.
//
//  ARGUMENTS
//    char  ch          - character to send
//    PMEM  CharInp     - pointer to DCX character input mailbox in dual 
//                        ported memory (offset 0x0800)
//    PBYTE Interpreter - pointer to DCX command interpreter in dual 
//                        ported memory (offset 0x0808)
//
//  RETURNS
//    0 if character couldn't be sent (controller busy), or 1 if
//    character was sent.
//
///////////////////////////////////////////////////////////////////////////
/*
char PutcAscii( char ch, PMEM CharInp, PBYTE Interpreter )
{
    if ((!(*CharInp) && !(*Interpreter & DCXBusy)) || (ch == ' ' || ch == ESC)) {
        *CharInp = ch;         // output character

//
//  Following a <cr> we must wait for mailbox to clear (or command 
//  interpreter to go busy (potential race condition).
//
        if (ch == '\r') {
            while (*CharInp && !(*Interpreter & DCXBusy))
                ;
        }
        return 1;               // number of bytes written is 1
    }
    return 0;                   // skip it, controller wants to talk
}

*/
///////////////////////////////////////////////////////////////////////////
//
//  NAME
//    GetBinary - read a binary reply from a DCX controller
//    
//  DESCRIPTION
//    This function reads a binary reply from the DCX controller's
//    binary reply buffer.
//
//  ARGUMENTS
//    PBYTE Reply       - pointer to buffer to hold reply from DCX, must
//                        be 256 bytes long (maximum reply length)
//    PBYTE RpyBuf      - pointer to DCX binary reply buffer in dual
//                        ported memory (offset 0x0A00)
//
//  RETURNS
//    The return value is the number of bytes read.
//
///////////////////////////////////////////////////////////////////////////

int GetBinary( char* Reply, char* RpyBuf )
{
    *Reply = 0;
    if (*RpyBuf) { 
//        memcpy( Reply, RpyBuf, *RpyBuf + 1 );
        *RpyBuf = 0; 
    }
    
    return  *Reply ;
}


///////////////////////////////////////////////////////////////////////////
//
//  NAME
//    PutBinary - write a binary command block to a DCX controller
//    
//  DESCRIPTION
//    This function writes a buffer full of binary commands to the 
//    controller's binary command buffer.
//
//  ARGUMENTS
//    PBYTE Command     - pointer to buffer holding commands for DCX (first
//                        byte is length of commands, in bytes)
//    PBYTE CmdBuf      - pointer to DCX binary command buffer in dual
//                        ported memory (offset 0x0900)
//
//  RETURNS
//    The return value is the number of bytes writen.
//
///////////////////////////////////////////////////////////////////////////

int PutBinary( char* Command, char* CmdBuf  )
{
    int len = 0;

    if (*CmdBuf == 0) {                                 // is DCX ready?
//        memcpy( CmdBuf + 1, &Command[1], *Command );    // copy commands commands to DCX
        *CmdBuf = len = *Command;                       // write length last (triggers DCX)!
    }

     return  len ;
}
