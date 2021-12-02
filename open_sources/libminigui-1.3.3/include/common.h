/**
 * \file common.h
 * \author Wei Yongming <ymwei@minigui.org>
 * \date 2002/01/06
 * 
 * This file includes macro definitions and typedefs that commonly used 
 * by MiniGUI.
 *
 \verbatim
    Copyright (C) 1998-2002 Wei Yongming.
    Copyright (C) 2002-2003 Feynman Software.

    This file is part of MiniGUI, a lightweight Graphics User Interface 
    support library for real-time embedded Linux.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 \endverbatim
 */

/*
 * $Id: common.h,v 1.47 2003/11/23 04:09:07 weiym Exp $
 *
 *             MiniGUI for Linux Version 1.3.x
 *             Copyright (C) 1998-2002 Wei Yongming.
 *             Copyright (C) 2002-2003 Feynman Software.
 *
 *             Some data types and byte order macros come from
 *             SDL by (Sam Lantinga, slouken@devolution.com).
 *             Copyright (C) 1997-2001 Sam Lantinga
 *
 *             Fix point math routines come from Allegro
 *             By Shawn Hargreaves and others.
 */

#ifndef _MGUI_COMMON_H
  #define _MGUI_COMMON_H
 
#ifdef __MINIGUI_LIB__
    #include "../config.h"
#else
    #include "config.h"
#endif

    /**
     * \defgroup macros_types Macros and data types commonly used
     * @{
     */

    /**
     * \defgroup version_info Version information
     * @{
     */

/**
 * \def _VERSION_CODE(major, minor, micro)
 * \brief A macro that returns the version code from \a major, \a minor 
 * and \a micro version number.
 *
 * MiniGUI uses this macro to evaluate the version code of current MiniGUI 
 * library installed in your system, and define it to _MINIGUI_VERSION_CODE. 
 *
 * \sa _MINIGUI_VERSION_CODE
 */
#define _VERSION_CODE(major, minor, micro)  (((major)<<16) | ((minor)<<8) | (micro))

/**
 * \def _MINIGUI_VERSION_CODE
 * \brief Version code of MiniGUI.
 *
 * \sa _VERSION_CODE
 */
#define _MINIGUI_VERSION_CODE \
        ((MINIGUI_MAJOR_VERSION << 16) | (MINIGUI_MINOR_VERSION << 8) | MINIGUI_MICRO_VERSION)

    /** @} end of version_info */

    /**
     * \defgroup basic_types Basic data types
     * @{
     */

/**
 * \var typedef unsigned char Uint8
 * \brief A type definition for an 8-bit unsigned character.
 */
typedef unsigned char   Uint8;
/**
 * \var typedef signed char Sint8
 * \brief A type definition for an 8-bit signed character.
 */
typedef signed char     Sint8;
/**
 * \var typedef unsigned short Uint16
 * \brief A type definition for a 16-bit unsigned integer.
 */
typedef unsigned short  Uint16;
/**
 * \var typedef signed short Sint16
 * \brief A type definition for a 16-bit signed integer.
 */
typedef signed short    Sint16;
/**
 * \var typedef unsigned int Uint32
 * \brief A type definition for a 32-bit unsigned integer.
 */
typedef unsigned int    Uint32;
/**
 * \var typedef signed int Sint32
 * \brief A type definition for a 32-bit signed integer.
 */
typedef signed int      Sint32;

/* Figure out how to support 64-bit datatypes */
#if !defined(__STRICT_ANSI__)
#if defined(__GNUC__)
#define MGUI_HAS_64BIT_TYPE	long long
#endif
#endif /* !__STRICT_ANSI__ */

/* The 64-bit datatype isn't supported on all platforms */
#ifdef MGUI_HAS_64BIT_TYPE

/**
 * \var typedef unsigned long long Uint64
 * \brief A type definition for a 64-bit unsigned integer.
 *
 * \warning Only available under GNU C.
 */
typedef unsigned MGUI_HAS_64BIT_TYPE Uint64;
/**
 * \var typedef signed long long Sint64
 * \brief A type definition for a 64-bit signed integer.
 *
 * \warning Only available under GNU C.
 */
typedef signed MGUI_HAS_64BIT_TYPE Sint64;
#else
/* This is really just a hack to prevent the compiler from complaining */
typedef struct {
	Uint32 hi;
	Uint32 lo;
} Uint64, Sint64;
#endif

