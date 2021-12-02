#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "global.h"
#include "csapi.h"
#include "cssi.h"

#define SECTION_LEN 		1024
#define MAX_SERVICE_NUM 	32

#define PAT_PID     		0x00
#define SDT_PID     		0x11
#define TDT_PID    	 	0x14
#define EIT_PID     		0x12
#define CAT_PID     		0x01

typedef struct tagSI_OBJ 
{
	CSSI_InitParams params;

} CSSI_OBJ;

typedef struct tagSI_Service 
{
	unsigned int service_id;
	unsigned int service_pmtpid;
	unsigned int service_vidpid;
	unsigned int service_audpid;
	unsigned int pmt_complete;

	CSDEMUX_HANDLE filter_handle;
	CSDEMUX_HANDLE pidft_handle;

	unsigned int vid_type;
	unsigned int aud_type;
	char providername[20];
	char servicename[20];

} CSSI_Service;

static CSSI_Service service[MAX_SERVICE_NUM];
static unsigned char section_buf[SECTION_LEN];

static unsigned int service_num = 0;
static unsigned int cur_pmt_parser_index = 0;
static unsigned int sdtcomplete = 0;
static unsigned char current_section_num = 0;
static unsigned char last_section_num = 0;

static void pat_table_parse(unsigned char *ptr, unsigned int len);
static void pmt_table_parse(unsigned char *ptr, unsigned int len);
static void sdt_table_parse(unsigned char *ptr, unsigned int len);
static void search_channel(CSSI_InitParams *params);
static void parse_ts_file(FILE *ts_fp);

CSSI_HANDLE  CSSI_Init(CSSI_InitParams *params)
{
	CSSI_OBJ *dev_obj;
	
	if (NULL == params) 
		return NULL;

	if (NULL == (dev_obj = malloc(sizeof(CSSI_OBJ))))
		return NULL;

	memset(dev_obj, 0, sizeof(CSSI_OBJ));

	memcpy(&dev_obj->params, params, sizeof(CSSI_OBJ));

	return (CSSI_HANDLE) dev_obj;
}

