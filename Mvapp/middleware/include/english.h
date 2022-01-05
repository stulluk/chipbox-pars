#ifndef  _CS_APP_ENGNISH_H_
#define  _CS_APP_ENGNISH_H_

#include "mwsetting.h"


static U8* 	EngnishString[CSAPP_STR_MAX]={
"",
"Main Menu",
"Installation",
"Install & Searching",
"Satellite Setting",
"TP Setting",
"Edit Program",
"System Setting",
"System Information",
"All Satellite TV",
"All Satellite Radio",
"FAV Channel",
"No channel...",
"No program...",
"LCN",
"Name",
"Unlock",
"Love",
"Level",
"SNR",
"HD",
"H264",
"AC3",
"CC",
"Progress",
"Mode",
"Frequency",
"Bandwidth",
"MHz",
"Start",
"Manual",
"Auto",
"NIT",
"Progress",
"TV number",
"Radio Number",
"Edit TV",
"Edit Radio",
"AV Setting",
"System Setting",
"Time Setting",
"Parental Control",
"Factory Reset",
"Type No.",
"HW Version",
"SW Version",
"Timer",                     // kb : Mar 25
"EPG",                       // kb : Mar 25
"Simple",                    // kb : Mar 25
"Weekly EPG",                 // kb : Mar 25
"Current",                   // kb : Mar 25
"Next",                      // kb : Mar 25
"No detail information...",  // kb : Mar 25
"Pre Day",
"Next Day",
"Desc.",
"Mon.",
"Tue.",
"Wed.",
"Thu.",
"Fri.",
"Sat.",
"Sun.",
"Volume",
"Audio setting",
"Audio lang",
"Audio track",
"Stereo",
"Left",
"Right",
"Mono",
"Frequency",
"Symbol Rate",
"Polarity",
"H",
"V",
"Signal Status:",
"No Signal",
"No Service",
"Scramble",
"Teletext",
"Subtitle",
"Aspect ratio",
"Aspect mode",
"Video Resolution",
"Video output",
"4:3",
"16:9",
"Automatic",
"Letterbox",
"PanScan",
"Full",
"CVBS",
"YC",
"YCbCr",
"YPbPr",
"RGB",
"HDMI",
"Language Setting",
"Logic CH Num",
"Parent rate",
"Time Zone",
"Time Mode",
"Satellite Time",
"Local Time",
"Date",
"Time",
"OK: save&exit,\nMENU/EXIT: cancel&exit",  /* By KB Kim 2011.05.06 */
"Time region",
"Transparency",
"Appearing order",
"Operator defined",
"Lock Status",
"Boot Lock",
"Menu Lock",
"Channel Edit Lock",
"Channel Lock",
"Favorite Lock",
"Password Change",
"Enable",
"Disable",
"Old pin",
"New pin",
"Confirm pin",
"Input Password",
"Password",
"Old password Error.",
"Confirm password Error",
"New Password success",
"Warning",
"This will clear all services, \ncontinue?",
"Data is changed, save?",
"OK",
"Cancel",
"Save",
"Press OK to comfirm,\nPress EXIT to cancel",
"Delete",
"Lock",
"Lock",
"Fav",
"Sort",
"This operation will load \n default and erase all \n the channels that user added, \n continue?",
"Loading default settings, \n please don't shut power!",
"This operation will data backup \n - satellite & tp & Channel - \n continue ?",
"This operation will restore \n backup data to database. \n - satellite & tp & Channel - \n continue ?",
"Red button will Reboot System",
"SPDIF",
"PCM",
"AC3",
"Strength",
"Quality",
"OSD Setting",
"Network Setting",
"Recorded File",
"Recorded Config",
"File Tools",
"USB Device Remove",
"Storage Information",
"MP3 Player",
"Upgrade",
"Common Interface",
"Conditional Access",
"Backup",
"Backup Restore",
"Plug-in",
"Summer Time",
"Year",
"Month",
"Day",
"Hour",
"Minute",
"On",
"Off",
"Video Setting",
"Bright",
"Contrast",
"Color",
"Media",
"Tool",
"Info Banner Time",
"DHCP",
"DHCP Setting",
"IP Address",
"Subnet Mask",
"Gateway",
"Primary DNS",
"Secondary DNS",
"Third DNS",
"Mac Address",
"Connection Type",
"Satellite",
"TP Select",
"LNB Type",
"LNB Power",
"22K Tone",
"High Frequency",
"Low Frequency",
"DiSEqC Setting",
"Scan Type",
"Multi Sat",
"Single Sat",
"Select Satellite",
"Select Transponder",
"ToneBurst",
"Rename",
"Scan",
"Add New Transponder",
"Manual Channel Addition",
"Find",
"Advanced",
"Edit",
"Transponder",
"Video PID",
"Audio PID",
"PCR PID",
"Service ID",
"Same TP",
"Invalid Data",
"No Data",       // by kb : 20100406
"No TP Data",    // by kb : 20110115
"Are You Sure ?",
"Move",
"All",
"TV",
"Radio",
"Delete current TP",
"Select",
"Jump",
"A-Z Sort",
"Z-A Sort",
"FTA-CAS Sort",
"CAS-FTA Sort",
"SD-HD Sort",
"HD-SD Sort",
"Restore Normal",
"All Satellite",
"Attention",
"Now Saving ...\nDon't Press Any key !",
"Pause Screen",
"Black Screen",
"Channel Change Type",
"Not Use",
"Use",
"Get DHCP",
"Now Get DHCP Data \nWait Some Second ...",
"FAIL Get DHCP Data \nChecking Network Enviroment",
"Invalid IP Address ..\nTry Again !!",
"Test PING",
"PINGING ..... DNS",
"PINGING ..... IP",
"PING Fail Check Network",
"DNS PING Fail, IP PING OK\nCheck DNS Address",
"PING OK !!!!",
"None",
"Language",
"Invalid Channel Number ...",
"Total Size",
"Used Size",
"Available Space",
"Storage Type",
"Storage Vender",
"Storage Product",
"Storage Serial Number",
"Storage Format",
"Erase All TP",
"Please Wait",
"Position No.",
"Goto X",
"Step Move",
"Auto Move",
"Limit Setting",
"West Limit Set",
"East Limit Set",
"Goto Reference",
"Recalculation",
"First Select Satellite",
"USALS Setting",
"Satellite Longitude",
"Local Longitude",
"Local Latitude",
"Modify Longitude",
"Unicable Setting",
"Unicable LNB Select",
"Set Position",
"Transmission Channel",
"Transmission Frequency",
"All Channel Delete",
"Delete Channel by Satellite",
"KEYBOARD",
"Numeric Keyboard",
"Search Condition",
"Transponder Addition",
"Channel Addition",
"Time Shift",
"Time Shift Record",
"Stream Type",
"Time Jump",
"Now PVR Recording\nPress Stop First",
"Now PVR Playing\nPress Stop First",
"To Internal Flesh",
"To USB",
"From Internal Flesh",
"From default data",
"From USB",
"Card Status",
"Card",
"File Not Found",
"CAS detail information",
"Recall",
"Single",
"Multi",
"Model Type",
"UBOOT Version",
"Kernel Version",
"ROOTFS Version",
"SW Main Version",
"Default Sat TP Version",
"Default Database",
"Video Background",
"Audio Background",
"Copy",
"Paste",
"Same Folder",
"There is Same File",
"Add",
"Scaning ..",
"Locked",
"UnLock",
"Extend Channel Information",
"Network Type",
"NewCamd Client",
"NewCamd Server",
"CCCAM",
"Server Number",
"URL",
"Server Port",
"User",
"DES Key",
"CA Type",
"Auto Connect",
"Log in",
"Start",
"Font",
"Font Size",
"English",
"Turkish",
"French",
"German",
"Greek",
"Arabic",
"Persian",
"Internet Time",
"DNS Method",
"Slot Empty",
"Inserted Card",
"File explore",
"Database",
"All Flash Backup",
"ALL Flash Restore",
"No Files",
"No Select File",
"Recording Error",
"Playing Error",
"Continue ?",
"No.",
"On/Off",
"Type",
"Repeat",
"Channel",
"End",
"Duration",
"Power Off Mode",
"Sleep Mode",
"Real Off",
"Time Display on Standby",
"REBOOT SYSTEM\nAre You SURE ?",
"System Reboot",
"Extended",
"Normal",
"Channel List Type",
"Wake Up",
"Sleep",
"Record",
"Once",
"Everyday",
"Every Week",
"Game",
"Goodbye...",
"Good .. Next ?",
"Stage",
"Restart",
"Back",
"Next Stage",
"Prev Stage",
"January",
"February",
"March",
"April",
"May",
"June",
"July",
"August",
"September",
"October",
"November",
"December",
"Calendar",
"Next Month",
"Prev Month",
"Next Year",
"Prev Year",
"Change Partition",
"Partition",
"Title",
"Artist",
"Album",
"Edit BISS",
"Move Dish",
"Downloading ..",
"Get List .. FAIL !!\n Retry !!",
"Download File .. FAIL !!\n Retry !!", /* By KB Kim : 2011.05.07*/
"Upgrade .. FAIL !!\n Retry !!",
"Do You Want to Upgrade ?", /* By KB Kim : 2011.05.07*/
"Space",
"Skin",
"Reset",
"Sat A",
"Sat B",
"Menu Animation",
"Heart Bit",
"FTA+CAS Search",
"FTA+CAS NIT Search",
"FTA Search",
"FTA NIT Search",
"FTA+CAS Blind Search",
"FTA Blind Search",
"DiSEqC Port",
"DiSEqC Motor",
"USALS",
"UniCable",
"System Ready",
"Boot Block",
"Kernel",
"Plug-In",
"Application 1",
"Application 2",
"Root file System",
"Finish",
"Boot Block Upgrading",
"Kernel Upgrading",
"Plug-In Upgrading",
"Application 1 Upgrading",
"Application 2 Upgrading",
"Root file System Upgrading",
"Finishing",
"USB",
"NETWORK",
"Software UPGRADING ... Please Wait",
"OSCAM Setting",
"Port End",
"Username",
"Key",
"Status",
"CCCamd",
"NewCamd",
"Enable",
"Disable",
"Connected",
"Not Connected",
"Server Number",
"Protocol",
"Port Start",
"DES Key",

"Unknown",    /* By KB 2011.04.09 */
"Can Not Change\nPincode Error! Retry ?",
"Can Not Change\nNew Code Error! Retry ?",
"Test",
"If upgrade does not finish in 10mins,\npls power-off and on manually", /* By KB Kim : 2011.05.07*/
"USB Device CONNECTING",
"USB device MOUNT",
"USB device DISCONNECTING",
"USB device UNMOUNT",
"All Clear",
//"Save",
"Back Space",
"NAME",
"SIZE",
"Record Time",
"Program",
//"Duration",
"End Time",
//"TIME",
"Capslock",
"Caps",
"Del",
"SMART card insert ...",
"SMART card Remove ...",
"Install Plug-In", /* By KB Kim for Plugin Setting : 2011.05.07 */
"Install PlugIn .. FAIL !!\n Retry !!", /* By KB Kim for Plugin Setting : 2011.05.07 */
"Do You Want to Install?", /* By KB Kim for Plugin Setting : 2011.05.07 */
"Add Server", /* By KB Kim for Plugin Setting : 2011.05.07 */
"Server Data Full", /* By KB Kim for Plugin Setting : 2011.05.07 */
"Delete Server",    /* By KB Kim for Plugin Setting : 2011.05.25 */
"Extend",
"Detail Information",
"No Module Inserted",
"Module Initialising......",
"Module OK !",
"Change Focus",
"Locked\nChannel",
"Site List",		/* By KB Kim for Plugin Site List : 2011.09.20 */
"Can Not Delete",		/* By KB Kim for Plugin Site List : 2011.09.20 */
"Can Not Add",		/* By KB Kim for Plugin Site List : 2011.09.20 */
"Remove channel from Favorite",
"Application Starting...",
"Installing Plugin...",
"Plugin Installed Succesfully!!",
"Currently Streaming \n Press STOP first!!",
"Streaming Starting..."
};
#endif
/*   E O F  */