/* Make sure the types really have the right sizes */
#define MGUI_COMPILE_TIME_ASSERT(name, x)               \
       typedef int MGUI_dummy_ ## name[(x) * 2 - 1]

MGUI_COMPILE_TIME_ASSERT(uint8, sizeof(Uint8) == 1);
MGUI_COMPILE_TIME_ASSERT(sint8, sizeof(Sint8) == 1);
MGUI_COMPILE_TIME_ASSERT(uint16, sizeof(Uint16) == 2);
MGUI_COMPILE_TIME_ASSERT(sint16, sizeof(Sint16) == 2);
MGUI_COMPILE_TIME_ASSERT(uint32, sizeof(Uint32) == 4);
MGUI_COMPILE_TIME_ASSERT(sint32, sizeof(Sint32) == 4);
MGUI_COMPILE_TIME_ASSERT(uint64, sizeof(Uint64) == 8);
MGUI_COMPILE_TIME_ASSERT(sint64, sizeof(Sint64) == 8);

#undef MGUI_COMPILE_TIME_ASSERT

    /** @} end of basic_types */

    /**
     * \defgroup endian_info Endianness information
     * @{
     */

/**
 * \def MGUI_LIL_ENDIAN
 * \brief Little endianness.
 */
#define MGUI_LIL_ENDIAN  1234
/**
 * \def MGUI_BIG_ENDIAN
 * \brief Big endianness.
 */
#define MGUI_BIG_ENDIAN  4321

/* Pardon the mess, I'm trying to determine the endianness of this host.
 *    I'm doing it by preprocessor defines rather than some sort of configure
 *    script so that application code can use this too.  The "right" way would
 *    be to dynamically generate this file on install, but that's a lot of work.
 */

/**
 * \def MGUI_BYTEORDER
 * \brief The byte order (endianness) of the target system.
 *
 * This macro will be either defined to MGUI_LIL_ENDIAN or MGUI_BIG_ENDIAN.
 * You can use the code like below
 *
 * \code
 * #if MGUI_BYTEORDER == MGUI_LIL_ENDIAN
 *     ... // code for little endian system.
 * #else
 *     ... // code for big endian system.
 * #endif
 * \endcode
 *
 * to write endianness independent code.
 */
#if  defined(__i386__) || defined(__ia64__) || \
    (defined(__alpha__) || defined(__alpha)) || \
     defined(__arm__) || \
    (defined(__mips__) && defined(__MIPSEL__)) || \
     defined(__LITTLE_ENDIAN__)
#define MGUI_BYTEORDER   MGUI_LIL_ENDIAN
#else
#define MGUI_BYTEORDER   MGUI_BIG_ENDIAN
#endif

    /** @} end of endian_info */

    /**
     * \defgroup simple_types Simple and common types and macros
     * @{
     */

/**
 * \var typedef int BOOL
 * \brief A type definition for boolean value.
 */
typedef int BOOL;

/**
 * \def FALSE
 * \brief FALSE value, defined as 0 by MiniGUI.
 */
#ifndef FALSE
    #define FALSE       0
#endif
/**
 * \def TRUE
 * \brief TRUE value, defined as 1 by MiniGUI.
 */
#ifndef TRUE
    #define TRUE        1
#endif

/**
 * \def NULL
 * \brief A value indicates null pointer.
 */
#ifndef NULL
#define NULL            ((void *)0)
#endif

#define VOID            void
#define GUIAPI

    /** @} end of simple_types */

    /**
     * \defgroup handles MiniGUI handles
     * @{
     */

/**
 * \var typedef unsigned int GHANDLE
 * \brief General handle.
 */
typedef unsigned int GHANDLE;
/**
 * \var typedef unsigned int HWND
 * \brief Handle to main window or control.
 */
typedef unsigned int HWND;
/**
 * \var typedef unsigned int HDC
 * \brief Handle to device context.
 */
typedef unsigned int HDC;
/**
 * \var typedef unsigned int HCURSOR
 * \brief Handle to cursor.
 */
typedef unsigned int HCURSOR;
/**
 * \var typedef unsigned int HICON
 * \brief Handle to icon.
 */
typedef unsigned int HICON;
/**
 * \var typedef unsigned int HMENU
 * \brief Handle to menu.
 */
typedef unsigned int HMENU;
/**
 * \var typedef unsigned int HACCEL
 * \brief Handle to accelarator.
 */
