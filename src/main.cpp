#define SERIALBASIC             //define this for using serial terminal in basic
#define websocketLineLength 64  //a print line longer than this will be split

const char *ssid = "rhombus";
const char *pass = "blindhike77";
const char *softApSsid = "ESP32-1";
const char *softApPass = "password";

#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include <LoopbackStream.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <webServer.h>
#include <webSocketsServer.h>

#define blinkLED 33  //blinks when connecting, then stays on. On the esp cam, it's 33. On the dev boards, it's 2

#include "rover.h"
uint8_t joystick = 255;  //global variable to store joystick state
bool joystickLock = 0;   //and another for locking while reading

WebSocketsServer webSocket = WebSocketsServer(81);

WebServer webServer(80);
File fsUploadFile;
//***********************************WEB SERVER FUNCTIONS**********************************
String getContentType(String filename) {
    if (webServer.hasArg("download")) {
        return "application/octet-stream";
    } else if (filename.endsWith(".htm")) {
        return "text/html";
    } else if (filename.endsWith(".html")) {
        return "text/html";
    } else if (filename.endsWith(".css")) {
        return "text/css";
    } else if (filename.endsWith(".js")) {
        return "application/javascript";
    } else if (filename.endsWith(".png")) {
        return "image/png";
    } else if (filename.endsWith(".gif")) {
        return "image/gif";
    } else if (filename.endsWith(".jpg")) {
        return "image/jpeg";
    } else if (filename.endsWith(".ico")) {
        return "image/x-icon";
    } else if (filename.endsWith(".xml")) {
        return "text/xml";
    } else if (filename.endsWith(".pdf")) {
        return "application/x-pdf";
    } else if (filename.endsWith(".zip")) {
        return "application/x-zip";
    } else if (filename.endsWith(".gz")) {
        return "application/x-gzip";
    }
    return "text/plain";
}

bool exists(String path) {
    bool yes = false;
    File file = SPIFFS.open(path, "r");
    if (!file.isDirectory()) {
        yes = true;
    }
    file.close();
    return yes;
}
//**********************************END OF WEB SERVER FUNCTIONS****************************

#define SerialDebug Serial

LoopbackStream streamKeyboard(256);
LoopbackStream streamScreen(256);

TaskHandle_t TaskBasic;

////////////////////////////////////////////////////////////////////////////////
// TinyBasic Plus
////////////////////////////////////////////////////////////////////////////////
//
// Authors:
//    Gordon Brandly (Tiny Basic for 68000)
//    Mike Field <hamster@snap.net.nz> (Arduino Basic) (port to Arduino)
//    Scott Lawrence <yorgle@gmail.com> (TinyBasic Plus) (features, etc)
//
// Contributors:
//          Brian O'Dell <megamemnon@megamemnon.com> (INPUT)
//    (full list tbd)

//  For full history of Tiny Basic, please see the wikipedia entry here:
//    https://en.wikipedia.org/wiki/Tiny_BASIC

// LICENSING NOTES:
//    Mike Field based his C port of Tiny Basic on the 68000
//    Tiny BASIC which carried the following license:
/*
******************************************************************
*                                                                *
*               Tiny BASIC for the Motorola MC68000              *
*                                                                *
* Derived from Palo Alto Tiny BASIC as published in the May 1976 *
* issue of Dr. Dobb's Journal.  Adapted to the 68000 by:         *
*       Gordon Brandly                                           *
*       12147 - 51 Street                                        *
*       Edmonton AB  T5W 3G8                                     *
*       Canada                                                   *
*       (updated mailing address for 1996)                       *
*                                                                *
* This version is for MEX68KECB Educational Computer Board I/O.  *
*                                                                *
******************************************************************
*    Copyright (C) 1984 by Gordon Brandly. This program may be   *
*    freely distributed for personal use only. All commercial    *
*                      rights are reserved.                      *
******************************************************************
*/
//    ref: http://members.shaw.ca:80/gbrandly/68ktinyb.html
//
//    However, Mike did not include a license of his own for his
//    version of this.
//    ref: http://hamsterworks.co.nz/mediawiki/index.php/Arduino_Basic
//
//    From discussions with him, I felt that the MIT license is
//    the most applicable to his intent.
//
//    I am in the process of further determining what should be
//    done wrt licensing further.  This entire header will likely
//    change with the next version 0.16, which will hopefully nail
//    down the whole thing so we can get back to implementing
//    features instead of licenses.  Thank you for your time.

#define kVersion "v0.77"

// v0.15: 2018-06-23
//      Integrating some contributions
//      Corrected some of the #ifdef nesting atop this page
//      Licensing issues beginning to be addressed

// v0.14: 2013-11-07
//      Modified Input command to accept an expression using getn()
//      Syntax is "input x" where x is any variable
//      NOTE: This only works for numbers, expressions. not strings.
//
// v0.13: 2013-03-04
//      Support for Arduino 1.5 (SPI.h included, additional changes for DUE support)
//
// v0.12: 2013-03-01
//      EEPROM load and save routines added: EFORMAT, ELIST, ELOAD, ESAVE, ECHAIN
//      added EAUTORUN option (chains to EEProm saved program on startup)
//      Bugfixes to build properly on non-arduino systems (PROGMEM #define workaround)
//      cleaned up a bit of the #define options wrt TONE
//
// v0.11: 2013-02-20
//      all display strings and tables moved to PROGMEM to save space
//      removed second serial
//      removed pinMode completely, autoconf is explicit
//      beginnings of EEPROM related functionality (new,load,save,list)
//
// v0.10: 2012-10-15
//      added kAutoConf, which eliminates the "PINMODE" statement.
//      now, DWRITE,DREAD,AWRITE,AREAD automatically set the PINMODE appropriately themselves.
//      should save a few bytes in your programs.
//
// v0.09: 2012-10-12
//      Fixed directory listings.  FILES now always works. (bug in the SD library)
//      ref: http://arduino.cc/forum/index.php/topic,124739.0.html
//      fixed filesize printouts (added printUnum for unsigned numbers)
//      #defineable baud rate for slow connection throttling
//e
// v0.08: 2012-10-02
//      Tone generation through piezo added (TONE, TONEW, NOTONE)
//
// v0.07: 2012-09-30
//      Autorun buildtime configuration feature
//
// v0.06: 2012-09-27
//      Added optional second serial input, used for an external keyboard
//
// v0.05: 2012-09-21
//      CHAIN to load and run a second file
//      RND,RSEED for random stuff
//      Added "!=" for "<>" synonym
//      Added "END" for "STOP" synonym (proper name for the functionality anyway)
//
// v0.04: 2012-09-20
//      DELAY ms   - for delaying
//      PINMODE <pin>, INPUT|IN|I|OUTPUT|OUT|O
//      DWRITE <pin>, HIGH|HI|1|LOW|LO|0
//      AWRITE <pin>, [0..255]
//      fixed "save" appending to existing files instead of overwriting
// 	Updated for building desktop command line app (incomplete)
//
// v0.03: 2012-09-19
//	Integrated Jurg Wullschleger whitespace,unary fix
//	Now available through github
//	Project renamed from "Tiny Basic in C" to "TinyBasic Plus"
//
// v0.02b: 2012-09-17  Scott Lawrence <yorgle@gmail.com>
// 	Better FILES listings
//
// v0.02a: 2012-09-17  Scott Lawrence <yorgle@gmail.com>
// 	Support for SD Library
// 	Added: SAVE, FILES (mostly works), LOAD (mostly works) (redirects IO)
// 	Added: MEM, ? (PRINT)
// 	Quirk:  "10 LET A=B+C" is ok "10 LET A = B + C" is not.
// 	Quirk:  INPUT seems broken?

// IF testing with Visual C, this needs to be the first thing in the file.
//#include "stdafx.h"

char eliminateCompileErrors = 1;  // fix to suppress arduino build errors

// hack to let makefiles work with this file unchanged
#ifdef FORCE_DESKTOP
#undef ARDUINO
#include "desktop.h"
#else
#define ARDUINO 1
#endif

////////////////////////////////////////////////////////////////////////////////
// Feature option configuration...

// This enables LOAD, SAVE, FILES commands through the Arduino SD Library
// it adds 9k of usage as well.
//#define ENABLE_FILEIO 1
//#undef ENABLE_FILEIO

// this turns on "autorun".  if there's FileIO, and a file "autorun.bas",
// then it will load it and run it when starting up
//#define ENABLE_AUTORUN 1
#undef ENABLE_AUTORUN
// and this is the file that gets run
#define kAutorunFilename "autorun.bas"

// this is the alternate autorun.  Autorun the program in the eeprom.
// it will load whatever is in the EEProm and run it
#define ENABLE_EAUTORUN 1
//#undef ENABLE_EAUTORUN

// this will enable the "TONE", "NOTONE" command using a piezo
// element on the specified pin.  Wire the red/positive/piezo to the kPiezoPin,
// and the black/negative/metal disc to ground.
// it adds 1.5k of usage as well.
//#define ENABLE_TONES 1
#undef ENABLE_TONES
#define kPiezoPin 5

