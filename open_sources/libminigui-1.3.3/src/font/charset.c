/*
** $Id: charset.c,v 1.71 2007-01-29 08:43:34 weiym Exp $
** 
** charset.c: The charset operation set.
** 
** Copyright (C) 2003 ~ 2006 Feynman Software
** Copyright (C) 2000 ~ 2002 Wei Yongming.
**
** All right reserved by Feynman Software.
**
** Current maintainer: Wei Yongming.
**
** Create date: 2000/06/13
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "common.h"
#include "minigui.h"
#include "gdi.h"
#include "charset.h"

/*************** Common Operations for single byte charsets ******************/
static int sb_len_first_char (const unsigned char* mstr, int len)
{
    if (len < 1) return 0;
    if (*mstr != '\0')  return 1;
    return 0;
}

static unsigned int sb_char_offset (const unsigned char* mchar)
{
    return (int)(*mchar);
}

static unsigned int sb_char_type (const unsigned char* mchar)
{
    unsigned int ch_type = MCHAR_TYPE_UNKNOWN;

    switch (*mchar) {
        case '\0':
            ch_type = MCHAR_TYPE_NUL;
            break;
        case '\a':
            ch_type = MCHAR_TYPE_BEL;
            break;
        case '\b':
            ch_type = MCHAR_TYPE_BS;
            break;
        case '\t':
            ch_type = MCHAR_TYPE_HT;
            break;
        case '\n':
            ch_type = MCHAR_TYPE_LF;
            break;
        case '\v':
            ch_type = MCHAR_TYPE_VT;
            break;
        case '\f':
            ch_type = MCHAR_TYPE_FF;
            break;
        case '\r':
            ch_type = MCHAR_TYPE_CR;
            break;
        case ' ':
            ch_type = MCHAR_TYPE_SPACE;
            break;
    }

    if (ch_type == MCHAR_TYPE_UNKNOWN) {
        if (*mchar < '\a')
            ch_type = MCHAR_TYPE_CTRL1;
        else if (*mchar < ' ')
            ch_type = MCHAR_TYPE_CTRL2;
        else
            ch_type = MCHAR_TYPE_GENERIC;
    }

    return ch_type;
}

static int sb_nr_chars_in_str (const unsigned char* mstr, int mstrlen)
{
    return mstrlen;
}

static int sb_len_first_substr (const unsigned char* mstr, int mstrlen)
{
    return mstrlen;
}

static int sb_pos_first_char (const unsigned char* mstr, int mstrlen)
{
    return 0;
}

static const unsigned char* sb_get_next_word (const unsigned char* mstr,
                int mstrlen, WORDINFO* word_info)
{
    int i;

    word_info->len = 0;
    word_info->delimiter = '\0';
    word_info->nr_delimiters = 0;

    if (mstrlen == 0) return NULL;

    for (i = 0; i < mstrlen; i++) {
        if (mstr[i] > 127) {
            if (word_info->len > 0) {
                word_info->delimiter = mstr[i];
                word_info->nr_delimiters ++;
            }
            else {
                word_info->len ++;
                word_info->delimiter = ' ';
                word_info->nr_delimiters ++;
            }
            break;
        }

        switch (mstr[i]) {
        case ' ':
        case '\t':
        case '\n':
        case '\r':
            if (word_info->delimiter == '\0') {
                word_info->delimiter = mstr[i];
                word_info->nr_delimiters ++;
            }
            else if (word_info->delimiter == mstr[i])
                word_info->nr_delimiters ++;
            else
                return mstr + word_info->len + word_info->nr_delimiters;
        break;

        default:
            if (word_info->delimiter != '\0')
                break;

            word_info->len ++;
        }
    }
            
    return mstr + word_info->len + word_info->nr_delimiters;
}

/************************* US-ASCII Specific Operations **********************/
static int ascii_is_this_charset (const unsigned char* charset)
{
    int i;
    char name [LEN_FONT_NAME + 1];

    for (i = 0; i < LEN_FONT_NAME + 1; i++) {
        if (charset [i] == '\0')
            break;
        name [i] = toupper (charset [i]);
    }
    name [i] = '\0';

    if (strstr (name, "ASCII"))
        return 0;

    return 1;
}

#ifdef _UNICODE_SUPPORT
static UChar32 ascii_conv_to_uc32 (const unsigned char* mchar)
{
    if (*mchar < 128)
        return (UChar32) (*mchar);
    else
        return '?';
}

static int ascii_conv_from_uc32 (UChar32 wc, unsigned char* mchar)
{
    if (wc < 128) {
        *mchar = (unsigned char) wc;
        return 1;
    }

    return 0;
}
#endif

static CHARSETOPS CharsetOps_ascii = {
    128,
    1,
    FONT_CHARSET_US_ASCII,
    {' '},
    sb_len_first_char,
    sb_char_offset,
    sb_char_type,
    sb_nr_chars_in_str,
    ascii_is_this_charset,
    sb_len_first_substr,
    sb_get_next_word,
    sb_pos_first_char,
#ifdef _UNICODE_SUPPORT
    ascii_conv_to_uc32,
    ascii_conv_from_uc32
#endif
};

/************************* ISO8859-1 Specific Operations **********************/
static int iso8859_1_is_this_charset (const unsigned char* charset)
{
    int i;
    char name [LEN_FONT_NAME + 1];

    for (i = 0; i < LEN_FONT_NAME + 1; i++) {
        if (charset [i] == '\0')
            break;
        name [i] = toupper (charset [i]);
    }
    name [i] = '\0';

    if (strstr (name, "ISO") && strstr (name, "8859-1"))
        return 0;

    if (strstr (name, "LATIN1"))
        return 0;

    return 1;
}

#ifdef _UNICODE_SUPPORT
static UChar32 iso8859_1_conv_to_uc32 (const unsigned char* mchar)
{
    return (UChar32) (*mchar);
}

static int iso8859_1_conv_from_uc32 (UChar32 wc, unsigned char* mchar)
{
    if (wc < 256) {
        *mchar = (unsigned char) wc;
        return 1;
    }

    return 0;
}
#endif

static CHARSETOPS CharsetOps_iso8859_1 = {
    256,
    1,
    FONT_CHARSET_ISO8859_1,
    {' '},
    sb_len_first_char,
    sb_char_offset,
    sb_char_type,
    sb_nr_chars_in_str,
    iso8859_1_is_this_charset,
    sb_len_first_substr,
    sb_get_next_word,
    sb_pos_first_char,
#ifdef _UNICODE_SUPPORT
    iso8859_1_conv_to_uc32,
    iso8859_1_conv_from_uc32
#endif
};

#ifdef _LATIN2_SUPPORT

/************************* ISO8859-2 Specific Operations **********************/
static int iso8859_2_is_this_charset (const unsigned char* charset)
{
    int i;
    char name [LEN_FONT_NAME + 1];

    for (i = 0; i < LEN_FONT_NAME + 1; i++) {
        if (charset [i] == '\0')
            break;
        name [i] = toupper (charset [i]);
    }
    name [i] = '\0';

    if (strstr (name, "ISO") && strstr (name, "8859-2"))
        return 0;

    if (strstr (name, "LATIN2"))
        return 0;

    return 1;
}

#ifdef _UNICODE_SUPPORT
static unsigned short iso8859_2_unicode_map [] =
{
    0x0104, 0x02D8,
    0x0141, 0x00A4, 0x013D,
    0x015A, 0x00A7, 0x00A8,
    0x0160, 0x015E, 0x0164,
    0x0179, 0x00AD, 0x017D,
    0x017B, 0x00B0, 0x0105,
    0x02DB, 0x0142, 0x00B4,
    0x013E, 0x015B, 0x02C7,
    0x00B8, 0x0161, 0x015F,
    0x0165, 0x017A, 0x02DD,
    0x017E, 0x017C, 0x0154,
    0x00C1, 0x00C2, 0x0102,
    0x00C4, 0x0139, 0x0106,
    0x00C7, 0x010C, 0x00C9,
    0x0118, 0x00CB, 0x011A,
    0x00CD, 0x00CE, 0x010E,
    0x0110, 0x0143, 0x0147,
    0x00D3, 0x00D4, 0x0150,
    0x00D6, 0x00D7, 0x0158,
    0x016E, 0x00DA, 0x0170,
    0x00DC, 0x00DD, 0x0162,
    0x00DF, 0x0155, 0x00E1,
    0x00E2, 0x0103, 0x00E4,
    0x013A, 0x0107, 0x00E7,
    0x010D, 0x00E9, 0x0119,
    0x00EB, 0x011B, 0x00ED,
    0x00EE, 0x010F, 0x0111,
    0x0144, 0x0148, 0x00F3,
    0x00F4, 0x0151, 0x00F6,
    0x00F7, 0x0159, 0x016F,
    0x00FA, 0x0171, 0x00FC,
    0x00FD, 0x0163, 0x02D9
};

static UChar32 iso8859_2_conv_to_uc32 (const unsigned char* mchar)
{
    if (*mchar < 0xA1)
        return (UChar32) (*mchar);
    else
        return (UChar32) iso8859_2_unicode_map [*mchar - 0xA1];
}

static int iso8859_2_conv_from_uc32 (UChar32 wc, unsigned char* mchar)
{
    int i;

    if (wc < 0xA1) {
        *mchar = (unsigned char)wc;
        return 1;
    }

    for (i = 0; i < TABLESIZE (iso8859_2_unicode_map); i++) {
        if (((UChar32)iso8859_2_unicode_map [i]) == wc) {
            *mchar = 0xA1 + i;
            return 1;
        }
    }

    return 0;
}
#endif

static CHARSETOPS CharsetOps_iso8859_2 = {
    256,
    1,
    FONT_CHARSET_ISO8859_2,
    {' '},
    sb_len_first_char,
    sb_char_offset,
    sb_char_type,
    sb_nr_chars_in_str,
    iso8859_2_is_this_charset,
    sb_len_first_substr,
    sb_get_next_word,
    sb_pos_first_char,
#ifdef _UNICODE_SUPPORT
    iso8859_2_conv_to_uc32,
    iso8859_2_conv_from_uc32,
#endif
};

#endif /* _LATIN2_SUPPORT */

#ifdef _LATIN3_SUPPORT

/************************* ISO8859-3 Specific Operations **********************/
static int iso8859_3_is_this_charset (const unsigned char* charset)
{
    int i;
    char name [LEN_FONT_NAME + 1];

    for (i = 0; i < LEN_FONT_NAME + 1; i++) {
        if (charset [i] == '\0')
            break;
        name [i] = toupper (charset [i]);
    }
    name [i] = '\0';

    if (strstr (name, "ISO") && strstr (name, "8859-3"))
        return 0;

    if (strstr (name, "LATIN3"))
        return 0;

    return 1;
}

#ifdef _UNICODE_SUPPORT
static unsigned short iso8859_3_unicode_map [] =
{
    0x0126, 0x02D8, 0x00A3,
    0x00A4, 0x00A5, 0x0124,
    0x00A7, 0x00A8, 0x0130,
    0x015E, 0x011E, 0x0134,
    0x00AD, 0x00AE, 0x017B,
    0x00B0, 0x0127, 0x00B2,
    0x00B3, 0x00B4, 0x00B5,
    0x0125, 0x00B7, 0x00B8,
    0x0131, 0x015F, 0x011F,
    0x0135, 0x00BD, 0x00BE,
    0x017C, 0x00C0, 0x00C1,
    0x00C2, 0x00C3, 0x00C4,
    0x010A, 0x0108, 0x00C7,
    0x00C8, 0x00C9, 0x00CA,
    0x00CB, 0x00CC, 0x00CD,
    0x00CE, 0x00CF, 0x00D0,
    0x00D1, 0x00D2, 0x00D3,
    0x00D4, 0x0120, 0x00D6,
    0x00D7, 0x011C, 0x00D9,
    0x00DA, 0x00DB, 0x00DC,
    0x016C, 0x015C, 0x00DF,
    0x00E0, 0x00E1, 0x00E2,
    0x00E3, 0x00E4, 0x010B,
    0x0109, 0x00E7, 0x00E8,
    0x00E9, 0x00EA, 0x00EB,
    0x00EC, 0x00ED, 0x00EE,
    0x00EF, 0x00F0, 0x00F1,
    0x00F2, 0x00F3, 0x00F4,
    0x0121, 0x00F6, 0x00F7,
    0x011D, 0x00F9, 0x00FA,
    0x00FB, 0x00FC, 0x016D,
    0x015D, 0x02D9
};

static UChar32 iso8859_3_conv_to_uc32 (const unsigned char* mchar)
{
    if (*mchar < 0xA1)
        return (UChar32) (*mchar);
    else
        return (UChar32) iso8859_3_unicode_map [*mchar - 0xA1];
}

static int iso8859_3_conv_from_uc32 (UChar32 wc, unsigned char* mchar)
{
    int i;

    if (wc < 0xA1) {
        *mchar = (unsigned char) wc;
        return 1;
    }

    for (i = 0; i < TABLESIZE (iso8859_3_unicode_map); i++) {
        if (((UChar32)iso8859_3_unicode_map [i]) == wc) {
            *mchar = 0xA1 + i;
            return 1;
        }
    }
    return 0;
}
#endif

static CHARSETOPS CharsetOps_iso8859_3 = {
    256,
    1,
    FONT_CHARSET_ISO8859_3,
    {' '},
    sb_len_first_char,
    sb_char_offset,
    sb_char_type,
    sb_nr_chars_in_str,
    iso8859_3_is_this_charset,
    sb_len_first_substr,
    sb_get_next_word,
    sb_pos_first_char,
#ifdef _UNICODE_SUPPORT
    iso8859_3_conv_to_uc32,
    iso8859_3_conv_from_uc32,
#endif
};

#endif /* _LATIN4_SUPPORT */

#ifdef _LATIN4_SUPPORT

/************************* ISO8859-4 Specific Operations **********************/
static int iso8859_4_is_this_charset (const unsigned char* charset)
{
    int i;
    char name [LEN_FONT_NAME + 1];

    for (i = 0; i < LEN_FONT_NAME + 1; i++) {
        if (charset [i] == '\0')
            break;
        name [i] = toupper (charset [i]);
    }
    name [i] = '\0';

    if (strstr (name, "ISO") && strstr (name, "8859-4"))
        return 0;

    if (strstr (name, "LATIN4"))
        return 0;

    return 1;
}