typedef unsigned int HACCEL;
/**
 * \var typedef unsigned int HDLG
 * \brief Handle to dialog box, same as HWND.
 */
typedef unsigned int HDLG;
/**
 * \var typedef unsigned int HHOOK
 * \brief Handle to keyboard or mouse event hook.
 */
typedef unsigned int HHOOK;

    /** @} end of handles */

    /**
     * \defgroup win32_types Win32-like data types and macros
     * @{
     */

/**
 * \var typedef unsigned char BYTE
 * \brief A type definition for unsigned character (byte).
 */
typedef unsigned char   BYTE;
/**
 * \var typedef signed char BYTE
 * \brief A type definition for signed character.
 */
typedef signed char     SBYTE;
/**
 * \var typedef unsigned short WORD 
 * \brief A type definition for unsigned short integer (word).
 */
typedef unsigned short  WORD;
/**
 * \var typedef signed short SWORD 
 * \brief A type definition for signed short integer.
 */
typedef signed short    SWORD;
/**
 * \var typedef unsigned long DWORD
 * \brief A type definition for unsigned long integer (double word).
 */
typedef unsigned long   DWORD;
/**
 * \var typedef signed long SDWORD
 * \brief A type definition for signed long integer.
 */
typedef signed long     SDWORD;

/**
 * \var typedef unsigned int UINT
 * \brief A type definition for unsigned integer.
 */
typedef unsigned int    UINT;
/**
 * \var typedef long LONG
 * \brief A type definition for long integer.
 */
typedef long            LONG;

/**
 * \var typedef UINT WPARAM
 * \brief A type definition for the first message paramter.
 */
typedef UINT            WPARAM;
/**
 * \var typedef DWORD WPARAM
 * \brief A type definition for the second message paramter.
 */
typedef DWORD           LPARAM;

/**
 * \def LOBYTE(w)
 * \brief Returns the low byte of the word \a w.
 *
 * \sa MAKEWORD
 */
#define LOBYTE(w)           ((BYTE)(w))
/**
 * \def HIBYTE(w)
 * \brief Returns the high byte of the word \a w.
 *
 * \sa MAKEWORD
 */
#define HIBYTE(w)           ((BYTE)(((WORD)(w) >> 8) & 0xFF))

/**
 * \def MAKEWORD(low, high)
 * \brief Makes a word from \a low byte and \a high byte.
 */
#define MAKEWORD(low, high) ((WORD)(((BYTE)(low)) | (((WORD)((BYTE)(high))) << 8)))

/**
 * \def LOWORD(l)
 * \brief Returns the low word of the double word \a l
 *
 * \sa MAKELONG
 */
#define LOWORD(l)           ((WORD)(DWORD)(l))
/**
 * \def HIWORD(l)
 * \brief Returns the high word of the double word \a l
 *
 * \sa MAKELONG
 */
#define HIWORD(l)           ((WORD)((((DWORD)(l)) >> 16) & 0xFFFF))

/**
 * \def LOSWORD(l)
 * \brief Returns the low signed word of the double word \a l
 *
 * \sa MAKELONG
 */
#define LOSWORD(l)          ((SWORD)(DWORD)(l))
/**
 * \def HISWORD(l)
 * \brief Returns the high signed word of the double word \a l
 *
 * \sa MAKELONG
 */
#define HISWORD(l)          ((SWORD)((((DWORD)(l)) >> 16) & 0xFFFF))

/**
 * \def MAKELONG(low, high)
 * \brief Makes a double word from \a low word and \a high word.
 */
#define MAKELONG(low, high) ((DWORD)(((WORD)(low)) | (((DWORD)((WORD)(high))) << 16)))

/**
 * \def GetRValue(rgb)
 * \brief Gets the red component from a RGB triple value \a rgb.
 *
 * You can make a RGB triple by using MakeRGB.
 *
 * \sa MakeRGB
 */
#define GetRValue(rgb)      ((BYTE)(rgb))
/**
 * \def GetGValue(rgb)
 * \brief Gets the green component from a RGB triple value \a rgb.
 *
 * You can make a RGB triple by using MakeRGB.
 *
 * \sa MakeRGB
 */
#define GetGValue(rgb)      ((BYTE)(((WORD)(rgb)) >> 8))
/**
 * \def GetBValue(rgb)
 * \brief Gets the blue component from a RGB triple value \a rgb.
 *
 * You can make a RGB triple by using MakeRGB.
 *
 * \sa MakeRGB
 */
