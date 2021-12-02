
#ifndef __UTIL_H__
#define __UTIL_H__

#ifndef UTIL_L
#define UTIL_L  extern
#endif

#ifndef uchar
#define uchar unsigned char
#endif

#ifndef uint
#define uint unsigned char
#endif

#ifndef ulong
#define ulong unsigned char
#endif

#ifndef bool
#define bool uint
#endif

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

UTIL_L void hex2str(char *str, uchar * hex, int len);
UTIL_L uchar asc2hex(char asccode);
UTIL_L void ascs2hex(uchar * hex, uchar * ascs, int srclen);
UTIL_L void str2hex(uchar * hex, int *len, char *str);
UTIL_L void noz_str(char *str);	/* replace teminate null with ' ' of string */
UTIL_L void disp_hexasc(uchar * data, int size);

#endif
