
#include "stdhdr.h"
#define UTIL_L
#include "util.h"

void hex2str(char *str, uchar * hex, int len)
{
	int i;
	uchar l4, h4;

	for (i = 0; i < len; i++) {
		h4 = (hex[i] & 0xf0) >> 4;
		l4 = hex[i] & 0x0f;
		if (h4 <= 9)
			str[2 * i] = h4 + ('0' - 0);
		else
			str[2 * i] = h4 + ('A' - 10);

		if (l4 <= 9)
			str[2 * i + 1] = l4 + ('0' - 0);
		else
			str[2 * i + 1] = l4 + ('A' - 10);
	}
	str[2 * i] = 0;
}

uchar asc2hex(char asccode)
{
	uchar ret;
	if ('0' <= asccode && asccode <= '9')
		ret = asccode - '0';
	else if ('a' <= asccode && asccode <= 'f')
		ret = asccode - 'a' + 10;
	else if ('A' <= asccode && asccode <= 'F')
		ret = asccode - 'A' + 10;
	else
		ret = 0;
	return ret;
}

void ascs2hex(uchar * hex, uchar * ascs, int srclen)
{
	uchar l4, h4;
	int i, lenstr;
	lenstr = srclen;
	if (lenstr == 0) {
		return;
	}

	if (lenstr % 2)
		return;

	for (i = 0; i < lenstr; i += 2) {
		h4 = asc2hex(ascs[i]);
		l4 = asc2hex(ascs[i + 1]);
		hex[i / 2] = (h4 << 4) + l4;
	}
}

void str2hex(uchar * hex, int *len, char *str)
{
	uchar l4, h4;
	int i, lenstr;
	lenstr = strlen(str);
	if (lenstr == 0) {
		*len = 0;
		return;
	}
	for (i = 0; i < lenstr - (lenstr % 2); i += 2) {
		h4 = asc2hex(str[i]);
		l4 = asc2hex(str[i + 1]);
		hex[i / 2] = (h4 << 4) + l4;
	}
	if (lenstr % 2)
		hex[(lenstr + 1) / 2 - 1] = asc2hex(str[lenstr - 1]) << 4;
	*len = (lenstr + 1) / 2;
}

void noz_str(char *str)
{
	str[strlen(str)] = ' ';
}

#define fillfmt(x,y,z) do{ sprintf(x,y,z); noz_str(x); } while(0)

void disp_hexasc(uchar * data, int size)
{
	int i;
	char outln[100];
	memset(outln, ' ', sizeof(outln));
	for (i = 0; i < size; i++) {
		if (!(i % 16))
			fillfmt(&outln[0], "%04X", i);
		if ((i % 16) < 8) {
			fillfmt(&outln[(i % 16) * 3 + 9], "%02X ", data[i]);
		}
		else {
			fillfmt(&outln[8 * 3 + 9], "%s", "-");
			fillfmt(&outln[(i % 16) * 3 + 9 + 2], "%02X ", data[i]);
		}
		fillfmt(&outln[64 + (i % 16)], "%c", (data[i] >= ' ' ? data[i] : '.'));
		if (!((i + 1) % 16) || i == (size - 1)) {
			outln[80] = 0;
			printf("%s\n", outln);
			memset(outln, ' ', sizeof(outln));
		}
	}
}
