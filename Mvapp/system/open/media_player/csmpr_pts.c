#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "csmpr_pts.h"

typedef unsigned char bool;

#define pts_buffer_num  2*1024*1024
#define pts_seek_unit 20*188

#define true 1
#define false 0
static  unsigned int total_pts = 0;

#define PAY_START      0x40
#define TRANS_PRIO     0x20


//flags
#define TRANS_SCRMBL1  0x80
#define TRANS_SCRMBL2  0x40
#define ADAPT_FIELD    0x20
#define PAYLOAD        0x10
#define COUNT_MASK     0x0F

#define TS_SIZE        	188

static ts_info_t		ts_info;
static int   		ReadBuff_Offset = 0;
static char 		find_ts_package_flag = 0;

//CSTVOUT_MODE Temp_TimingMode = TVOUT_MODE_576I ;////////////////
	
char Get_Pts(unsigned char *input_buffer,int lenth, int target_pid, unsigned int * pts ,char flag);
static char _find_ts_header(unsigned char *input_buffer );
static char _get_pts(unsigned char *input_buffer,ts_info_t *ts_info, unsigned int *ts_pts);
static char _get_pid(unsigned char *input_buffer, ts_info_t *ts_info);


#if 0
#define PLAY_BUFF_SIZE   	1316
char file_path[256] = "/home/Weidong/temp/Jade_1.ts";
#define VPID            0x32b
int main()
{
   int file_fd;
   unsigned int duration;

	//unsigned char st_data[PLAY_BUFF_SIZE];
	duration = PVR_get_file_duration( file_path, VPID);

	printf("=====duration = %d.\n", duration );
	
	file_fd = open(file_path, O_RDONLY);
	PVR_seek_position( 50, file_fd, VPID, duration );  //?50%в
	close( file_fd );
	//read_bytes = read(file_fd, st_data, PLAY_BUFF_SIZE);
	//CSDEMUX_CHL_DMA_Write(chl_handle, st_data, read_bytes);
return(0);	
}

#endif


unsigned int PVR_get_file_duration(char *file_path, int vid_pid ) 
{
  unsigned int duration;
   unsigned int first_pts;
   unsigned int last_pts;

   PVR_parse_pts( file_path, vid_pid, &first_pts, &last_pts );

   if( last_pts > first_pts )
   {
      duration = (last_pts - first_pts)/45000;
   } else {
      duration = (0xFFFFFFFF-first_pts+last_pts+1)/45000;   
   }

return( duration );
}


//  true/false.
bool PVR_parse_pts(char *file_path,int vid_pid, unsigned int *first_pts, unsigned int *last_pts)
{
	int  ts_file;
	bool result_flag=false;  //
	int seek_num = 1;
	int filelength;
	long tempmaxbuffer;
	if(file_path == NULL) 
	{
		printf("PVR_parsepts filepath invalid \n");
		return false;
	}
	ts_file = open(file_path,O_RDONLY);
	if(ts_file==-1)
	{
		printf("PVR_parsepts openfile failed [%s]\n" , file_path);
		return false;
	}
	
	while(1)
	{
		tempmaxbuffer = pts_buffer_num*seek_num;
		char *pts_buffer = (char *)malloc(tempmaxbuffer);
		if((filelength =read(ts_file,pts_buffer,tempmaxbuffer) )!= tempmaxbuffer)
		{
			if( Get_Pts(pts_buffer,filelength, vid_pid, first_pts ,0) )
			{
				result_flag = true;
				free(pts_buffer);
				break;
			}
			else
			{
				printf("PVR_parsepts file end not get first pts\n");
				result_flag = false;
				free(pts_buffer);
				break;
			}
		}
		else
		{
			if( Get_Pts(pts_buffer,pts_buffer_num, vid_pid, first_pts ,0) )
			{	
				result_flag = true;
				free(pts_buffer);
				break;
			}
			seek_num++;
		}
	}
	if(result_flag)
	{
		printf("PVR_parsepts get fisrt pts=%d",*first_pts);
		seek_num = 1;
		while(1)
		{
			tempmaxbuffer = pts_buffer_num*seek_num;
			char *pts_buffer = (char *)malloc(tempmaxbuffer);
			lseek(ts_file, -pts_buffer_num*seek_num, SEEK_END);
			if((filelength =read(ts_file,pts_buffer,tempmaxbuffer))!= tempmaxbuffer)
			{
				if( Get_Pts(pts_buffer,filelength, vid_pid, last_pts ,1) )
				{
					printf("PVR_parsepts get last pts=%d",*last_pts);
					result_flag = true;
					free(pts_buffer);
					//total_pts = *last_pts -*first_pts;
					break;
				}
				else
				{
					printf("PVR_parsepts file end not get last pts\n");
					result_flag = false;
					free(pts_buffer);
					break;
				}
			}
			else
			{
				if( Get_Pts(pts_buffer,pts_buffer_num, vid_pid, last_pts ,1) )
				{	
					printf("PVR_parsepts get last pts=%d",*last_pts);
					result_flag = true;
					free(pts_buffer);
					//total_pts = *last_pts -*first_pts;
					break;
				}
				seek_num++;
			}
		}
		close(ts_file);
	}
	else
	{
		close(ts_file);
		printf("PVR_parsepts cannot get first pts\n");
		return false;
	}
	printf(" leave PVR_parsepts\n");
	return result_flag;
}