#ifdef _UNICODE_SUPPORT
static unsigned short iso8859_4_unicode_map [] =
{
    0x0104, 0x0138, 0x0156,
    0x00A4, 0x0128, 0x013B,
    0x00A7, 0x00A8, 0x0160,
    0x0112, 0x0122, 0x0166,
    0x00AD, 0x017D, 0x00AF,
    0x00B0, 0x0105, 0x02DB,
    0x0157, 0x00B4, 0x0129,
    0x013C, 0x02C7, 0x00B8,
    0x0161, 0x0113, 0x0123,
    0x0167, 0x014A, 0x017E,
    0x014B, 0x0100, 0x00C1,
    0x00C2, 0x00C3, 0x00C4,
    0x00C5, 0x00C6, 0x012E,
    0x010C, 0x00C9, 0x0118,
    0x00CB, 0x0116, 0x00CD,
    0x00CE, 0x012A, 0x0110,
    0x0145, 0x014C, 0x0136,
    0x00D4, 0x00D5, 0x00D6,
    0x00D7, 0x00D8, 0x0172,
    0x00DA, 0x00DB, 0x00DC,
    0x0168, 0x016A, 0x00DF,
    0x0101, 0x00E1, 0x00E2,
    0x00E3, 0x00E4, 0x00E5,
    0x00E6, 0x012F, 0x010D,
    0x00E9, 0x0119, 0x00EB,
    0x0117, 0x00ED, 0x00EE,
    0x012B, 0x0111, 0x0146,
    0x014D, 0x0137, 0x00F4,
    0x00F5, 0x00F6, 0x00F7,
    0x00F8, 0x0173, 0x00FA,
    0x00FB, 0x00FC, 0x0169,
    0x016B, 0x02D9
};

static UChar32 iso8859_4_conv_to_uc32 (const unsigned char* mchar)
{
    if (*mchar < 0xA1)
        return (UChar32) (*mchar);
    else
        return (UChar32) iso8859_4_unicode_map [*mchar - 0xA1];
   
}

static int iso8859_4_conv_from_uc32 (UChar32 wc, unsigned char* mchar)
{
    int i;

    if (wc < 0xA1) {
        *mchar = (unsigned char) wc;
        return 1;
    }

    for (i = 0; i < TABLESIZE (iso8859_4_unicode_map); i++) {
        if (((UChar32)iso8859_4_unicode_map [i]) == wc) {
            *mchar = 0xA1 + i;
            return 1;
        }
    }

    return 0;
}
#endif

static CHARSETOPS CharsetOps_iso8859_4 = {
    256,
    1,
    FONT_CHARSET_ISO8859_4,
    {' '},
    sb_len_first_char,
    sb_char_offset,
    sb_char_type,
    sb_nr_chars_in_str,
    iso8859_4_is_this_charset,
    sb_len_first_substr,
    sb_get_next_word,
    sb_pos_first_char,
#ifdef _UNICODE_SUPPORT
    iso8859_4_conv_to_uc32,
    iso8859_4_conv_from_uc32
#endif
};

#endif /* _LATIN4_SUPPORT */

#ifdef _CYRILLIC_SUPPORT

/************************* ISO8859-5 Specific Operations **********************/
static int iso8859_5_is_this_charset (const unsigned char* charset)
{
    int i;
    char name [LEN_FONT_NAME + 1];

    for (i = 0; i < LEN_FONT_NAME + 1; i++) {
        if (charset [i] == '\0')
            break;
        name [i] = toupper (charset [i]);
    }
    name [i] = '\0';

    if (strstr (name, "ISO") && strstr (name, "8859-5"))
        return 0;

    if (strstr (name, "CYRILLIC"))
        return 0;

    return 1;
}

#ifdef _UNICODE_SUPPORT
static UChar32 iso8859_5_conv_to_uc32 (const unsigned char* mchar)
{
    if (*mchar < 0xA1)
        return (UChar32) (*mchar);
   
    if (*mchar == 0xAD)
        return 0x00AD;  /* SOFT HYPHEN */

    if (*mchar == 0xF0)
        return 0x2116;  /* NUMERO SIGN */

    if (*mchar == 0xFD)
        return 0x00A7;  /* SECTION SIGN */

    return ((UChar32)*mchar) + (0x0401 - 0xA1);
}

static int iso8859_5_conv_from_uc32 (UChar32 wc, unsigned char* mchar)
{
    if (wc < 0xA1) {
        *mchar = (unsigned char) wc;
        return 1;
    }

    switch (wc) {
        case 0x00A7:
            *mchar = 0xFD;
            return 1;
        case 0x00AD:
            *mchar = 0xAD;
            return 1;
        case 0x2116:
            *mchar = 0xF0;
            return 1;
    }

    if (wc >= 0x0401 && wc <= (0x0401 + 0xFF - 0xA1)) {
        *mchar = (unsigned char) (wc - 0x0401 + 0xA1);
        return 1;
    }

    return 0;
}
#endif

static CHARSETOPS CharsetOps_iso8859_5 = {
    256,
    1,
    FONT_CHARSET_ISO8859_5,
    {' '},
    sb_len_first_char,
    sb_char_offset,
    sb_char_type,
    sb_nr_chars_in_str,
    iso8859_5_is_this_charset,
    sb_len_first_substr,
    sb_get_next_word,
    sb_pos_first_char,
#ifdef _UNICODE_SUPPORT
    iso8859_5_conv_to_uc32,
    iso8859_5_conv_from_uc32,
#endif
};

#endif /* _CYRILLIC_SUPPORT */

#ifdef _ARABIC_SUPPORT

/************************* ISO8859-6 Specific Operations **********************/
static int iso8859_6_is_this_charset (const unsigned char* charset)
{
    int i;
    char name [LEN_FONT_NAME + 1];

    for (i = 0; i < LEN_FONT_NAME + 1; i++) {
        if (charset [i] == '\0')
            break;
        name [i] = toupper (charset [i]);
    }
    name [i] = '\0';

    if (strstr (name, "ISO") && strstr (name, "8859-6"))
        return 0;

    if (strstr (name, "ARABIC"))
        return 0;

    return 1;
}

#ifdef _UNICODE_SUPPORT
static UChar32 iso8859_6_conv_to_uc32 (const unsigned char* mchar)
{
    if (*mchar >= 0xC1 && *mchar <= 0xF2) {
        return  ((UChar32)*mchar) - 0xC1 + 0x0621;
    }
    else {
        switch (*mchar) {
            case 0xAC:
                return 0x060C;
            case 0xBB:
                return 0x061B;
            case 0xBF:
                return 0x061F;
        }

        return (UChar32) (*mchar);
    }
}

static int iso8859_6_conv_from_uc32 (UChar32 wc, unsigned char* mchar)
{
    switch (wc) {
        case 0x060C:
            *mchar = 0xAC;
            return 1;
        case 0x061B:
            *mchar = 0xBB;
            return 1;
        case 0x061F:
            *mchar = 0xBF;
            return 1;
    }

    if (wc < 0xC1) {
        *mchar = (unsigned char) wc;
        return 1;
    }

    if (wc >= 0x0621 && wc <= (0x0621 + 0xF2 - 0xC1)) {
        *mchar = (unsigned char) (wc - 0x0621 + 0xC1);
        return 1;
    }

    return 0;
}
#endif

static CHARSETOPS CharsetOps_iso8859_6 = {
    256,
    1,
    FONT_CHARSET_ISO8859_6,
    {' '},
    sb_len_first_char,
    sb_char_offset,
    sb_char_type,
    sb_nr_chars_in_str,
    iso8859_6_is_this_charset,
    sb_len_first_substr,
    sb_get_next_word,
    sb_pos_first_char,
#ifdef _UNICODE_SUPPORT
    iso8859_6_conv_to_uc32,
    iso8859_6_conv_from_uc32
#endif
};

#endif /* _ARABIC_SUPPORT */

#ifdef _GREEK_SUPPORT

/************************* ISO8859-7 Specific Operations **********************/
static int iso8859_7_is_this_charset (const unsigned char* charset)
{
    int i;
    char name [LEN_FONT_NAME + 1];

    for (i = 0; i < LEN_FONT_NAME + 1; i++) {
        if (charset [i] == '\0')
            break;
        name [i] = toupper (charset [i]);
    }
    name [i] = '\0';

    if (strstr (name, "ISO") && strstr (name, "8859-7"))
        return 0;

    if (strstr (name, "GREEK"))
        return 0;

    return 1;
}

#ifdef _UNICODE_SUPPORT
static unsigned short iso8859_7_unicode_map [] =
{
    0x2018, 0x2019, 0x00A3,
    0x00A4, 0x00A5, 0x00A6,
    0x00A7, 0x00A8, 0x00A9,
    0x00AA, 0x00AB, 0x00AC,
    0x00AD, 0x00AE, 0x2015,
    0x00B0, 0x00B1, 0x00B2,
    0x00B3, 0x0384, 0x0385,
    0x0386, 0x00B7, 0x0388,
    0x0389, 0x038A, 0x00BB,
    0x038C, 0x00BD
};

static UChar32 iso8859_7_conv_to_uc32 (const unsigned char* mchar)
{
    if (*mchar >= 0xBE && *mchar <= 0xFE) {
        return ((UChar32)*mchar) - 0xBE + 0x038E;
    }
    else if (*mchar >= 0xA1 && *mchar <= 0xBD) {
        return (UChar32)iso8859_7_unicode_map [*mchar - 0xA1];
    }
    else {
        return (UChar32) (*mchar);
    }
}

static int iso8859_7_conv_from_uc32 (UChar32 wc, unsigned char* mchar)
{
    int i;

    if (wc < 0xA1) {
        *mchar = (unsigned char) wc;
        return 1;
    }

    if (wc >= 0x038E && wc <= (0x038E + 0xFE - 0xBE)) {
        *mchar = (unsigned char) (wc - 0x038E + 0xBE);
        return 1;
    }

    for (i = 0; i < TABLESIZE (iso8859_7_unicode_map); i++) {
        if (((UChar32)iso8859_7_unicode_map [i]) == wc) {
            *mchar = 0xA1 + i;
            return 1;
        }
    }

    return 0;
}
#endif

static CHARSETOPS CharsetOps_iso8859_7 = {
    256,
    1,
    FONT_CHARSET_ISO8859_7,
    {' '},
    sb_len_first_char,
    sb_char_offset,
    sb_char_type,
    sb_nr_chars_in_str,
    iso8859_7_is_this_charset,
    sb_len_first_substr,
    sb_get_next_word,
    sb_pos_first_char,
#ifdef _UNICODE_SUPPORT
    iso8859_7_conv_to_uc32,
    iso8859_7_conv_from_uc32
#endif
};

#endif /* _GREEK_SUPPORT */

#ifdef _HEBREW_SUPPORT

/************************* ISO8859-8 Specific Operations **********************/
static int iso8859_8_is_this_charset (const unsigned char* charset)
{
    int i;
    char name [LEN_FONT_NAME + 1];

    for (i = 0; i < LEN_FONT_NAME + 1; i++) {
        if (charset [i] == '\0')
            break;
        name [i] = toupper (charset [i]);
    }
    name [i] = '\0';

    if (strstr (name, "ISO") && strstr (name, "8859-8"))
        return 0;

    if (strstr (name, "HEBREW"))
        return 0;

    return 1;
}

#ifdef _UNICODE_SUPPORT
static UChar32 iso8859_8_conv_to_uc32 (const unsigned char* mchar)
{
    if (*mchar >= 0xE0 && *mchar <= 0xFA) {
        return  ((UChar32)*mchar) - 0xE0 + 0x05D0;
    }
    else {
        switch (*mchar) {
            case 0xAA:
                return 0x00D7;
            case 0xBA:
                return 0x00F7;
            case 0xDF:
                return 0x2017;
            case 0xFD:
                return 0x200E;
            case 0xFE:
                return 0x200F;
        }
        return (UChar32) (*mchar);
    }
}

static int iso8859_8_conv_from_uc32 (UChar32 wc, unsigned char* mchar)
{
    switch (wc) {
        case 0x00D7:
            *mchar = 0xAA;
            return 1;
        case 0x00F7:
            *mchar = 0xBA;
            return 1;
        case 0x2017:
            *mchar = 0xDF;
            return 1;
        case 0x200E:
            *mchar = 0xFD;
            return 1;
        case 0x200F:
            *mchar = 0xFE;
            return 1;
    }

    if (wc < 0xE0) {
        *mchar = (unsigned char) wc;
        return 1;
    }

    if (wc >= 0x05D0 && wc <= (0x05D0 + 0xFA - 0xE0)) {
        *mchar = (unsigned char) (wc - 0x05D0 + 0xE0);
        return 1;
    }

    return 0;
}
#endif

static CHARSETOPS CharsetOps_iso8859_8 = {
    256,
    1,
    FONT_CHARSET_ISO8859_8,
    {' '},
    sb_len_first_char,
    sb_char_offset,
    sb_char_type,
    sb_nr_chars_in_str,
    iso8859_8_is_this_charset,
    sb_len_first_substr,
    sb_get_next_word,
    sb_pos_first_char,
#ifdef _UNICODE_SUPPORT
    iso8859_8_conv_to_uc32,
    iso8859_8_conv_from_uc32
#endif
};

#endif /* _GREEK_SUPPORT */

#ifdef _LATIN5_SUPPORT

/************************* ISO8859-9 Specific Operations **********************/
static int iso8859_9_is_this_charset (const unsigned char* charset)
{
    int i;
    char name [LEN_FONT_NAME + 1];

    for (i = 0; i < LEN_FONT_NAME + 1; i++) {
        if (charset [i] == '\0')
            break;
        name [i] = toupper (charset [i]);
    }
    name [i] = '\0';

    if (strstr (name, "ISO") && strstr (name, "8859-9"))
        return 0;

    if (strstr (name, "LATIN5"))
        return 0;

    return 1;
}

#ifdef _UNICODE_SUPPORT
static UChar32 iso8859_9_conv_to_uc32 (const unsigned char* mchar)
{
    switch (*mchar) {
        case 0xD0:
                return 0x011E;
        case 0xDD:
                return 0x0130;
        case 0xDE:
                return 0x015E;
        case 0xF0:
                return 0x011F;
        case 0xFD:
                return 0x0131;
        case 0xFE:
                return 0x015F;
    }

    return (UChar32) (*mchar);
}