// we can use the EEProm to store a program during powerdown.  This is
// 1kbyte on the '328, and 512 bytes on the '168.  Enabling this here will
// allow for this funcitonality to work.  Note that this only works on AVR
// arduino.  Disable it for DUE/other devices.
//#define ENABLE_EEPROM 1
#undef ENABLE_EEPROM

// Sometimes, we connect with a slower device as the console.
// Set your console D0/D1 baud rate here (9600 baud default)
#define kConsoleBaud 115200

////////////////////////////////////////////////////////////////////////////////
// fixes for RAMEND on some platforms
// #ifndef RAMEND
//     // RAMEND is defined for Uno type Arduinos
// #ifdef ARDUINO
//     // probably DUE or 8266?
// #ifdef ESP8266
// #define RAMEND (8192 - 1)
// #else
//     // probably DUE - ARM rather than AVR
// #define RAMEND (4096 - 1)
// #endif
// #endif
// #endif
#define RAMEND (65536 - 1)

// Enable memory alignment for certain processers (e.g. some ESP8266-based devices)
#ifdef ESP8266
// Uses up to one extra byte per program line of memory
#define ALIGN_MEMORY 1
#else
#undef ALIGN_MEMORY
#endif

#ifndef ARDUINO
// not an arduino, so we can disable these features.
// turn off EEProm
#undef ENABLE_EEPROM
#undef ENABLE_TONES
#endif

// includes, and settings for Arduino-specific features
#ifdef ARDUINO

// EEPROM
#ifdef ENABLE_EEPROM
#include <EEPROM.h> /* NOTE: case sensitive */
int eepos = 0;
#endif

//for saving/loading basic programs from SPIFFS
File fp;

// set up our RAM buffer size for program and user input
// NOTE: This number will have to change if you include other libraries.
//       It is also an estimation.  Might require adjustments...
#ifdef ENABLE_FILEIO
#define kRamFileIO (1030) /* approximate */
#else
#define kRamFileIO (0)
#endif

#ifdef ENABLE_TONES
#define kRamTones (40)
#else
#define kRamTones (0)
#endif

#define kRamSize (RAMEND - 1160 - kRamFileIO - kRamTones)

#endif /* ARDUINO Specifics */

// set up file includes for things we need, or desktop specific stuff.

#ifdef ARDUINO
// Use pgmspace/PROGMEM directive to store strings in progmem to save RAM
//#include <avr/pgmspace.h>
#else
#include <stdio.h>
#include <stdlib.h>
#undef ENABLE_TONES

// size of our program ram
#define kRamSize 64 * 1024 /* arbitrary - not dependant on libraries */

#ifdef ENABLE_FILEIO
FILE *fp;
#endif
#endif

////////////////////

// memory alignment
//  necessary for some esp8266-based devices
#ifdef ALIGN_MEMORY
// Align memory addess x to an even page
#define ALIGN_UP(x) ((unsigned char *)(((unsigned int)(x + 1) >> 1) << 1))
#define ALIGN_DOWN(x) ((unsigned char *)(((unsigned int)x >> 1) << 1))
#else
#define ALIGN_UP(x) x
#define ALIGN_DOWN(x) x
#endif

////////////////////
// various other desktop-tweaks and such.

#ifndef boolean
#define boolean int
#define true 1
#define false 0
#endif

#ifndef byte
typedef unsigned char byte;
#endif

// some catches for AVR based text string stuff...
#ifndef PROGMEM
#define PROGMEM
#endif
#ifndef pgm_read_byte
#define pgm_read_byte(A) *(A)
#endif

////////////////////

// functions defined elsehwere
void cmd_Files(void);
unsigned char *filenameWord(void);

// some settings based things

boolean inhibitOutput = false;
static boolean runAfterLoad = false;
static boolean triggerRun = false;

// these will select, at runtime, where IO happens through for load/save
enum {
    kStreamSerial = 0,
    kStreamEEProm,
    kStreamFile
};
static unsigned char inStream = kStreamSerial;
static unsigned char outStream = kStreamSerial;

////////////////////////////////////////////////////////////////////////////////
// ASCII Characters
#define CR '\r'
#define NL '\n'
#define LF 0x0a
#define TAB '\t'
#define BELL '\b'
#define SPACE ' '
#define SQUOTE '\''
#define DQUOTE '\"'
#define CTRLC 0x03
#define CTRLH 0x08
#define CTRLS 0x13
#define CTRLX 0x18

typedef short unsigned LINENUM;
#ifdef ARDUINO
#define ECHO_CHARS 1
#else
#define ECHO_CHARS 0
#endif

static unsigned char program[kRamSize];
static const char *sentinel = "HELLO";
static unsigned char *txtpos, *list_line, *tmptxtpos;
static unsigned char expression_error;
static unsigned char *tempsp;

/***********************************************************/
// Keyword table and constants - the last character has 0x80 added to it
const static unsigned char keywords[] = {
    'L', 'I', 'S', 'T' + 0x80,
    'L', 'O', 'A', 'D' + 0x80,
    'N', 'E', 'W' + 0x80,
    'R', 'U', 'N' + 0x80,
    'S', 'A', 'V', 'E' + 0x80,
    'N', 'E', 'X', 'T' + 0x80,
    'L', 'E', 'T' + 0x80,
    'I', 'F' + 0x80,
    'G', 'O', 'T', 'O' + 0x80,
    'G', 'O', 'S', 'U', 'B' + 0x80,
    'R', 'E', 'T', 'U', 'R', 'N' + 0x80,
    'R', 'E', 'M' + 0x80,
    'F', 'O', 'R' + 0x80,
    'I', 'N', 'P', 'U', 'T' + 0x80,
    'P', 'R', 'I', 'N', 'T' + 0x80,
    'P', 'O', 'K', 'E' + 0x80,
    'S', 'T', 'O', 'P' + 0x80,
    'B', 'Y', 'E' + 0x80,
    'F', 'I', 'L', 'E', 'S' + 0x80,
    'K', 'I', 'L', 'L' + 0x80,
    'M', 'E', 'M' + 0x80,
    '?' + 0x80,
    '\'' + 0x80,
    'A', 'W', 'R', 'I', 'T', 'E' + 0x80,
    'D', 'W', 'R', 'I', 'T', 'E' + 0x80,
    'D', 'E', 'L', 'A', 'Y' + 0x80,
    'E', 'N', 'D' + 0x80,
    'R', 'S', 'E', 'E', 'D' + 0x80,
    'C', 'H', 'A', 'I', 'N' + 0x80,
    'D', 'R', 'I', 'V', 'E' + 0x80,
    'C', 'L', 'A', 'W' + 0x80,
#ifdef ENABLE_TONES
    'T', 'O', 'N', 'E', 'W' + 0x80,
    'T', 'O', 'N', 'E' + 0x80,
    'N', 'O', 'T', 'O', 'N', 'E' + 0x80,
#endif
#ifdef ARDUINO
#ifdef ENABLE_EEPROM
    'E', 'C', 'H', 'A', 'I', 'N' + 0x80,
    'E', 'L', 'I', 'S', 'T' + 0x80,
    'E', 'L', 'O', 'A', 'D' + 0x80,
    'E', 'F', 'O', 'R', 'M', 'A', 'T' + 0x80,
    'E', 'S', 'A', 'V', 'E' + 0x80,
#endif
#endif
    0};

// by moving the command list to an enum, we can easily remove sections
// above and below simultaneously to selectively obliterate functionality.
enum {
    KW_LIST = 0,
    KW_LOAD,
    KW_NEW,
    KW_RUN,
    KW_SAVE,
    KW_NEXT,
    KW_LET,
    KW_IF,
    KW_GOTO,
    KW_GOSUB,
    KW_RETURN,
    KW_REM,
    KW_FOR,
    KW_INPUT,
    KW_PRINT,
    KW_POKE,
    KW_STOP,
    KW_BYE,
    KW_FILES,
    KW_KILL,
    KW_MEM,
    KW_QMARK,
    KW_QUOTE,
    KW_AWRITE,
    KW_DWRITE,
    KW_DELAY,
    KW_END,
    KW_RSEED,
    KW_CHAIN,
    KW_ROVDRIVE,
    KW_ROVCLAW,
#ifdef ENABLEDTONES
    KW_TONEW,
    KW_TONE,
    KW_NOTONE,
#endif
#ifdef ARDUINO
#ifdef ENABLE_EEPROM
    KW_ECHAIN,
    KW_ELIST,
    KW_ELOAD,
    KW_EFORMAT,
    KW_ESAVE,
#endif
#endif
    KW_DEFAULT /* always the final one*/
};

struct stack_for_frame {
    char frame_type;
    char for_var;
    short int terminal;
    short int step;
    unsigned char *current_line;
    unsigned char *txtpos;
};

