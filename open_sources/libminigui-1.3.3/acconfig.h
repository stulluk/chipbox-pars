/* acconfig.h
   This file is in the public domain.

   Descriptive text for the C preprocessor macros that
   the distributed Autoconf macros can define.
   No software package will use all of them; autoheader copies the ones
   your configure.in uses into your configuration header file templates.

   The entries are in sort -df order: alphabetical, case insensitive,
   ignoring punctuation (such as underscores).  Although this order
   can split up related entries, it makes it easier to check whether
   a given entry is in the file.

   Leave the following blank line there!!  Autoheader needs it.  */


/* Define if on AIX 3.
   System headers sometimes define this.
   We just want to avoid a redefinition error message.  */
#ifndef _ALL_SOURCE
#undef _ALL_SOURCE
#endif

/* Define if using alloca.c.  */
#undef C_ALLOCA

/* Define if type char is unsigned and you are not using gcc.  */
#ifndef __CHAR_UNSIGNED__
#undef __CHAR_UNSIGNED__
#endif

/* Define if the closedir function returns void instead of int.  */
#undef CLOSEDIR_VOID

/* Define to empty if the keyword does not work.  */
#undef const

/* Define to one of _getb67, GETB67, getb67 for Cray-2 and Cray-YMP systems.
   This function is required for alloca.c support on those systems.  */
#undef CRAY_STACKSEG_END

/* Define for DGUX with <sys/dg_sys_info.h>.  */
#undef DGUX

/* Define if you have <dirent.h>.  */
#undef DIRENT

/* Define to the type of elements in the array set by `getgroups'.
   Usually this is either `int' or `gid_t'.  */
#undef GETGROUPS_T

/* Define if the `getloadavg' function needs to be run setuid or setgid.  */
#undef GETLOADAVG_PRIVILEGED

/* Define if the `getpgrp' function takes no argument.  */
#undef GETPGRP_VOID

/* Define to `int' if <sys/types.h> doesn't define.  */
#undef gid_t

/* Define if you have alloca, as a function or macro.  */
#undef HAVE_ALLOCA

/* Define if you have <alloca.h> and it should be used (not on Ultrix).  */
#undef HAVE_ALLOCA_H

/* Define if you don't have vprintf but do have _doprnt.  */
#undef HAVE_DOPRNT

/* Define if your system has a working fnmatch function.  */
#undef HAVE_FNMATCH

/* Define if your system has its own `getloadavg' function.  */
#undef HAVE_GETLOADAVG

/* Define if you have the getmntent function.  */
#undef HAVE_GETMNTENT

/* Define if the `long double' type works.  */
#undef HAVE_LONG_DOUBLE

/* Define if you support file names longer than 14 characters.  */
#undef HAVE_LONG_FILE_NAMES

/* Define if you have a working `mmap' system call.  */
#undef HAVE_MMAP

/* Define if system calls automatically restart after interruption
   by a signal.  */
#undef HAVE_RESTARTABLE_SYSCALLS

/* Define if your struct stat has st_blksize.  */
#undef HAVE_ST_BLKSIZE

/* Define if your struct stat has st_blocks.  */
#undef HAVE_ST_BLOCKS

/* Define if you have the strcoll function and it is properly defined.  */
#undef HAVE_STRCOLL

/* Define if your struct stat has st_rdev.  */
#undef HAVE_ST_RDEV

/* Define if you have the strftime function.  */
#undef HAVE_STRFTIME

/* Define if you have the ANSI # stringizing operator in cpp. */
#undef HAVE_STRINGIZE

/* Define if you have <sys/wait.h> that is POSIX.1 compatible.  */
#undef HAVE_SYS_WAIT_H

/* Define if your struct tm has tm_zone.  */
#undef HAVE_TM_ZONE

/* Define if you don't have tm_zone but do have the external array
   tzname.  */
#undef HAVE_TZNAME

/* Define if you have <unistd.h>.  */
#undef HAVE_UNISTD_H

/* Define if utime(file, NULL) sets file's timestamp to the present.  */
#undef HAVE_UTIME_NULL

/* Define if you have <vfork.h>.  */
#undef HAVE_VFORK_H

