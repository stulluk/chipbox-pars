#include <stdio.h>
#include <stdlib.h>
#include "csapi.h"
#include "csuhfmod.h"


CSUHFMOD_HANDLE modulator_handle;



int main(void)
{
    CSAPI_RESULT ret;
    CSUHFMOD_CHANNEL channel;

//     printf("Plese input channel(21--69):");
//     scanf("%d", (int *)&channel);
    CSUHFMOD_STATUS status;
    channel = 36;
    modulator_handle = CSUHFMOD_Open(CSUHFMOD_TENA_TNF0170U623R);
    if (modulator_handle == NULL){
        printf("Error to Open CSUHFMOD module!\n");
        return 0;
    }

    if (CSUHFMOD_SetMode(modulator_handle, CSUHFMOD_ACTIVE) != CSAPI_SUCCEED){
        printf("Error to set active to modulator!\n");
        CSUHFMOD_Close(modulator_handle);
        return 0;
    }

    ret = CSUHFMOD_SetChannel(modulator_handle, channel);
    if (ret != CSAPI_SUCCEED){
        printf("%s\n", CSUHFMOD_GetErrString(modulator_handle));
    }
	
	while (1){
        if(CSUHFMOD_GetStatus(modulator_handle, &status) != CSAPI_SUCCEED){
            printf("Get UHF Modulator Status error!\n");
        }
		sleep(2);
	}
    return 0;
}