static int iso8859_9_conv_from_uc32 (UChar32 wc, unsigned char* mchar)
{
    switch (wc) {
        case 0x011E:
            *mchar = 0xD0;
            return 1;
        case 0x0130:
            *mchar = 0xDD;
            return 1;
        case 0x015E:
            *mchar = 0xDE;
            return 1;
        case 0x011F:
            *mchar = 0xF0;
            return 1;
        case 0x0131:
            *mchar = 0xFD;
            return 1;
        case 0x015F:
            *mchar = 0xFE;
            return 1;
    }

    if (wc <= 0xFF) {
        *mchar = (unsigned char)wc;
        return 1;
    }

    return 0;
}
#endif

static CHARSETOPS CharsetOps_iso8859_9 = {
    256,
    1,
    FONT_CHARSET_ISO8859_9,
    {' '},
    sb_len_first_char,
    sb_char_offset,
    sb_char_type,
    sb_nr_chars_in_str,
    iso8859_9_is_this_charset,
    sb_len_first_substr,
    sb_get_next_word,
    sb_pos_first_char,
#ifdef _UNICODE_SUPPORT
    iso8859_9_conv_to_uc32,
    iso8859_9_conv_from_uc32
#endif
};

#endif /* _LATIN5_SUPPORT */

#ifdef _LATIN6_SUPPORT

/************************* ISO8859-10 Specific Operations **********************/
static int iso8859_10_is_this_charset (const unsigned char* charset)
{
    int i;
    char name [LEN_FONT_NAME + 1];

    for (i = 0; i < LEN_FONT_NAME + 1; i++) {
        if (charset [i] == '\0')
            break;
        name [i] = toupper (charset [i]);
    }
    name [i] = '\0';

    if (strstr (name, "ISO") && strstr (name, "8859-10"))
        return 0;

    if (strstr (name, "LATIN6"))
        return 0;

    return 1;
}

#ifdef _UNICODE_SUPPORT
static unsigned short iso8859_10_unicode_map [] =
{
    0x0104, 0x0112, 0x0122,
    0x012A, 0x0128, 0x0136,
    0x00A7, 0x013B, 0x0110,
    0x0160, 0x0166, 0x017D,
    0x00AD, 0x016A, 0x014A,
    0x00B0, 0x0105, 0x0113,
    0x0123, 0x012B, 0x0129,
    0x0137, 0x00B7, 0x013C,
    0x0111, 0x0161, 0x0167,
    0x017E, 0x2015, 0x016B,
    0x014B, 0x0100, 0x00C1,
    0x00C2, 0x00C3, 0x00C4,
    0x00C5, 0x00C6, 0x012E,
    0x010C, 0x00C9, 0x0118,
    0x00CB, 0x0116, 0x00CD,
    0x00CE, 0x00CF, 0x00D0,
    0x0145, 0x014C, 0x00D3,
    0x00D4, 0x00D5, 0x00D6,
    0x0168, 0x00D8, 0x0172,
    0x00DA, 0x00DB, 0x00DC,
    0x00DD, 0x00DE, 0x00DF,
    0x0101, 0x00E1, 0x00E2,
    0x00E3, 0x00E4, 0x00E5,
    0x00E6, 0x012F, 0x010D,
    0x00E9, 0x0119, 0x00EB,
    0x0117, 0x00ED, 0x00EE,
    0x00EF, 0x00F0, 0x0146,
    0x014D, 0x00F3, 0x00F4,
    0x00F5, 0x00F6, 0x0169,
    0x00F8, 0x0173, 0x00FA,
    0x00FB, 0x00FC, 0x00FD,
    0x00FE, 0x0138
};

static UChar32 iso8859_10_conv_to_uc32 (const unsigned char* mchar)
{
    if (*mchar < 0xA1)
        return (UChar32) (*mchar);
    else
        return (UChar32)iso8859_10_unicode_map [*mchar - 0xA1];
}

static int iso8859_10_conv_from_uc32 (UChar32 wc, unsigned char* mchar)
{
    int i;

    if (wc < 0xA1) {
        *mchar = (unsigned char)wc;
        return 1;
    }

    for (i = 0; i < TABLESIZE (iso8859_10_unicode_map); i++) {
        if (((UChar32)iso8859_10_unicode_map [i]) == wc) {
            *mchar = 0xA1 + i;
            return 1;
        }
    }

    return 0;
}
#endif

static CHARSETOPS CharsetOps_iso8859_10 = {
    256,
    1,
    FONT_CHARSET_ISO8859_10,
    {' '},
    sb_len_first_char,
    sb_char_offset,
    sb_char_type,
    sb_nr_chars_in_str,
    iso8859_10_is_this_charset,
    sb_len_first_substr,
    sb_get_next_word,
    sb_pos_first_char,
#ifdef _UNICODE_SUPPORT
    iso8859_10_conv_to_uc32,
    iso8859_10_conv_from_uc32
#endif
};

#endif /* _LATIN6_SUPPORT */

#ifdef _THAI_SUPPORT

/************************* ISO8859-11 Specific Operations **********************/
static int iso8859_11_is_this_charset (const unsigned char* charset)
{
    int i;
    char name [LEN_FONT_NAME + 1];

    for (i = 0; i < LEN_FONT_NAME + 1; i++) {
        if (charset [i] == '\0')
            break;
        name [i] = toupper (charset [i]);
    }
    name [i] = '\0';

    if (strstr (name, "ISO") && strstr (name, "8859-11"))
        return 0;

    if (strstr (name, "TIS") && strstr (name, "620"))
        return 0;

    return 1;
}

#ifdef _UNICODE_SUPPORT
static unsigned short iso8859_11_unicode_map [] =
{
    0x0E01, 0x0E02, 0x0E03,
    0x0E04, 0x0E05, 0x0E06,
    0x0E07, 0x0E08, 0x0E09,
    0x0E0A, 0x0E0B, 0x0E0C,
    0x0E0D, 0x0E0E, 0x0E0F,
    0x0E10, 0x0E11, 0x0E12,
    0x0E13, 0x0E14, 0x0E15,
    0x0E16, 0x0E17, 0x0E18,
    0x0E19, 0x0E1A, 0x0E1B,
    0x0E1C, 0x0E1D, 0x0E1E,
    0x0E1F, 0x0E20, 0x0E21,
    0x0E22, 0x0E23, 0x0E24,
    0x0E25, 0x0E26, 0x0E27,
    0x0E28, 0x0E29, 0x0E2A,
    0x0E2B, 0x0E2C, 0x0E2D,
    0x0E2E, 0x0E2F, 0x0E30,
    0x0E31, 0x0E32, 0x0E33,
    0x0E34, 0x0E35, 0x0E36,
    0x0E37, 0x0E38, 0x0E39,
    0x0E3A, 0x00DB, 0x00DC,
    0x00DD, 0x00DE, 0x0E3F,
    0x0E40, 0x0E41, 0x0E42,
    0x0E43, 0x0E44, 0x0E45,
    0x0E46, 0x0E47, 0x0E48,
    0x0E49, 0x0E4A, 0x0E4B,
    0x0E4C, 0x0E4D, 0x0E4E,
    0x0E4F, 0x0E50, 0x0E51,
    0x0E52, 0x0E53, 0x0E54,
    0x0E55, 0x0E56, 0x0E57,
    0x0E58, 0x0E59, 0x0E5A,
    0x0E5B, 0x00FC, 0x00FD,
    0x00FE, 0x00FF
};

static UChar32 iso8859_11_conv_to_uc32 (const unsigned char* mchar)
{
    if (*mchar < 0xA1)
        return (UChar32) (*mchar);
    else
        return (UChar32) iso8859_11_unicode_map [*mchar - 0xA1];
}

static int iso8859_11_conv_from_uc32 (UChar32 wc, unsigned char* mchar)
{
    int i;

    if (wc < 0xA1) {
        *mchar = (unsigned char)wc;
        return 1;
    }

    for (i = 0; i < TABLESIZE (iso8859_11_unicode_map); i++) {
        if (((UChar32)iso8859_11_unicode_map [i]) == wc) {
            *mchar = 0xA1 + i;
            return 1;
        }
    }
    return 0;
}
#endif

static CHARSETOPS CharsetOps_iso8859_11 = {
    256,
    1,
    FONT_CHARSET_ISO8859_11,
    {' '},
    sb_len_first_char,
    sb_char_offset,
    sb_char_type,
    sb_nr_chars_in_str,
    iso8859_11_is_this_charset,
    sb_len_first_substr,
    sb_get_next_word,
    sb_pos_first_char,
#ifdef _UNICODE_SUPPORT
    iso8859_11_conv_to_uc32,
    iso8859_11_conv_from_uc32
#endif
};

#endif /* _THAI_SUPPORT */

#ifdef _LATIN7_SUPPORT

/************************* ISO8859-13 Specific Operations **********************/
static int iso8859_13_is_this_charset (const unsigned char* charset)
{
    int i;
    char name [LEN_FONT_NAME + 1];

    for (i = 0; i < LEN_FONT_NAME + 1; i++) {
        if (charset [i] == '\0')
            break;
        name [i] = toupper (charset [i]);
    }
    name [i] = '\0';

    if (strstr (name, "ISO") && strstr (name, "8859-13"))
        return 0;

    if (strstr (name, "LATIN7"))
        return 0;

    return 1;
}

#ifdef _UNICODE_SUPPORT
static unsigned short iso8859_13_unicode_map [] =
{
    0x201D, 0x00A2, 0x00A3,
    0x00A4, 0x201E, 0x00A6,
    0x00A7, 0x00D8, 0x00A9,
    0x0156, 0x00AB, 0x00AC,
    0x00AD, 0x00AE, 0x00C6,
    0x00B0, 0x00B1, 0x00B2,
    0x00B3, 0x201C, 0x00B5,
    0x00B6, 0x00B7, 0x00F8,
    0x00B9, 0x0157, 0x00BB,
    0x00BC, 0x00BD, 0x00BE,
    0x00E6, 0x0104, 0x012E,
    0x0100, 0x0106, 0x00C4,
    0x00C5, 0x0118, 0x0112,
    0x010C, 0x00C9, 0x0179,
    0x0116, 0x0122, 0x0136,
    0x012A, 0x013B, 0x0160,
    0x0143, 0x0145, 0x00D3,
    0x014C, 0x00D5, 0x00D6,
    0x00D7, 0x0172, 0x0141,
    0x015A, 0x016A, 0x00DC,
    0x017B, 0x017D, 0x00DF,
    0x0105, 0x012F, 0x0101,
    0x0107, 0x00E4, 0x00E5,
    0x0119, 0x0113, 0x010D,
    0x00E9, 0x017A, 0x0117,
    0x0123, 0x0137, 0x012B,
    0x013C, 0x0161, 0x0144,
    0x0146, 0x00F3, 0x014D,
    0x00F5, 0x00F6, 0x00F7,
    0x0173, 0x0142, 0x015B,
    0x016B, 0x00FC, 0x017C,
    0x017E, 0x2019
};

static UChar32 iso8859_13_conv_to_uc32 (const unsigned char* mchar)
{
    if (*mchar < 0xA1)
        return (UChar32) (*mchar);
    else
        return (UChar32) iso8859_13_unicode_map [*mchar - 0xA1];
}

static int iso8859_13_conv_from_uc32 (UChar32 wc, unsigned char* mchar)
{
    int i;

    if (wc < 0xA1) {
        *mchar = (unsigned char)wc;
        return 1;
    }

    for (i = 0; i < TABLESIZE (iso8859_13_unicode_map); i++) {
        if (((UChar32)iso8859_13_unicode_map [i]) == wc) {
            *mchar = 0xA1 + i;
            return 1;
        }
    }
    return 0;
}
#endif

static CHARSETOPS CharsetOps_iso8859_13 = {
    256,
    1,
    FONT_CHARSET_ISO8859_13,
    {' '},
    sb_len_first_char,
    sb_char_offset,
    sb_char_type,
    sb_nr_chars_in_str,
    iso8859_13_is_this_charset,
    sb_len_first_substr,
    sb_get_next_word,
    sb_pos_first_char,
#ifdef _UNICODE_SUPPORT
    iso8859_13_conv_to_uc32,
    iso8859_13_conv_from_uc32
#endif
};

#endif /* _LATIN7_SUPPORT */

#ifdef _LATIN8_SUPPORT

/************************* ISO8859-14 Specific Operations **********************/
static int iso8859_14_is_this_charset (const unsigned char* charset)
{
    int i;
    char name [LEN_FONT_NAME + 1];

    for (i = 0; i < LEN_FONT_NAME + 1; i++) {
        if (charset [i] == '\0')
            break;
        name [i] = toupper (charset [i]);
    }
    name [i] = '\0';

    if (strstr (name, "ISO") && strstr (name, "8859-14"))
        return 0;

    if (strstr (name, "LATIN8"))
        return 0;

    return 1;
}

#ifdef _UNICODE_SUPPORT
static unsigned short iso8859_14_unicode_map [] =
{
    0x1E02, 0x1E03, 0x00A3,
    0x010A, 0x010B, 0x1E0A,
    0x00A7, 0x1E80, 0x00A9,
    0x1E82, 0x1E0B, 0x1EF2,
    0x00AD, 0x00AE, 0x0178,
    0x1E1E, 0x1E1F, 0x0120,
    0x0121, 0x1E40, 0x1E41,
    0x00B6, 0x1E56, 0x1E81,
    0x1E57, 0x1E83, 0x1E60,
    0x1EF3, 0x1E84, 0x1E85,
    0x1E61, 0x00C0, 0x00C1,
    0x00C2, 0x00C3, 0x00C4,
    0x00C5, 0x00C6, 0x00C7,
    0x00C8, 0x00C9, 0x00CA,
    0x00CB, 0x00CC, 0x00CD,
    0x00CE, 0x00CF, 0x0174,
    0x00D1, 0x00D2, 0x00D3,
    0x00D4, 0x00D5, 0x00D6,
    0x1E6A, 0x00D8, 0x00D9,
    0x00DA, 0x00DB, 0x00DC,
    0x00DD, 0x0176, 0x00DF,
    0x00E0, 0x00E1, 0x00E2,
    0x00E3, 0x00E4, 0x00E5,
    0x00E6, 0x00E7, 0x00E8,
    0x00E9, 0x00EA, 0x00EB,
    0x00EC, 0x00ED, 0x00EE,
    0x00EF, 0x0175, 0x00F1,
    0x00F2, 0x00F3, 0x00F4,
    0x00F5, 0x00F6, 0x1E6B,
    0x00F8, 0x00F9, 0x00FA,
    0x00FB, 0x00FC, 0x00FD,
    0x0177, 0x00FF
};

