///////////////////////////////////////////////////////////////////////////
//
//  NAME
//    mccl.h - motion control command language mnemonics & flags
//
//  DESCRIPTION
//    Include this header file in your source code to provide mnemonics
//    for MCCL commands, status registers, and memory locations. These
//    mnemonics make programming with pmccmd() and pmcrpy() easier. This
//    file is NOT required for general MCAPI programming.
//
//    Some of the advanced DCX-AT200 mnemonics conflict with mnemonics 
//    from other controllers - define the constant USE_DCXAT200 in your
//    code before including this header file to select the DCX-AT200
//    values for these mnemonics.
//
//    #include "mccl.h"
//
//  RELEASE HISTORY
//    Copyright (c) 1995-99 by Precision Micro Control Corp. All rights
//    reserved.
//
//    Version 2.1a      22-Feb-99      Programmer: R. Brian Gaynor
//      Added latch channel mnemonic, enhanced DCX-AT200 mnemonics.
//
//    Version 2.1b       7-May-99       Programmer: R. Brian Gaynor
//      No changes to this file.
//
//    Version 2.1c      25-Jun-99       Programmer: R. Brian Gaynor
//      Version number bumped to match API release.
//
//    Version 2.20      17-Jan-00       Programmer: R. Brian Gaynor
//      - Updated DCX-AT200/DCX-AT300 mnemonics
//
//    Version 2.21       6-Mar-00       Programmer: R. Brian Gaynor
//      No changes to this file.
//
//    Version 2.22      31-Mar-00       Programmer: R. Brian Gaynor
//      Added the DC2 AG and RD mnemonics.
//
//    Version 2.23      16-May-00      Programmer: R. Brian Gaynor
//      No changes to this module.
//
///////////////////////////////////////////////////////////////////////////

#ifndef _INC_MCCL
#define _INC_MCCL

//
//  Command opcodes
//
// added PMC_ since some of it clash with other define


#define PMC_AA      133     // accumulator add 
#define PMC_AB       10     // abort 
#define PMC_AC      140     // accumulator complement 
#define PMC_AD      136     // accumulator divide 
#define PMC_AE      143     // accumulator exclusive or 
#define PMC_AF      235     // auxiliary encoder find index 

#ifndef USE_DCXAT200
  #define PMC_AG    226     // set acceleration feed-forward gain 
#else
  #define PMC_AG    183     // arc angle
#endif

#define PMC_AH      234     // auxiliary encoder define home 
#define PMC_AI      131     // load accumulator indirect
#define PMC_AL      130     // load accumulator with constanat 
#define PMC_AM      135     // accumulator multiply 
#define PMC_AN      141     // accumulator and 
#define PMC_AO      142     // accumulator or 
#define PMC_AP      110     // adjust position
#define PMC_AR      132     // copy accumulator to register 
#define PMC_AS      134     // Accumulator Subtract 
#define PMC_AT      233     // Auxiliary encoder Tell position 
#define PMC_AV      139     // Accumulator eValuate 
#define PMC_AX      180     // arc center x
#define PMC_AY      181     // arc center y
#define PMC_AZ      236     // auxiliary encoder tell index 
#define PMC_BD      208     // backlash distance
#define PMC_BF      207     // backlash off
#define PMC_BK      121     // break 
#define PMC_BM       59     // binary mode
#define PMC_BN      206     // backlash on
#define PMC_BR       30     // baud rate 
#define PMC_CA      180     // arc center absolute 
#define PMC_CC       70     // current count 
#define PMC_CD      193     // contour increment distance 
#define PMC_CE      311     // close file
#define PMC_CF       31     // channel off 
#define PMC_CH       66     // channel high true 
#define PMC_CI       32     // channel in 
#define PMC_CL       67     // channel low true 
#define PMC_CM       27     // set contouring mode 
#define PMC_CN       33     // channel on 
#define PMC_CP      192     // contour path 
#define PMC_CR      181     // arc center relative 
#define PMC_CS       71     // checksum 
#define PMC_CT       34     // channel out 
#define PMC_CX       28     // contouring count
#define PMC_DA      219     // position capture display & arm
#define PMC_DB      118     // set deadband 
#define PMC_DE      127     // debug (dump memory)
#define PMC_DF      107     // do if channel off 
#define PMC_DG      227     // set deceleration feed-forward gain 
#define PMC_DH       35     // define home 
#define PMC_DI       36     // direction 
#define PMC_DL      271     // directory list
#define PMC_DM       60     // decimal mode 
#define PMC_DN      106     // do if channel on 
#define PMC_DO      214     // display optimal
#define PMC_DQ      189     // display torque
#define PMC_DR      213     // display recorded
#define PMC_DS      117     // deceleration set 
#define PMC_DT      197     // at target delay 
#define PMC_DY      190     // position capture delay
#define PMC_EA      187     // end angle absolute
#define PMC_EF       37     // echo off 
#define PMC_EI      124     // enable interrupts
#define PMC_EM        1     // execute macro
#define PMC_EN       38     // echo on 
#define PMC_ER      188     // end angle relative
#define PMC_ET      251     // escape task 
#define PMC_FC       64     // full current 
#define PMC_FD      204     // fail delay
#define PMC_FE       11     // find edge 
#define PMC_FF       51     // fail input off 
#define PMC_FG      313     // file get 
#define PMC_FI       12     // find index 
#define PMC_FN       50     // fail input on 
#define PMC_FO      270     // format file system
#define PMC_FP      312     // file put 
#define PMC_FR       39     // set derivative sampling period 
#define PMC_FS      314     // file seek
#define PMC_GA      248     // get analog input reading into accumulator 
#define PMC_GB      240     // get byte
#define PMC_GD      246     // get device ID
#define PMC_GE      169     // go until edge
#define PMC_GF      167     // go until coarse off
#define PMC_GH       13     // go home 
#define PMC_GI      168     // go until index
#define PMC_GL      242     // get long
#define PMC_GM        8     // gain mode 
#define PMC_GN      166     // go until coarse on
#define PMC_GO       14     // go 
#define PMC_GP      315     // file get position
#define PMC_GT      250     // generate task 
#define PMC_GU      137     // get user axis
#define PMC_GW      241     // get word
#define PMC_GX      247     // get auxiliary encoder position into accumulator 
#define PMC_HC       65     // half current 
#define PMC_HE       72     // help 
#define PMC_HF       48     // RS232 module handshake off 
#define PMC_HL      211     // set motion high limit 
#define PMC_HM       61     // hex mode 
#define PMC_HN       49     // RS232 module handshake on 
#define PMC_HO       15     // home
#define PMC_HS      232     // high speed 
#define PMC_IA       93     // index arm 
#define PMC_IB      165     // if accumulator below (less then) 
#define PMC_IC      161     // if bit clear in accumulator 
#define PMC_IE      162     // if accumulator equal 
#define PMC_IF      109     // if channel off then, else skip two 
#define PMC_IG      164     // if accumulator greater than 
#define PMC_II       68     // if intermediate 
#define PMC_IL       40     // set integration limit 
#define PMC_IM      114     // integration mode