#if 0
// ?1%????seek?λ
//??????λ. true/false.  seek50% pts_percentnum = 50 . 
bool PVR_seek_position(int pts_percentnum, int file_fd, int vpid, unsigned int duration )
{
	unsigned totle_pts = duration*45000;
	if (pts_percentnum > 100) pts_percentnum %= 100;
	//??PTS1%
	if( (pts_percentnum ==0)||(pts_percentnum ==100) )
	{
		lseek(file_fd, 0, SEEK_SET);
		return true;
	}
	unsigned int target_pts = (totle_pts/100)* pts_percentnum;
	//pts ????1/1000
	unsigned int pos_pts = totle_pts/1000; 
	int  temp_filepos = lseek(file_fd,0,SEEK_CUR);
	unsigned int filesize = lseek(file_fd,0,SEEK_END)-lseek(file_fd,0,SEEK_SET);
	printf("?С?filesize =%d\n",filesize);
	// ?pts?е?λ
	unsigned int pos = (int)((float)(filesize) * ((float)(pts_percentnum) / (float)(100)));
	unsigned int temp_pts = 0;
	unsigned int temp_position;
	char pts_buffer[pts_seek_unit];
	int filelength;
	bool result_flag=false; 
	printf("??pts=%d",target_pts);
	
	while(1)
	{
		temp_position = pos;
		lseek(file_fd, temp_position, SEEK_SET);

	repeat_search:
		read(file_fd,pts_buffer,pts_seek_unit); 
		if((filelength = read(file_fd,pts_buffer,pts_seek_unit) ) == pts_seek_unit)
		{
			 Get_Pts(pts_buffer,pts_seek_unit, vpid, &temp_pts ,0);
			if( (target_pts-pos_pts<temp_pts<target_pts+pos_pts)&&(temp_pts!=0) )
			{
				printf("? find a valid pts=%d",temp_pts);
				temp_position = lseek(file_fd, 0, SEEK_CUR);	
				result_flag = true;
				break;
				
			}
			else if(temp_pts>target_pts)
			{
				temp_position-=pts_seek_unit;
				lseek(file_fd, temp_position, SEEK_SET);
				goto repeat_search;

			}
			else if(temp_pts<target_pts)
			{
				temp_position+=pts_seek_unit;
				lseek(file_fd, temp_position, SEEK_SET);
				goto repeat_search;
			}
		}
		else
		{
			if( Get_Pts(pts_buffer,filelength, vpid, &temp_pts ,0) )
			{
				if( (target_pts-pos_pts<temp_pts<target_pts+pos_pts)&&(temp_pts!=0) )
				{
					printf("? find a valid pts=%d",temp_pts);
					temp_position = lseek(file_fd, 0, SEEK_CUR);	
					result_flag = true;
					break;
				}
				result_flag = false;
				printf("?!?? pts....\n");
			}
			else
			{
				printf("?!?? pts....\n");
				result_flag = false;
				break;
			}
		}	
	}
	// ??seek????
	if(!result_flag) pos = temp_filepos;
		else pos = temp_position;
	pos-=pos%188;
	lseek(file_fd, pos, SEEK_SET);
	return result_flag;
}
#endif