static UChar32 iso8859_14_conv_to_uc32 (const unsigned char* mchar)
{
    if (*mchar < 0xA1)
        return (UChar32) (*mchar);
    else
        return (UChar32) iso8859_14_unicode_map [*mchar - 0xA1];
}

static int iso8859_14_conv_from_uc32 (UChar32 wc, unsigned char* mchar)
{
    int i;

    if (wc < 0xA1) {
        *mchar = (unsigned char)wc;
        return 1;
    }

    for (i = 0; i < TABLESIZE (iso8859_14_unicode_map); i++) {
        if (((UChar32)iso8859_14_unicode_map [i]) == wc) {
            *mchar = 0xA1 + i;
            return 1;
        }
    }
    return 0;
}
#endif

static CHARSETOPS CharsetOps_iso8859_14 = {
    256,
    1,
    FONT_CHARSET_ISO8859_14,
    {' '},
    sb_len_first_char,
    sb_char_offset,
    sb_char_type,
    sb_nr_chars_in_str,
    iso8859_14_is_this_charset,
    sb_len_first_substr,
    sb_get_next_word,
    sb_pos_first_char,
#ifdef _UNICODE_SUPPORT
    iso8859_14_conv_to_uc32,
    iso8859_14_conv_from_uc32
#endif
};

#endif /* _LATIN8_SUPPORT */

#ifdef _LATIN9_SUPPORT

/************************* ISO8859-15 Specific Operations **********************/
static int iso8859_15_is_this_charset (const unsigned char* charset)
{
    int i;
    char name [LEN_FONT_NAME + 1];

    for (i = 0; i < LEN_FONT_NAME + 1; i++) {
        if (charset [i] == '\0')
            break;
        name [i] = toupper (charset [i]);
    }
    name [i] = '\0';

    if (strstr (name, "ISO") && strstr (name, "8859-15"))
        return 0;

    if (strstr (name, "LATIN") && strstr (name, "9"))
        return 0;

    return 1;
}

#ifdef _UNICODE_SUPPORT
static UChar32 iso8859_15_conv_to_uc32 (const unsigned char* mchar)
{
    switch (*mchar) {
        case 0xA4:
            return 0x20AC;  /* EURO SIGN */
        case 0xA6:
            return 0x0160;  /* LATIN CAPITAL LETTER S WITH CARO */
        case 0xA8:
            return 0x0161;  /* LATIN SMALL LETTER S WITH CARON */
        case 0xB4:
            return 0x017D;  /* LATIN CAPITAL LETTER Z WITH CARON */
        case 0xB8:
            return 0x017E;  /* LATIN SMALL LETTER Z WITH CARON */
        case 0xBC:
            return 0x0152;  /* LATIN CAPITAL LIGATURE OE */
        case 0xBD:
            return 0x0153;  /* LATIN SMALL LIGATURE OE */
        case 0xBE:
            return 0x0178;  /* LATIN CAPITAL LETTER Y WITH DIAERESIS */
        default:
            return *mchar;
    }
}

static int iso8859_15_conv_from_uc32 (UChar32 wc, unsigned char* mchar)
{
    switch (wc) {
        case 0x20AC:  /* EURO SIGN */
            *mchar = 0xA4;
            return 1;
        case 0x0160:  /* LATIN CAPITAL LETTER S WITH CARO */
            *mchar = 0xA6;
            return 1;
        case 0x0161:  /* LATIN SMALL LETTER S WITH CARON */
            *mchar = 0xA8;
            return 1;
        case 0x017D:  /* LATIN CAPITAL LETTER Z WITH CARON */
            *mchar = 0xB4;
            return 1;
        case 0x017E:  /* LATIN SMALL LETTER Z WITH CARON */
            *mchar = 0xB8;
            return 1;
        case 0x0152:  /* LATIN CAPITAL LIGATURE OE */
            *mchar = 0xBC;
            return 1;
        case 0x0153:  /* LATIN SMALL LIGATURE OE */
            *mchar = 0xBD;
            return 1;
        case 0x0178:  /* LATIN CAPITAL LETTER Y WITH DIAERESIS */
            *mchar = 0xBE;
            return 1;
    }

    if (wc <= 0xFF) {
        *mchar = (unsigned char)wc;
        return 1;
    }

    return 0;
}
#endif

static CHARSETOPS CharsetOps_iso8859_15 = {
    256,
    1,
    FONT_CHARSET_ISO8859_15,
    {' '},
    sb_len_first_char,
    sb_char_offset,
    sb_char_type,
    sb_nr_chars_in_str,
    iso8859_15_is_this_charset,
    sb_len_first_substr,
    sb_get_next_word,
    sb_pos_first_char,
#ifdef _UNICODE_SUPPORT
    iso8859_15_conv_to_uc32,
    iso8859_15_conv_from_uc32
#endif
};
#endif /* _LATIN9_SUPPORT */

#ifdef _LATIN10_SUPPORT

/************************* ISO8859-16 Specific Operations **********************/
static int iso8859_16_is_this_charset (const unsigned char* charset)
{
    int i;
    char name [LEN_FONT_NAME + 1];

    for (i = 0; i < LEN_FONT_NAME + 1; i++) {
        if (charset [i] == '\0')
            break;
        name [i] = toupper (charset [i]);
    }
    name [i] = '\0';

    if (strstr (name, "ISO") && strstr (name, "8859-16"))
        return 0;

    if (strstr (name, "LATIN") && strstr (name, "10"))
        return 0;

    return 1;
}

#ifdef _UNICODE_SUPPORT
static UChar32 iso8859_16_conv_to_uc32 (const unsigned char* mchar)
{
    switch (*mchar) {
        case 0xA1: return 0x0104;
        case 0xA2: return 0x0105;
        case 0xA3: return 0x0141;
        case 0xA4: return 0x20AC;
        case 0xA5: return 0x201E;
        case 0xA6: return 0x0160;
        case 0xA8: return 0x0161;
        case 0xAA: return 0x0218;
        case 0xAC: return 0x0179;
        case 0xAE: return 0x017A;
        case 0xAF: return 0x017B;
        case 0xB2: return 0x010C;
        case 0xB3: return 0x0142;
        case 0xB4: return 0x017D;
        case 0xB5: return 0x201D;
        case 0xB8: return 0x017E;
        case 0xB9: return 0x010D;
        case 0xBA: return 0x0219;
        case 0xBC: return 0x0152;
        case 0xBD: return 0x0153;
        case 0xBE: return 0x0178;
        case 0xBF: return 0x017C;
        case 0xC3: return 0x0102;
        case 0xC5: return 0x0106;
        case 0xD0: return 0x0110;
        case 0xD1: return 0x0143;
        case 0xD5: return 0x0150;
        case 0xD7: return 0x015A;
        case 0xD8: return 0x0170;
        case 0xDD: return 0x0118;
        case 0xDE: return 0x021A;
        case 0xE3: return 0x0103;
        case 0xE5: return 0x0107;
        case 0xF0: return 0x0111;
        case 0xF1: return 0x0144;
        case 0xF5: return 0x0151;
        case 0xF7: return 0x015B;
        case 0xF8: return 0x0171;
        case 0xFD: return 0x0119;
        case 0xFE: return 0x021B;
    }

    return (UChar32) (*mchar);
}

static int iso8859_16_conv_from_uc32 (UChar32 wc, unsigned char* mchar)
{
    switch (wc) {
        case 0x0104: *mchar = 0xA1; return 1;
        case 0x0105: *mchar = 0xA2; return 1;
        case 0x0141: *mchar = 0xA3; return 1;
        case 0x20AC: *mchar = 0xA4; return 1;
        case 0x201E: *mchar = 0xA5; return 1;
        case 0x0160: *mchar = 0xA6; return 1;
        case 0x0161: *mchar = 0xA8; return 1;
        case 0x0218: *mchar = 0xAA; return 1;
        case 0x0179: *mchar = 0xAC; return 1;
        case 0x017A: *mchar = 0xAE; return 1;
        case 0x017B: *mchar = 0xAF; return 1;
        case 0x010C: *mchar = 0xB2; return 1;
        case 0x0142: *mchar = 0xB3; return 1;
        case 0x017D: *mchar = 0xB4; return 1;
        case 0x201D: *mchar = 0xB5; return 1;
        case 0x017E: *mchar = 0xB8; return 1;
        case 0x010D: *mchar = 0xB9; return 1;
        case 0x0219: *mchar = 0xBA; return 1;
        case 0x0152: *mchar = 0xBC; return 1;
        case 0x0153: *mchar = 0xBD; return 1;
        case 0x0178: *mchar = 0xBE; return 1;
        case 0x017C: *mchar = 0xBF; return 1;
        case 0x0102: *mchar = 0xC3; return 1;
        case 0x0106: *mchar = 0xC5; return 1;
        case 0x0110: *mchar = 0xD0; return 1;
        case 0x0143: *mchar = 0xD1; return 1;
        case 0x0150: *mchar = 0xD5; return 1;
        case 0x015A: *mchar = 0xD7; return 1;
        case 0x0170: *mchar = 0xD8; return 1;
        case 0x0118: *mchar = 0xDD; return 1;
        case 0x021A: *mchar = 0xDE; return 1;
        case 0x0103: *mchar = 0xE3; return 1;
        case 0x0107: *mchar = 0xE5; return 1;
        case 0x0111: *mchar = 0xF0; return 1;
        case 0x0144: *mchar = 0xF1; return 1;
        case 0x0151: *mchar = 0xF5; return 1;
        case 0x015B: *mchar = 0xF7; return 1;
        case 0x0171: *mchar = 0xF8; return 1;
        case 0x0119: *mchar = 0xFD; return 1;
        case 0x021B: *mchar = 0xFE; return 1;
    }

    if (wc <= 0xFF) {
        *mchar = (unsigned char)wc;
        return 1;
    }

    return 0;
}
#endif

static CHARSETOPS CharsetOps_iso8859_16 = {
    256,
    1,
    FONT_CHARSET_ISO8859_16,
    {' '},
    sb_len_first_char,
    sb_char_offset,
    sb_char_type,
    sb_nr_chars_in_str,
    iso8859_16_is_this_charset,
    sb_len_first_substr,
    sb_get_next_word,
    sb_pos_first_char,
#ifdef _UNICODE_SUPPORT
    iso8859_16_conv_to_uc32,
    iso8859_16_conv_from_uc32
#endif
};

#endif /* _LATIN10_SUPPORT */

/*************** Common Operations for double bytes charsets ******************/
#if defined(_GB_SUPPORT) | defined(_GBK_SUPPORT) | defined(_BIG5_SUPPORT) \
        | defined(_EUCKR_SUPPORT) | defined(_EUCJP_SUPPORT) \
        | defined(_SHIFTJIS_SUPPORT) \
        | defined(_UNICODE_SUPPORT)

static unsigned int mb_char_type (const unsigned char* mchar)
{
    /* TODO: get the subtype of the char */
    return MCHAR_TYPE_GENERIC;
}

static int db_nr_chars_in_str (const unsigned char* mstr, int mstrlen)
{
    assert ((mstrlen % 2) == 0);
    return mstrlen >> 1;
}

static const unsigned char* db_get_next_word (const unsigned char* mstr,
                int mstrlen, WORDINFO* word_info)
{
    assert ((mstrlen % 2) == 0);

    word_info->len = 0;
    word_info->delimiter = '\0';
    word_info->nr_delimiters = 0;

    if (mstrlen < 2) return NULL;

    word_info->len = 2;

    return mstr + 2;
}

#endif

#ifdef _GB_SUPPORT
/************************* GB2312 Specific Operations ************************/
#define IS_GB2312_CHAR(ch1, ch2) \
        if (((ch1 >= 0xA1 && ch1 <= 0xA9) || (ch1 >= 0xB0 && ch1 <= 0xF7)) \
                        && ch2 >= 0xA1 && ch2 <= 0xFE)

static int gb2312_0_len_first_char (const unsigned char* mstr, int len)
{
    unsigned char ch1;
    unsigned char ch2;

    if (len < 2) return 0;

    ch1 = mstr [0];
    if (ch1 == '\0')
        return 0;

    ch2 = mstr [1];
    IS_GB2312_CHAR (ch1, ch2)
        return 2;

    return 0;
}

static unsigned int gb2312_0_char_offset (const unsigned char* mchar)
{
    int area = mchar [0] - 0xA1;

#if !defined(_TTF_SUPPORT)
    return (area * 94 + mchar [1] - 0xA1);  /* added by xm.chen : for Orion board, MS hzk */
#else
    
    if (area < 9) {
        return (area * 94 + mchar [1] - 0xA1);
    }
    else if (area >= 15)
        return ((area - 6)* 94 + mchar [1] - 0xA1);
#endif

    return 0;
}

static int gb2312_0_is_this_charset (const unsigned char* charset)
{
    int i;
    char name [LEN_FONT_NAME + 1];

    for (i = 0; i < LEN_FONT_NAME + 1; i++) {
        if (charset [i] == '\0')
            break;
        name [i] = toupper (charset [i]);
    }
    name [i] = '\0';

    if (strstr (name, "GB") && strstr (name, "2312"))
        return 0;

    return 1;
}

static int gb2312_0_len_first_substr (const unsigned char* mstr, int mstrlen)
{
    unsigned char ch1;
    unsigned char ch2;
    int i, left;
    int sub_len = 0;

    left = mstrlen;
    for (i = 0; i < mstrlen; i += 2) {
        if (left < 2) return sub_len;

        ch1 = mstr [i];
        if (ch1 == '\0') return sub_len;

        ch2 = mstr [i + 1];
        IS_GB2312_CHAR (ch1, ch2)
            sub_len += 2;
        else
            return sub_len;

        left -= 2;
    }

    return sub_len;
}

static int gb2312_0_pos_first_char (const unsigned char* mstr, int mstrlen)
{
    unsigned char ch1;
    unsigned char ch2;
    int i, left;

    i = 0;
    left = mstrlen;
    while (left) {
        if (left < 2) return -1;

        ch1 = mstr [i];
        if (ch1 == '\0') return -1;

        ch2 = mstr [i + 1];
        IS_GB2312_CHAR (ch1, ch2)
            return i;

        i += 1;
        left -= 1;
    }

    return -1;
}