/* Define if you have the vprintf function.  */
#undef HAVE_VPRINTF

/* Define if you have the wait3 system call.  */
#undef HAVE_WAIT3

/* Define as __inline if that's what the C compiler calls it.  */
#undef inline

/* Define if int is 16 bits instead of 32.  */
#undef INT_16_BITS

/* Define if long int is 64 bits.  */
#undef LONG_64_BITS

/* Define if major, minor, and makedev are declared in <mkdev.h>.  */
#undef MAJOR_IN_MKDEV

/* Define if major, minor, and makedev are declared in <sysmacros.h>.  */
#undef MAJOR_IN_SYSMACROS

/* Define if on MINIX.  */
#undef _MINIX

/* Define to `int' if <sys/types.h> doesn't define.  */
#undef mode_t

/* Define if you don't have <dirent.h>, but have <ndir.h>.  */
#undef NDIR

/* Define if you have <memory.h>, and <string.h> doesn't declare the
   mem* functions.  */
#undef NEED_MEMORY_H

/* Define if your struct nlist has an n_un member.  */
#undef NLIST_NAME_UNION

/* Define if you have <nlist.h>.  */
#undef NLIST_STRUCT

/* Define if your C compiler doesn't accept -c and -o together.  */
#undef NO_MINUS_C_MINUS_O

/* Define if your Fortran 77 compiler doesn't accept -c and -o together. */
#undef F77_NO_MINUS_C_MINUS_O

/* Define to `long' if <sys/types.h> doesn't define.  */
#undef off_t

/* Define to `int' if <sys/types.h> doesn't define.  */
#undef pid_t

/* Define if the system does not provide POSIX.1 features except
   with this defined.  */
#undef _POSIX_1_SOURCE

/* Define if you need to in order for stat and other things to work.  */
#undef _POSIX_SOURCE

/* Define as the return type of signal handlers (int or void).  */
#undef RETSIGTYPE

/* Define to the type of arg1 for select(). */
#undef SELECT_TYPE_ARG1

/* Define to the type of args 2, 3 and 4 for select(). */
#undef SELECT_TYPE_ARG234

/* Define to the type of arg5 for select(). */
#undef SELECT_TYPE_ARG5

/* Define if the `setpgrp' function takes no argument.  */
#undef SETPGRP_VOID

/* Define if the setvbuf function takes the buffering type as its second
   argument and the buffer pointer as the third, as on System V
   before release 3.  */
#undef SETVBUF_REVERSED

/* Define to `unsigned' if <sys/types.h> doesn't define.  */
#undef size_t

/* If using the C implementation of alloca, define if you know the
   direction of stack growth for your system; otherwise it will be
   automatically deduced at run-time.
	STACK_DIRECTION > 0 => grows toward higher addresses
	STACK_DIRECTION < 0 => grows toward lower addresses
	STACK_DIRECTION = 0 => direction of growth unknown
 */
#undef STACK_DIRECTION

/* Define if the `S_IS*' macros in <sys/stat.h> do not work properly.  */
#undef STAT_MACROS_BROKEN

/* Define if you have the ANSI C header files.  */
#undef STDC_HEADERS

/* Define on System V Release 4.  */
#undef SVR4

/* Define if you don't have <dirent.h>, but have <sys/dir.h>.  */
#undef SYSDIR

/* Define if you don't have <dirent.h>, but have <sys/ndir.h>.  */
#undef SYSNDIR

/* Define if `sys_siglist' is declared by <signal.h>.  */
#undef SYS_SIGLIST_DECLARED

/* Define if you can safely include both <sys/time.h> and <time.h>.  */
#undef TIME_WITH_SYS_TIME

/* Define if your <sys/time.h> declares struct tm.  */
#undef TM_IN_SYS_TIME

/* Define to `int' if <sys/types.h> doesn't define.  */
#undef uid_t

/* Define for Encore UMAX.  */
#undef UMAX

/* Define for Encore UMAX 4.3 that has <inq_status/cpustats.h>
   instead of <sys/cpustats.h>.  */
#undef UMAX4_3

