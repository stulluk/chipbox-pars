
#include "stdhdr.h"
#include "util.h"
#include "cmd.h"
#include "tt_gpio.c"
#include "tt_i2c.c"
#include "tt_fpc.c"
#include "tt_dispout.c"
#include "tt_vid.c"
#include "tt_aud.c"
#include "tt_osd.c"
#include "tt_hdmi.c"
#include "tt_eeprom.c"
//#include "tt_df.c"
//#include "tt_sqc.c"
#include "tt_tuner.c"
#include "tt_sci.c"


static struct cmd_t dummy_tt[] = {
{
	NULL,
 	NULL,
 	NULL,
 	NULL
}};


cmd_module cmd_module_tbl[] ={
    REGISTER_CSAPI_MODULE(csgpio_tt),
     REGISTER_CSAPI_MODULE(csi2c_tt),
     REGISTER_CSAPI_MODULE(csfpc_tt),
     REGISTER_CSAPI_MODULE(cstvout_tt),
     REGISTER_CSAPI_MODULE (csosd_tt),
     REGISTER_CSAPI_MODULE(cshdmi_tt),
     REGISTER_CSAPI_MODULE(eeprom_tt),
     //REGISTER_CSAPI_MODULE(csdf_tt),
     //REGISTER_CSAPI_MODULE(cssqc_tt),
     REGISTER_CSAPI_MODULE(vid_tt),
     REGISTER_CSAPI_MODULE(aud_tt),
     REGISTER_CSAPI_MODULE(cstuner_tt),
     REGISTER_CSAPI_MODULE(cssci_tt),
     REGISTER_CSAPI_MODULE(dummy_tt)
};


int main(int argc, char **argv)
{
	printf("\n Load default set ...... \n");
	docmd("=>", cmd_module_tbl);

	return 0;
}

