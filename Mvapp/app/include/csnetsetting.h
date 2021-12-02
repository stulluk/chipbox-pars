#ifndef  _CS_APP_NETWORK_SETTING_H_
#define  _CS_APP_NETWORK_SETTING_H_

typedef enum
{	
	CSAPP_NET_DHCP=0,
	CSAPP_NET_DNS_TYPE,
	CSAPP_NET_IPADDR,
	CSAPP_NET_SUBNET,
	CSAPP_NET_GATEWAY,
	CSAPP_NET_DNS1,
	CSAPP_NET_DNS2,
	CSAPP_NET_DNS3,
	CSAPP_NET_MAC,
	CSAPP_NET_PING,
	CSAPP_NET_ITEM_MAX
} eMV_Nerwork_Items;

typedef enum
{	
	CSAPP_NET_NEWC_S_NUM=0,
	CSAPP_NET_NEWC_S_URL,
	CSAPP_NET_NEWC_S_PORT1,
	CSAPP_NET_NEWC_S_PORT2,
	CSAPP_NET_NEWC_S_USER,
	CSAPP_NET_NEWC_S_PASS,
	CSAPP_NET_NEWC_S_DES1,
	CSAPP_NET_NEWC_S_DES2,
	CSAPP_NET_NEWC_S_CAS,
	CSAPP_NET_NEWC_S_AUTO,
	CSAPP_NET_NEWC_S_LOGIN,
	CSAPP_NEWC_S_ITEM_MAX
} eMV_NewCamd_Client_Items;

typedef enum
{	
	CSAPP_NET_NEWS_S_NUM=0,
	CSAPP_NET_NEWS_S_IP,
	CSAPP_NET_NEWS_S_PORT,
	CSAPP_NET_NEWS_S_DES1,
	CSAPP_NET_NEWS_S_DES2,
	CSAPP_NET_NEWS_S_USER,
	CSAPP_NET_NEWS_S_PASS,
	CSAPP_NET_NEWS_S_CAS,
	CSAPP_NET_NEWS_S_AUTO,
	CSAPP_NEWS_S_ITEM_MAX
} eMV_NewCamd_Server_Items;

typedef enum
{	
	CSAPP_NET_CCCAM_NUM=0,
	CSAPP_NET_CCCAM_IP,
	CSAPP_NET_CCCAM_PORT,
	CSAPP_NET_CCCAM_USER,
	CSAPP_NET_CCCAM_PASS,
	CSAPP_NET_CCCAM_AUTO,
	CSAPP_CCCAM_ITEM_MAX
} eMV_CCCamd_Server_Items;

typedef enum
{	
	CSAPP_DHCP_ON=0,
	CSAPP_DHCP_OFF,
	CSAPP_DHCP_MAX
} eMV_DHCP_Items;

typedef enum
{	
	CSAPP_DNS_AUTO=0,
	CSAPP_DNS_MANUAL,
	CSAPP_DNS_MAX
} eMV_DNS_Items;

typedef enum
{	
	CSAPP_NEWCAM_C=0,
	CSAPP_NEWCAM_S,
	CSAPP_CCCAM,
	CSAPP_NETTYPE_MAX
} eMV_NETTYPE_Items;

typedef enum
{	
	CSAPP_DHCP_IPADDR=0,
	CSAPP_DHCP_SUBNET,
	CSAPP_DHCP_GATEWAY,
	CSAPP_DHCP_DNS1,
	CSAPP_DHCP_IP_MAX
} eMV_DHCP_FILE_Items;

typedef enum
{	
	CSAPP_UDHCPC_IPADDR=0,
	CSAPP_UDHCPC_SUBNET,
	CSAPP_UDHCPC_GATEWAY,
	CSAPP_UDHCPC_IP_MAX
} eMV_UDHCPC_FILE_Items;

U8					IPKeypad_X;
U8					IPKeypad_Y;
U8					u8max_IP_length;
U8					IPKeypad_Index;
U8					IPFocus_Index;
BOOL				IP_keypad_enable;

stIPStr_Struct		IP_TempData[IP_ITEM];

void MV_Draw_NetMenuBar(HDC hdc, U8 u8Focuskind, eMV_Nerwork_Items esItem);
void MV_Draw_Net_MenuBar(HDC hdc);
CSAPP_Applet_t CSApp_NetworkSetting(void);
int Network_Msg_cb(HWND hwnd,int message,WPARAM wparam,LPARAM lparam);
void MV_Parser_DHCP_Value( char *Temp, char *tempSection );
BOOL MV_DHCP_Namecheck(char *tempSection);
MV_File_Return MV_LoadDHCPFile(void);
void MV_DHCP_Data_Read(void);
void MV_Draw_NetSelectBar(HDC hdc, int y_gap, eMV_Nerwork_Items esItem);
void MV_Draw_Net_MenuBar_By_DHCP(HDC hdc);
void MV_Draw_Net_MenuBar_By_DHCP_CHANGE(HDC hdc);
void MV_Net_Draw_IP_Field(HDC hdc, U8 u8FiledNum, U16 u16IpNum, U8 u8Focus);
void IPSelected_Key(HWND hwnd, U8 Select_Kind);
U8 IPPress_Key(void);
BOOL UI_IPKeypad_Proc(HWND hwnd, WPARAM u8Key);
void Draw_IPKeypad(HDC hdc);
void MV_Draw_IP_KeyPad(HWND hwnd, U8 max_string_length, eMV_Nerwork_Items esItem);
void MV_Close_IP_Keypad( HWND hwnd );
#endif
/*   E O F  */