#ifdef _UNICODE_SUPPORT
static UChar32 gb2312_0_conv_to_uc32 (const unsigned char* mchar)
{
    return (UChar32)gbunicode_map [gb2312_0_char_offset (mchar)];
}

#include "mapunitogb.c"

extern const unsigned char* __mg_map_uc16_to_gb (unsigned short uc16);

static int gb2312_0_conv_from_uc32 (UChar32 wc, unsigned char* mchar)
{
#if defined(_TTF_SUPPORT)
    const unsigned char* got;

    if (wc > 0xFFFF)
        return 0;

    got = __mg_map_uc16_to_gb ((unsigned short)wc);

    if (got) {
        mchar [0] = got [0];
        mchar [1] = got [1];
        return 2;
    }

    return 0;

#else
    /** added by xm.chen **/
    int i;
    for(i = 0; i < (0xFF-0xA1)*(72 + 9)/*CharsetOps_big5.nr_chars*//*sizeof(big5_unicode_map)/sizeof(unsigned short)*/; i++)
    {
	if(gbunicode_map[i] == wc)
	    break;
    }
    return i;
    /** added by xm.chen over **/
#endif

}
#endif

static CHARSETOPS CharsetOps_gb2312_0 = {
    (0xFF-0xA1)*(72 + 9),
    2,
    FONT_CHARSET_GB2312_0,
    {'\xA1', '\xA1'},
    gb2312_0_len_first_char,
    gb2312_0_char_offset,
    mb_char_type,
    db_nr_chars_in_str,
    gb2312_0_is_this_charset,
    gb2312_0_len_first_substr,
    db_get_next_word,
    gb2312_0_pos_first_char,
#ifdef _UNICODE_SUPPORT
    gb2312_0_conv_to_uc32,
    gb2312_0_conv_from_uc32
#endif
};
#endif /* _GB_SUPPORT */

#ifdef _GBK_SUPPORT
/************************* GBK Specific Operations ************************/

#define IS_GBK_CHAR(ch1, ch2) \
    if (ch1 >= 0x81 && ch1 <= 0xFE && ch2 >= 0x40 && ch2 <= 0xFE && ch2 != 0x7F)

static int gbk_len_first_char (const unsigned char* mstr, int len)
{
    unsigned char ch1;
    unsigned char ch2;

    if (len < 2) return 0;

    ch1 = mstr [0];
    if (ch1 == '\0')
        return 0;

    ch2 = mstr [1];
    IS_GBK_CHAR(ch1, ch2)
        return 2;

    return 0;
}

static unsigned int gbk_char_offset (const unsigned char* mchar)
{
    if (mchar [1] > 0x7F)
        return ((mchar [0] - 0x81) * 190 + mchar [1] - 0x41);
    else
        return ((mchar [0] - 0x81) * 190 + mchar [1] - 0x40);
}

static int gbk_is_this_charset (const unsigned char* charset)
{
    int i;
    char name [LEN_FONT_NAME + 1];

    for (i = 0; i < LEN_FONT_NAME + 1; i++) {
        if (charset [i] == '\0')
            break;
        name [i] = toupper (charset [i]);
    }
    name [i] = '\0';

    if (strstr (name, "GBK"))
        return 0;

    if (strstr (name, "GB") && strstr (name, "2312"))
        return 0;

    return 1;
}

static int gbk_len_first_substr (const unsigned char* mstr, int mstrlen)
{
    unsigned char ch1;
    unsigned char ch2;
    int i, left;
    int sub_len = 0;

    left = mstrlen;
    for (i = 0; i < mstrlen; i += 2) {
        if (left < 2) return sub_len;

        ch1 = mstr [i];
        if (ch1 == '\0') return sub_len;

        ch2 = mstr [i + 1];
        IS_GBK_CHAR(ch1, ch2)
            sub_len += 2;
        else
            return sub_len;

        left -= 2;
    }

    return sub_len;
}

static int gbk_pos_first_char (const unsigned char* mstr, int mstrlen)
{
    unsigned char ch1;
    unsigned char ch2;
    int i, left;

    i = 0;
    left = mstrlen;
    while (left) {
        if (left < 2) return -1;

        ch1 = mstr [i];
        if (ch1 == '\0') return -1;

        ch2 = mstr [i + 1];
        IS_GBK_CHAR(ch1, ch2)
            return i;

        i += 1;
        left -= 1;
    }

    return -1;
}

#ifdef _UNICODE_SUPPORT
static UChar32 gbk_conv_to_uc32 (const unsigned char* mchar)
{
    return (UChar32)gbkunicode_map [(mchar[0] - 0x81) * 192 + mchar[1] - 0x40];
}

static int gbk_conv_from_uc32 (UChar32 wc, unsigned char* mchar)
{
    /* TODO */
    return 0;
}
#endif

static CHARSETOPS CharsetOps_gbk = {
    (0xFF - 0x80) * 190,
    2,
    FONT_CHARSET_GBK,
    {'\xA1', '\xA1'},
    gbk_len_first_char,
    gbk_char_offset,
    mb_char_type,
    db_nr_chars_in_str,
    gbk_is_this_charset,
    gbk_len_first_substr,
    db_get_next_word,
    gbk_pos_first_char,
#ifdef _UNICODE_SUPPORT
    gbk_conv_to_uc32,
    gbk_conv_from_uc32,
#endif
};

#endif /* _GBK_SUPPORT */

#ifdef _GB18030_SUPPORT

/************************* GBK Specific Operations ************************/
static int gb18030_0_len_first_char (const unsigned char* mstr, int len)
{
    unsigned char ch1;
    unsigned char ch2;
    unsigned char ch3;
    unsigned char ch4;

    if (len < 2) return 0;

    ch1 = mstr [0];
    if (ch1 == '\0')
        return 0;

    ch2 = mstr [1];
    if (ch1 >= 0x81 && ch1 <= 0xFE && ch2 >= 0x40 && ch2 <= 0xFE && ch2 != 0x7F)
        return 2;

    if (len < 4) return 0;

    ch3 = mstr [2];
    ch4 = mstr [3];
    if (ch2 >= 0x30 && ch2 <= 0x39 && ch4 >= 0x30 && ch4 >= 0x39
            && ch1 >= 0x81 && ch1 <= 0xFE && ch3 >= 0x81 && ch3 <= 0xFE)
        return 4;

    return 0;
}

static unsigned int gb18030_0_char_offset (const unsigned char* mchar)
{
    unsigned char ch1;
    unsigned char ch2;
    unsigned char ch3;
    unsigned char ch4;

    ch1 = mchar [0];
    ch2 = mchar [1];
    if (ch2 >= 0x40)
        return ((ch1 - 0x81) * 192 + (ch2 - 0x40));

    ch3 = mchar [2];
    ch4 = mchar [3];
    return ((126 * 192) + 
            ((ch1 - 0x81) * 12600 + (ch2 - 0x30) * 1260 + 
             (ch3 - 0x81) * 10 + (ch4 - 0x30)));
}

static int gb18030_0_is_this_charset (const unsigned char* charset)
{
    int i;
    char name [LEN_FONT_NAME + 1];

    for (i = 0; i < LEN_FONT_NAME + 1; i++) {
        if (charset [i] == '\0')
            break;
        name [i] = toupper (charset [i]);
    }
    name [i] = '\0';

    if (strstr (name, "GB") && strstr (name, "18030"))
        return 0;

    return 1;
}

static int gb18030_0_len_first_substr (const unsigned char* mstr, int mstrlen)
{
    unsigned char ch1, ch2, ch3, ch4;
    int i, left;
    int sub_len = 0;

    left = mstrlen;
    for (i = 0; i < mstrlen; i += 2) {
        if (left < 2) return sub_len;

        ch1 = mstr [i];
        if (ch1 == '\0') return sub_len;

        ch2 = mstr [i + 1];
        if (ch1 >= 0x81 && ch1 <= 0xFE && ch2 >= 0x40 && ch2 <= 0xFE && ch2 != 0x7F) {
            sub_len += 2;
            left -= 2;
        }
        else if (left >= 4) {
            
            ch3 = mstr [i + 2];
            ch4 = mstr [i + 3];
            if (ch2 >= 0x30 && ch2 <= 0x39 && ch4 >= 0x30 && ch4 >= 0x39
                    && ch1 >= 0x81 && ch1 <= 0xFE && ch3 >= 0x81 && ch3 <= 0xFE) {
                sub_len += 4;
                left -= 4;
                i += 2;
            }
        }
        else
            return sub_len;
    }

    return sub_len;
}

static int gb18030_0_pos_first_char (const unsigned char* mstr, int mstrlen)
{
    unsigned char ch1, ch2, ch3, ch4;
    int i, left;

    i = 0;
    left = mstrlen;
    while (left) {
        if (left < 2) return -1;

        ch1 = mstr [i];
        if (ch1 == '\0') return -1;

        ch2 = mstr [i + 1];
        if (ch1 >= 0x81 && ch1 <= 0xFE 
                && ch2 >= 0x40 && ch2 <= 0xFE && ch2 != 0x7F)
            return i;

        if (left < 4) return -1;

        ch3 = mstr [i + 2];
        ch4 = mstr [i + 3];
        if (ch2 >= 0x30 && ch2 <= 0x39 && ch4 >= 0x30 && ch4 >= 0x39
                && ch1 >= 0x81 && ch1 <= 0xFE && ch3 >= 0x81 && ch3 <= 0xFE)
            return i;

        i += 1;
        left -= 1;
    }

    return -1;
}

static int gb18030_0_nr_chars_in_str (const unsigned char* mstr, int mstrlen)
{
    int i, left;
    int n;

    assert ((mstrlen % 2) == 0);

    n = 0;
    i = 0;
    left = mstrlen;
    while (left) {
        if (left >= 2 && mstr [i + 1] >= 0x40) {
            left -= 2;
            i += 2;
        }
        else if (left >= 4) {
            left -= 4;
            i += 4;
        }
        else
            break;

        n++;
    }

    return n;

}

static const unsigned char* gb18030_0_get_next_word (const unsigned char* mstr,
                int mstrlen, WORDINFO* word_info)
{
    assert ((mstrlen % 2) == 0);

    word_info->len = 0;
    word_info->delimiter = '\0';
    word_info->nr_delimiters = 0;

    if (mstrlen < 2) return NULL;

    if (mstrlen >= 2 && mstr [1] >= 0x40) {
        word_info->len = 2;
    }
    else if (mstrlen >= 4) {
        word_info->len = 4;
    }

    return mstr + word_info->len;
}

#ifdef _UNICODE_SUPPORT
static UChar32 gb18030_0_conv_to_uc32 (const unsigned char* mchar)
{
    /* FIXME */
    unsigned int index = gb18030_0_char_offset (mchar);

    if (index > 63611)
        return '?';
    else
        return (UChar32)gb18030_0_unicode_map [index];
}

static int gb18030_0_conv_from_uc32 (UChar32 wc, unsigned char* mchar)
{
    /* TODO */
    return 0;
}
#endif

static CHARSETOPS CharsetOps_gb18030_0 = {
    1587600 + 23940,
    4,
    FONT_CHARSET_GB18030_0,
    {'\xA1', '\xA1'},
    gb18030_0_len_first_char,
    gb18030_0_char_offset,
    mb_char_type,
    gb18030_0_nr_chars_in_str,
    gb18030_0_is_this_charset,
    gb18030_0_len_first_substr,
    gb18030_0_get_next_word,
    gb18030_0_pos_first_char,
#ifdef _UNICODE_SUPPORT
    gb18030_0_conv_to_uc32,
    gb18030_0_conv_from_uc32
#endif
};

#endif /* _GB18030_SUPPORT */

#ifdef _BIG5_SUPPORT

/************************** BIG5 Specific Operations ************************/
static int big5_len_first_char (const unsigned char* mstr, int len)
{
    unsigned char ch1;
    unsigned char ch2;

    if (len < 2) return 0;

    ch1 = mstr [0];
    if (ch1 == '\0')
        return 0;

    ch2 = mstr [1];
    if (ch1 >= 0xA1 && ch1 <= 0xFE && 
            ((ch2 >=0x40 && ch2 <= 0x7E) || (ch2 >= 0xA1 && ch2 <= 0xFE)))
        return 2;

    return 0;
}

static unsigned int big5_char_offset (const unsigned char* mchar)
{
#if defined(_TTF_SUPPORT)
/* MiniGUI self-defined Big font */
    if (mchar [1] & 0x80)
        return (mchar [0] - 0xA1) * 94 + mchar [1] - 0xA1;
    else
        return 94 * 94 + (mchar [0] - 0xa1) * 63 + (mchar [1] - 0x40);

#else
/* standard Big5 font */
/*--------------------------------------------------
*     if (mchar [1] & 0x80)
*         return (mchar [0] - 0xA1) * 157 + mchar [1] - 0x62;
*     else
*         return (mchar [0] - 0xA1) * 157 + (mchar [1] - 0x40);
*--------------------------------------------------*/

/* HongKong Complimentary Big5 font */
    if (mchar [1] & 0x80)
        return (mchar [0] - 0x81) * 157 + mchar [1] - 0x62;
    else
        return (mchar [0] - 0x81) * 157 + (mchar [1] - 0x40);
#endif
}

static int big5_is_this_charset (const unsigned char* charset)
{
    int i;
    char name [LEN_FONT_NAME + 1];

    for (i = 0; i < LEN_FONT_NAME + 1; i++) {
        if (charset [i] == '\0')
            break;
        name [i] = toupper (charset [i]);
    }
    name [i] = '\0';

    if (strstr (name, "BIG5"))
        return 0;

    return 1;
}

static int big5_len_first_substr (const unsigned char* mstr, int mstrlen)
{
    unsigned char ch1;
    unsigned char ch2;
    int i, left;
    int sub_len = 0;

    left = mstrlen;
    for (i = 0; i < mstrlen; i += 2) {
        if (left < 2) return sub_len;

        ch1 = mstr [i];
        if (ch1 == '\0') return sub_len;

        ch2 = mstr [i + 1];
        if (ch1 >= 0xA1 && ch1 <= 0xFE && 
                ((ch2 >=0x40 && ch2 <= 0x7E) || (ch2 >= 0xA1 && ch2 <= 0xFE)))
            sub_len += 2;
        else
            return sub_len;

        left -= 2;
    }

    return sub_len;
}