#ifndef _WIN32
  #define PMC_IN    108     // if channel on then, else skip two 
#else
  #define PMC_MC_IN 108     // if channel on ... - WIN32 safe version
#endif

#define PMC_IP       96     // interrupt on absolute position 
#define PMC_IR       97     // interrupt on relative position 
#define PMC_IS      160     // if bit set in accumulator 
#define PMC_IU      163     // if accumulator unequal 
#define PMC_IX       95     // if contouring
#define PMC_JA       56     // jog acceleration 
#define PMC_JB      223     // jog deadband 
#define PMC_JD      221     // jog derivative gain
#define PMC_JF       58     // jog off 
#define PMC_JG      220     // jog gain 
#define PMC_JN       57     // jog on 
#define PMC_JO      222     // jog offset 
#define PMC_JP        6     // jump to command absolute 
#define PMC_JR        7     // jump to command relative 
#define PMC_JV       55     // jog velocity 
#define PMC_LC       69     // latch channel
#define PMC_LD      205     // limit delay
#define PMC_LF       54     // limit switch off 
#define PMC_LI      111     // learn incrementing
#define PMC_LL      210     // set motion low limit 
#define PMC_LM       52     // limit switch mode 
#define PMC_LN       53     // limit switch on
#define PMC_LO      274     // load file
#define PMC_LP      112     // learn position 
#define PMC_LS      230     // low speed 
#define PMC_LT      113     // learn target 
#define PMC_MA       16     // move absolute
#define PMC_MC        2     // macro command
#define PMC_MD        3     // macro definition
#define PMC_MF       17     // motor off
#define PMC_MI       18     // move incrementing
#define PMC_MJ        5     // macro jump
#define PMC_MN       19     // motor on 
#define PMC_MP       20     // move to point 
#define PMC_MR       21     // move relative 
#define PMC_MS      231     // medium speed 
#define PMC_MV      196     // set minimum velocity 
#define PMC_NO      120     // no operation 
#define PMC_NS      171     // no syncronization 
#define PMC_OA      249     // output analog value from accumulator 
#define PMC_OB      195     // output deadband
#define PMC_OD      195     // output deadband
#define PMC_OF      310     // open file
#define PMC_OM      216     // set output mode 
#define PMC_OO      203     // output offset

#ifndef USE_DCXAT200
  #define PMC_PA    217     // position capture arm
#else
  #define PMC_PA    289     // plotter acceleration
#endif

#define PMC_PB      243     // put byte
#define PMC_PC      128     // prompt character 

#ifndef USE_DCXAT200
  #define PMC_PD    218     // position display
#else
  #define PMC_PD    293     // pen down macro
#endif