/* Define if you do not have <strings.h>, index, bzero, etc..  */
#undef USG

/* Define vfork as fork if vfork does not work.  */
#undef vfork

/* Define if the closedir function returns void instead of int.  */
#undef VOID_CLOSEDIR

/* Define if your processor stores words with the most significant
   byte first (like Motorola and SPARC, unlike Intel and VAX).  */
#undef WORDS_BIGENDIAN

/* Define if the X Window System is missing or not being used.  */
#undef X_DISPLAY_MISSING

/* Define if lex declares yytext as a char * by default, not a char[].  */
#undef YYTEXT_POINTER

/* Version of MiniGUI */
#undef MINIGUI_MAJOR_VERSION
#undef MINIGUI_MINOR_VERSION
#undef MINIGUI_MICRO_VERSION
#undef MINIGUI_INTERFACE_AGE
#undef MINIGUI_BINARY_AGE

/* Define if build lite version of MiniGUI */
#undef _LITE_VERSION

/* Define if build stand-alone version of MiniGUI-Lite */
#undef _STAND_ALONE

/* Define if build MiniGUI for no file I/O system */
#undef _INCORE_RES

/* Define if use debug version of MiniGUI */
#undef _DEBUG

/* Define if use coordinate transformation */
#undef _COOR_TRANS

/* Define to 0 if use clockwise rotation of screen, otherwise to 1 */
#undef _ROT_DIR_CCW

/* Define if use new GAL interfaces */
#undef _USE_NEWGAL

/* Define if include fixed math routines */
#undef _FIXED_MATH

/* Define if trace message dispatching of MiniGUI */
#undef _TRACE_MSG

/* Define if the unit of timer is 10ms */
#undef _TIMER_UNIT_10MS

/* Define if we can move window by mouse */
#undef _MOVE_WINDOW_BY_MOUSE

/* Define if use flat window style */
#undef _FLAT_WINDOW_STYLE

/* Define if mouse button can do double click */
#undef _DOUBLE_CLICK

/* Define if include messages' string names */
#undef _MSG_STRING

/* Define if is gray screen */
#undef _GRAY_SCREEN

/* Define if is tiny size screen */
#undef _TINY_SCREEN

/* Define if include VGA 16-color graphics engine */
#undef _VGA16_GAL

/* Define if include SVGALib engine */
#undef _SVGALIB

/* Define if include LibGGI engine */
#undef _LIBGGI

/* Define if include EP7211 input engine */
#undef _EP7211_IAL

/* Define if include ADS input engine */
#undef _ADS_IAL

/* Define if include iPAQ input engine */
#undef _IPAQ_IAL

/* Define if include mpc823 input engine */
#undef _MPC823_IAL

/* Define if include px255b input engine */
#undef _PX255B_IAL

/* Define if include iPAQ input engine */
#undef _THOR_IAL

/* Define if include NEC VR4181 input engine */
#undef _VR4181_IAL

/* Define if include IAL engine for Helio Touch Panel */
#undef _HELIO_IAL

/* Define if include IAL engine for Tongfang STB */
#undef _TFSTB_IAL

/* Define if include IAL engine for MT T800 */
#undef _T800_IAL

/* Define if include IAL engine for uClinux touch screen palm/mc68ez328 */
#undef _MC68X328_IAL

/* Define if include IAL engine for smdk2410 touch screen */
#undef _SMDK2410_IAL


/* Define if include the dummy IAL engine */
#undef _DUMMY_IAL

/* Define if include the QVFB IAL engine */
#undef _QVFB_IAL

/* Define if include native graphics engine */
#undef _NATIVE_GAL_ENGINE

/* Define if support native graphics engine on FBCON */
#undef _NATIVE_GAL_FBCON

/* Define if support native graphics engine on QVFB */
#undef _NATIVE_GAL_QVFB

/* Define if support native graphics engine on eCos LCD */
#undef _NATIVE_GAL_ECOSLCD

/* Define if include 1BPP FB subdriver (MSB is right) */
#undef _FBLIN1R_SUPPORT

/* Define if include 1BPP FB subdriver (MSB is left) */
#undef _FBLIN1L_SUPPORT

