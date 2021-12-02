#ifndef __CI_API_H_
#define __CI_API_H_

typedef struct{
	unsigned char * text;
	int length;
} text_t;

typedef struct{
        int     is_valid;
	text_t title;
	text_t subtitle;
	text_t btmtext;
	unsigned char cnt;
	text_t item[256];
} menu_t;

typedef struct {
        void	(*clear_display)(void);
        int		(*display_enquiry)(	unsigned char * str, 
									unsigned char length, 
									unsigned char blind_answer, 
									unsigned char expected_answer_length);
        int		(*display_list)(void);
        int		(*display_menu)(void);
        void	(*insert_notify )(unsigned char messageNo);
        void	(*cas_info_arrived)(void);
        void	(*app_info_changed)(int flag);
        void	(*mmi_broken)(void);
}tCS_CI_NotifyFunctions;

#define MAX_MANUFACTURE_LENGTH 64

struct application_information_t{
	unsigned char 	application_type;
	unsigned short 	application_manufacture;
	unsigned short	manufacture_code;
	unsigned char 	string_length;
	unsigned char 	manufacture_info[MAX_MANUFACTURE_LENGTH];
};

typedef struct{
    int gpio_ready;
    int gpio_reset;
}tCS_CI_InitParam;

int CS_CI_init(tCS_CI_InitParam param);

void CS_CI_Register_CI_Notify(tCS_CI_NotifyFunctions funcs);
void CS_CI_UnRegister_CI_Notify(void);

/*
interfact to other software in the host
see the Detailed Design of CI Resource
*/
#define AI_SESSION_BROKEN                       0
#define AI_APP_INFO_CHANGE		        1

int ai_get_application_information ( struct application_information_t ** p);
int ai_enter_menu (unsigned char app_index);

//interface to other parts of host software
int dt_set_utc_time (unsigned char * utc_time);
int set_local_time_offset (int local_offset);

/*
this function maybe think about to stop the current ca application
if an other new ca application is used for the new program.

return value:
	0	success
	-1 	parameter error
	-2	content error
	-3	not enough memory
	-4	no ca application
	-5  send message error
*/
#define MAX_CAS_SESSION_AMOUNT 8

typedef struct
{
	BOOL    Isvalid;
	U8          ca_system_id_amount;
	U16       * ca_system_id_data;
} tCS_CI_CAS_Table;

typedef enum{
	CI_CA_PMT_LIST_MORE   = 0x00,
	CI_CA_PMT_LIST_FIRST  = 0x01,
	CI_CA_PMT_LIST_LAST   = 0x02,
	CI_CA_PMT_LIST_ONLY   = 0x03,
	CI_CA_PMT_LIST_ADD    = 0x04,
	CI_CA_PMT_LIST_UPDATE = 0x05
	/* 0x06 - 0xFF reserved */
}eCS_CI_ca_pmt_list_management;



/*
** EN 50221:1997
** 8.4.3.4 CA_PMT
** ca_pmt_cmd_id
** ----------------------------------------------------
** Enumeration values for ca_pmt_cmd_id field(8-bit).
*/
typedef enum{
	/* 0x00 reserved */
	CI_CA_PMT_CMD_OK_DESCRAMBLING = 0x01,
	CI_CA_PMT_CMD_OK_MMI          = 0x02,
	CI_CA_PMT_CMD_QUERY           = 0x03,
	CI_CA_PMT_CMD_NOT_SELECTED    = 0x04
	/* 0x05 - 0xFF reserved */
} eCS_CI_ca_pmt_cmd_id;


int cas_send_ca_pmt_list_to_all(unsigned char *p_ca_pmt, unsigned int length);
BOOL Check_ca_system_id_valid_status(unsigned short ca_system_id);

/*
return value:
	0	success
	-1	command error
	-2 	not enough memory
	-4	no application request this message
*/

menu_t * mmi_get_menu(void);

void mmi_free_menu(menu_t * p_menu);

int mmi_enter_input(unsigned char command, unsigned char * str, int length);
int mmi_enter_choice(unsigned char num);

#endif