#define GetBValue(rgb)      ((BYTE)((rgb) >> 16))

/**
 * \def MakeRGB(r, g, b)
 * \brief Makes a RGB triple value from red \a r, green \a g, and blue \a b components.
 *
 * \note The red, green, and blue components are all ranged from 0 to 255,
 * and the returned value will be a double word.
 *
 * \sa GetRValue, GetGValue, GetBValue
 */
#define MakeRGB(r, g, b)    (((DWORD)((BYTE)(r))) | ((DWORD)((BYTE)(g)) << 8) \
                | ((DWORD)((BYTE)(b)) << 16))

/**
 * A rectangle defined by coordinates of corners.
 *
 * \note The lower-right corner does not belong to the rectangle,
 * i.e. the bottom horizontal line and the right vertical line are excluded
 * from the retangle.
 *
 * \sa PRECT, GAL_Rect
 */
typedef struct _RECT
{
    /**
     * the x coordinate of the upper-left corner of the rectangle.
     */
    int left;
    /**
     * the y coordinate of the upper-left corner of the rectangle.
     */
    int top;
    /**
     * the x coordinate of the lower-right corner of the rectangle.
     */
    int right;
    /**
     * the y coordinate of the lower-right corner of the rectangle.
     */
    int bottom;
} RECT;
/**
 * \var typedef RECT* PRECT
 * \brief Data type of the pointer to a RECT.
 *
 * \sa RECT
 */
typedef RECT* PRECT;

/**
 * Point structure.
 * \sa PPOINT
 */
typedef struct _POINT
{
    /**
     * the x coordinate of the point.
     */
    int x;
    /**
     * the y coordinate of the point.
     */
    int y;
} POINT;
/**
 * \var typedef POINT* PPOINT
 * \brief Data type of the pointer to a POINT.
 *
 * \sa POINT
 */
typedef POINT* PPOINT;

/**
 * Size structure of a 2-dimension object.
 * \sa PSIZE
 */
typedef struct _SIZE
{
    /**
     * the extent in x coordinate of a 2D object.
     */
    int cx;
    /**
     * the extent in y coordinate of a 2D object.
     */
    int cy;
} SIZE;
/**
 * \var typedef SIZE* PSIZE
 * \brief Data type of the pointer to a SIZE.
 *
 * \sa SIZE
 */
typedef SIZE* PSIZE;

/**
 * RGB triple structure.
 * \sa PRGB, GAL_Color
 */
typedef struct _RGB
{
    /**
     * the red component of a RGB triple.
     */
    BYTE r;
    /**
     * the green component of a RGB triple.
     */
    BYTE g;
    /**
     * the blue component of a RGB triple.
     */
    BYTE b;
    /**
     * Reserved for alignment, maybe used for the alpha component of a RGB triple. 
     */
    BYTE a;
} RGB;
typedef RGB* PRGB;

    /** @} end of win32_types */

    /**
     * \defgroup gdi_types Data types for GDI
     * @{
     */
/**
 * \var typedef Sint8 gal_sint8
 * \brief Data type of 8-bit signed integer.
 *
 * \sa Sint8
 */
typedef Sint8       gal_sint8;
/**
 * \var typedef Uint8 gal_uint8
 * \brief Data type of 8-bit unsigned integer.
 *
 * \sa Uint8
 */
typedef Uint8       gal_uint8;

/**
 * \var typedef Sint16 gal_sint16
 * \brief Data type of 16-bit signed integer.
 *
 * \sa Sint16
 */
typedef Sint16      gal_sint16;
/**
 * \var typedef Uint16 gal_uint16
 * \brief Data type of 16-bit unsigned integer.
 *
 * \sa Uint16
 */
typedef Uint16      gal_uint16;

/**
 * \var typedef Sint32 gal_sint16
 * \brief Data type of 32-bit signed integer.
 *
 * \sa Sint32
 */
typedef Sint32      gal_sint32;
/**
 * \var typedef Uint32 gal_uint16
 * \brief Data type of 32-bit unsigned integer.
 *
 * \sa Uint32
 */
typedef Uint32      gal_uint32;

/**
 * \var typedef signed int gal_sint
 * \brief Data type of signed integer.
 */