static int big5_pos_first_char (const unsigned char* mstr, int mstrlen)
{
    unsigned char ch1;
    unsigned char ch2;
    int i, left;

    i = 0;
    left = mstrlen;
    while (left) {
        if (left < 2) return -1;

        ch1 = mstr [i];
        if (ch1 == '\0') return -1;

        ch2 = mstr [i + 1];
        if (ch1 >= 0xA1 && ch1 <= 0xFE && 
                ((ch2 >=0x40 && ch2 <= 0x7E) || (ch2 >= 0xA1 && ch2 <= 0xFE)))
            return i;

        i += 1;
        left -= 1;
    }

    return -1;
}

#ifdef _UNICODE_SUPPORT
static UChar32 big5_conv_to_uc32 (const unsigned char* mchar)
{
    unsigned short ucs_code = big5_unicode_map [big5_char_offset (mchar)];

    if (ucs_code == 0)
        return '?';

    return ucs_code;
}

static int big5_conv_from_uc32 (UChar32 wc, unsigned char* mchar)
{
    /* TODO */
    int i;
    for(i = 0; i < 14758/*CharsetOps_big5.nr_chars*//*sizeof(big5_unicode_map)/sizeof(unsigned short)*/; i++)
    {
	if(big5_unicode_map[i] == wc)
	    break;
    }
    return i;
}
#endif

static CHARSETOPS CharsetOps_big5 = {
    /*13973,*/14758,
    2,
    FONT_CHARSET_BIG5,
    {'\xA1', '\x40'},
    big5_len_first_char,
    big5_char_offset,
    mb_char_type,
    db_nr_chars_in_str,
    big5_is_this_charset,
    big5_len_first_substr,
    db_get_next_word,
    big5_pos_first_char,
#ifdef _UNICODE_SUPPORT
    big5_conv_to_uc32,
    big5_conv_from_uc32,
#endif
};

#endif /* _BIG5_SUPPORT */

#ifdef _EUCKR_SUPPORT

/************************* EUCKR Specific Operations ************************/
static int ksc5601_0_len_first_char (const unsigned char* mstr, int len)
{
    unsigned char ch1;
    unsigned char ch2;

    if (len < 2) return 0;

    ch1 = mstr [0];
    if (ch1 == '\0')
        return 0;

    ch2 = mstr [1];
    if (ch1 >= 0xA1 && ch1 <= 0xFE && ch2 >= 0xA1 && ch2 <= 0xFE)
        return 2;

    return 0;
}

static unsigned int ksc5601_0_char_offset (const unsigned char* mchar)
{
#if 1
    return ((mchar [0] - 0xA1) * 94 + mchar [1] - 0xA1);
#else
    if(mchar [0] > 0xAD)
        return ((mchar [0] - 0xA4) * 94 + mchar [1] - 0xA1 - 0x8E);
    else
        return ((mchar [0] - 0xA1) * 94 + mchar [1] - 0xA1 - 0x8E);
#endif
}

static int ksc5601_0_is_this_charset (const unsigned char* charset)
{
    int i;
    char name [LEN_FONT_NAME + 1];

    for (i = 0; i < LEN_FONT_NAME + 1; i++) {
        if (charset [i] == '\0')
            break;
        name [i] = toupper (charset [i]);
    }
    name [i] = '\0';

    if ((strstr (name, "KSC5601") && strstr (name, "-0")) || 
            (strstr (name, "EUC") && strstr (name, "KR")))
        return 0;

    return 1;
}

static int ksc5601_0_len_first_substr (const unsigned char* mstr, int mstrlen)
{
    unsigned char ch1;
    unsigned char ch2;
    int i, left;
    int sub_len = 0;

    left = mstrlen;
    for (i = 0; i < mstrlen; i += 2) {
        if (left < 2) return sub_len;

        ch1 = mstr [i];
        if (ch1 == '\0') return sub_len;

        ch2 = mstr [i + 1];
        if (ch1 >= 0xA1 && ch1 <= 0xFE && ch2 >= 0xA1 && ch2 <= 0xFE)
            sub_len += 2;
        else
            return sub_len;

        left -= 2;
    }

    return sub_len;
}

static int ksc5601_0_pos_first_char (const unsigned char* mstr, int mstrlen)
{
    unsigned char ch1;
    unsigned char ch2;
    int i, left;

    i = 0;
    left = mstrlen;
    while (left) {
        if (left < 2) return -1;

        ch1 = mstr [i];
        if (ch1 == '\0') return -1;

        ch2 = mstr [i + 1];
        if (ch1 >= 0xA1 && ch1 <= 0xFE && ch2 >= 0xA1 && ch2 <= 0xFE)
            return i;

        i += 1;
        left -= 1;
    }

    return -1;
}

#ifdef _UNICODE_SUPPORT
static UChar32 ksc5601_0_conv_to_uc32 (const unsigned char* mchar)
{
    unsigned short ucs_code = 
            ksc5601_0_unicode_map [ksc5601_0_char_offset (mchar)];

    if (ucs_code == 0)
        return '?';

    return ucs_code;
}

static int ksc5601_0_conv_from_uc32 (UChar32 wc, unsigned char* mchar)
{
    /* TODO */
    return 0;
}
#endif

static CHARSETOPS CharsetOps_ksc5601_0 = {
    8836,
    2,
    FONT_CHARSET_EUCKR,
    {'\xA1', '\xA1'},
    ksc5601_0_len_first_char,
    ksc5601_0_char_offset,
    mb_char_type,
    db_nr_chars_in_str,
    ksc5601_0_is_this_charset,
    ksc5601_0_len_first_substr,
    db_get_next_word,
    ksc5601_0_pos_first_char,
#ifdef _UNICODE_SUPPORT
    ksc5601_0_conv_to_uc32,
    ksc5601_0_conv_from_uc32,
#endif
};
/************************* End of EUCKR *************************************/

#endif  /* _EUCKR_SUPPORT */

#ifdef _EUCJP_SUPPORT

/************************* EUCJP Specific Operations ************************/
static int jisx0201_0_len_first_char (const unsigned char* mstr, int len)
{
    unsigned char ch1;
    unsigned char ch2;

    ch1 = mstr [0];
    if (ch1 == '\0')
        return 0;

    ch2 = mstr [1];
    if (ch1 == 0x8E && ch2 >= 0xA1 && ch2 <= 0xDF)
        return 2;
    else
        return 1;

    return 0;
}

static unsigned int jisx0201_0_char_offset (const unsigned char* mchar)
{
    if (mchar [0] == 0x8E)
        return mchar [1];
    else
        return mchar [0];
}

static int jisx0201_0_nr_chars_in_str (const unsigned char* mstr, int mstrlen)
{
    unsigned char ch1;
    unsigned char ch2;
    int left;
    int nr_chars = 0;

    left = mstrlen;
    while (left > 0) {

        ch1 = mstr [0];
        if (ch1 == '\0') return nr_chars;

        ch2 = mstr [1];
        if (ch1 == 0x8E && ch2 >= 0xA1 && ch2 <= 0xDF) {
            nr_chars ++;
            left -= 2;
            mstr += 2;
        }
        else {
            nr_chars ++;
            left -= 1;
            mstr += 1;
        }
    }

    return nr_chars;
}

static int jisx0201_0_is_this_charset (const unsigned char* charset)
{
    int i;
    char name [LEN_FONT_NAME + 1];

    for (i = 0; i < LEN_FONT_NAME + 1; i++) {
        if (charset [i] == '\0')
            break;
        name [i] = toupper (charset [i]);
    }
    name [i] = '\0';

    if (strstr (name, "JISX0201") && strstr (name, "-0"))
        return 0;

    return 1;
}


#ifdef _UNICODE_SUPPORT
static UChar32 jisx0201_0_conv_to_uc32 (const unsigned char* mchar)
{
    unsigned char ch1;
    unsigned char ch2;

    ch1 = mchar [0]; ch2 = mchar [1];
    if (ch1 == 0x8E && ch2 >= 0xA1 && ch2 <= 0xDF)
        return 0xFF61 + ch2 - 0xA1;
    else if (ch1 == 0x5C)
        return 0x00A5;
    else if (ch1 == 0x80)
        return 0x005C;
    return ch1;
}

static int jisx0201_0_conv_from_uc32 (UChar32 wc, unsigned char* mchar)
{
    /* TODO */
    return 0;
}
#endif

static CHARSETOPS CharsetOps_jisx0201_0 = {
    190,
    1,
    FONT_CHARSET_JISX0201_0,
    {' '},
    jisx0201_0_len_first_char,
    jisx0201_0_char_offset,
    sb_char_type,
    jisx0201_0_nr_chars_in_str,
    jisx0201_0_is_this_charset,
    sb_len_first_substr,
    sb_get_next_word,
    sb_pos_first_char,
#ifdef _UNICODE_SUPPORT
    jisx0201_0_conv_to_uc32,
    jisx0201_0_conv_from_uc32,
#endif
};

static int jisx0208_0_len_first_char (const unsigned char* mstr, int len)
{
    unsigned char ch1;
    unsigned char ch2;

    if (len < 2) return 0;

    ch1 = mstr [0];
    if (ch1 == '\0')
        return 0;

    ch2 = mstr [1];
    if (ch1 >= 0xA1 && ch1 <= 0xFE && ch2 >= 0xA1 && ch2 <= 0xFE)
        return 2;

    return 0;
}

static unsigned int jisx0208_0_char_offset (const unsigned char* mchar)
{
#if 1
    return ((mchar [0] - 0xA1) * 94 + mchar [1] - 0xA1);
#else
    if(mchar [0] > 0xAA)
        return ((mchar [0] - 0xA6 - 0x02) * 94 + mchar [1] - 0xC1 - 0xC4);
    else
        return ((mchar [0] - 0xA1 - 0x02) * 83 + mchar [1] - 0xA1 + 0x7E);
#endif
}

static int jisx0208_0_is_this_charset (const unsigned char* charset)
{
    int i;
    char name [LEN_FONT_NAME + 1];

    for (i = 0; i < LEN_FONT_NAME + 1; i++) {
        if (charset [i] == '\0')
            break;
        name [i] = toupper (charset [i]);
    }
    name [i] = '\0';

    if ((strstr (name, "JISX0208") && strstr (name, "-0")) ||
            (strstr (name, "EUC") && strstr (name, "JP")))
        return 0;

    return 1;
}

static int jisx0208_0_len_first_substr (const unsigned char* mstr, int mstrlen)
{
    unsigned char ch1;
    unsigned char ch2;
    int i, left;
    int sub_len = 0;

    left = mstrlen;
    for (i = 0; i < mstrlen; i += 2) {
        if (left < 2) return sub_len;

        ch1 = mstr [i];
        if (ch1 == '\0') return sub_len;

        ch2 = mstr [i + 1];
        if (ch1 >= 0xA1 && ch1 <= 0xFE && ch2 >= 0xA1 && ch2 <= 0xFE)
            sub_len += 2;
        else
            return sub_len;

        left -= 2;
    }

    return sub_len;
}

static int jisx0208_0_pos_first_char (const unsigned char* mstr, int mstrlen)
{
    unsigned char ch1;
    unsigned char ch2;
    int i, left;

    i = 0;
    left = mstrlen;
    while (left) {
        if (left < 2) return -1;

        ch1 = mstr [i];
        if (ch1 == '\0') return -1;

        ch2 = mstr [i + 1];
        if (ch1 >= 0xA1 && ch1 <= 0xFE && ch2 >= 0xA1 && ch2 <= 0xFE)
            return i;

        i += 1;
        left -= 1;
    }

    return -1;
}

#ifdef _UNICODE_SUPPORT
static UChar32 jisx0208_0_conv_to_uc32 (const unsigned char* mchar)
{
    unsigned short ucs_code;

    ucs_code = jisx0208_0_unicode_map [jisx0208_0_char_offset (mchar)];

    if (ucs_code == 0)
        return '?';

    return ucs_code;
}

static int jisx0208_0_conv_from_uc32 (UChar32 wc, unsigned char* mchar)
{
    /* TODO */
    return 0;
}
#endif

static CHARSETOPS CharsetOps_jisx0208_0 = {
    8836,
    2,
    FONT_CHARSET_JISX0208_0,
    {'\xA1', '\xA1'},
    jisx0208_0_len_first_char,
    jisx0208_0_char_offset,
    mb_char_type,
    db_nr_chars_in_str,
    jisx0208_0_is_this_charset,
    jisx0208_0_len_first_substr,
    db_get_next_word,
    jisx0208_0_pos_first_char,
#ifdef _UNICODE_SUPPORT
    jisx0208_0_conv_to_uc32,
    jisx0208_0_conv_from_uc32,
#endif
};
/************************* End of EUCJP *************************************/

#endif /* _EUCJP_SUPPORT */

#ifdef _SHIFTJIS_SUPPORT

/************************* SHIFTJIS Specific Operations ************************/
static int jisx0201_1_is_this_charset (const unsigned char* charset)
{
    int i;
    char name [LEN_FONT_NAME + 1];

    for (i = 0; i < LEN_FONT_NAME + 1; i++) {
        if (charset [i] == '\0')
            break;
        name [i] = toupper (charset [i]);
    }
    name [i] = '\0';

    if (strstr (name, "JISX0201") && strstr (name, "-1"))
        return 0;

    return 1;
}


#ifdef _UNICODE_SUPPORT
static UChar32 jisx0201_1_conv_to_uc32 (const unsigned char* mchar)
{
    unsigned char ch1 = mchar [0];

    if (ch1 >= 0xA1 && ch1 <= 0xDF)
        return 0xFF61 + ch1 - 0xA1;
    else if (ch1 == 0x5C)
        return 0x00A5;
    else if (ch1 == 0x80)
        return 0x005C;

    return ch1;
}

static int jisx0201_1_conv_from_uc32 (UChar32 wc, unsigned char* mchar)
{
    if (wc < 0xA1) {
        *mchar = (unsigned char)wc;
        return 1;
    }

    switch (wc) {
        case 0x00A5:
            *mchar = 0x5C;
            return 1;
        case 0x005C:
            *mchar = 0x80;
            return 1;
    }

    if (wc >= 0xFF61 && wc <= (0xFF61 + 0xDF - 0xA1)) {
        *mchar = (unsigned char) (wc - 0xFF61 + 0xA1);
        return 1;
    }

    return 0;
}
#endif