/* Define if include 2BPP FB subdriver (MSB is right) */
#undef _FBLIN2R_SUPPORT

/* Define if include 2BPP FB subdriver (MSB is left) */
#undef _FBLIN2L_SUPPORT

/* Define if include 4BPP FB subdriver (MSB is right) */
#undef _FBLIN4R_SUPPORT

/* Define if include 4BPP FB subdriver (MSB is left) */
#undef _FBLIN4L_SUPPORT

/* Define if include 8BPP FB subdriver */
#undef _FBLIN8_SUPPORT

/* Define if include 26BPP FB subdriver */
#undef _FBLIN16_SUPPORT

/* Define if include 24BPP FB subdriver */
#undef _FBLIN24_SUPPORT

/* Define if include 32BPP FB subdriver */
#undef _FBLIN32_SUPPORT

/* Define if include VGA16 FB subdriver */
#undef _FBVGA16_SUPPORT

/* Define if include native input engine */
#undef _NATIVE_IAL_ENGINE

/* Define if include PS2 mouse subdriver */
#undef _PS2_SUPPORT

/* Define if include IMPS2 mouse subdriver */
#undef _IMPS2_SUPPORT

/* Define if include GPM mouse subdriver */
#undef _GPM_SUPPORT

/* Define if include MS mouse subdriver */
#undef _MS_SUPPORT

/* Define if include MS3 mouse subdriver */
#undef _MS3_SUPPORT

/* Define if build pure FrameBuffer-based graphics engine */
#undef _PURE_FB_GFX

/* Define if your Linux have text mode */
#undef _HAVE_TEXT_MODE

/* Define if include cursor support */
#undef _CURSOR_SUPPORT

/* Define if support raw bitmap fonts */
#undef _RBF_SUPPORT

/* Define if include incore GB2312 12x12 RBF font */
#undef _INCORERBF_GB12

/* Define if support var bitmap fonts */
#undef _VBF_SUPPORT

/* Define if include in-core font: SansSerif */
#undef _INCOREFONT_SANSSERIF

/* Define if include in-core font: Courier */
#undef _INCOREFONT_COURIER

/* Define if include in-core font: Symbol */
#undef _INCOREFONT_SYMBOL

/* Define if include in-core font: VGAS */
#undef _INCOREFONT_VGAS

/* Define if support QPF font */
#undef _QPF_SUPPORT

/* Define if support TrueType font based on FreeType 1.3 or FreeType 2 */
// #undef _TTF_SUPPORT
#define _TTF_SUPPORT 1

/* Define if has FreeType 2 */
#undef _HAS_FREETYPE2

/* Define if support Adobe Type1 */
#undef _TYPE1_SUPPORT

/* Define if support Latin 2 charset */
#undef _LATIN2_SUPPORT

/* Define if support Latin 3 charset */
#undef _LATIN3_SUPPORT

/* Define if support Latin 4 charset */
#undef _LATIN4_SUPPORT

/* Define if support Cyrillic charset */
#undef _CYRILLIC_SUPPORT

/* Define if support Arabic charset */
#undef _ARABIC_SUPPORT

/* Define if support Greek charset */
#undef _GREEK_SUPPORT

/* Define if support Hebrew charset */
#undef _HEBREW_SUPPORT

/* Define if support Latin 5 charset */
#undef _LATIN5_SUPPORT

/* Define if support Latin 6 charset */
#undef _LATIN6_SUPPORT

/* Define if support Thai charset */
#undef _THAI_SUPPORT

/* Define if support Latin 7 charset */
#undef _LATIN7_SUPPORT

/* Define if support Latin 8 charset */
#undef _LATIN8_SUPPORT

/* Define if support Latin 9 charset */
#undef _LATIN9_SUPPORT

/* Define if support Latin 10 charset */
#undef _LATIN10_SUPPORT

/* Define if support GB2312 charset */
#undef _GB_SUPPORT

/* Define if support GBK charset */
#undef _GBK_SUPPORT

/* Define if support GB18030 charset */
#undef _GB18030_SUPPORT

/* Define if support BIG5 charset */
#undef _BIG5_SUPPORT