#if 0
// 搜索到1秒精度的文件偏移量，成功seek到目标位置
//失败还原到之前播放的位置. 返回true/false.
bool PVR_seek_PTS_position(int pts_target,unsigned int first_pts,unsigned int last_pts, int file_fd,int vpid)
{
	unsigned int  totle_pts = last_pts-first_pts;
	if((totle_pts <= 0)||(!file_fd)||(pts_target<=first_pts)||(pts_target>=last_pts)) return false;

	unsigned int target_pts = pts_target;
	unsigned int temp_pts = 0;
	unsigned int temp_position;
	char pts_buffer[pts_seek_unit];
	int filelength;
	bool result_flag=false; 
	bool leftflag =false;
	bool rightflag =false;
	int ff_cnt = 0;

	

	//搜索到的pts 允许误差为正负1秒
	unsigned int pos_pts = 45000;
	int  temp_filepos = lseek(file_fd,0,SEEK_CUR);
	unsigned int filesize = lseek(file_fd,0,SEEK_END)-lseek(file_fd,0,SEEK_SET);
	//printf("我算的文件大小为filesize =%d\n",filesize);
	// 搜索目标pts在文件中的初步位置
	unsigned int pos = 0;
	
	int a,b,c,d,e,f;

	f = filesize/1000;
	a = target_pts/45000;
	b = first_pts/45000;
	c = totle_pts/45000;
       d = f*(a - b);
	e = d/c;

	printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!a = %d,b = %d,c = %d,d = %d ,e = %d,f = %d\n",a,b,c,d,e,f);
	pos = e*1000;
	ff_cnt = 1;
	if(Temp_TimingMode == TVOUT_MODE_720P50||Temp_TimingMode == TVOUT_MODE_720P60)
	{
		ff_cnt = 4;		
	}

	if(Temp_TimingMode == TVOUT_MODE_1080I25 ||Temp_TimingMode == TVOUT_MODE_1080I30)
	{
		ff_cnt = 6;
	}

	temp_position = pos;
	
	printf("\n\n总PTS数量为totle_pts = %u,第一个pts=%u,最后一个pts=%u,filesize = %d, temp_position = %d\n",totle_pts,first_pts,last_pts,filesize,temp_position);
	printf("正在寻找的pts=%u\n",target_pts);
	while(1)
	{
		
		lseek(file_fd, temp_position, SEEK_SET);
	repeat_search: 
		if((filelength = read(file_fd,pts_buffer,pts_seek_unit) ) == pts_seek_unit)
		{
			 if(Get_Pts(pts_buffer,pts_seek_unit, vpid, &temp_pts ,0))
			 {
				if( (target_pts-pos_pts<=temp_pts)&&(temp_pts<=target_pts+pos_pts))
				{
					printf("恭喜你temp_position = %d, pts=%d\n",temp_position,temp_pts);
					temp_position = lseek(file_fd, 0, SEEK_CUR);	
					result_flag = true;
					//usleep((4/PVR_dev_obj.speed)*180000);
					break;
				}
				else if(temp_pts>target_pts+pos_pts)
				{
					rightflag = true;
					//printf("111111111111 temp_pts = %u\n",temp_pts);
					if(leftflag == true)
					{
				
						printf("find pts serious problem...death loop...............\n");
						break;
					}
					temp_position =temp_position - pts_seek_unit*ff_cnt;
					lseek(file_fd, temp_position, SEEK_SET);
					goto repeat_search;
				}  
				else if(temp_pts<target_pts-pos_pts)
				{
					leftflag = true;
					
	
					if(rightflag == true)
					{
					//	printf("222222222222 temp_pts = %u\n",temp_pts);
						printf("find pts serious problem.......death loop...........\n");
						break;
					}
					temp_position=temp_position + pts_seek_unit*ff_cnt;
					printf("222222222222 temp_position = %u,temp_pts = %d\n",temp_position,temp_pts);
					lseek(file_fd, temp_position, SEEK_SET);
					goto repeat_search;
				}
			 }
			 else
			 {
			 	printf("no pts area@@@@@@@@@@@@@@@\n");
				if(rightflag)	temp_position-=pts_seek_unit;
				else		temp_position+=pts_seek_unit;
				lseek(file_fd, temp_position, SEEK_SET);
				goto repeat_search;
			 }
		}
		else
		{
			if( Get_Pts(pts_buffer,filelength, vpid, &temp_pts ,0) )
			{
				if( (target_pts-pos_pts<=temp_pts)&&(temp_pts<=target_pts+pos_pts))
				{
					printf("恭喜你 find a valid pts=%u\n",temp_pts);
					temp_position = lseek(file_fd, 0, SEEK_CUR);	
					result_flag = true;
					//usleep((4/PVR_dev_obj.speed)*180000);
					break;
				}
				result_flag = false;
				printf("真失败!找不到你要的 pts....\n");
			}
			else
			{
				printf("真失败!找不到你要的 pts....\n");
				result_flag = false;
				break;
			}
		}	
	}
	// 失败返回seek之前的状态
	if(!result_flag) pos = temp_filepos;
		else pos = temp_position;
	pos-=pos%188;
	lseek(file_fd, pos, SEEK_SET);
	return result_flag;
}
#endif

