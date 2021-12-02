#include <stdio.h>
#include <stdlib.h>

#include "cstvout.h"

int
main(int argc, char **argv)
{
	char *errstr = NULL;
	CSTVOUT_HANDLE htvout;
	CSTVOUT_WSSSTD wssstd;
	CSTVOUT_WSSINFO wssinfo;

	if (argc < 3) {
		printf("%s  paramater error!\n", __FUNCTION__);
		return 1;
	}

	htvout = CSTVOUT_Open();
	if (htvout == NULL) {
		printf("%s CSTVOUT_Open error!\n", __FUNCTION__);
		return 1;
	}

	wssstd = strtoul(argv[1], NULL, 16);
	wssinfo.WssType = strtoul(argv[2], NULL, 16);
	wssinfo.ARatio = strtoul(argv[2], NULL, 16);

	if (CSTVOUT_WSS_SetFormat(htvout, wssstd)) {
		errstr = CSTVOUT_GetErrString(htvout);
		printf("%s  %s\n", __FUNCTION__, errstr);
	}

	if (CSTVOUT_WSS_SetInfo(htvout, &wssinfo)) {
		errstr = CSTVOUT_GetErrString(htvout);
		printf("%s  %s\n", __FUNCTION__, errstr);
	}

	CSTVOUT_Close(htvout);

	return 0;
}