struct stack_gosub_frame {
    char frame_type;
    unsigned char *current_line;
    unsigned char *txtpos;
};

const static unsigned char func_tab[] = {
    'P', 'E', 'E', 'K' + 0x80,
    'A', 'B', 'S' + 0x80,
    'A', 'R', 'E', 'A', 'D' + 0x80,
    'D', 'R', 'E', 'A', 'D' + 0x80,
    'R', 'N', 'D' + 0x80,
    'J', 'O', 'Y' + 0x80,
    0};
#define FUNC_PEEK 0
#define FUNC_ABS 1
#define FUNC_AREAD 2
#define FUNC_DREAD 3
#define FUNC_RND 4
#define FUNC_JOY 5
#define FUNC_UNKNOWN 6

const static unsigned char to_tab[] = {
    'T', 'O' + 0x80,
    0};

const static unsigned char step_tab[] = {
    'S', 'T', 'E', 'P' + 0x80,
    0};

const static unsigned char relop_tab[] = {
    '>', '=' + 0x80,
    '<', '>' + 0x80,
    '>' + 0x80,
    '=' + 0x80,
    '<', '=' + 0x80,
    '<' + 0x80,
    '!', '=' + 0x80,
    0};

#define RELOP_GE 0
#define RELOP_NE 1
#define RELOP_GT 2
#define RELOP_EQ 3
#define RELOP_LE 4
#define RELOP_LT 5
#define RELOP_NE_BANG 6
#define RELOP_UNKNOWN 7

const static unsigned char highlow_tab[] = {
    'H', 'I', 'G', 'H' + 0x80,
    'H', 'I' + 0x80,
    'L', 'O', 'W' + 0x80,
    'L', 'O' + 0x80,
    0};
#define HIGHLOW_HIGH 1
#define HIGHLOW_UNKNOWN 4

#define STACK_SIZE (sizeof(struct stack_for_frame) * 5)
#define VAR_SIZE sizeof(short int)  // Size of variables in bytes

static unsigned char *stack_limit;
static unsigned char *program_start;
static unsigned char *program_end;
static unsigned char *stack;  // Software stack for things that should go on the CPU stack
static unsigned char *variables_begin;
static unsigned char *current_line;
static unsigned char *sp;
#define STACK_GOSUB_FLAG 'G'
#define STACK_FOR_FLAG 'F'
static unsigned char table_index;
static LINENUM linenum;

static const unsigned char okmsg[] = "OK";
static const unsigned char whatmsg[] = "What? ";
static const unsigned char howmsg[] = "How?";
static const unsigned char sorrymsg[] = "Sorry!";
static const unsigned char initmsg[] = "Winter Camp BASIC " kVersion;
static const unsigned char memorymsg[] = " bytes free.";
#ifdef ARDUINO
#ifdef ENABLE_EEPROM
static const unsigned char eeprommsg[] = " EEProm bytes total.";
static const unsigned char eepromamsg[] = " EEProm bytes available.";
#endif
#endif
static const unsigned char breakmsg[] = "break!";
static const unsigned char unimplimentedmsg[] = "Unimplemented";
static const unsigned char backspacemsg[] = "\b \b";
static const unsigned char indentmsg[] = "    ";
static const unsigned char sderrormsg[] = "SD card error.";
static const unsigned char sdfilemsg[] = "SD file error.";
static const unsigned char sdnofile[] = "No such File.";
static const unsigned char dirextmsg[] = "(dir)";
static const unsigned char slashmsg[] = "/";
static const unsigned char spacemsg[] = " ";

static int inchar(void);
static void outchar(unsigned char c);
static void line_terminator(void);
static short int expression(void);
static unsigned char breakcheck(void);
/***************************************************************************/
static void ignore_blanks(void) {
    while (*txtpos == SPACE || *txtpos == TAB)
        txtpos++;
}

/***************************************************************************/
static void scantable(const unsigned char *table) {
    int i = 0;
    table_index = 0;
    while (1) {
        // Run out of table entries?
        if (pgm_read_byte(table) == 0)
            return;

        // Do we match this character?
        if (txtpos[i] == pgm_read_byte(table)) {
            i++;
            table++;
        } else {
            // do we match the last character of keywork (with 0x80 added)? If so, return
            if (txtpos[i] + 0x80 == pgm_read_byte(table)) {
                txtpos += i + 1;  // Advance the pointer to following the keyword
                ignore_blanks();
                return;
            }

            // Forward to the end of this keyword
            while ((pgm_read_byte(table) & 0x80) == 0)
                table++;

            // Now move on to the first character of the next word, and reset the position index
            table++;
            table_index++;
            ignore_blanks();
            i = 0;
        }
    }
}

/***************************************************************************/
static void pushb(unsigned char b) {
    sp--;
    *sp = b;
}

/***************************************************************************/
static unsigned char popb() {
    unsigned char b;
    b = *sp;
    sp++;
    return b;
}

/***************************************************************************/
void printnum(int num) {
    int digits = 0;

    if (num < 0) {
        num = -num;
        outchar('-');
    }
    do {
        pushb(num % 10 + '0');
        num = num / 10;
        digits++;
    } while (num > 0);

    while (digits > 0) {
        outchar(popb());
        digits--;
    }
}

void printUnum(unsigned int num) {
    int digits = 0;

    do {
        pushb(num % 10 + '0');
        num = num / 10;
        digits++;
    } while (num > 0);

    while (digits > 0) {
        outchar(popb());
        digits--;
    }
}

/***************************************************************************/
static unsigned short testnum(void) {
    unsigned short num = 0;
    ignore_blanks();

    while (*txtpos >= '0' && *txtpos <= '9') {
        // Trap overflows
        if (num >= 0xFFFF / 10) {
            num = 0xFFFF;
            break;
        }

        num = num * 10 + *txtpos - '0';
        txtpos++;
    }
    return num;
}

/***************************************************************************/
static unsigned char print_quoted_string(void) {
    int i = 0;
    unsigned char delim = *txtpos;
    if (delim != '"' && delim != '\'')
        return 0;
    txtpos++;

    // Check we have a closing delimiter
    while (txtpos[i] != delim) {
        if (txtpos[i] == NL)
            return 0;
        i++;
    }

    // Print the characters
    while (*txtpos != delim) {
        outchar(*txtpos);
        txtpos++;
    }
    txtpos++;  // Skip over the last delimiter

    return 1;
}

/***************************************************************************/
void printmsgNoNL(const unsigned char *msg) {
    while (pgm_read_byte(msg) != 0) {
        outchar(pgm_read_byte(msg++));
    };
}

/***************************************************************************/
void printmsg(const unsigned char *msg) {
    printmsgNoNL(msg);
    line_terminator();
}

/***************************************************************************/
static void getln(char prompt) {
    outchar(prompt);
    txtpos = program_end + sizeof(LINENUM);

    while (1) {
        char c = inchar();
        switch (c) {
            case NL:
                //break;
            case CR:
                line_terminator();
                // Terminate all strings with a NL
                txtpos[0] = NL;
                return;
            case CTRLH:
            case 127:                           //putty default for backspace key
                if (txtpos - 2 <= program_end)  //offset of 2 here seems to stop backspace from going behind the ">" prompt
                    break;
                txtpos--;

                printmsgNoNL(backspacemsg);
                break;
            default:
                // We need to leave at least one space to allow us to shuffle the line into order
                if (txtpos == variables_begin - 2)
                    outchar(BELL);
                else {
                    txtpos[0] = c;
                    txtpos++;
                    outchar(c);
                }
        }
    }
}

/***************************************************************************/
static unsigned char *findline(void) {
    unsigned char *line = program_start;
    while (1) {
        if (line == program_end)
            return line;

        if (((LINENUM *)line)[0] >= linenum)
            return line;

        // Add the line length onto the current address, to get to the next line;
        line += line[sizeof(LINENUM)];
    }
}

/***************************************************************************/
static void toUppercaseBuffer(void) {
    unsigned char *c = program_end + sizeof(LINENUM);
    unsigned char quote = 0;

    while (*c != NL) {
        // Are we in a quoted string?
        if (*c == quote)
            quote = 0;
        else if (*c == '"' || *c == '\'')
            quote = *c;
        else if (quote == 0 && *c >= 'a' && *c <= 'z')
            *c = *c + 'A' - 'a';
        c++;
    }
}

/***************************************************************************/
void printline() {
    LINENUM line_num;

    line_num = *((LINENUM *)(list_line));
    list_line += sizeof(LINENUM) + sizeof(char);

    // Output the line */
    printnum(line_num);
    outchar(' ');
    while (*list_line != NL) {
        outchar(*list_line);
        list_line++;
    }
    list_line++;
#ifdef ALIGN_MEMORY
    // Start looking for next line on even page
    if (ALIGN_UP(list_line) != list_line)
        list_line++;
#endif
    line_terminator();
}