typedef signed int      gal_sint;
/**
 * \var typedef unsigned int gal_uint
 * \brief Data type of unsigned integer.
 */
typedef unsigned int    gal_uint;

/**
 * \var typedef Uint32 gal_pixel 
 * \brief Data type of pixel value
 */
typedef Uint32          gal_pixel;
/**
 * \var typedef Uint32 gal_attr
 * \brief Data type of attribute value
 */
typedef Uint32          gal_attr;

/**
 * \var typedef long fixed.
 * \brief Data type of fixed point.
 */
typedef long fixed;

/**
 * RGBA quarter structure.
 * \sa RGB
 */
typedef struct GAL_Color
{
    /**
     * the red component of a RGBA quarter.
     */
    gal_uint8 r;
    /**
     * the green component of a RGBA quarter.
     */
    gal_uint8 g;
    /**
     * the blue component of a RGBA quarter.
     */
    gal_uint8 b;
    /**
     * the alpha component of a RGBA quarter.
     */
    gal_uint8 a;
} GAL_Color;

/**
 * Palette structure.
 * \sa GAL_Color
 */
typedef struct GAL_Palette
{
    /**
     * the number of palette items.
     */
    int        ncolors;
    /**
     * the pointer to the array of palette items.
     */
    GAL_Color* colors;
} GAL_Palette;

/**
 * A rectangle defined by upper-left coordinates and width/height.
 * \sa RECT 
 */
typedef struct GAL_Rect {
    /**
     * the coordinates of the upper-left corner of the rectangle.
     */
    Sint32      x, y;
    /**
     * the width and height of the rectangle.
     */
    Sint32      w, h;
} GAL_Rect;

    /** @} end of gdi_types */

    /**
     * \defgroup key_defs Macros for key codes and shift status
     * @{
     */

/**
 * \def MGUI_NR_KEYS
 * \brief Number of MiniGUI keys.
 *
 * The number of MiniGUI keys is defined to 255 by default. This means that
 * MiniGUI can destinguish 255 different keys with each has an unique scan code.
 * The scan codes below 129 are defined for PC keyboard by default. 
 * If your system has a large amount of keys, you can define the scan code of keys 
 * ranged from 1 to 255 in your IAL engine. And your application will receive
 * a MSG_KEYDOWN and MSG_KEYUP messages when a key pressed and released, and the
 * wParam of the messages will be defined to be equal to the scan code
 * of the key.
 *
 * \sa NR_KEYS, SCANCODE_USER
 */
#define MGUI_NR_KEYS                    0x1ff /* modified by zhongkai Du, for distinguishing key source. */

/**
 * \def NR_KEYS
 * \brief The number of keys defined by Linux operating system. 
 *
 * For a PC box, NR_KEYS is defined to 128 by default. You can define
 * some input events from an input device other than keyboard, e.g. 
 * your remote controller, as key events with different scan codes from 
 * those of PC's. MiniGUI can support 255 keys, and the constant
 * define by MGUI_NR_KEYS.
 *
 * \sa MGUI_NR_KEYS
 */
#ifndef NR_KEYS
#define NR_KEYS                         0x1fe
#endif

/**
 * \def SCANCODE_USER
 * \brief The first key scan code different from OS defined ones.
 *
 * You can define your special key scan codes like below
 *
 * \code
 * #define SCANCODE_PLAY    (SCANCODE_USER)
 * #define SCANCODE_STOP    (SCANCODE_USER + 1)
 * #define SCANCODE_PAUSE   (SCANCODE_USER + 2)
 * \endcode
 *
 * to distinguish the keys on your remote controller.
 *
 * \sa MGUI_NR_KEYS, NR_KEYS
 */
#define SCANCODE_USER                   (NR_KEYS + 1)

#define SCANCODE_ESCAPE                 1

#define SCANCODE_1                      2
#define SCANCODE_2                      3
#define SCANCODE_3                      4
#define SCANCODE_4                      5
#define SCANCODE_5                      6
#define SCANCODE_6                      7
#define SCANCODE_7                      8
#define SCANCODE_8                      9
#define SCANCODE_9                      10
#define SCANCODE_0                      11

#define SCANCODE_MINUS                  12
#define SCANCODE_EQUAL                  13

#define SCANCODE_BACKSPACE              14
#define SCANCODE_TAB                    15