// 搜索到总时间的1%精度的文件偏移量，成功seek到目标位置
//失败还原到之前播放的位置. 返回true/false.
bool PVR_seek_position(int pts_percentnum,int file_fd,int vpid,unsigned int temp_total_pts,unsigned int first_pts)
{
	unsigned int  totle_pts = temp_total_pts;
	if((temp_total_pts <= 0)||(!file_fd)) return false;

	unsigned int temp_pts = 0;
	unsigned int temp_position;
	char pts_buffer[pts_seek_unit];
	int filelength;
	bool result_flag=false; 
	bool leftflag =false;
	bool rightflag =false;
	
	printf("总PTS数量为totle_pts = %u\n",totle_pts);
	if (pts_percentnum > 100) pts_percentnum %= 100;
	//精确度为总PTS的1%
	if( (pts_percentnum ==0)||(pts_percentnum ==100) )
	{
		lseek(file_fd, 0, SEEK_SET);
		return true;
	}
	unsigned int target_pts = (totle_pts/100)* pts_percentnum+first_pts;
	if( (target_pts>=first_pts+totle_pts)||(target_pts<=first_pts)) return false;

	//搜索到的pts 允许误差为正负1秒
	//unsigned int pos_pts = totle_pts/100; 
	unsigned int pos_pts = 45000;
	int  temp_filepos = lseek(file_fd,0,SEEK_CUR);
	unsigned int filesize = lseek(file_fd,0,SEEK_END)-lseek(file_fd,0,SEEK_SET);
	//printf("我算的文件大小为filesize =%d\n",filesize);
	// 搜索目标pts在文件中的初步位置
	unsigned int pos = (int)((float)(filesize) * ((float)(pts_percentnum) / (float)(100)));
	printf("正在寻找的pts=%u,误差为pos_pts=%u\n",target_pts,pos_pts);
	
	while(1)
	{
		temp_position = pos;
		lseek(file_fd, temp_position, SEEK_SET);

	repeat_search:
		if((filelength = read(file_fd,pts_buffer,pts_seek_unit) ) == pts_seek_unit)
		{
			 if(Get_Pts(pts_buffer,pts_seek_unit, vpid, &temp_pts ,0))
			 {
				if( (target_pts-pos_pts<=temp_pts)&&(temp_pts<=target_pts+pos_pts))
				{
					printf("恭喜你 find a valid pts=%d\n",temp_pts);
					temp_position = lseek(file_fd, 0, SEEK_CUR);	
					result_flag = true;
					break;
				}
				else if(temp_pts>target_pts+pos_pts)
				{
					//printf("1111111\n");
						printf("111111111111 temp_pts = %u\n",temp_pts);
					rightflag = true;
					if(leftflag == true)
					{
						printf("111111111111 temp_pts = %u\n",temp_pts);
						printf("find pts serious problem...death loop...............\n");
						break;
					}
					temp_position-=pts_seek_unit;
					lseek(file_fd, temp_position, SEEK_SET);
					goto repeat_search;
				}
				else if(temp_pts<target_pts-pos_pts)
				{
					//printf("222222\n");
					printf("222222222222 temp_pts = %u\n",temp_pts);
					leftflag = true;
					if(rightflag == true)
					{
						printf("222222222222 temp_pts = %u\n",temp_pts);
						printf("find pts serious problem.......death loop...........\n");
						break;
					}
					temp_position+=pts_seek_unit;
					lseek(file_fd, temp_position, SEEK_SET);
					goto repeat_search;
				}
			 }
			 else
			 {
			 	printf("no pts area@@@@@@@@@@@@@@@\n");
				if(rightflag)	temp_position-=pts_seek_unit;
				else		temp_position+=pts_seek_unit;
				lseek(file_fd, temp_position, SEEK_SET);
				goto repeat_search;
			 }
		}
		else
		{
			if( Get_Pts(pts_buffer,filelength, vpid, &temp_pts ,0) )
			{
				if( (target_pts-pos_pts<=temp_pts)&&(temp_pts<=target_pts+pos_pts))
				{
					printf("恭喜你 find a valid pts=%d\n",temp_pts);
					temp_position = lseek(file_fd, 0, SEEK_CUR);	
					result_flag = true;
					break;
				}
				result_flag = false;
				printf("真失败!找不到你要的 pts....\n");
			}
			else
			{
				printf("真失败!找不到你要的 pts....\n");
				result_flag = false;
				break;
			}
		}	
	}
	// 失败返回seek之前的状态
	if(!result_flag) pos = temp_filepos;
		else pos = temp_position;
	pos-=pos%188;
	lseek(file_fd, pos, SEEK_SET);
	return result_flag;
}