/***************************************************************************/
static short int expr4(void) {
    // fix provided by Jurg Wullschleger wullschleger@gmail.com
    // fixes whitespace and unary operations
    ignore_blanks();

    if (*txtpos == '-') {
        txtpos++;
        return -expr4();
    }
    // end fix

    if (*txtpos == '0') {
        txtpos++;
        return 0;
    }

    if (*txtpos >= '1' && *txtpos <= '9') {
        short int a = 0;
        do {
            a = a * 10 + *txtpos - '0';
            txtpos++;
        } while (*txtpos >= '0' && *txtpos <= '9');
        return a;
    }

    // Is it a function or variable reference?
    if (txtpos[0] >= 'A' && txtpos[0] <= 'Z') {
        short int a;
        // Is it a variable reference (single alpha)
        if (txtpos[1] < 'A' || txtpos[1] > 'Z') {
            a = ((short int *)variables_begin)[*txtpos - 'A'];
            txtpos++;
            return a;
        }

        // Is it a function with a single parameter
        scantable(func_tab);
        if (table_index == FUNC_UNKNOWN)
            goto expr4_error;

        unsigned char f = table_index;

        switch (f) {  //for those functions that don't need a () after it
            case FUNC_JOY: {
                joystickLock = 1;
                uint8_t j = joystick;
                joystickLock = 0;
                return j;
            }
            default:
                break;
        }

        if (*txtpos != '(')
            goto expr4_error;

        txtpos++;
        a = expression();
        if (*txtpos != ')')
            goto expr4_error;
        txtpos++;
        switch (f) {
            case FUNC_PEEK:
                return program[a];

            case FUNC_ABS:
                if (a < 0)
                    return -a;
                return a;

#ifdef ARDUINO
            case FUNC_AREAD:
                pinMode(a, INPUT);
                return analogRead(a);
            case FUNC_DREAD:
                pinMode(a, INPUT);
                return digitalRead(a);
#endif

            case FUNC_RND:
#ifdef ARDUINO
                return (random(a));
#else
                return (rand() % a);
#endif
        }
    }

    if (*txtpos == '(') {
        short int a;
        txtpos++;
        a = expression();
        if (*txtpos != ')')
            goto expr4_error;

        txtpos++;
        return a;
    }

expr4_error:
    expression_error = 1;
    return 0;
}

/***************************************************************************/
static short int expr3(void) {
    short int a, b;

    a = expr4();

    ignore_blanks();  // fix for eg:  100 a = a + 1

    while (1) {
        if (*txtpos == '*') {
            txtpos++;
            b = expr4();
            a *= b;
        } else if (*txtpos == '/') {
            txtpos++;
            b = expr4();
            if (b != 0)
                a /= b;
            else
                expression_error = 1;
        } else
            return a;
    }
}

/***************************************************************************/
static short int expr2(void) {
    short int a, b;

    if (*txtpos == '-' || *txtpos == '+')
        a = 0;
    else
        a = expr3();

    while (1) {
        if (*txtpos == '-') {
            txtpos++;
            b = expr3();
            a -= b;
        } else if (*txtpos == '+') {
            txtpos++;
            b = expr3();
            a += b;
        } else
            return a;
    }
}
/***************************************************************************/
static short int expression(void) {
    short int a, b;

    a = expr2();

    // Check if we have an error
    if (expression_error) return a;

    scantable(relop_tab);
    if (table_index == RELOP_UNKNOWN)
        return a;

    switch (table_index) {
        case RELOP_GE:
            b = expr2();
            if (a >= b) return 1;
            break;
        case RELOP_NE:
        case RELOP_NE_BANG:
            b = expr2();
            if (a != b) return 1;
            break;
        case RELOP_GT:
            b = expr2();
            if (a > b) return 1;
            break;
        case RELOP_EQ:
            b = expr2();
            if (a == b) return 1;
            break;
        case RELOP_LE:
            b = expr2();
            if (a <= b) return 1;
            break;
        case RELOP_LT:
            b = expr2();
            if (a < b) return 1;
            break;
    }
    return 0;
}

// returns 1 if the character is valid in a filename
static int isValidFnChar(char c) {
    if (c >= '0' && c <= '9') return 1;  // number
    if (c >= 'A' && c <= 'Z') return 1;  // LETTER
    if (c >= 'a' && c <= 'z') return 1;  // letter (for completeness)
    if (c == '_') return 1;
    if (c == '+') return 1;
    if (c == '.') return 1;
    if (c == '~') return 1;  // Window~1.txt

    return 0;
}

unsigned char *filenameWord(void) {
    // SDL - I wasn't sure if this functionality existed above, so I figured i'd put it here
    unsigned char *ret = txtpos;
    expression_error = 0;

    // make sure there are no quotes or spaces, search for valid characters
    //while(*txtpos == SPACE || *txtpos == TAB || *txtpos == SQUOTE || *txtpos == DQUOTE ) txtpos++;
    while (!isValidFnChar(*txtpos)) txtpos++;
    ret = txtpos;

    if (*ret == '\0') {
        expression_error = 1;
        return ret;
    }

    // now, find the next nonfnchar
    txtpos++;
    while (isValidFnChar(*txtpos)) txtpos++;
    if (txtpos != ret) *txtpos = '\0';

    // set the error code if we've got no string
    if (*ret == '\0') {
        expression_error = 1;
    }

    return ret;
}

/***************************************************************************/
static void line_terminator(void) {
    outchar(NL);
    outchar(CR);
}