CSAPI_RESULT CSSI_Term(CSSI_HANDLE handle)
{
	CSSI_OBJ *dev_obj = handle;

	if (NULL != dev_obj)
	{
		free(dev_obj);
		dev_obj = NULL;
	}

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSSI_GetServiceInfoList(CSSI_HANDLE handle, int *si_num, CSSI_ServiceInfo *info)
{
	unsigned int i;
	CSSI_OBJ *dev_obj = (CSSI_OBJ*)handle;

	if (NULL == handle) return CSAPI_FAILED;
	if (NULL == info) return CSAPI_FAILED;

	search_channel(&dev_obj->params);
	for (i = 0; i < service_num; i++)
	{
		info[i].vpid   = service[i].service_vidpid;
		info[i].apid   = service[i].service_audpid;
		info[i].pmtpid = service[i].service_pmtpid;
		
		info[i].service_id = service[i].service_id;;
		strcpy(info[i].service_name, service[i].servicename);
	}

	*si_num = service_num;

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSSI_GetServiceInfoFromStream(CSSI_HANDLE handle, unsigned int service_id, CSSI_ServiceInfo *info)
{
	unsigned int i;
	CSSI_OBJ *dev_obj = (CSSI_OBJ*)handle;

	if (NULL == handle) return CSAPI_FAILED;
	if (NULL == info) return CSAPI_FAILED;

	search_channel(&dev_obj->params);

	for (i = 0; i < service_num; i++)
	{
		if (service[i].service_id == service_id)
		{
			info->vpid   = service[i].service_vidpid;
			info->apid   = service[i].service_audpid;
			info->pmtpid = service[i].service_pmtpid;
			
			info->service_id = service_id;
			strcpy(info->service_name, service[i].servicename);
	
			return CSAPI_SUCCEED;
		}
	}

	return CSAPI_FAILED;
}

CSAPI_RESULT CSSI_GetServiceInfoListFromFile(CSSI_HANDLE handle, int *si_num, CSSI_ServiceInfo *info)
{
	unsigned int i;
	CSSI_OBJ *dev_obj = (CSSI_OBJ*)handle;

	if (NULL == handle) return CSAPI_FAILED;
	if (NULL == info) return CSAPI_FAILED;

	parse_ts_file(dev_obj->params.ts_fp);

	for (i = 0; i < service_num; i++)
	{
		info[i].vpid   = service[i].service_vidpid;
		info[i].apid   = service[i].service_audpid;
		info[i].pmtpid = service[i].service_pmtpid;
		
		info[i].service_id = service[i].service_id;;
		strcpy(info[i].service_name, service[i].servicename);
	}

	*si_num = service_num;

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSSI_GetServiceInfoFromFile(CSSI_HANDLE handle, unsigned int service_id, CSSI_ServiceInfo *info)
{
	unsigned int i;
	CSSI_OBJ *dev_obj = (CSSI_OBJ*)handle;

	if (NULL == handle) return CSAPI_FAILED;
	if (NULL == info) return CSAPI_FAILED;

	parse_ts_file(dev_obj->params.ts_fp);

	for (i = 0; i < service_num; i++)
	{
		if (service[i].service_id == service_id)
		{
			info->vpid   = service[i].service_vidpid;
			info->apid   = service[i].service_audpid;
			info->pmtpid = service[i].service_pmtpid;
			
			info->service_id = service_id;
			strcpy(info->service_name, service[i].servicename);
	
			return CSAPI_SUCCEED;
		}
	}

	return CSAPI_FAILED;
}

static void pat_table_parse(unsigned char *ptr, unsigned int len)
{
	int pmtpid;
	int serverid;
	int section_len;

	if (len < 12 || (ptr[1] & 0x80) == 0)
		return;

	if (!(ptr[5] & 0x1))
		return;

	section_len = ((ptr[1] & 0xf) << 8 | ptr[2]);

	ptr += 8;
	int i = section_len - 9;

	while (i >= 4) 
	{
		serverid = (ptr[0] << 8) | ptr[1];
		pmtpid = ((ptr[2] & 0x1f) << 8) | ptr[3];

		if (service_num < MAX_SERVICE_NUM && serverid != 0) 
		{
			service[service_num].service_id = serverid;
			service[service_num].service_pmtpid = pmtpid;
			service[service_num].service_vidpid = 0x1fff;
			service[service_num].service_audpid = 0x1fff;
			service[service_num].pmt_complete = 0;
			service_num++;
		}

		ptr += 4;
		i -= 4;
	}
}

static void pmt_table_parse(unsigned char *ptr, unsigned int len)
{
	int l;
	int tmp;
	int vpid = 0x1fff;
	int apid = 0x1fff;
	int audionum = 0;

	if (len < 12 || (ptr[1] & 0x80) == 0)
		return;

	if (!(ptr[5] & 0x1))
		return;

	tmp = (((ptr[10] & 0xf) << 8) | ptr[11]) + 12;

	l = len - tmp - 4;
	ptr += tmp;

	while (l >= 5) 
	{
		if (ptr[0] == 0x01 || ptr[0] == 0x02 || ptr[0] == 0x1b) 
		{
			vpid = ((ptr[1] & 0x1f) << 8) | ptr[2];
			service[cur_pmt_parser_index].service_vidpid = vpid;
			if (ptr[0] == 0x1b)
				service[cur_pmt_parser_index].vid_type = 0;
			else
				service[cur_pmt_parser_index].vid_type = 1;
		}

		else if ((ptr[0] == 0x03 || ptr[0] == 0x04 || ptr[0] == 0x81 || ptr[0] == 0x6a) && (audionum == 0)) 
		{
			apid = ((ptr[1] & 0x1f) << 8) | ptr[2];
			service[cur_pmt_parser_index].service_audpid = apid;
			if (ptr[0] == 0x6a || ptr[0] == 0x81)
				service[cur_pmt_parser_index].aud_type = 0;
			else
				service[cur_pmt_parser_index].aud_type = 1;

			audionum++;
		}
		else {
			; /* invalid type. */
		}

		tmp = (((ptr[3] & 0xf) << 8) | ptr[4]) + 5;

		l -= tmp;
		ptr += tmp;
	}
}

static void sdt_table_parse(unsigned char *ptr, unsigned int len)
{
	char *aaa = NULL;
	unsigned short serviceid;
	int l, tmp, descriptorlooplen, descriptorlen, namelen, templen;
	unsigned int i;

	if (len < 12 || (ptr[1] & 0x80) == 0)
		return;

	if (!(ptr[5] & 0x1))
		return;

	tmp = ((ptr[1] << 8) & 0xf00) | ptr[2];

	current_section_num = ptr[6];
	last_section_num = ptr[7];

	if (current_section_num == last_section_num)
		sdtcomplete = 1;
	else
		sdtcomplete = 0;

	l = tmp - 4 - 8;
	ptr += 11;

	while (l) {
		serviceid = ptr[0] << 8 | ptr[1];
		templen = descriptorlooplen = ((ptr[3] << 8) & 0xf00) | ptr[4];
		aaa = ptr;
		for (i = 0; i < service_num; i++) {
			if (service[i].service_id == serviceid) {
				aaa += 5;
				descriptorlen = aaa[1];
				if (aaa[0] == 0x48) {
					aaa += 3;
					namelen = aaa[0];
					aaa++;
					memset((char *) service[i].providername, 0, 20);
					memcpy((char *) service[i].providername, (char *) aaa, namelen);
					aaa += namelen;
					namelen = aaa[0];
					aaa++;
					memset((char *) service[i].servicename, 0, 20);
					memcpy((char *) service[i].servicename, (char *) aaa, namelen);
					aaa += namelen;
				}
				else {
					aaa = aaa + descriptorlen + 2;
				}
			}
		}

		l = l - templen - 5;
		ptr = ptr + templen + 5;
	}
}

static void search_channel(CSSI_InitParams *params)
{
	CSDEMUX_HANDLE hfilter;
	CSDEMUX_HANDLE hpidfilter;

	unsigned char filter[12];
	unsigned char mask[12];
	unsigned int data_length = 0;
	int pmt_parsed_num = 0;

	unsigned int service_id = 0;
	unsigned int service_pmtpid = 0;
	unsigned int service_vidpid = 0;
	unsigned int service_audpid = 0;

	service_num = 0;
	cur_pmt_parser_index = 0;

	memset(filter, 0, 12);
	memset(mask, 0, 12);

	filter[0] = 0x00;
	mask[0] = 0xff;

	hfilter    = params->secft_handle[0];
	hpidfilter = params->pidft_handle[0];

	CSDEMUX_PIDFT_SetPID(hpidfilter, PAT_PID);
	CSDEMUX_PIDFT_Enable(hpidfilter);

	CSDEMUX_Filter_SetFilter(hfilter, filter, mask);
	CSDEMUX_Filter_AddPID(hfilter, PAT_PID);
	CSDEMUX_Filter_SetFilterType(hfilter, DEMUX_FILTER_TYPE_SEC);
	CSDEMUX_Filter_Enable(hfilter);

	while (1) {
		data_length = 1024;
		if (CSDEMUX_Filter_ReadWait(hfilter, 500) == CSAPI_SUCCEED) {
			if (CSDEMUX_Filter_ReadSectionData(hfilter, section_buf, &data_length) == CSAPI_SUCCEED) {
				pat_table_parse(section_buf, data_length);
				break;
			}
		}
		else{
			printf("wait pat!\n");
		}
	}

	CSDEMUX_Filter_Disable(hfilter);
	CSDEMUX_PIDFT_Disable(hpidfilter);

	for (cur_pmt_parser_index = 0; 
		(cur_pmt_parser_index < params->filter_num) && 
		(cur_pmt_parser_index < service_num); 
		cur_pmt_parser_index++) 
	{
		if (service[cur_pmt_parser_index].service_id == 0)
			continue;

		hfilter    = params->secft_handle[cur_pmt_parser_index+1];
		hpidfilter = params->pidft_handle[cur_pmt_parser_index+1];

		service[cur_pmt_parser_index].filter_handle = hfilter;
		service[cur_pmt_parser_index].pidft_handle  = hpidfilter;

		filter[0] = 0x02;
		mask[0] = 0xff;

		CSDEMUX_PIDFT_SetPID(hpidfilter, service[cur_pmt_parser_index].service_pmtpid);
		CSDEMUX_PIDFT_Enable(hpidfilter);

		CSDEMUX_Filter_SetFilter(hfilter, filter, mask);
		CSDEMUX_Filter_AddPID(hfilter, service[cur_pmt_parser_index].service_pmtpid);
		CSDEMUX_Filter_SetFilterType(hfilter, DEMUX_FILTER_TYPE_SEC);
		CSDEMUX_Filter_Enable(hfilter);
	}

	pmt_parsed_num = service_num;

	while (pmt_parsed_num >= 1) 
	{
		for (cur_pmt_parser_index = 0; cur_pmt_parser_index < service_num; cur_pmt_parser_index++) 
		{
			if (service[cur_pmt_parser_index].pmt_complete)
				continue;

			data_length = 1024;
			hfilter = service[cur_pmt_parser_index].filter_handle;

			if (CSDEMUX_Filter_ReadWait(hfilter, 500) == CSAPI_SUCCEED) 
			{
				if (CSDEMUX_Filter_ReadSectionData(hfilter, section_buf, &data_length) == CSAPI_SUCCEED) 
				{
					pmt_table_parse(section_buf, data_length);
					service[cur_pmt_parser_index].pmt_complete = 1;

					pmt_parsed_num--;

					break;
				}
			}
			else {
				printf("PMT---> ReadWait() timeout. \n");
			}
		}
	}

	for (cur_pmt_parser_index = 0; cur_pmt_parser_index < service_num; cur_pmt_parser_index++) 
	{
		service_id = service[cur_pmt_parser_index].service_id;

		service_pmtpid = service[cur_pmt_parser_index].service_pmtpid;
		service_vidpid = service[cur_pmt_parser_index].service_vidpid;
		service_audpid = service[cur_pmt_parser_index].service_audpid;

		CSDEMUX_PIDFT_Disable(service[cur_pmt_parser_index].pidft_handle);

		CSDEMUX_Filter_Disable(service[cur_pmt_parser_index].filter_handle);
	}

	return;
}

static void parse_ts_file(FILE *ts_fp)
{
	unsigned char filter[12];
	unsigned char mask[12];
	unsigned int data_length = 0;
	int pmt_parsed_num = 0;

	service_num = 0;
	cur_pmt_parser_index = 0;

	memset(filter, 0, 12);
	memset(mask, 0, 12);

	filter[0] = 0x00;
	mask[0] = 0xff;

	while (1) {
		data_length = 1024;
		if (data_length == fread(section_buf, 1, data_length, ts_fp)) {
			pat_table_parse(section_buf, data_length);
			break;
		}
	}

	filter[0] = 0x02;
	mask[0] = 0xff;

	pmt_parsed_num = service_num;

	while (pmt_parsed_num >= 1) 
	{
		for (cur_pmt_parser_index = 0; cur_pmt_parser_index < service_num; cur_pmt_parser_index++) 
		{
			if (service[cur_pmt_parser_index].pmt_complete)
				continue;

			data_length = 1024;
			if (data_length == fread(section_buf, 1, data_length, ts_fp)) 
			{
					pmt_table_parse(section_buf, data_length);
					service[cur_pmt_parser_index].pmt_complete = 1;

					pmt_parsed_num--;

					break;
     			}
			else {
				printf("PMT---> ReadWait() timeout. \n");
			}
		}
	}

// DELME 	for (cur_pmt_parser_index = 0; cur_pmt_parser_index < service_num; cur_pmt_parser_index++) 
// DELME 	{
// DELME 		service_id = service[cur_pmt_parser_index].service_id;
// DELME 
// DELME 		service_pmtpid = service[cur_pmt_parser_index].service_pmtpid;
// DELME 		service_vidpid = service[cur_pmt_parser_index].service_vidpid;
// DELME 		service_audpid = service[cur_pmt_parser_index].service_audpid;
// DELME 	}

	return;
}