//把当前pts保存到全局变量PVR_dev_obj, 返回true/false.
bool PVR_get_current_pts(int file_fd,int vpid, unsigned int *p_current_pts)
{
	bool result_flag=false;  //
	int seek_num = 1;
	int filelength;
	long tempmaxbuffer;
	#define pts_buffer_num 2*1024
	char *tempbuff = (char *)malloc(pts_buffer_num);
	int  temp_filepos = lseek(file_fd,0,SEEK_CUR);

	//pthread_mutex_lock(&PVR_dev_obj->PVR_filemutex);
	while(1)
	{
		
		if((filelength =read(file_fd,tempbuff,pts_buffer_num) )!= pts_buffer_num)
		{
			if( Get_Pts(tempbuff,filelength, vpid, p_current_pts ,0) )
			{
				printf("PVR_.....get current pts current_pts=%d",*p_current_pts);
				result_flag = true;
				break;
			}
			else
			{
				printf("PVR_parsepts file end not get current  pts\n");
				result_flag = false;
				break;
			}
		}
		else
		{
			if( Get_Pts(tempbuff,pts_buffer_num, vpid,p_current_pts,0) )
			{	
				printf("PVR_.....get current pts current_pts=%d",*p_current_pts);
				result_flag = true;
				break;
			}
			lseek(file_fd, -pts_buffer_num*2, SEEK_CUR);
		}
	}
	 free(tempbuff);
	 lseek(file_fd,temp_filepos,SEEK_SET);
	// pthread_mutex_unlock(&PVR_dev_obj->PVR_filemutex);
	return result_flag;
}