void TaskBasiccode(void *pvParameters) {
    Serial.print("TinyBasicPlus running on core ");
    Serial.println(xPortGetCoreID());

    Serial.println(sentinel);
    printmsg(initmsg);

    for (;;) {  //originally loop() void
        unsigned char *start;
        unsigned char *newEnd;
        unsigned char linelen;
        boolean isDigital;
        boolean alsoWait = false;
        int val;

#ifdef ARDUINO
#ifdef ENABLE_TONES
        noTone(kPiezoPin);
#endif
#endif

        program_start = program;
        program_end = program_start;
        sp = program + sizeof(program);  // Needed for printnum
#ifdef ALIGN_MEMORY
        // Ensure these memory blocks start on even pages
        stack_limit = ALIGN_DOWN(program + sizeof(program) - STACK_SIZE);
        variables_begin = ALIGN_DOWN(stack_limit - 27 * VAR_SIZE);
#else
        stack_limit = program + sizeof(program) - STACK_SIZE;
        variables_begin = stack_limit - 27 * VAR_SIZE;
#endif

        // memory free
        printnum(variables_begin - program_end);
        printmsg(memorymsg);
#ifdef ARDUINO
#ifdef ENABLE_EEPROM
        // eprom size
        printnum(E2END + 1);
        printmsg(eeprommsg);
#endif /* ENABLE_EEPROM */
#endif /* ARDUINO */

    warmstart:
        // this signifies that it is running in 'direct' mode.
        current_line = 0;
        sp = program + sizeof(program);
        printmsg(okmsg);

    prompt:
        if (triggerRun) {
            triggerRun = false;
            current_line = program_start;
            goto execline;
        }

        getln('>');
        toUppercaseBuffer();

        txtpos = program_end + sizeof(unsigned short);

        // Find the end of the freshly entered line
        while (*txtpos != NL)
            txtpos++;

        // Move it to the end of program_memory
        {
            unsigned char *dest;
            dest = variables_begin - 1;
            while (1) {
                *dest = *txtpos;
                if (txtpos == program_end + sizeof(unsigned short))
                    break;
                dest--;
                txtpos--;
            }
            txtpos = dest;
        }

        // Now see if we have a line number
        linenum = testnum();
        ignore_blanks();
        if (linenum == 0)
            goto direct;

        if (linenum == 0xFFFF)
            goto qhow;

        // Find the length of what is left, including the (yet-to-be-populated) line header
        linelen = 0;
        while (txtpos[linelen] != NL)
            linelen++;
        linelen++;                                         // Include the NL in the line length
        linelen += sizeof(unsigned short) + sizeof(char);  // Add space for the line number and line length

        // Now we have the number, add the line header.
        txtpos -= 3;

#ifdef ALIGN_MEMORY
        // Line starts should always be on 16-bit pages
        if (ALIGN_DOWN(txtpos) != txtpos) {
            txtpos--;
            linelen++;
            // As the start of the line has moved, the data should move as well
            unsigned char *tomove;
            tomove = txtpos + 3;
            while (tomove < txtpos + linelen - 1) {
                *tomove = *(tomove + 1);
                tomove++;
            }
        }
#endif

        *((unsigned short *)txtpos) = linenum;
        txtpos[sizeof(LINENUM)] = linelen;

        // Merge it into the rest of the program
        start = findline();

        // If a line with that number exists, then remove it
        if (start != program_end && *((LINENUM *)start) == linenum) {
            unsigned char *dest, *from;
            unsigned tomove;

            from = start + start[sizeof(LINENUM)];
            dest = start;

            tomove = program_end - from;
            while (tomove > 0) {
                *dest = *from;
                from++;
                dest++;
                tomove--;
            }
            program_end = dest;
        }

        if (txtpos[sizeof(LINENUM) + sizeof(char)] == NL)  // If the line has no txt, it was just a delete
            goto prompt;

        // Make room for the new line, either all in one hit or lots of little shuffles
        while (linelen > 0) {
            unsigned int tomove;
            unsigned char *from, *dest;
            unsigned int space_to_make;

            space_to_make = txtpos - program_end;

            if (space_to_make > linelen)
                space_to_make = linelen;
            newEnd = program_end + space_to_make;
            tomove = program_end - start;

            // Source and destination - as these areas may overlap we need to move bottom up
            from = program_end;
            dest = newEnd;
            while (tomove > 0) {
                from--;
                dest--;
                *dest = *from;
                tomove--;
            }

            // Copy over the bytes into the new space
            for (tomove = 0; tomove < space_to_make; tomove++) {
                *start = *txtpos;
                txtpos++;
                start++;
                linelen--;
            }
            program_end = newEnd;
        }
        goto prompt;

        // unimplemented:
        //     printmsg(unimplimentedmsg);
        //     goto prompt;

    qhow:
        printmsg(howmsg);
        goto prompt;

    qwhat:
        printmsgNoNL(whatmsg);
        if (current_line != NULL) {
            unsigned char tmp = *txtpos;
            if (*txtpos != NL)
                *txtpos = '^';
            list_line = current_line;
            printline();
            *txtpos = tmp;
        }
        line_terminator();
        goto prompt;

    qsorry:
        printmsg(sorrymsg);
        goto warmstart;

    run_next_statement:
        while (*txtpos == ':')
            txtpos++;
        ignore_blanks();
        if (*txtpos == NL)
            goto execnextline;
        goto interperateAtTxtpos;

    direct:
        txtpos = program_end + sizeof(LINENUM);
        if (*txtpos == NL)
            goto prompt;

    interperateAtTxtpos:
        if (breakcheck()) {
            printmsg(breakmsg);
            goto warmstart;
        }

        scantable(keywords);

        switch (table_index) {
            case KW_DELAY: {
#ifdef ARDUINO
                expression_error = 0;
                val = expression();
                delay(val);
                goto execnextline;
#else
                goto unimplemented;
#endif
            }

            case KW_FILES:
                goto files;
            case KW_KILL:
                goto kill;
            case KW_LIST:
                goto list;
            case KW_CHAIN:
                goto chain;
            case KW_LOAD:
                goto load;
            case KW_MEM:
                goto mem;
            case KW_NEW:
                if (txtpos[0] != NL)
                    goto qwhat;
                program_end = program_start;
                goto prompt;
            case KW_RUN:
                current_line = program_start;
                goto execline;
            case KW_SAVE:
                goto save;
            case KW_NEXT:
                goto next;
            case KW_LET:
                goto assignment;
            case KW_IF:
                short int val;
                expression_error = 0;
                val = expression();
                if (expression_error || *txtpos == NL)
                    goto qhow;
                if (val != 0)
                    goto interperateAtTxtpos;
                goto execnextline;

            case KW_GOTO:
                expression_error = 0;
                linenum = expression();
                if (expression_error || *txtpos != NL)
                    goto qhow;
                current_line = findline();
                goto execline;

            case KW_GOSUB:
                goto gosub;
            case KW_RETURN:
                goto gosub_return;
            case KW_REM:
            case KW_QUOTE:
                goto execnextline;  // Ignore line completely
            case KW_FOR:
                goto forloop;
            case KW_INPUT:
                goto input;
            case KW_PRINT:
            case KW_QMARK:
                goto print;
            case KW_POKE:
                goto poke;
            case KW_ROVDRIVE:
                goto roverDrive;
            case KW_ROVCLAW:
                goto roverClaw;
            case KW_END:
            case KW_STOP:
                // This is the easy way to end - set the current line to the end of program attempt to run it
                if (txtpos[0] != NL)
                    goto qwhat;
                current_line = program_end;
                goto execline;
            case KW_BYE:
                // Leave the basic interperater
                return;

            case KW_AWRITE:  // AWRITE <pin>, HIGH|LOW
                isDigital = false;
                goto awrite;
            case KW_DWRITE:  // DWRITE <pin>, HIGH|LOW
                isDigital = true;
                goto dwrite;

            case KW_RSEED:
                goto rseed;

#ifdef ENABLE_TONES
            case KW_TONEW:
                alsoWait = true;
            case KW_TONE:
                goto tonegen;
            case KW_NOTONE:
                goto tonestop;
#endif

#ifdef ARDUINO
#ifdef ENABLE_EEPROM
            case KW_EFORMAT:
                goto eformat;
            case KW_ESAVE:
                goto esave;
            case KW_ELOAD:
                goto eload;
            case KW_ELIST:
                goto elist;
            case KW_ECHAIN:
                goto echain;
#endif
#endif

            case KW_DEFAULT:
                goto assignment;
            default:
                break;
        }

    execnextline:
        if (current_line == NULL)  // Processing direct commands?
            goto prompt;
        current_line += current_line[sizeof(LINENUM)];

    execline:
        yield();
        if (current_line == program_end)  // Out of lines to run
            goto warmstart;
        txtpos = current_line + sizeof(LINENUM) + sizeof(char);
        goto interperateAtTxtpos;

#ifdef ARDUINO
#ifdef ENABLE_EEPROM
    elist : {
        int i;
        for (i = 0; i < (E2END + 1); i++) {
            val = EEPROM.read(i);

            if (val == '\0') {
                goto execnextline;
            }

            if (((val < ' ') || (val > '~')) && (val != NL) && (val != CR)) {
                outchar('?');
            } else {
                outchar(val);
            }
        }
    }
        goto execnextline;

    eformat : {
        for (int i = 0; i < E2END; i++) {
            if ((i & 0x03f) == 0x20) outchar('.');
            EEPROM.write(i, 0);
        }
        outchar(LF);
    }
        goto execnextline;

    esave : {
        outStream = kStreamEEProm;
        eepos = 0;

        // copied from "List"
        list_line = findline();
        while (list_line != program_end) {
            printline();
        }
        outchar('\0');

        // go back to standard output, close the file
        outStream = kStreamSerial;

        goto warmstart;
    }

    echain:
        runAfterLoad = true;

    eload:
        // clear the program
        program_end = program_start;

        // load from a file into memory
        eepos = 0;
        inStream = kStreamEEProm;
        inhibitOutput = true;
        goto warmstart;
#endif /* ENABLE_EEPROM */
#endif

    input : {
        unsigned char var;
        int value;
        ignore_blanks();
        if (*txtpos < 'A' || *txtpos > 'Z')
            goto qwhat;
        var = *txtpos;
        txtpos++;
        ignore_blanks();
        if (*txtpos != NL && *txtpos != ':')
            goto qwhat;
    inputagain:
        tmptxtpos = txtpos;
        getln('?');
        toUppercaseBuffer();
        txtpos = program_end + sizeof(unsigned short);
        ignore_blanks();
        expression_error = 0;
        value = expression();
        if (expression_error)
            goto inputagain;
        ((short int *)variables_begin)[var - 'A'] = value;
        txtpos = tmptxtpos;

        goto run_next_statement;
    }

    forloop : {
        unsigned char var;
        short int initial, step, terminal;
        ignore_blanks();
        if (*txtpos < 'A' || *txtpos > 'Z')
            goto qwhat;
        var = *txtpos;
        txtpos++;
        ignore_blanks();
        if (*txtpos != '=')
            goto qwhat;
        txtpos++;
        ignore_blanks();

        expression_error = 0;
        initial = expression();
        if (expression_error)
            goto qwhat;

        scantable(to_tab);
        if (table_index != 0)
            goto qwhat;

        terminal = expression();
        if (expression_error)
            goto qwhat;

        scantable(step_tab);
        if (table_index == 0) {
            step = expression();
            if (expression_error)
                goto qwhat;
        } else
            step = 1;
        ignore_blanks();
        if (*txtpos != NL && *txtpos != ':')
            goto qwhat;

        if (!expression_error && *txtpos == NL) {
            struct stack_for_frame *f;
            if (sp + sizeof(struct stack_for_frame) < stack_limit)
                goto qsorry;

            sp -= sizeof(struct stack_for_frame);
            f = (struct stack_for_frame *)sp;
            ((short int *)variables_begin)[var - 'A'] = initial;
            f->frame_type = STACK_FOR_FLAG;
            f->for_var = var;
            f->terminal = terminal;
            f->step = step;
            f->txtpos = txtpos;
            f->current_line = current_line;
            goto run_next_statement;
        }
    }
        goto qhow;

    gosub:
        expression_error = 0;
        linenum = expression();
        if (!expression_error && *txtpos == NL) {
            struct stack_gosub_frame *f;
            if (sp + sizeof(struct stack_gosub_frame) < stack_limit)
                goto qsorry;

            sp -= sizeof(struct stack_gosub_frame);
            f = (struct stack_gosub_frame *)sp;
            f->frame_type = STACK_GOSUB_FLAG;
            f->txtpos = txtpos;
            f->current_line = current_line;
            current_line = findline();
            goto execline;
        }
        goto qhow;

    next:
        // Fnd the variable name
        ignore_blanks();
        if (*txtpos < 'A' || *txtpos > 'Z')
            goto qhow;
        txtpos++;
        ignore_blanks();
        if (*txtpos != ':' && *txtpos != NL)
            goto qwhat;

    gosub_return:
        // Now walk up the stack frames and find the frame we want, if present
        tempsp = sp;
        while (tempsp < program + sizeof(program) - 1) {
            switch (tempsp[0]) {
                case STACK_GOSUB_FLAG:
                    if (table_index == KW_RETURN) {
                        struct stack_gosub_frame *f = (struct stack_gosub_frame *)tempsp;
                        current_line = f->current_line;
                        txtpos = f->txtpos;
                        sp += sizeof(struct stack_gosub_frame);
                        goto run_next_statement;
                    }
                    // This is not the loop you are looking for... so Walk back up the stack
                    tempsp += sizeof(struct stack_gosub_frame);
                    break;
                case STACK_FOR_FLAG:
                    // Flag, Var, Final, Step
                    if (table_index == KW_NEXT) {
                        struct stack_for_frame *f = (struct stack_for_frame *)tempsp;
                        // Is the the variable we are looking for?
                        if (txtpos[-1] == f->for_var) {
                            short int *varaddr = ((short int *)variables_begin) + txtpos[-1] - 'A';
                            *varaddr = *varaddr + f->step;
                            // Use a different test depending on the sign of the step increment
                            if ((f->step > 0 && *varaddr <= f->terminal) || (f->step < 0 && *varaddr >= f->terminal)) {
                                // We have to loop so don't pop the stack
                                txtpos = f->txtpos;
                                current_line = f->current_line;
                                goto run_next_statement;
                            }
                            // We've run to the end of the loop. drop out of the loop, popping the stack
                            sp = tempsp + sizeof(struct stack_for_frame);
                            goto run_next_statement;
                        }
                    }
                    // This is not the loop you are looking for... so Walk back up the stack
                    tempsp += sizeof(struct stack_for_frame);
                    break;
                default:
                    //printf("Stack is stuffed!\n");
                    goto warmstart;
            }
        }
        // Didn't find the variable we've been looking for
        goto qhow;

    assignment : {
        short int value;
        short int *var;

        if (*txtpos < 'A' || *txtpos > 'Z')
            goto qhow;
        var = (short int *)variables_begin + *txtpos - 'A';
        txtpos++;

        ignore_blanks();

        if (*txtpos != '=')
            goto qwhat;
        txtpos++;
        ignore_blanks();
        expression_error = 0;
        value = expression();
        if (expression_error)
            goto qwhat;
        // Check that we are at the end of the statement
        if (*txtpos != NL && *txtpos != ':')
            goto qwhat;
        *var = value;
    }
        goto run_next_statement;
    poke : {
        short int value;
        unsigned char *address;

        // Work out where to put it
        expression_error = 0;
        value = expression();
        if (expression_error)
            goto qwhat;
        address = (unsigned char *)value;

        // check for a comma
        ignore_blanks();
        if (*txtpos != ',')
            goto qwhat;
        txtpos++;
        ignore_blanks();

        // Now get the value to assign
        expression_error = 0;
        value = expression();
        if (expression_error)
            goto qwhat;
        //printf("Poke %p value %i\n",address, (unsigned char)value);
        // Check that we are at the end of the statement
        if (*txtpos != NL && *txtpos != ':')
            goto qwhat;
    }
        goto run_next_statement;
    roverDrive : {
        short int value;

        // Work out where to put it
        expression_error = 0;
        value = expression();
        if (expression_error)
            goto qwhat;
        roverDrive(value);
        // Check that we are at the end of the statement
        if (*txtpos != NL && *txtpos != ':')
            goto qwhat;
    }
        goto run_next_statement;
    roverClaw : {
        short int value;

        // Work out where to put it
        expression_error = 0;
        value = expression();
        if (expression_error)
            goto qwhat;
        roverClaw(value);
        // Check that we are at the end of the statement
        if (*txtpos != NL && *txtpos != ':')
            goto qwhat;
    }
        goto run_next_statement;

    list:
        linenum = testnum();  // Retuns 0 if no line found.

        // Should be EOL
        if (txtpos[0] != NL)
            goto qwhat;

        // Find the line
        list_line = findline();
        while (list_line != program_end)
            printline();
        goto warmstart;

    print:
        // If we have an empty list then just put out a NL
        if (*txtpos == ':') {
            line_terminator();
            txtpos++;
            goto run_next_statement;
        }
        if (*txtpos == NL) {
            goto execnextline;
        }

        while (1) {
            ignore_blanks();
            if (print_quoted_string()) {
                ;
            } else if (*txtpos == '"' || *txtpos == '\'')
                goto qwhat;
            else {
                short int e;
                expression_error = 0;
                e = expression();
                if (expression_error)
                    goto qwhat;
                printnum(e);
            }

            // At this point we have three options, a comma or a new line
            if (*txtpos == ',')
                txtpos++;  // Skip the comma and move onto the next
            else if (txtpos[0] == ';' && (txtpos[1] == NL || txtpos[1] == ':')) {
                txtpos++;  // This has to be the end of the print - no newline
                break;
            } else if (*txtpos == NL || *txtpos == ':') {
                line_terminator();  // The end of the print statement
                break;
            } else
                goto qwhat;
        }
        goto run_next_statement;

    mem:
        // memory free
        printnum(variables_begin - program_end);
        printmsg(memorymsg);
#ifdef ARDUINO
#ifdef ENABLE_EEPROM
        {
            // eprom size
            printnum(E2END + 1);
            printmsg(eeprommsg);

            // figure out the memory usage;
            val = ' ';
            int i;
            for (i = 0; (i < (E2END + 1)) && (val != '\0'); i++) {
                val = EEPROM.read(i);
            }
            printnum((E2END + 1) - (i - 1));

            printmsg(eepromamsg);
        }
#endif /* ENABLE_EEPROM */
#endif /* ARDUINO */
        goto run_next_statement;

        /*************************************************/

#ifdef ARDUINO
    awrite:  // AWRITE <pin>,val
    dwrite : {
        short int pinNo;
        short int value;
        unsigned char *txtposBak;

        // Get the pin number
        expression_error = 0;
        pinNo = expression();
        if (expression_error)
            goto qwhat;

        // check for a comma
        ignore_blanks();
        if (*txtpos != ',')
            goto qwhat;
        txtpos++;
        ignore_blanks();

        txtposBak = txtpos;
        scantable(highlow_tab);
        if (table_index != HIGHLOW_UNKNOWN) {
            if (table_index <= HIGHLOW_HIGH) {
                value = 1;
            } else {
                value = 0;
            }
        } else {
            // and the value (numerical)
            expression_error = 0;
            value = expression();
            if (expression_error)
                goto qwhat;
        }
        pinMode(pinNo, OUTPUT);
        if (isDigital) {
            digitalWrite(pinNo, value);
        } else {
            //analogWrite(pinNo, value);
        }
    }
        goto run_next_statement;
#else
    pinmode:  // PINMODE <pin>, I/O
    awrite:   // AWRITE <pin>,val
    dwrite:
        goto unimplemented;
#endif

        /*************************************************/
    files:
        // display a listing of files on the device.
        // version 1: no support for subdirectories
        cmd_Files();
        goto warmstart;

    chain:
        runAfterLoad = true;

    load:
        // clear the program
        program_end = program_start;
        // load from a file into memory
        {
            unsigned char *filename;
            // Work out the filename
            expression_error = 0;
            filename = filenameWord();
            if (expression_error)
                goto qwhat;
            // Arduino specific
            String path = String(F("/")) + String((char *)filename) + String(F(".bas"));
            if (!SPIFFS.exists(path)) {
                printmsg(sdnofile);
            } else {
                fp = SPIFFS.open(path);
                inStream = kStreamFile;
                inhibitOutput = true;
            }
        }
        goto warmstart;
    save:
        // save from memory out to a file
        {
            unsigned char *filename;

            // Work out the filename
            expression_error = 0;
            filename = filenameWord();
            if (expression_error)
                goto qwhat;

            // remove the old file if it exists
            String path = String(F("/")) + String((char *)filename) + String(F(".bas"));
            if (SPIFFS.exists(path)) {
                SPIFFS.remove(path);
            }

            // open the file, switch over to file output
            fp = SPIFFS.open(path, FILE_WRITE);
            outStream = kStreamFile;

            // copied from "List"
            list_line = findline();
            while (list_line != program_end)
                printline();

            // go back to standard output, close the file
            outStream = kStreamSerial;
            Serial.println(F("Done with save..."));

            fp.close();
            goto warmstart;
        }
    kill : {
        unsigned char *filename;
        // Work out the filename
        expression_error = 0;
        filename = filenameWord();
        if (expression_error)
            goto qwhat;
        // Arduino specific
        String path = String(F("/")) + String((char *)filename) + String(F(".bas"));
        if (SPIFFS.exists(path)) {
            SPIFFS.remove(path);
        } else {
            printmsg(sdnofile);
        }
    }
        goto warmstart;

    rseed : {
        short int value;

        //Get the pin number
        expression_error = 0;
        value = expression();
        if (expression_error)
            goto qwhat;

#ifdef ARDUINO
        randomSeed(value);
#else   // ARDUINO
        srand(value);
#endif  // ARDUINO
        goto run_next_statement;
    }

#ifdef ENABLE_TONES
    tonestop:
        noTone(kPiezoPin);
        goto run_next_statement;

    tonegen : {
        // TONE freq, duration
        // if either are 0, tones turned off
        short int freq;
        short int duration;

        //Get the frequency
        expression_error = 0;
        freq = expression();
        if (expression_error)
            goto qwhat;

        ignore_blanks();
        if (*txtpos != ',')
            goto qwhat;
        txtpos++;
        ignore_blanks();

        //Get the duration
        expression_error = 0;
        duration = expression();
        if (expression_error)
            goto qwhat;

        if (freq == 0 || duration == 0)
            goto tonestop;

        tone(kPiezoPin, freq, duration);
        if (alsoWait) {
            delay(duration);
            alsoWait = false;
        }
        goto run_next_statement;
    }
#endif /* ENABLE_TONES */
    }
}

