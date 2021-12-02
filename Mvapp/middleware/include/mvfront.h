/****************************************************************
*
* FILENAME
*	mvfront.h
*
* PURPOSE 
*	Header for MerihVideo Front Command
*
* AUTHOR
*	KB Kim
*
* HISTORY
*  Status                            Date              Author
*  Create                         2010.02.21           KB
*
****************************************************************/
#ifndef MERIH_FRONT_H
#define MERIH_FRONT_H

/****************************************************************
 *                       Include files                          *
 ****************************************************************/

/****************************************************************
*	                    Define Values                           *
*****************************************************************/
#define FB_COMM_SEND_LENGTH            30
#define FB_COMM_MAX_WAIT               5  /* For Front Communication By KB Kim 2011.02.01 */

/* Define Front COMMAND */
#define FB_START_CODE                 0xF0

#define FB_BOOT_MODE                  0x00
#define FB_REQUEST_BOOT_MODE          0x01
#define FB_CLEAR_BOOT_END             0x02

#define FB_SET_RCU                    0x1F   /* Set RCU Manutacture ID */

#define FB_DISPLAY_FND_SINGLE         0x20
#define FB_DISPLAY_FND_ALL            0x21
#define FB_DISPLAY_VFD_SINGLE         0x22
#define FB_DISPLAY_VFD_ALL            0x23
#define FB_DISPLAY_VFD_TEST           0x24

#define FB_SET_CURRENT_TIME           0x40
#define FB_SET_TIMER                  0x41
#define FB_REQUEST_TIME               0x42
#define FB_RETURN_TIME                0x43

#define FB_SET_AUDIO_MUTE_ON          0x54
#define FB_SET_AUDIO_MUTE_OFF         0x55

#define FB_WATCHDOG_START             0x60
#define FB_WATCHDOG_STOP              0x61
#define FB_WATCHDOG_KICK              0x62

#define FB_SET_RCU_MODE               0xA0
#define FB_SET_FRONT_LEVEL            0xA1
#define FB_SET_FRONT_CLOCK            0xA2

#define FB_REQUEST_MCU_VERSION        0xE0
#define FB_RETURN_MCU_VERSION         0xE1

#define FB_MCU_ACK                    0xFF

#define FB_SET_STATUS                 0x70

#define FB_STAND_BY                   0x00
#define FB_SHUT_DOWN                  0x01
#define FB_STAND_BY_SIGNAL            0x02
#define FB_RUN_NORMAL                 0x04

#define FB_BOOT_MODE_NORMAL           0x10
#define FB_BOOT_MODE_TIMER            0x11
#define FB_BOOT_MODE_AUX              0x12
#define FB_BOOT_MODE_DIAG             0x13
#define FB_BOOT_MODE_WATCH            0x14
#define FB_BOOT_MODE_AC_ON            0x15
#define FB_BOOT_MODE_IR_NORMAL        0x17 /* For Front Communication By KB Kim 2011.02.01 */

#define FB_KEY_POWER                  0x70
#define FB_KEY_HOLD                   0x0F
#define FB_KEY_AUTO_UPDATE            0x33

#define FB_END_CODE                   0x0F

#endif //MERIH_FRONT_H