#define PMC_PE      281     // plot enable
#define PMC_PF      280     // plot file
#define PMC_PH      115     // set phasing 
#define PMC_PI      291     // plotter initialize macro
#define PMC_PL      245     // put long
#define PMC_PM       23     // position mode 
#define PMC_PP      237     // profile parabolic
#define PMC_PQ      290     // plot quick velocity
#define PMC_PR      212     // position recording
#define PMC_PS      238     // profile s-curve 
#define PMC_PT      239     // profile trapezoidal 
#define PMC_PU      292     // pen up macro
#define PMC_PV      288     // plotter velocity
#define PMC_PW      244     // put word
#define PMC_PX      282     // plotter x axis
#define PMC_PY      283     // plotter y axis
#define PMC_QM        9     // torque mode 
#define PMC_RA      131     // accumulator load with register 
#define PMC_RB      150     // read accumulator Byte 

#ifndef USE_DCXAT200
  #define PMC_RC     41     // reset counter 
#else
  #define PMC_RC    277     // restore configuration 
#endif

#ifndef USE_DCXAT200
  #define PMC_RD    147     // read accumulator double (64 bit floating point) 
#else
  #define PMC_RD    182     // arc radius
#endif

#define PMC_RF      273     // remove file
#define PMC_RI      191     // position record interval
#define PMC_RL      152     // read accumulator long 
#define PMC_RM        4     // reset macros 
#define PMC_RP      100     // repeat 
#define PMC_RR      182     // arc radius
#define PMC_RT       42     // reset system 
#define PMC_RV      146     // read accumulator float (32 bit floating point) 
#define PMC_RW      151     // read accumulator word 
#define PMC_RX      184     // radius x
#define PMC_RY      185     // radius y
#define PMC_SA       43     // set acceleration 
#define PMC_SB      122     // select bank
#define PMC_SC      276     // save configuration 
#define PMC_SD       44     // set derivative gain 
#define PMC_SE       25     // stop on error 
#define PMC_SF       62     // step full 
#define PMC_SG       45     // set gain 
#define PMC_SH       63     // step Half 
#define PMC_SI       46     // set integral gain 
#define PMC_SL      144     // accumulator shift left 
#define PMC_SM       26     // set master 
#define PMC_SN      170     // syncroniztion on 
#define PMC_SP      294     // select pen macro
#define PMC_SQ      116     // set torque 
#define PMC_SR      145     // accumulator shift right 
#define PMC_SS       29     // set slave ratio 
#define PMC_ST       22     // stop 
#define PMC_SV       47     // set velocity 
#define PMC_TA       73     // tell a/d 
#define PMC_TB       91     // tell breakpoint position 
#define PMC_TC       74     // tell channel 
#define PMC_TD       75     // tell derivative gain 
#define PMC_TE       76     // tell error
#define PMC_TF       77     // tell following error 
#define PMC_TG       78     // tell position gain 
#define PMC_TI       79     // tell integral gain 
#define PMC_TK       92     // tell velocity constant 
#define PMC_TL       80     // tell integration limit 
#define PMC_TM       81     // tell macros 
#define PMC_TO       89     // tell optimal 
#define PMC_TP       82     // tell position 
#define PMC_TQ      209     // tell torque 
#define PMC_TR       87     // tell register 
#define PMC_TS       83     // tell status 
#define PMC_TT       84     // tell target 
#define PMC_TV       85     // tell velocity 
#define PMC_TX       88     // tell contouring count 
#define PMC_TY      272     // type file
#define PMC_TZ       90     // tell index 
#define PMC_UA      156     // use axis
#define PMC_UC      159     // user command
#define PMC_UK      215     // set user output constant 
#define PMC_UL      158     // user load
#define PMC_UO      179     // set user offset 
#define PMC_UR      177     // set user rate conversion 
#define PMC_US      175     // set user scale 
#define PMC_UT      178     // set user time conversion 
#define PMC_UZ      176     // set user zero 
#define PMC_VA      173     // vector acceleration 
#define PMC_VD      174     // vector deceleration 
#define PMC_VE       86     // tell version 
#define PMC_VG      119     // set velocity gain 
#define PMC_VM       24     // velocity mode 
#define PMC_VO      224     // set velocity override 
#define PMC_VV      172     // vector velocity 
#define PMC_WA      101     // wait (time) 
#define PMC_WB      153     // write accumulator byte 
#define PMC_WD      149     // write accumulator double (64 bit floating point) 
#define PMC_WE      102     // wait edge 
#define PMC_WF      103     // wait for channel off 
#define PMC_WI       94     // wait for index 
#define PMC_WL      155     // write accumulator long 
#define PMC_WN      104     // wait for channel on 
#define PMC_WP       98     // wait for absolute position 
#define PMC_WR       99     // wait for relative position 
#define PMC_WS      105     // wait for stop 
#define PMC_WT      198     // wait for target 
#define PMC_WV      148     // write accumulator float (32 bit floating point) 
#define PMC_WW      154     // write accumulator word 
#define PMC_XF      126     // set xoff character 
#define PMC_XN      125     // set xon character 
#define PMC_XO      286     // plotter x offset
#define PMC_XS      284     // plotter x scale
#define PMC_YO      287     // plotter y offset
#define PMC_YS      285     // plotter y scale

#endif