/***********************************************************/
void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length);
void setup() {
    pinMode(blinkLED, OUTPUT);
    roverInit();
#ifdef ARDUINO
    Serial.begin(kConsoleBaud);  // opens serial port
    while (!Serial)
        ;  // for Leonardo

#ifdef ENABLE_FILEIO
    initSD();

#ifdef ENABLE_AUTORUN
    if (SD.exists(kAutorunFilename)) {
        program_end = program_start;
        fp = SD.open(kAutorunFilename);
        inStream = kStreamFile;
        inhibitOutput = true;
        runAfterLoad = true;
    }
#endif /* ENABLE_AUTORUN */

#endif /* ENABLE_FILEIO */

#ifdef ENABLE_EEPROM
#ifdef ENABLE_EAUTORUN
    // read the first byte of the eeprom. if it's a number, assume it's a program we can load
    int val = EEPROM.read(0);
    if (val >= '0' && val <= '9') {
        program_end = program_start;
        inStream = kStreamEEProm;
        eepos = 0;
        inhibitOutput = true;
        runAfterLoad = true;
    }
#endif /* ENABLE_EAUTORUN */
#endif /* ENABLE_EEPROM */

#endif /* ARDUINO */

    WiFi.mode(WIFI_MODE_APSTA);
    WiFi.softAP(softApSsid, softApPass);
    WiFi.begin(ssid, pass);
    int waitConnectCount = 10;
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print('.');
        digitalWrite(blinkLED, HIGH);
        delay(200);
        digitalWrite(blinkLED, LOW);
        delay(200);
        waitConnectCount--;
        if (waitConnectCount == 0) {
            WiFi.disconnect();
            Serial.print(F("Given up on connect to AP\r\n"));
            break;
        }
    }
    Serial.println(WiFi.localIP());
    Serial.print("ESP32 IP as soft AP: ");
    Serial.println(WiFi.softAPIP());
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
    MDNS.begin("basic");
    SPIFFS.begin();
    {  //************************************PRETTY MUCH THE ENTIRE WEB SERVER CODE************************************************
        webServer.on("/files", HTTP_GET, []() {
            String content = F("<html><head><style>a{color:#f3bf00;}body{background-color:#000000;color:#f3bf00;}table{border-collapse:collapse;}td,th{border: 1px solid #f3bf00;}input[type=text]{background-color:#000000;color:#f3bf00;border: 1px solid #f3bf00;width:6em;}</style></head><body><h1>File list</h1><table><tr><th>Delete</th><th>Size</th><th>Filename</th></tr>");
            File root = SPIFFS.open("/");
            if (root.isDirectory()) {
                File file = root.openNextFile();
                while (file) {
                    String fileName = String(file.name()).substring(1);
                    String fileSize = String(file.size());
                    content += F("<tr><td><a href=\"/delete?n=");  //make delete link
                    content += fileName;
                    content += F("\">rm</a></td><td>");
                    content += fileSize;                  //print file size
                    content += F("</td><td><a href=\"");  //make a link to the file
                    content += fileName;
                    content += F("\">");
                    content += fileName;
                    content += F("</a></td></tr>");
                    file = root.openNextFile();
                }
                file.close();
            }
            root.close();

            content += F("</table><br><form method='POST' action='/upload' enctype='multipart/form-data'><input type='file' name='payload'><input type='submit' value='Upload'></form></body></html>");
            webServer.sendHeader(F("Connection"), F("close"));
            webServer.sendHeader(F("Access-Control-Allow-Origin"), F("*"));
            webServer.send(200, F("text/html"), content);
        });
        webServer.on(
            "/upload", HTTP_POST, []() { webServer.send(200, F("text/html"), F("<html><head><meta http-equiv=\"refresh\" content=\"0; url=/files\" /></head></html>")); }, []() {
      HTTPUpload& upload = webServer.upload();
    if (upload.status == UPLOAD_FILE_START) {
        String filename = upload.filename;
        if (!filename.startsWith("/")) {
            filename = "/" + filename;
        }
        //DBG_OUTPUT_PORT.print("handleFileUpload Name: ");
        //DBG_OUTPUT_PORT.println(filename);
        fsUploadFile = SPIFFS.open(filename, "w");
        filename = String();
    } else if (upload.status == UPLOAD_FILE_WRITE) {////DBG_OUTPUT_PORT.print("handleFileUpload Data: "); //DBG_OUTPUT_PORT.println(upload.currentSize);
        if (fsUploadFile) {
            fsUploadFile.write(upload.buf, upload.currentSize);
        }
    } else if (upload.status == UPLOAD_FILE_END) {
        if (fsUploadFile) {
            fsUploadFile.close();
        }
        //DBG_OUTPUT_PORT.print("handleFileUpload Size: ");
        //DBG_OUTPUT_PORT.println(upload.totalSize);
    } });
        webServer.on("/delete", HTTP_GET, []() {
            String path = String(F("/")) + webServer.arg(F("n"));
            Serial.print(path);
            if (SPIFFS.exists(path)) {
                SPIFFS.remove(path);
            }
            webServer.sendHeader(F("Connection"), F("close"));
            webServer.sendHeader(F("Access-Control-Allow-Origin"), F("*"));
            webServer.send(200, F("text/html"), F("<html><head><meta http-equiv=\"refresh\" content=\"0; url=/files\" /></head></html>"));
        });
        webServer.on("/format", HTTP_GET, []() {
            SPIFFS.format();
            webServer.sendHeader(F("Connection"), F("close"));
            webServer.sendHeader(F("Access-Control-Allow-Origin"), F("*"));
            webServer.send(200, F("text/html"), F("<html><head><meta http-equiv=\"refresh\" content=\"0; url=/files\" /></head></html>"));
        });
        webServer.on("/", HTTP_GET, []() {
            if (exists(F("/index.html"))) {
                File file = SPIFFS.open(F("/index.html"), "r");
                webServer.streamFile(file, F("text/html"));
                file.close();
            } else {
                webServer.send(404, F("text/plain"), F("No index.html"));
            }
        });
        webServer.onNotFound([]() {  //called when the url is not defined here; use it to load content from SPIFFS
            String path = webServer.uri();
            //DBG_OUTPUT_PORT.println("handleFileRead: " + path);
            if (path.endsWith("/")) {
                path += "index.htm";
            }
            String contentType = getContentType(path);
            if (exists(path)) {
                File file = SPIFFS.open(path, "r");
                webServer.streamFile(file, contentType);
                file.close();
            } else {
                webServer.send(404, F("text/plain"), F("FileNotFound"));
            }
        });
        webServer.begin();
    }
    {  //***********************************************ARDUINO OTA SETUP**********************************************************
        ArduinoOTA.setHostname("esp32Cam");
        ArduinoOTA
            .onStart([]() {
                String type;
                if (ArduinoOTA.getCommand() == U_FLASH)
                    type = "sketch";
                else  // U_SPIFFS
                    type = "filesystem";

                // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
                Serial.println("Start updating " + type);
            })
            .onEnd([]() {
                Serial.println("\nEnd");
            })
            .onProgress([](unsigned int progress, unsigned int total) {
                Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
            })
            .onError([](ota_error_t error) {
                Serial.printf("Error[%u]: ", error);
                if (error == OTA_AUTH_ERROR)
                    Serial.println("Auth Failed");
                else if (error == OTA_BEGIN_ERROR)
                    Serial.println("Begin Failed");
                else if (error == OTA_CONNECT_ERROR)
                    Serial.println("Connect Failed");
                else if (error == OTA_RECEIVE_ERROR)
                    Serial.println("Receive Failed");
                else if (error == OTA_END_ERROR)
                    Serial.println("End Failed");
            });
        ArduinoOTA.begin();
    }

    xTaskCreatePinnedToCore(
        TaskBasiccode,   /* Task function. */
        "TinyBasicPlus", /* name of task. */
        10000,           /* Stack size of task */
        NULL,            /* parameter of the task */
        1,               /* priority of the task */
        &TaskBasic,      /* Task handle to keep track of created task */
        0);              /* pin task to core 0 */
    disableCore0WDT();   //BASIC is blocking. That's why I'm trying to run it on another core
    delay(200);
}

