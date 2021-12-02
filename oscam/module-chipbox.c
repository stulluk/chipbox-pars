#include "globals.h"
#ifdef MVAPP
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <sys/time.h>
#include "./chipbox/mvipc.h"
#include "module-chipbox.h"

int  Host2PluginMsgId;
int  Plugin2HostMsgId;
struct s_client *ChipBoxClient=NULL;
pthread_t CipBoxCwProcess;

U32 Array2Word(U8 *data, U8 len)
{
	U8   count;
	U32  val;
	
	if ((len > 4) || (len < 1))
	{
		return 0;
	}

	val = 0;
	for (count = 0; count < len; count++)
	{
		val = (val << 8) | ((U32)data[count]);
	}

	return val;
}

void Word2Array(U8 *buff, U32 data, U32 size)
{
	U32 count;

	if ((size < 1) || (size > 4))
	{
		return;
	}

	for (count = size; count > 0; count--)
	{
		buff[count - 1] = (U8)(data & 0xFF);
		data >>= 8;
	}
}

int ChipboxSendControlData(U8 control)
{
	long          dataType;
	int           length;
	unsigned char data[MAX_IPC_DATA_LENGTH];
	
	dataType = DATA_TYPE_OSCAM;
	length = 3;
	data[0] = DATA_CONTROL;
	data[1] = 1;
	data[2] = control;
	
	return MvSendMessage(Plugin2HostMsgId, dataType, length, data);
}

int ChipboxInit(void)
{
	int           result;
	long          dataType;
	int           length;
	U8            data[MAX_IPC_DATA_LENGTH];
	U8            user[20];
	U8            password[20];

	memset(user, 0x00, 20);
	memset(password, 0x00, 20);
	
	Host2PluginMsgId = MvApiGetIpcMessage(0);
	if (Host2PluginMsgId == (-1))
	{
		printf ("ChipboxInit : Error to get Host2PluginMsgId\n");

		return (-1);
	}
	else
	{
		printf ("ChipboxInit : Host2PluginMsgId [0x%X]\n", (unsigned)Host2PluginMsgId);
	}
	
	Plugin2HostMsgId = MvApiGetIpcMessage(1);
	if (Plugin2HostMsgId == (-1))
	{
		printf ("ChipboxInit : Error to get Plugin2HostMsgId\n");
		
		return (-1);
	}
	else
	{
		printf ("ChipboxInit : Plugin2HostMsgId [0x%X]\n", (unsigned)Plugin2HostMsgId);
	}
	
	result = ChipboxSendControlData(CAM_READY);
	if (result == (-1))
	{
		return result;
	}

	do
	{
		result = MvReceiveMessage(Host2PluginMsgId, &dataType, &length, data);
		if (result == (-1))
		{
			return result;
		}
	}
	while((dataType != DATA_TYPE_OSCAM) || (data[0] != DATA_CONTROL) || (data[2] != CAM_ENABLE));
	
	if (data[1] == 41)
	{
		memcpy(user, data + 3, 20);
		memcpy(password, data + 23, 20);
		printf ("Get OSCAM Enable user[%s] password[%s] \n", user, password);
		result = ChipboxSendControlData(CAM_ENABLE);
		cs_strncpy(cfg.chip_user, user, sizeof(cfg.chip_user));
		cs_strncpy(cfg.chip_pwd, password, sizeof(cfg.chip_pwd));
	}
	else
	{
		return (-1);
	}

	return result;
}

void ChipBoxProcessEcm(struct s_client* client, U8 *data, int length)
{
	ECM_REQUEST *er;
	short        dataLength;
	short        ecmLength;
		
	if (!(er = get_ecmtask()))
		return;

	dataLength = (short)data[1];
	ecmLength  = (short)data[2];
	er->ecmlen = ecmLength;
	er->source = data[3];
	er->bank   = data[4];
	er->slot   = data[5];
	er->chid  = (ushort)Array2Word(data + 6, 2);
	er->srvid = (ushort)Array2Word(data + 8, 2);
	er->caid  = (ushort)Array2Word(data + 10, 2);
	er->prid  = (ulong)Array2Word(data + 12, 4);
	er->pid   = (ushort)Array2Word(data + 16, 2);
	er->client = client;
	memcpy(er->ecm, data + 20, er->ecmlen);
	
	get_cw(client, er);
}