/* Define if support EUCKR charset */
#undef _EUCKR_SUPPORT

/* Define if support EUCJP charset */
#undef _EUCJP_SUPPORT

/* Define if support SHIFTJIS charset */
#undef _SHIFTJIS_SUPPORT

/* Define if support UNICODE */
#undef _UNICODE_SUPPORT

/* Define if use the default keyboard layout */
#undef _KBD_LAYOUT_DEFAULT

/* Define if use the French PC keyboard layout */
#undef _KBD_LAYOUT_FRPC

/* Define if use the French keyboard layout */
#undef _KBD_LAYOUT_FR

/* Define if use the German keyboard layout */
#undef _KBD_LAYOUT_DE

/* Define if use the German-Latin1 keyboard layout */
#undef _KBD_LAYOUT_DELATIN1

/* Define if use the Italian keyboard layout */
#undef _KBD_LAYOUT_IT

/* Define if use the Spanish keyboard layout */
#undef _KBD_LAYOUT_ES

/* Define if use the Spanish CP850 keyboard layout */
#undef _KBD_LAYOUT_ESCP850

/* Define if include SaveBitmap function */
#undef _SAVE_BITMAP

/* Define if support PCX bmp file format */
#undef _PCX_FILE_SUPPORT

/* Define if support LBM bmp file format */
#undef _LBM_FILE_SUPPORT

/* Define if support TGA bmp file format */
#undef _TGA_FILE_SUPPORT

/* Define if support GIF bmp file format */
#undef _GIF_FILE_SUPPORT

/* Define if support JPG bmp file format */
#undef _JPG_FILE_SUPPORT

/* Define if support PNG bmp file format */
#undef _PNG_FILE_SUPPORT

/* Define if provide GB2312 IME */
#undef _IME_GB2312

/* Define if provide GB2312 Intelligent Pinyin IME module */
#undef _IME_GB2312_PINYIN

/* Define if include About Dialog Box */
#undef _MISC_ABOUTDLG

/* Define if include code for screenshots */
#undef _MISC_SAVESCREEN

/* Define if include STATIC control */
#undef _CTRL_STATIC

/* Define if include BUTTON control */
#undef _CTRL_BUTTON

/* Define if include SIMEDIT control */
#undef _CTRL_SIMEDIT

/* Define if include SLEDIT control */
#undef _CTRL_SLEDIT

/* Define if include MLEDIT control */
#undef _CTRL_MLEDIT

/* Define if include LISTBOX control */
#undef _CTRL_LISTBOX

/* Define if include PROGRESSBAR control */
#undef _CTRL_PROGRESSBAR

/* Define if include TOOLBAR control */
#undef _CTRL_TOOLBAR

/* Define if include NEWTOOLBAR control */
#undef _CTRL_NEWTOOLBAR

/* Define if include MENUBUTTON control */
#undef _CTRL_MENUBUTTON

/* Define if include TRACKBAR control */
#undef _CTRL_TRACKBAR

/* Define if include COMBOBOX control */
#undef _CTRL_COMBOBOX

/* Define if include PROPSHEETcontrol */
#undef _CTRL_PROPSHEET

/* Define if include MONTHCALENDAR control */
#undef _EXT_CTRL_MONTHCAL

/* Define if include TREEVIEW control */
#undef _EXT_CTRL_TREEVIEW

/* Define if include SPINBOX control */
#undef _EXT_CTRL_SPINBOX

/* Define if include COOLBAR control */
#undef _EXT_CTRL_COOLBAR

/* Define if include LISTVIEW control */
#undef _EXT_CTRL_LISTVIEW

/* Define if include GRID control */
#undef _EXT_CTRL_GRID

/* Define if include full gif support */
#undef _EXT_FULLGIF

/* Define if include skin support */
#undef _EXT_SKIN

/* Define if include vcongui support */
#undef _LIB_VCONGUI


/* Leave that blank line there!!  Autoheader needs it.
   If you're adding to this file, keep in mind:
   The entries are in sort -df order: alphabetical, case insensitive,
   ignoring punctuation (such as underscores).  */