/***********************************************************/
static unsigned char breakcheck(void) {
#ifdef ARDUINO
    if (streamKeyboard.available())
        return streamKeyboard.read() == CTRLC;
    return 0;
#else
#ifdef __CONIO__
    if (kbhit())
        return getch() == CTRLC;
    else
#endif
        return 0;
#endif
}
/***********************************************************/
static int inchar() {
    int v;
#ifdef ARDUINO

    switch (inStream) {
        case (kStreamFile):
            v = fp.read();
            if (v == NL) v = CR;  // file translate
            if (!fp.available()) {
                fp.close();
                goto inchar_loadfinish;
            }
            return v;
            break;
        case (kStreamEEProm):
#ifdef ENABLE_EEPROM
#ifdef ARDUINO
            v = EEPROM.read(eepos++);
            if (v == '\0') {
                goto inchar_loadfinish;
            }
            return v;
#endif
#else
            inStream = kStreamSerial;
            return NL;
#endif
            break;
        case (kStreamSerial):
        default:
            while (1) {
                if (streamKeyboard.available())
                    return streamKeyboard.read();
            }
    }

inchar_loadfinish:
    inStream = kStreamSerial;
    inhibitOutput = false;
    printmsg(okmsg);
    if (runAfterLoad) {
        runAfterLoad = false;
        triggerRun = true;
    }
    return NL;  // trigger a prompt.

#else
    // otherwise. desktop!
    int got = getchar();

    // translation for desktop systems
    if (got == LF) got = CR;

    return got;
#endif
}