#if 0
void *ChipBoxGetCwTask (void *cli)
{
	struct s_client* client;
	struct pollfd pfd2;
	int    rc;
	// struct timeb tp;
	
	client = (struct s_client*)cli;
	
	client->thread=pthread_self();
	pthread_setspecific(getclient, cli);
	
	pfd2.fd     = client->fd_m2c_c;
	pfd2.events = (POLLIN | POLLPRI);

	/*
	cs_ftime(&tp);
	tp.time+=500;
	*/

	while(1)
	{
		chk_pending(500);
		rc = poll(&pfd2, 1, 500);
		if (rc<1)
		{
			continue;
		}

		if (pfd2.revents & (POLLIN | POLLPRI))
		{
			process_client_pipe(client, NULL, 0);
		}
	}

	return NULL;
}
#endif

void *ChipBoxMainTask (void *cli)
{
	struct s_client* client;
	int              result;
	long             dataType;
	int              length;
	U8               command;
	U8               data[MAX_IPC_DATA_LENGTH];
	struct s_auth   *account;
	int ok = 0;

	client = (struct s_client*)cli;
	
	client->thread=pthread_self();
	pthread_setspecific(getclient, cli);

	ChipBoxClient = client;

	while(ChipboxInit() == (-1))
	{
		cs_sleepms(500);
	}
	
	for (account = cfg.account; account && (!ok); account=account->next)
	{
		ok = !strcmp(cfg.chip_user, account->usr);
		if(ok)
		{
			break;
		}
	}
	cs_auth_client(client, ok ? account : (struct s_auth *)(-1), "chipapi");

	while(1)
	{
		result = MvReceiveMessage(Host2PluginMsgId, &dataType, &length, data);
		if (result != (-1))
		{
			if (dataType == DATA_TYPE_OSCAM)
			{
				command = data[0];
				switch(command)
				{
					case DATA_ECM :
						ChipBoxProcessEcm(client, data, length);
						break;
					case DATA_CONTROL :
						if ((data[1] == 1) && (data[2] == CAM_PING))
						{
							/* Ping arrived : Send back ping */
							result = ChipboxSendControlData(CAM_PING);
						}
						break;
					case DATA_PMT :
					case DATA_CAT :
					case DATA_EMM :
					default :
						break;
				}
			}
		}
	}
	
	return NULL;
}
// static void *ChipApiHandler(int ctyp)
static void *ChipApiHandler(struct s_client * cl, uchar* UNUSED(mbuf), int ctyp)
{
	cl = create_client(0);
	cl->typ='c';
	cl->ctyp=ctyp;

#if 0
	pthread_create(&CipBoxCwProcess, NULL, ChipBoxGetCwTask, (void*) cl);
	pthread_detach(CipBoxCwProcess);
#endif

	pthread_create(&cl->thread, NULL, ChipBoxMainTask, (void*) cl);
	pthread_detach(cl->thread);
	
	return NULL;
}

void ChipBoxSendCw (struct s_client* client, ECM_REQUEST *er)
{
	long          dataType;
	int           length;
	unsigned char data[50];

	length = 0;
	memset(data, 0x00, 50);

	printf("ChipBoxSendCw \n");

	dataType = DATA_TYPE_OSCAM;
	data[0]  = DATA_ECM_RESULT;
	data[3]  = er->source;
	data[4]  = er->bank;
	data[5]  = er->slot;
	Word2Array(data + 6, (U32)er->chid, 2);
	Word2Array(data + 8, (U32)er->srvid, 2);
	Word2Array(data + 10, (U32)er->caid, 2);
	Word2Array(data + 12, (U32)er->prid, 4);
	Word2Array(data + 16, (U32)er->pid, 2);
	
	if (er->rc < E_NOTFOUND)
	{
		data[2] = 16;
		memcpy(data + 20, er->cw, 16);
		length = 20 + 16;
	}
	else
	{
		data[2] = 0;
		length = 20;
	}

	data[1] = (U8)(length - 2) & 0xFF;

	MvSendMessage(Plugin2HostMsgId, dataType, length, data);
}

void module_Chipapi(struct s_module *ph)
{
	// strcpy(ph->desc, "ChipApi");
	ph->desc="ChipApi";
	ph->type=MOD_CONN_SERIAL;
	ph->multi=1;
	// ph->watchdog=0;
	ph->s_handler=ChipApiHandler;
	ph->send_dcw=ChipBoxSendCw;
}

#endif /* #ifdef MVAPP */