#define SCANCODE_Q                      16
#define SCANCODE_W                      17
#define SCANCODE_E                      18
#define SCANCODE_R                      19
#define SCANCODE_T                      20
#define SCANCODE_Y                      21
#define SCANCODE_U                      22
#define SCANCODE_I                      23
#define SCANCODE_O                      24
#define SCANCODE_P                      25
#define SCANCODE_BRACKET_LEFT           26
#define SCANCODE_BRACKET_RIGHT          27

#define SCANCODE_ENTER                  28

#define SCANCODE_LEFTCONTROL            29

#define SCANCODE_A                      30
#define SCANCODE_S                      31
#define SCANCODE_D                      32
#define SCANCODE_F                      33
#define SCANCODE_G                      34
#define SCANCODE_H                      35
#define SCANCODE_J                      36
#define SCANCODE_K                      37
#define SCANCODE_L                      38
#define SCANCODE_SEMICOLON              39
#define SCANCODE_APOSTROPHE             40
#define SCANCODE_GRAVE                  41

#define SCANCODE_LEFTSHIFT              42
#define SCANCODE_BACKSLASH              43

#define SCANCODE_Z                      44
#define SCANCODE_X                      45
#define SCANCODE_C                      46
#define SCANCODE_V                      47
#define SCANCODE_B                      48
#define SCANCODE_N                      49
#define SCANCODE_M                      50
#define SCANCODE_COMMA                  51
#define SCANCODE_PERIOD                 52
#define SCANCODE_SLASH                  53

#define SCANCODE_RIGHTSHIFT             54
#define SCANCODE_KEYPADMULTIPLY         55

#define SCANCODE_LEFTALT                56
#define SCANCODE_SPACE                  57
#define SCANCODE_CAPSLOCK               58

#define SCANCODE_F1                     59
#define SCANCODE_F2                     60
#define SCANCODE_F3                     61
#define SCANCODE_F4                     62
#define SCANCODE_F5                     63
#define SCANCODE_F6                     64
#define SCANCODE_F7                     65
#define SCANCODE_F8                     66
#define SCANCODE_F9                     67
#define SCANCODE_F10                    68

#define SCANCODE_NUMLOCK                69
#define SCANCODE_SCROLLLOCK             70

#define SCANCODE_KEYPAD7                71
#define SCANCODE_CURSORUPLEFT           71
#define SCANCODE_KEYPAD8                72
#define SCANCODE_CURSORUP               72
#define SCANCODE_KEYPAD9                73
#define SCANCODE_CURSORUPRIGHT          73
#define SCANCODE_KEYPADMINUS            74
#define SCANCODE_KEYPAD4                75
#define SCANCODE_CURSORLEFT             75
#define SCANCODE_KEYPAD5                76
#define SCANCODE_KEYPAD6                77
#define SCANCODE_CURSORRIGHT            77
#define SCANCODE_KEYPADPLUS             78
#define SCANCODE_KEYPAD1                79
#define SCANCODE_CURSORDOWNLEFT         79
#define SCANCODE_KEYPAD2                80
#define SCANCODE_CURSORDOWN             80
#define SCANCODE_KEYPAD3                81
#define SCANCODE_CURSORDOWNRIGHT        81
#define SCANCODE_KEYPAD0                82
#define SCANCODE_KEYPADPERIOD           83

#define SCANCODE_LESS                   86

#define SCANCODE_F11                    87
#define SCANCODE_F12                    88

#define SCANCODE_KEYPADENTER            96
#define SCANCODE_RIGHTCONTROL           97
#define SCANCODE_CONTROL                97
#define SCANCODE_KEYPADDIVIDE           98
#define SCANCODE_PRINTSCREEN            99
#define SCANCODE_RIGHTALT               100
#define SCANCODE_BREAK                  101    /* Beware: is 119     */
#define SCANCODE_BREAK_ALTERNATIVE      119    /* on some keyboards! */

#define SCANCODE_HOME                   102
#define SCANCODE_CURSORBLOCKUP          103    /* Cursor key block */
#define SCANCODE_PAGEUP                 104
#define SCANCODE_CURSORBLOCKLEFT        105    /* Cursor key block */
#define SCANCODE_CURSORBLOCKRIGHT       106    /* Cursor key block */
#define SCANCODE_END                    107
#define SCANCODE_CURSORBLOCKDOWN        108    /* Cursor key block */
#define SCANCODE_PAGEDOWN               109
#define SCANCODE_INSERT                 110
#define SCANCODE_REMOVE                 111