char Get_Pts(unsigned char *input_buffer,int lenth, int target_pid, unsigned int * pts ,char flag)
{
	unsigned int ts_pts = 0;
	
/****
for(i=0; i<50; i++)
	printf("%x ",input_buffer[i]);
printf("\n");
*****/

	if(flag)
		ReadBuff_Offset = lenth-600;	//min Lenth for get PTS
	else
		ReadBuff_Offset = 0;
//printf("--Get_Pts --In!  ReadBuff_Offset = %d\n",ReadBuff_Offset);

	find_ts_package_flag = 0;
	memset(&ts_info, 0,sizeof(ts_info_t));

	while(1)
	{
		if(_find_ts_header(&input_buffer[ReadBuff_Offset] ))
		{
			find_ts_package_flag = 1;
			if(_get_pid(&input_buffer[ReadBuff_Offset] , &ts_info))
			{
//printf("--pid = %d  ts_start_indicator = %d  offset=%d--Get!\n",ts_info.PID,ts_info.payload_unit_start_indicator,ReadBuff_Offset);
				if(ts_info.payload_unit_start_indicator && ts_info.PID == target_pid)
				{
					_get_pts(&input_buffer[ReadBuff_Offset] , &ts_info, &ts_pts);
					*pts = ts_pts;
					return true;
				}
				else
				{
					if(flag)
					{
						ReadBuff_Offset-=TS_SIZE;
						if(ReadBuff_Offset < 0)
						{
							printf("--_find_ts_header --no find--------!\n");
							return false;
						}
					}
					else
					{
						ReadBuff_Offset+=TS_SIZE;
						if(ReadBuff_Offset >= lenth)
						{
							printf("--_find_ts_header --no find--------!\n");
							return false;
						}
					}
					
					continue;
				}
			}
		}
		else
		{
			if(flag)
			{
				if(find_ts_package_flag == 0)
					ReadBuff_Offset-=1;
				else
					ReadBuff_Offset-=TS_SIZE;
				if(ReadBuff_Offset <0)
				{
					printf("--_find_ts_header --no find--------!\n");
					return false;
				}
			}
			else
			{
				if(find_ts_package_flag == 0)
					ReadBuff_Offset+=1;
				else
					ReadBuff_Offset+=TS_SIZE;
				if(ReadBuff_Offset >= lenth)
				{
					printf("--_find_ts_header --no find--------!\n");
					return false;
				}
			}
		}
	}
	return false;
}


static char _find_ts_header(unsigned char *input_buffer )
{

	if(input_buffer[0] == 0x47)
	{
		if(input_buffer[TS_SIZE] == 0x47)
		{
			if(input_buffer[TS_SIZE*2] == 0x47)
			{
				if(input_buffer[TS_SIZE*3] == 0x47)
				{
					return true;
				}
			}
		}
	}
	return false;
}


static char _get_pid(unsigned char *input_buffer, ts_info_t *ts_info)
{
	if(input_buffer[0] == 0x47)
	{
		ts_info->payload_unit_start_indicator = (input_buffer[1] & PAY_START)>>6;
		ts_info->transport_priority = (input_buffer[1] & TRANS_PRIO)>>5;
		ts_info->PID = ((input_buffer[1]&0x1f)<<8)|input_buffer[2];
		ts_info->transport_scrambling_control = (input_buffer[3] & (TRANS_SCRMBL1 | TRANS_SCRMBL2))>>6;
		ts_info->adaptation_field = (input_buffer[3] & ADAPT_FIELD)>>5;
		ts_info->payload_field = (input_buffer[3] & PAYLOAD)>>4;
		ts_info->continuity_counter = input_buffer[3] & COUNT_MASK;
		return true;
	}
	return false;
}

static char _get_pts(unsigned char *input_buffer,ts_info_t *ts_info, unsigned int *ts_pts)
{
 unsigned int  pts=0;
 unsigned int dts=0;
 int pes_offset_in_ts,pts_dts_flag;
 
 /* find pes offset(may pes packet start) in packet */
 pes_offset_in_ts = 4;
 if (ts_info->adaptation_field) 
 {
  pes_offset_in_ts += input_buffer[4] + 1;
 }
 
 input_buffer += pes_offset_in_ts; // offset to the start of pes in ts packet.
 pts_dts_flag = (input_buffer[7] >> 6);
 if (pts_dts_flag == 0x01)
 {
  printf("......_get_pts ......  That package no have pts!\n");
  return false;
 }
 /* pts */
 if (pts_dts_flag & 0x2)
 {
 
  pts = input_buffer[9]&0xe;
  pts <<= 7;
  pts |= input_buffer[10];
  pts <<= 7;
  pts |= input_buffer[11]>>1;
  pts <<= 8;
  pts |= input_buffer[12];
  pts <<= 6;
  pts |= input_buffer[13]>>2; // drop the lowest bit.
  
 }
 /* dts */
 if(pts_dts_flag & 0x1)
 {
  dts = input_buffer[14]&0xe;
  dts <<= 7;
  dts |= input_buffer[15];
  dts <<= 7;
  dts |= input_buffer[16]>>1;
  dts <<= 8;
  dts |= input_buffer[17];
  dts <<= 6;
  dts |= input_buffer[18]>>2;// drop the lowest bit.
 }
 *ts_pts = pts;
 printf("......_get_pts ......  pts = %d  ReadBuff_Offset=%d!\n",pts,ReadBuff_Offset);
 return true;
} 