/***********************************************************/
static void outchar(unsigned char c) {
    if (inhibitOutput) return;
    if (outStream == kStreamFile) {
        // output to a file
        fp.write(c);
    } else {
        while (!streamScreen.availableForWrite())
            ;
        streamScreen.write(c);
    }
}

/***********************************************************/
/* SD Card helpers */

#if ARDUINO && ENABLE_FILEIO

static int initSD(void) {
    // if the card is already initialized, we just go with it.
    // there is no support (yet?) for hot-swap of SD Cards. if you need to
    // swap, pop the card, reset the arduino.)

    if (sd_is_initialized == true) return kSD_OK;

    // due to the way the SD Library works, pin 10 always needs to be
    // an output, even when your shield uses another line for CS
    pinMode(10, OUTPUT);  // change this to 53 on a mega

    if (!SD.begin(kSD_CS)) {
        // failed
        printmsg(sderrormsg);
        return kSD_Fail;
    }
    // success - quietly return 0
    sd_is_initialized = true;

    // and our file redirection flags
    outStream = kStreamSerial;
    inStream = kStreamSerial;
    inhibitOutput = false;

    return kSD_OK;
}
#endif

void cmd_Files(void) {  //just print .bas filenames, directly to the screen
    File root = SPIFFS.open("/");
    if (root.isDirectory()) {
        File file = root.openNextFile();
        while (file) {
            String fileName = String(file.name()).substring(1);
            if (fileName.endsWith(F("bas"))) {
                streamScreen.println(fileName.substring(0, fileName.length() - 4));
            }
            file = root.openNextFile();
        }
        file.close();
    }
    root.close();

    // while (true) {
    //     File entry = dir.openNextFile();
    //     if (!entry) {
    //         entry.close();
    //         break;
    //     }

    //     // common header
    //     printmsgNoNL(indentmsg);
    //     printmsgNoNL((const unsigned char *)entry.name());
    //     if (entry.isDirectory()) {
    //         printmsgNoNL(slashmsg);
    //     }

    //     if (entry.isDirectory()) {
    //         // directory ending
    //         for (int i = strlen(entry.name()); i < 16; i++) {
    //             printmsgNoNL(spacemsg);
    //         }
    //         printmsgNoNL(dirextmsg);
    //     } else {
    //         // file ending
    //         for (int i = strlen(entry.name()); i < 17; i++) {
    //             printmsgNoNL(spacemsg);
    //         }
    //         printUnum(entry.size());
    //     }
    //     line_terminator();
    //     entry.close();
    // }
    // root.close();
}

/*****************************************************************************OTHER FUNCTIONS**********************************************************************************/
/***************************************************************************************************************************************************************/
void loop() {
    delay(500);
    Serial.print("Loop running on core ");
    Serial.println(xPortGetCoreID());
    char outputLine[websocketLineLength + 2];
    uint16_t outputLineIndex = 0;
    while (1) {
        webSocket.loop();
        webServer.handleClient();
        ArduinoOTA.handle();
#ifdef SERIALBASIC
        if (Serial.available()) {
            streamKeyboard.write(Serial.read());
        }
#endif
        static uint32_t lastAvailibleTime = 0;
        static bool streamScreenWasAvailible = 0;
        if (streamScreen.available()) {
            streamScreenWasAvailible = 1;
            lastAvailibleTime = millis();
            char c = streamScreen.read();
#ifdef SERIALBASIC
            Serial.write(c);
#endif
            if (c != 13) {  //ignoring CR
                outputLine[outputLineIndex] = c;
                outputLineIndex++;
                if (outputLineIndex >= websocketLineLength) {
                    outputLine[outputLineIndex] = 10;
                    outputLineIndex++;
                    c = 10;
                }
                if (c == 10) {
                    //done building the line
                    outputLine[outputLineIndex] = 0;     //null termination
                    outputLineIndex = 0;                 //reset line index
                    webSocket.broadcastTXT(outputLine);  //send out websocket
                }
            }
        }
        if (streamScreen.available() == 0 && streamScreenWasAvailible && millis() - lastAvailibleTime > 50) {
            //says we are done printing to the screen (screen buffer empty), so send out whatever is left in the line buffer
            outputLine[outputLineIndex] = 0;     //null termination
            outputLineIndex = 0;                 //reset line index
            webSocket.broadcastTXT(outputLine);  //send out websocket
            streamScreenWasAvailible = 0;
        }
        yield();
    }
}
void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
    switch (type) {
        case WStype_DISCONNECTED:
            SerialDebug.printf("[%u] Disconnected!\n", num);
            break;
        case WStype_CONNECTED: {
            IPAddress ip = webSocket.remoteIP(num);
            SerialDebug.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
            streamScreen.println((char *)initmsg);
            streamScreen.print(variables_begin - program_end);
            streamScreen.print(F(" bytes free.\n"));
            streamKeyboard.print('\n');
            //webSocket.sendTXT(num, "Connected");
        } break;
        case WStype_TEXT: {
            // SerialDebug.printf("[%u] get Text: %s\n", num, payload);
            // send message to client
            // webSocket.sendTXT(num, "message here");
            // send data to all connected clients
            // webSocket.broadcastTXT("message here");
            unsigned int index = 1;
            int8_t c = 0;
            switch (payload[0]) {
                case 0x12:  //joystick commands
                    while (joystickLock != 0)
                        ;
                    joystick = 0;
                    for (uint8_t i = 0; i <= 7; i++) {
                        if (payload[i + 1] & 1) {
                            joystick |= (1 << i);
                        }
                    }
                    //Serial.println(joystick);
                    break;
                case 0x11:  //keyboard text
                    while (1) {
                        c = payload[index];
                        if (c == 0) {
                            break;
                        }
                        while (!streamKeyboard.availableForWrite())
                            ;
                        streamKeyboard.write(c);
                        delay(1);  //delay here seems to help?
                        // Serial.print(index);
                        // Serial.print(' ');
                        // Serial.println(c);
                        index++;
                    }
                    break;
                default:
                    break;
            }
            //webSocket.broadcastTXT(payload);
        } break;
        // case WStype_BIN:
        //     SerialDebug.printf("[%u] get binary length: %u\n", num, length);
        //     //hexdump(payload, length);
        //     // send message to client
        //     // webSocket.sendBIN(num, payload, length);
        //     break;
        // case WStype_ERROR:
        // case WStype_FRAGMENT_TEXT_START:
        // case WStype_FRAGMENT_BIN_START:
        // case WStype_FRAGMENT:
        // case WStype_FRAGMENT_FIN:
        default:
            break;
    }
}