#define SCANCODE_PAUSE                  119
#define SCANCODE_LEFTWIN                125
#define SCANCODE_RIGHTWIN               126
#define SCANCODE_MENU                   127

#define CSAPI_USB_CONNECTION		0x1fe

#define SCANCODE_LEFTBUTTON             0x1000
#define SCANCODE_RIGHTBUTTON            0x2000
#define SCANCODE_MIDDLBUTTON            0x4000

/**
 * \def KS_CAPTURED 
 * \brief This status indicate that the mouse is captured by a window when 
 * the mouse message posted.
 *
 * You can test the status by AND'ed with lParam of the message, like below:
 *
 * \code
 *      switch (message) {
 *      case MSG_MOUSEMOVE:
 *          if (lParam & KS_CAPTURED) {
 *              // the mouse is captured by this window.
 *              ...
 *          }
 *          break;
 *      ...
 * \endcode
 *
 * \sa mouse_msgs
 */
#define KS_CAPTURED                     0x00000400
/**
 * \def KS_IMEPOST
 * \brief This status indicate that the key message is posted by the IME window.
 *
 * \sa key_msgs
 */
#define KS_IMEPOST                      0x00000200
/**
 * \def KS_CAPSLOCK
 * \brief This status indicate that the CapsLock key was locked when 
 * the key or mouse message posted to the window.
 *
 * You can test the status by AND'ed with lParam of the message, like below
 *
 * \code
 *      switch (message) {
 *      case MSG_KEYDOWN:
 *          if (lParam & KS_CAPSLOCK) {
 *              // the CapsLock key is locked.
 *              ...
 *          }
 *          break;
 *      ...
 * \endcode
 *
 * \sa key_msgs
 */
#define KS_CAPSLOCK                     0x00000100
/**
 * \def KS_NUMLOCK
 * \brief This status indicate that the NumLock key was locked when 
 * the key or mouse message posted to the window.
 *
 * \sa key_msgs
 */
#define KS_NUMLOCK                      0x00000080
/**
 * \def KS_SCROLLLOCK
 * \brief This status indicate that the ScrollLock key was locked when
 * the key or mouse message posted to the window.
 *
 * \sa key_msgs
 */
#define KS_SCROLLLOCK                   0x00000040
/**
 * \def KS_LEFTCTRL
 * \brief This status indicate that the left-Ctrl key was pressed when
 * the key or mouse message posted to the window.
 *
 * \sa key_msgs
 */
#define KS_LEFTCTRL                     0x00000020
/**
 * \def KS_RIGHTCTRL
 * \brief This status indicate that the right-Ctrl key was pressed when
 * the key or mouse message posted to the window.
 *
 * \sa key_msgs
 */
#define KS_RIGHTCTRL                    0x00000010
/**
 * \def KS_CTRL
 * \brief This status indicate that either the left-Ctrl key or the right-Ctrl key 
 * was pressed when the key or mouse message posted to the window.
 *
 * \sa key_msgs
 */
#define KS_CTRL                         0x00000030
/**
 * \def KS_LEFTALT
 * \brief This status indicate that left-Alt key was pressed when
 * the key  or mouse message posted to the window.
 *
 * \sa key_msgs
 */
#define KS_LEFTALT                      0x00000008
/**
 * \def KS_RIGHTALT
 * \brief This status indicate that right-Alt key was pressed when
 * the key  or mouse message posted to the window.
 *
 * \sa key_msgs
 */
#define KS_RIGHTALT                     0x00000004
/**
 * \def KS_ALT
 * \brief This status indicate that either the left-Alt key or the right-Alt key 
 * was pressed when the key or mouse message posted to the window.
 *
 * \sa key_msgs
 */
#define KS_ALT                          0x0000000C
/**
 * \def KS_LEFTSHIFT
 * \brief This status indicate that left-Shift key was pressed when
 * the key  or mouse message posted to the window.
 *
 * \sa key_msgs
 */
#define KS_LEFTSHIFT                    0x00000002
/**
 * \def KS_RIGHTSHIFT
 * \brief This status indicate that right-Shift key was pressed when
 * the key  or mouse message posted to the window.
 *
 * \sa key_msgs
 */