static CHARSETOPS CharsetOps_jisx0201_1 = {
    190,
    1,
    FONT_CHARSET_JISX0201_1,
    {' '},
    sb_len_first_char,
    sb_char_offset,
    sb_char_type,
    sb_nr_chars_in_str,
    jisx0201_1_is_this_charset,
    sb_len_first_substr,
    sb_get_next_word,
    sb_pos_first_char,
#ifdef _UNICODE_SUPPORT
    jisx0201_1_conv_to_uc32,
    jisx0201_1_conv_from_uc32,
#endif
};

static int jisx0208_1_len_first_char (const unsigned char* mstr, int len)
{
    unsigned char ch1;
    unsigned char ch2;

    if (len < 2) return 0;

    ch1 = mstr [0];
    if (ch1 == '\0')
        return 0;

    ch2 = mstr [1];
    if (((ch1 >= 0x81 && ch1 <= 0x9F) || (ch1 >= 0xE0 && ch1 <= 0xEF)) && 
            ((ch2 >= 0x40 && ch2 <= 0x7E) || (ch2 >= 0x80 && ch2 <= 0xFC)))
        return 2;

    return 0;
}

static unsigned int jisx0208_1_char_offset (const unsigned char* mchar)
{
    unsigned char ch1 = mchar [0];
    unsigned char ch2 = mchar [1];

#if 0
    if (ch1 >= 0x81 && ch1 <= 0x9F) {
        if (ch2 >= 0x40 && ch2 <= 0x7E) {
            return ((ch1 - 0x81) * 0x3F + ch2 - 0x40);
        }
        else {
            return (ch1 - 0x81) * 0x7D + ch2 - 0x80 + (0x1F * 0x3F);
        }
    }
    else {
        if (ch2 >= 0x40 && ch2 <= 0x7E) {
            return ((ch1 - 0xE0) * 0x3F + ch2 - 0x40) + 0x1F * (0x3F + 0x7D);
        }
        else {
            return ((ch1 - 0xE0) * 0x7D + ch2 - 0x80) + 0x1F * (0x3F + 0x7D) + 0x10 * 0x3F;
        }
    }
#else
    int adjust = ch2 < 159;
    int rowOffset = ch1 < 160 ? 112 : 176;
    int cellOffset = adjust ? (ch2 > 127 ? 32 : 31) : 126;

    ch1 = ((ch1 - rowOffset) << 1) - adjust;
    ch2 -= cellOffset;

    ch1 += 128;
    ch2 += 128;

    return ((ch1 - 0xA1) * 94 + ch2 - 0xA1);
#endif
}

static int jisx0208_1_is_this_charset (const unsigned char* charset)
{
    int i;
    char name [LEN_FONT_NAME + 1];

    for (i = 0; i < LEN_FONT_NAME + 1; i++) {
        if (charset [i] == '\0')
            break;
        name [i] = toupper (charset [i]);
    }
    name [i] = '\0';

    if ((strstr (name, "JISX0208") && strstr (name, "-1")) ||
            (strstr (name, "SHIFT") && strstr (name, "JIS")))
        return 0;

    return 1;
}

static int jisx0208_1_len_first_substr (const unsigned char* mstr, int mstrlen)
{
    unsigned char ch1;
    unsigned char ch2;
    int i, left;
    int sub_len = 0;

    left = mstrlen;
    for (i = 0; i < mstrlen; i += 2) {
        if (left < 2) return sub_len;

        ch1 = mstr [i];
        if (ch1 == '\0') return sub_len;

        ch2 = mstr [i + 1];
        if (((ch1 >= 0x81 && ch1 <= 0x9F) || (ch1 >= 0xE0 && ch1 <= 0xEF)) && 
                ((ch2 >= 0x40 && ch2 <= 0x7E) || (ch2 >= 0x80 && ch2 <= 0xFC)))
            sub_len += 2;
        else
            return sub_len;

        left -= 2;
    }

    return sub_len;
}

static int jisx0208_1_pos_first_char (const unsigned char* mstr, int mstrlen)
{
    unsigned char ch1;
    unsigned char ch2;
    int i, left;

    i = 0;
    left = mstrlen;
    while (left) {
        if (left < 2) return -1;

        ch1 = mstr [i];
        if (ch1 == '\0') return -1;

        ch2 = mstr [i + 1];
        if (((ch1 >= 0x81 && ch1 <= 0x9F) || (ch1 >= 0xE0 && ch1 <= 0xEF)) && 
                ((ch2 >= 0x40 && ch2 <= 0x7E) || (ch2 >= 0x80 && ch2 <= 0xFC)))
            return i;

        i += 1;
        left -= 1;
    }

    return -1;
}

#ifdef _UNICODE_SUPPORT
static UCha32 jisx0208_1_conv_to_uc32 (const unsigned char* mchar)
{
    unsigned short ucs_code;

    ucs_code  = jisx0208_1_unicode_map [jisx0208_1_char_offset (mchar)];

    if (ucs_code == 0)
        return '?';

    return ucs_code;
}

static int jisx0208_1_conv_from_uc32 (UChar32 wc, unsigned char *mchar)
{
    /* TODO */
    return 0;
}
#endif

static CHARSETOPS CharsetOps_jisx0208_1 = {
    8836,
    2,
    FONT_CHARSET_JISX0208_1,
    {'\xA1', '\xA1'},
    jisx0208_1_len_first_char,
    jisx0208_1_char_offset,
    mb_char_type,
    db_nr_chars_in_str,
    jisx0208_1_is_this_charset,
    jisx0208_1_len_first_substr,
    db_get_next_word,
    jisx0208_1_pos_first_char,
#ifdef _UNICODE_SUPPORT
    jisx0208_1_conv_to_uc32,
    jisx0208_1_conv_from_uc32
#endif
};
/************************* End of SHIFTJIS ************************************/

#endif /* _SHIFTJIS_SUPPORT */

#ifdef _UNICODE_SUPPORT

/************************* UTF-8 Specific Operations ************************/
static int utf8_len_first_char (const unsigned char* mstr, int len)
{
    int t, c = *((unsigned char *)(mstr++));
    int n = 1, ch_len = 0;

    if (c & 0x80) {
        while (c & (0x80 >> n))
            n++;

        if (n > len)
            return 0;

        ch_len = n;
        while (--n > 0) {
            t = *((unsigned char *)(mstr++));

            if ((!(t & 0x80)) || (t & 0x40))
                return 0;
        }
    }

    return ch_len;
}

static UChar32 utf8_conv_to_uc32 (const unsigned char* mchar)
{
    UChar32 wc = *((unsigned char *)(mchar++));
    int n, t;

    if (wc & 0x80) {
        n = 1;
        while (wc & (0x80 >> n))
            n++;

        wc &= (1 << (8-n)) - 1;
        while (--n > 0) {
            t = *((unsigned char *)(mchar++));

            wc = (wc << 6) | (t & 0x3F);
        }
    }

    return wc;
}

static unsigned int utf8_char_offset (const unsigned char* mchar)
{
    return utf8_conv_to_uc32 (mchar);
}

static unsigned int utf8_char_type (const unsigned char* mchar)
{
    unsigned int ch_type;

    if (mchar [0] < 0x80) {
        ch_type = sb_char_type (mchar);
    }
    else {
        /* TODO: get the subtype of the char */
        ch_type = MCHAR_TYPE_GENERIC;
    }

    return ch_type;
}

static int utf8_nr_chars_in_str (const unsigned char* mstr, int mstrlen)
{
    int charlen;
    int n = 0;
    int left = mstrlen;

    while (left >= 0) {
        charlen = utf8_len_first_char (mstr, left);
        if (charlen > 0)
            n ++;

        left -= charlen;
        mstr += charlen;
    }

    return n;
}

static int utf8_is_this_charset (const unsigned char* charset)
{
    int i;
    char name [LEN_FONT_NAME + 1];

    for (i = 0; i < LEN_FONT_NAME + 1; i++) {
        if (charset [i] == '\0')
            break;
        name [i] = toupper (charset [i]);
    }
    name [i] = '\0';

    if (strstr (name, "UTF") && strstr (name, "8"))
        return 0;

    return 1;
}

static int utf8_len_first_substr (const unsigned char* mstr, int mstrlen)
{
    int ch_len;
    int sub_len = 0;

    while (mstrlen > 0) {
        ch_len = utf8_len_first_char (mstr, mstrlen);

        if (ch_len == 0)
            break;

        sub_len += ch_len;
        mstrlen -= ch_len;
        mstr += ch_len;
    }

    return sub_len;
}

static const unsigned char* utf8_get_next_word (const unsigned char* mstr,
                int mstrlen, WORDINFO* word_info)
{
    const unsigned char* mchar = mstr;
    int ch_len;
    UChar32 wc;

    word_info->len = 0;
    word_info->delimiter = '\0';
    word_info->nr_delimiters = 0;

    if (mstrlen == 0) return NULL;

    while (mstrlen > 0) {
        ch_len = utf8_len_first_char (mchar, mstrlen);

        if (ch_len == 0)
            break;

        wc = utf8_conv_to_uc32 (mchar);
        if (wc > 0x0FFF) {
            if (word_info->len)
                return mstr + word_info->len + word_info->nr_delimiters;
            else { /* Treate the first char as a word */
                word_info->len = ch_len;
                return mstr + word_info->len + word_info->nr_delimiters;
            }
        }

        switch (mchar[0]) {
        case ' ':
        case '\t':
        case '\n':
        case '\r':
            if (word_info->delimiter == '\0') {
                word_info->delimiter = mchar[0];
                word_info->nr_delimiters ++;
            }
            else if (word_info->delimiter == mchar[0])
                word_info->nr_delimiters ++;
            else
                return mstr + word_info->len + word_info->nr_delimiters;

            break;

        default:
            if (word_info->delimiter != '\0')
                return mstr + word_info->len + word_info->nr_delimiters;

            word_info->len += ch_len;
            break;
        }

        mchar += ch_len;
        mstrlen -= ch_len;
    }

    return mstr + word_info->len + word_info->nr_delimiters;
}

static int utf8_pos_first_char (const unsigned char* mstr, int mstrlen)
{
    int ch_len;
    int pos = 0;

    while (mstrlen > 0) {
        ch_len = utf8_len_first_char (mstr, mstrlen);

#if defined(_UNICODE_SUPPORT) && !defined(_TTF_SUPPORT)
        if (ch_len >= 0)		//changed by xm.chen ; original :if(ch_len > 0)
#else
	if (ch_len >= 0)
#endif
            return pos;

        pos += ch_len;
        mstrlen -= ch_len;
        mstr += ch_len;
    }

    return -1;
}

static int utf8_conv_from_uc32 (UChar32 wc, unsigned char* mchar)
{
    int first, len;

    if (wc < 0x80) {
        first = 0;
        len = 1;
    }
    else if (wc < 0x800) {
        first = 0xC0;
        len = 2;
    }
    else if (wc < 0x10000) {
        first = 0xE0;
        len = 3;
    }
    else if (wc < 0x200000) {
        first = 0xF0;
        len = 4;
    }
    else if (wc < 0x400000) {
        first = 0xF8;
        len = 5;
    }
    else {
        first = 0xFC;
        len = 6;
    }

    switch (len) {
        case 6:
            mchar [5] = (wc & 0x3f) | 0x80; wc >>= 6; /* Fall through */
        case 5:
            mchar [4] = (wc & 0x3f) | 0x80; wc >>= 6; /* Fall through */
        case 4:
            mchar [3] = (wc & 0x3f) | 0x80; wc >>= 6; /* Fall through */
        case 3:
            mchar [2] = (wc & 0x3f) | 0x80; wc >>= 6; /* Fall through */
        case 2:
            mchar [1] = (wc & 0x3f) | 0x80; wc >>= 6; /* Fall through */
        case 1:
            mchar [0] = wc | first;
    }

    return len;
}

static CHARSETOPS CharsetOps_utf8 = {
    0x7FFFFFFF,
    6,
    FONT_CHARSET_UTF8,
    {' '},
    utf8_len_first_char,
    utf8_char_offset,
    utf8_char_type,
    utf8_nr_chars_in_str,
    utf8_is_this_charset,
    utf8_len_first_substr,
    utf8_get_next_word,
    utf8_pos_first_char,
    utf8_conv_to_uc32,
    utf8_conv_from_uc32,
};

/************************* UTF-16LE Specific Operations ***********************/
static int utf16le_len_first_char (const unsigned char* mstr, int len)
{
    UChar16 w1, w2;

    if (len < 2)
        return 0;

    w1 = MAKEWORD (mstr[0], mstr[1]);

    /* added by xm.chen */
#if defined(_UNICODE_SUPPORT) && !defined(_TTF_SUPPORT)
    if(w1 < 0x80)
	return -1;	//sbc font (ascii char)
#endif
    /* added by xm.chen over */

    if (w1 < 0xD800 || w1 > 0xDFFF)
        return 2;

    if (w1 >= 0xD800 && w1 <= 0xDBFF) {
        if (len < 4)
            return 0;
        w2 = MAKEWORD (mstr[2], mstr[3]);
        if (w2 < 0xDC00 || w2 > 0xDFFF)
            return 0;
    }

    return 4;
}

static UChar32 utf16le_conv_to_uc32 (const unsigned char* mchar)
{
    UChar16 w1, w2;
    UChar32 wc;

    w1 = MAKEWORD (mchar[0], mchar[1]);

    if (w1 < 0xD800 || w1 > 0xDFFF)
        return w1;

    w2 = MAKEWORD (mchar[2], mchar[3]);

    wc = w1;
    wc <<= 10;
    wc |= (w2 & 0x03FF);
    wc += 0x10000;

    return wc;
}

static unsigned int utf16le_char_offset (const unsigned char* mchar)
{
    return utf16le_conv_to_uc32 (mchar);
}

static unsigned int utf16le_char_type (const unsigned char* mchar)
{
    unsigned int ch_type;
    unsigned char c1 = mchar [0], c2 = mchar [1];

    if (c1 == 0xFE && c2 == 0xFF) {
        ch_type = MCHAR_TYPE_ZEROWIDTH;
    }
    else if (c2 == 0) {
        ch_type = sb_char_type (&c1);
    }
    else {
        /* TODO: get the subtype of the char */
        ch_type = MCHAR_TYPE_GENERIC;
    }

    return ch_type;
}