#define KS_RIGHTSHIFT                   0x00000001
/**
 * \def KS_SHIFT
 * \brief This status indicate that either the left-Shift key or the right-Shift key 
 * was pressed when the key or mouse message posted to the window.
 *
 * \sa key_msgs
 */
#define KS_SHIFT                        0x00000003
/**
 * \def MASK_KS_SHIFTKEYS
 * \brief The mask of key status.
 */
#define MASK_KS_SHIFTKEYS               0x00000FFF

/**
 * \def KS_LEFTBUTTON
 * \brief This status indicate that left button was pressed when 
 * the key or mouse message posted to the window.
 *
 * \sa key_msgs
 */
#define KS_LEFTBUTTON                   0x00001000
/**
 * \def KS_RIGHTBUTTON
 * \brief This status indicate that right button was pressed when 
 * the key or mouse message posted to the window.
 *
 * \sa key_msgs
 */
#define KS_RIGHTBUTTON                  0x00002000
/**
 * \def KS_MIDDLBUTTON
 * \brief This status indicate that middle button was pressed when 
 * the key or mouse message posted to the window.
 *
 * \sa key_msgs
 */
#define KS_MIDDLBUTTON                  0x00004000
/**
 * \def MASK_KS_BUTTONS
 * \brief The mask of mouse button status.
 */
#define MASK_KS_BUTTONS                 0x0000F000

    /** @} end of key_defs */

    /**
     * \defgroup err_codes Error codes
     * @{
     */
#define ERR_OK                   0
#define ERR_INV_HWND            -1
#define ERR_QUEUE_FULL          -2

#define ERR_INVALID_HANDLE      -3
#define ERR_INVALID_HMENU       -4
#define ERR_INVALID_POS         -5
#define ERR_INVALID_ID          -6
#define ERR_RES_ALLOCATION      -7

#define ERR_CTRLCLASS_INVNAME   -8
#define ERR_CTRLCLASS_INVLEN    -9
#define ERR_CTRLCLASS_MEM       -10
#define ERR_CTRLCLASS_INUSE     -11

#define ERR_ALREADY_EXIST       -12
#define ERR_NO_MATCH            -13
#define ERR_BAD_OWNER           -14

#define ERR_IME_TOOMUCHIMEWND   -15
#define ERR_IME_NOSUCHIMEWND    -16
#define ERR_IME_NOIMEWND        -17

#define ERR_CONFIG_FILE         -18
#define ERR_FILE_IO             -19
#define ERR_GFX_ENGINE          -20
#define ERR_INPUT_ENGINE        -21
#define ERR_NO_ENGINE           -22

    /** @} end of err_codes */

    /**
     * \defgroup misc_macros Miscellaneous macros
     * @{
     */
/**
 * \def TABLESIZE(table)
 * \brief A macro returns the number of elements in a \a table.
 */
#define TABLESIZE(table)    (sizeof(table)/sizeof(table[0]))

/* MAX/MIN/ABS macors */
/**
 * \def MAX(x, y)
 * \brief A macro returns the maximum of \a x and \a y.
 */
#ifndef MAX
#define MAX(x, y)           ((x > y)?x:y)
#endif
/**
 * \def MIN(x, y)
 * \brief A macro returns the minimum of \a x and \a y.
 */
#ifndef MIN
#define MIN(x, y)           ((x < y)?x:y)
#endif
/**
 * \def ABS(x)
 * \brief A macro returns the absolute value of \a x.
 */
#ifndef ABS
#define ABS(x)              (((x)<0) ? -(x) : (x))
#endif

/* Common used definitions */
#ifndef PATH_MAX
    #include <dirent.h>
#endif
/**
 * \def MAX_PATH
 * \brief The possible maximal length of a path name.
 * \note This definition is an alias of PATH_MAX
 */
#define MAX_PATH        PATH_MAX
/**
 * \def MAX_NAME
 * \brief The possible maximal length of a file name.
 * \note This definition is an alias of NAME_MAX
 */
#define MAX_NAME        NAME_MAX

    /** @} end of misc_macros */

    /** @} end of macros_types */

#ifndef HAVE_STRDUP
char *strdup(const char *s);
#endif

#ifndef HAVE_STRCASECMP
int strcasecmp(const char *s1, const char *s2);
#endif

#ifndef HAVE_GETTIMEOFDAY
#include <sys/time.h>
int gettimeofday(struct timeval *tv, void* tz);
#endif

#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif

#endif /* _MGUI_COMMON_H */