static int utf16le_nr_chars_in_str (const unsigned char* mstr, int mstrlen)
{
    int charlen;
    int n = 0;
    int left = mstrlen;

    while (left >= 0) {
        charlen = utf16le_len_first_char (mstr, left);
        if (charlen > 0)
            n ++;

        left -= charlen;
        mstr += charlen;
    }

    return n;
}

static int utf16le_is_this_charset (const unsigned char* charset)
{
    int i;
    char name [LEN_FONT_NAME + 1];

    for (i = 0; i < LEN_FONT_NAME + 1; i++) {
        if (charset [i] == '\0')
            break;
        name [i] = toupper (charset [i]);
    }
    name [i] = '\0';

    if (strstr (name, "UTF") && strstr (name, "16LE"))
        return 0;

    return 1;
}

static int utf16le_len_first_substr (const unsigned char* mstr, int mstrlen)
{
    int ch_len;
    int sub_len = 0;

    while (mstrlen > 0) {
        ch_len = utf16le_len_first_char (mstr, mstrlen);

        if (ch_len == 0)
            break;

        sub_len += ch_len;
        mstrlen -= ch_len;
        mstr += ch_len;
    }

    return sub_len;
}

static const unsigned char* utf16le_get_next_word (const unsigned char* mstr,
                int mstrlen, WORDINFO* word_info)
{
    const unsigned char* mchar = mstr;
    UChar32 wc;
    int ch_len;

    word_info->len = 0;
    word_info->delimiter = '\0';
    word_info->nr_delimiters = 0;

    if (mstrlen == 0) return NULL;

    while (mstrlen > 0) {
        ch_len = utf16le_len_first_char (mchar, mstrlen);

        if (ch_len == 0)
            break;

        wc = utf16le_conv_to_uc32 (mchar);
        if (wc > 0x0FFF) {
            if (word_info->len)
                return mstr + word_info->len + word_info->nr_delimiters;
            else { /* Treate the first char as a word */
                word_info->len = ch_len;
                return mstr + word_info->len + word_info->nr_delimiters;
            }
        }

        switch (wc) {
        case ' ':
        case '\t':
        case '\n':
        case '\r':
            if (word_info->delimiter == '\0') {
                word_info->delimiter = (unsigned char)wc;
                word_info->nr_delimiters += ch_len;
            }
            else if (word_info->delimiter == (unsigned char)wc)
                word_info->nr_delimiters += ch_len;
            else
                return mstr + word_info->len + word_info->nr_delimiters;
            break;

        default:
            if (word_info->delimiter != '\0')
                return mstr + word_info->len + word_info->nr_delimiters;

            word_info->len += ch_len;
            break;
        }

        mstrlen -= ch_len;
        mchar += ch_len;
    }

    return mstr + word_info->len + word_info->nr_delimiters;
}

static int utf16le_pos_first_char (const unsigned char* mstr, int mstrlen)
{
    int ch_len;
    int pos = 0;

    while (mstrlen > 0) {
        ch_len = utf16le_len_first_char (mstr, mstrlen);

        if (ch_len > 0)
            return pos;

        pos += ch_len;
        mstrlen -= ch_len;
        mstr += ch_len;
    }

    return -1;
}

static int utf16le_conv_from_uc32 (UChar32 wc, unsigned char* mchar)
{
    UChar16 w1, w2;

    if (wc > 0x10FFFF) {
        return 0;
    }

    if (wc < 0x10000) {
        mchar [0] = LOBYTE (wc);
        mchar [1] = HIBYTE (wc);
        return 2;
    }

    wc -= 0x10000;
    w1 = 0xD800;
    w2 = 0xDC00;

    w1 |= (wc >> 10);
    w2 |= (wc & 0x03FF);

    mchar [0] = LOBYTE (w1);
    mchar [1] = HIBYTE (w1);
    mchar [2] = LOBYTE (w2);
    mchar [3] = HIBYTE (w2);
    return 4;
}

static CHARSETOPS CharsetOps_utf16le = {
    0x7FFFFFFF,
    4,
    FONT_CHARSET_UTF16LE,
    {'\xA1', '\xA1'},
    utf16le_len_first_char,
    utf16le_char_offset,
    utf16le_char_type,
    utf16le_nr_chars_in_str,
    utf16le_is_this_charset,
    utf16le_len_first_substr,
    utf16le_get_next_word,
    utf16le_pos_first_char,
    utf16le_conv_to_uc32,
    utf16le_conv_from_uc32
};

/************************* UTF-16BE Specific Operations ***********************/
static int utf16be_len_first_char (const unsigned char* mstr, int len)
{
    UChar16 w1, w2;

    if (len < 2)
        return 0;

    w1 = MAKEWORD (mstr[1], mstr[0]);

    /* added by xm.chen */
#if defined(_UNICODE_SUPPORT) && !defined(_TTF_SUPPORT)
    if(w1 < 0x80)
	return -1;	// this is a sbc font (ascii char)
#endif
    /* added by xm.chen over */

    if (w1 < 0xD800 || w1 > 0xDFFF)
        return 2;

    if (w1 >= 0xD800 && w1 <= 0xDBFF) {
        if (len < 4)
            return 0;
        w2 = MAKEWORD (mstr[3], mstr[2]);
        if (w2 < 0xDC00 || w2 > 0xDFFF)
            return 0;
    }

    return 4;
}

static UChar32 utf16be_conv_to_uc32 (const unsigned char* mchar)
{
    UChar16 w1, w2;
    UChar32 wc;

    w1 = MAKEWORD (mchar[1], mchar[0]);

    if (w1 < 0xD800 || w1 > 0xDFFF)
        return w1;

    w2 = MAKEWORD (mchar[3], mchar[2]);

    wc = w1;
    wc <<= 10;
    wc |= (w2 & 0x03FF);
    wc += 0x10000;

    return wc;
}

static unsigned int utf16be_char_offset (const unsigned char* mchar)
{
    return utf16be_conv_to_uc32 (mchar);
}

static unsigned int utf16be_char_type (const unsigned char* mchar)
{
    unsigned int ch_type;
    unsigned char c1 = mchar [1], c2 = mchar [0];

    if (c1 == 0xFE && c2 == 0xFF) {
        ch_type = MCHAR_TYPE_ZEROWIDTH;
    }
    else if (c2 == 0) {
        ch_type = sb_char_type (&c1);
    }
    else {
        /* TODO: get the subtype of the char */
        ch_type = MCHAR_TYPE_GENERIC;
    }

    return ch_type;
}

static int utf16be_nr_chars_in_str (const unsigned char* mstr, int mstrlen)
{
    int charlen;
    int n = 0;
    int left = mstrlen;

    while (left >= 0) {
        charlen = utf16be_len_first_char (mstr, left);
        if (charlen > 0)
            n ++;

        left -= charlen;
        mstr += charlen;
    }

    return n;
}

static int utf16be_is_this_charset (const unsigned char* charset)
{
    int i;
    char name [LEN_FONT_NAME + 1];

    for (i = 0; i < LEN_FONT_NAME + 1; i++) {
        if (charset [i] == '\0')
            break;
        name [i] = toupper (charset [i]);
    }
    name [i] = '\0';

    if (strstr (name, "UTF") && strstr (name, "16BE"))
        return 0;

    return 1;
}

static int utf16be_len_first_substr (const unsigned char* mstr, int mstrlen)
{
    int ch_len;
    int sub_len = 0;

    while (mstrlen > 0) {
        ch_len = utf16be_len_first_char (mstr, mstrlen);

        if (ch_len == 0)
            break;

        sub_len += ch_len;
        mstrlen -= ch_len;
        mstr += ch_len;
    }

    return sub_len;
}

static const unsigned char* utf16be_get_next_word (const unsigned char* mstr,
                int mstrlen, WORDINFO* word_info)
{
    const unsigned char* mchar = mstr;
    UChar32 wc;
    int ch_len;

    word_info->len = 0;
    word_info->delimiter = '\0';
    word_info->nr_delimiters = 0;

    if (mstrlen == 0) return NULL;

    while (mstrlen > 0) {
        ch_len = utf16be_len_first_char (mchar, mstrlen);

        if (ch_len == 0)
            break;

        wc = utf16be_conv_to_uc32 (mchar);
        if (wc > 0x0FFF) {
            if (word_info->len)
                return mstr + word_info->len + word_info->nr_delimiters;
            else { /* Treate the first char as a word */
                word_info->len = ch_len;
                return mstr + word_info->len + word_info->nr_delimiters;
            }
        }

        switch (wc) {
        case ' ':
        case '\t':
        case '\n':
        case '\r':
            if (word_info->delimiter == '\0') {
                word_info->delimiter = (unsigned char)wc;
                word_info->nr_delimiters += ch_len;
            }
            else if (word_info->delimiter == (unsigned char)wc)
                word_info->nr_delimiters += ch_len;
            else
                return mstr + word_info->len + word_info->nr_delimiters;
            break;

        default:
            if (word_info->delimiter != '\0')
                return mstr + word_info->len + word_info->nr_delimiters;

            word_info->len += ch_len;
            break;
        }

        mstrlen -= ch_len;
        mchar += ch_len;
    }

    return mstr + word_info->len + word_info->nr_delimiters;
}

static int utf16be_pos_first_char (const unsigned char* mstr, int mstrlen)
{
    int ch_len;
    int pos = 0;

    while (mstrlen > 0) {
        ch_len = utf16be_len_first_char (mstr, mstrlen);

        if (ch_len > 0)
            return pos;

        pos += ch_len;
        mstrlen -= ch_len;
        mstr += ch_len;
    }

    return -1;
}

static int utf16be_conv_from_uc32 (UChar32 wc, unsigned char* mchar)
{
    UChar16 w1, w2;

    if (wc > 0x10FFFF) {
        return 0;
    }

    if (wc < 0x10000) {
        mchar [1] = LOBYTE (wc);
        mchar [0] = HIBYTE (wc);
        return 2;
    }

    wc -= 0x10000;
    w1 = 0xD800;
    w2 = 0xDC00;

    w1 |= (wc >> 10);
    w2 |= (wc & 0x03FF);

    mchar [1] = LOBYTE (w1);
    mchar [0] = HIBYTE (w1);
    mchar [3] = LOBYTE (w2);
    mchar [2] = HIBYTE (w2);
    return 4;
}

static CHARSETOPS CharsetOps_utf16be = {
    0x7FFFFFFF,
    4,
    FONT_CHARSET_UTF16BE,
    {'\xA1', '\xA1'},
    utf16be_len_first_char,
    utf16be_char_offset,
    utf16be_char_type,
    utf16be_nr_chars_in_str,
    utf16be_is_this_charset,
    utf16be_len_first_substr,
    utf16be_get_next_word,
    utf16be_pos_first_char,
    utf16be_conv_to_uc32,
    utf16be_conv_from_uc32
};

/************************* End of UNICODE *************************************/
#endif /* _UNICODE_SUPPORT */

static CHARSETOPS* Charsets [] =
{
    &CharsetOps_ascii,
#ifdef _LATIN10_SUPPORT
    &CharsetOps_iso8859_16,
#endif
#ifdef _LATIN9_SUPPORT
    &CharsetOps_iso8859_15,
#endif
#ifdef _LATIN8_SUPPORT
    &CharsetOps_iso8859_14,
#endif
#ifdef _LATIN7_SUPPORT
    &CharsetOps_iso8859_13,
#endif
#ifdef _THAI_SUPPORT
    &CharsetOps_iso8859_11,
#endif
#ifdef _LATIN6_SUPPORT
    &CharsetOps_iso8859_10,
#endif
#ifdef _LATIN5_SUPPORT
    &CharsetOps_iso8859_9,
#endif
#ifdef _HEBREW_SUPPORT
    &CharsetOps_iso8859_8,
#endif
#ifdef _GREEK_SUPPORT
    &CharsetOps_iso8859_7,
#endif
#ifdef _ARABIC_SUPPORT
    &CharsetOps_iso8859_6,
#endif
#ifdef _CYRILLIC_SUPPORT
    &CharsetOps_iso8859_5,
#endif
#ifdef _LATIN4_SUPPORT
    &CharsetOps_iso8859_4,
#endif
#ifdef _LATIN3_SUPPORT
    &CharsetOps_iso8859_3,
#endif
#ifdef _LATIN2_SUPPORT
    &CharsetOps_iso8859_2,
#endif
    &CharsetOps_iso8859_1,
#ifdef _GB18030_SUPPORT
    &CharsetOps_gb18030_0,
#endif
#ifdef _GBK_SUPPORT
    &CharsetOps_gbk,
#endif
#ifdef _GB_SUPPORT
    &CharsetOps_gb2312_0,
#endif
#ifdef _BIG5_SUPPORT
    &CharsetOps_big5,
#endif
#ifdef _EUCKR_SUPPORT
    &CharsetOps_ksc5601_0,
#endif
#ifdef _EUCJP_SUPPORT
    &CharsetOps_jisx0201_0,
    &CharsetOps_jisx0208_0,
#endif
#ifdef _SHIFTJIS_SUPPORT
    &CharsetOps_jisx0201_1,
    &CharsetOps_jisx0208_1,
#endif
#ifdef _UNICODE_SUPPORT
    &CharsetOps_utf8,
    &CharsetOps_utf16le,
    &CharsetOps_utf16be,
#endif
};

#define NR_CHARSETS     (sizeof(Charsets)/sizeof(CHARSETOPS*))

CHARSETOPS* GetCharsetOps (const char* charset)
{
    int i;

    for (i = 0; i < NR_CHARSETS; i++) {
        if ((*Charsets [i]->is_this_charset) 
                        ((const unsigned char*)charset) == 0)
            return Charsets [i];
    }

    return NULL;
}

CHARSETOPS* GetCharsetOpsEx (const char* charset)
{
    int i;
    char name [LEN_FONT_NAME + 1];

    for (i = 0; i < LEN_FONT_NAME + 1; i++) {
        if (charset [i] == '\0')
            break;
        name [i] = toupper (charset [i]);
    }
    name [i] = '\0';

    for (i = 0; i < NR_CHARSETS; i++) {
        if (strcmp (Charsets [i]->name, name) == 0)
            return Charsets [i];
    }

    return NULL;
}

BOOL IsCompatibleCharset (const char* charset, CHARSETOPS* ops)
{
    int i;

    for (i = 0; i < NR_CHARSETS; i++) {
        if ((*Charsets [i]->is_this_charset) 
                        ((const unsigned char*)charset) == 0)
            if (Charsets [i] == ops)
                return TRUE;
    }

    return FALSE;
}

