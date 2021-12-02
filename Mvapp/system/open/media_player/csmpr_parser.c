#include <stdio.h>
#include <stdlib.h>
#include "crc.h"
#include "csmpr_common.h"
#include "av_zapping.h"
#include "csmpr_parser.h"

#define DEBUG_PRINT printf
#define SUCCESS        0
#define ERROR          (-1)

#define SYNC_BYTE      0x47
#define PAT_PID        0x0000
#define PAT_TABLE_ID   0x00
#define PMT_TABLE_ID   0x02

enum 
{
	VIDEO_MPEG1    = 0x01,
	VIDEO_MPEG2    = 0x02,
	VIDEO_MPEG4    = 0x10,
	VIDEO_H264     = 0x1b,
	AUDIO_MP2      = 0x03,
	AUDIO_MP3      = 0x04,
	AUDIO_AAC_ADTS = 0xf,
	AUDIO_AAC_LOAS = 0x11,
	AUDIO_AC3      = 0x81,
	AUDIO_HDMV_DTS = 0x82,
	AUDIO_DTS_HD   = 0x86,
};

static void print_section( const unsigned char *buf )
{
	int i;
	for( i=0; i<188; i++ )
	{
	   printf( "%02x, ", buf[i] );

	   if( (i%4)==(4-1) )
	   {
		   if( (i%8)==(8-1) )
		   {
			  printf( "\n" );
		   } else  {
			  printf( "- " );
		   }
	   }
	}
    printf( "\n" );

return;
}

static unsigned long CaculateSectionCrc( unsigned char* SectionData, unsigned int SectionLength  )
{
	unsigned long Crc;										    /* CRC */
	int i, j;													/* Loop Counter */	
	
	Crc = 0xffffffffL;											/* Clear CRC */
	
	for( i = 0 ; i < SectionLength ; i++ )                      /* Calcurate CRC */
	{			
		Crc = Crc ^ ( (unsigned long)SectionData[i] << 24 );
		
		for( j = 0 ; j < 8 ; j++ ) 
		{
			if( ( Crc & 0x80000000L) != 0 ) 
			{
				Crc = ( Crc << 1 ) ^ 0x04c11db7L;
			}
			else 
			{
				Crc = Crc << 1;
			}
		}
	}
	
	return( Crc );
}

static int parse_PAT_from188Packet(const unsigned char *buf, unsigned short *pmtID)
{
	unsigned char adap_flag, table_id;
	unsigned char *patptr,*payloadptr,*p;
	int adaplen;
	unsigned short progNO;
	unsigned short section_len;

	if (PAT_PID != (((buf[1]&0x1f)<<8)|buf[2]))
	{
		return ERROR;
	}
    
	print_section( buf );

	adap_flag = (buf[3]&0x30); 

	if (adap_flag == 0x10)
	{
		patptr = ((unsigned char*)buf)+4+1;
		adaplen = 0;
	}
	else if (adap_flag == 0x30)
	{
		adaplen = buf[4];
		patptr = ((unsigned char*)buf)+5+adaplen+1;
	}
	else
	{
		DEBUG_PRINT("[parse_PAT_from188Packet]no payload data\n");
		return ERROR;
	}
	
	table_id = patptr[0];
	section_len = (((patptr[1]&0x0f)<<8) | patptr[2]);
	printf( "section_len=%d.\n", section_len);
	
	if (PAT_TABLE_ID != table_id)
	{
		DEBUG_PRINT("[parse_PAT_from188Packet] it's not a PAT table\n");
		return ERROR;
	}

	DEBUG_PRINT("[parse_PAT_from188Packet] start Parse\n");
	payloadptr = patptr;
	p = payloadptr+8;

	while(p<buf+188)
	{
		//buf[13]<<8|buf[14];
		progNO = ((p[0])<<8)|(p[1]);

		if (progNO!=0)
		{
			//13 bits PMTID, if progNO=0 it's networkid 
			*pmtID = ((p[2]&0x1f)<<8)|p[3];
			DEBUG_PRINT("[parse_PAT_from188Packet] pmtPID=0x%0x\n", *pmtID);
			
            printf( "CRC=%08x\n", CaculateSectionCrc( patptr, section_len+3-4 ));			
            
			return SUCCESS;
		}
		//move to next program
		p+=4;
	}  
	
	return ERROR;
}

static int parse_PMT_from188Packet(const unsigned char *buf, unsigned short pmtID, AVParam_t *pAvparam)
{
	unsigned char adap_flag,table_id,Str_type;
	int adaplen,proglen,eslen;
	unsigned char *pmtptr,*payloadptr,*p;
	int vpflag=0, apflag=0;
	int vid_pid, aud_pid, pcr_pid;
	int vtype, atype;
	int offset;

	if (pmtID!=(unsigned short)(((buf[1]&0x1f)<<8)|buf[2]))
	{
		return ERROR;
	}

	adap_flag = buf[3]&0x30;
	if (adap_flag == 0x10)
	{
		pmtptr = ((unsigned char*)buf)+5;
		adaplen = 0;
	}
	else if (adap_flag == 0x30)
	{
		adaplen = buf[4]&0xff;
		pmtptr = ((unsigned char*)buf)+5+adaplen+1;
	}
	else
	{
		DEBUG_PRINT("[parse_PMT_from188Packet] no payload data\n");
		return ERROR;
	}
	
	table_id = pmtptr[0];
	if (PMT_TABLE_ID != table_id)
	{
		DEBUG_PRINT("[parse_PMT_from188Packet] it's not a pmt table\n");
		return ERROR;
	}

	DEBUG_PRINT("[parse_PMT_from188Packet] start parse\n");
	payloadptr = pmtptr;
	p = payloadptr+8;

	pcr_pid = (((p[0])&0x1f)<<8)|p[1];
	proglen = ((p[2]&0x0f)<<8)|p[3];
	//program_info_length
	p+=4+proglen;

	while(p<buf+188)
	{
		Str_type = p[0];
		// printf("PMT: Stream Type [0x%02X]\n", Str_type);

		if (!vpflag)
		{
			if ((VIDEO_MPEG1 == Str_type) || (VIDEO_MPEG2 == Str_type) || (VIDEO_H264 == Str_type) || (VIDEO_MPEG4 == Str_type))
			{
				vid_pid = ((p[1]&0x1f)<<8) | p[2];
				vpflag = 1;
				vtype = Str_type;
			}
		}
		if (!apflag)
		{
			if ((AUDIO_MP2 == Str_type) || (AUDIO_MP3 == Str_type) || (AUDIO_AC3 == Str_type) || (AUDIO_AAC_ADTS == Str_type) || (AUDIO_AAC_LOAS == Str_type))
			{
				aud_pid = ((p[1]&0x1f)<<8) | p[2];
				apflag = 1;
				atype = Str_type;
			}
			else if( 0x05 == Str_type || 0x06 == Str_type )
			{
			   eslen  = ((p[3]&0x0f)<<8) | p[4];
			   offset = 0;
			   while( offset < eslen )
			   {
               // Private stream.
               if( p[offset+5] == 0x6a )  // AC3_DESCRIPTOR
               {  
                  // printf("PMT: Get AC3 descriptor.\n");
      				aud_pid = ((p[1]&0x1f)<<8) | p[2];
      				apflag = 1;
      				atype = AUDIO_AC3;
      				break;
               }
               offset += p[offset+5+1]+2;
			   }			   
			}
			else if ((AUDIO_HDMV_DTS == Str_type) || (AUDIO_DTS_HD == Str_type))
			{
                  // printf("PMT: Get DTS Audio.\n");
				  aud_pid = ((p[1]&0x1f)<<8) | p[2];
				  apflag = 1;
				  atype = AUDIO_HDMV_DTS;
			}

		}
		
		if (vpflag && apflag)
		{
			pAvparam->pcr_pid = pcr_pid;
			pAvparam->audio_pid = aud_pid;
			pAvparam->video_pid = vid_pid;
			pAvparam->audio_type = atype;
			pAvparam->video_type = vtype;
				
			return SUCCESS;
		}

		eslen = ((p[3]&0x0f)<<8) | p[4];
		//stream info+ES_info_length
		p+=5+eslen;
	}
	
	DEBUG_PRINT("[parse_PMT_from188Packet] Maybe No Streamtype==(01|02|1B|10)&(03|04|81|0f|11) in the PMT Table\n");
	return ERROR;
}


CSMPR_RESULT CSMPR_ExtractAvParam( const char *filename, tCS_AV_PlayParams *pProgramInfo )
{
    CSMPR_RESULT Result = CSMPR_ERROR;
    AVParam_t    Avparam;
	FILE        *fp = NULL;    
	char         ptr[188];
	short        pmtpid = 0x1FFF;
	int          trycnt;

    DEBUG_PRINT( "CSMPR_ExtractAvParam()\n" );
    
	if( (fp = fopen( filename, "rb" )) == NULL )
	{
		DEBUG_PRINT( "[Get_AudioVideo_PID_from_TSfile] No file: %s\n", filename );
		return( CSMPR_ERROR );
	}

	while( !feof(fp) )
	{
		fread(ptr, 1, 188, fp);
		if( SYNC_BYTE != ptr[0] )
		{
			DEBUG_PRINT("[Get_AudioVideo_PID_from_TSfile] TS file Format Error!\n");
			fclose(fp);
		    return( CSMPR_ERROR );
		}

		if( SUCCESS==parse_PAT_from188Packet(ptr, &pmtpid) )
		{
			DEBUG_PRINT("parse PAT successful. pmtpid=%x\n", pmtpid);
			break;
		}
	}

	if( 0x1FFF == pmtpid )
	{
		DEBUG_PRINT("[Get_AudioVideo_PID_from_TSfile] No PAT in TS file; %s\n", filename);
		fclose(fp);        
		return( CSMPR_ERROR );
	}

	fseek( fp, 0, SEEK_SET );
    trycnt = 0;
	while( !feof(fp) &&  trycnt < 10000 )
	{
		fread( ptr, 1, 188, fp );
		if( SYNC_BYTE != ptr[0] )
		{
			DEBUG_PRINT("[Get_AudioVideo_PID_from_TSfile] TS file Format Error!\n");
			fclose(fp);
			return ERROR;
		}

		if( SUCCESS==parse_PMT_from188Packet(ptr, pmtpid, &Avparam) )
		{
			DEBUG_PRINT( "parse PMT successful. %d\n", trycnt);
		    DEBUG_PRINT( "vpid = 0x%x\n  vtype= %d\n  apid = 0x%x\n  atype= %d\n  pcrpid=0x%x\n",
    					  Avparam.video_pid, 
    					  Avparam.video_type, 		    
    					  Avparam.audio_pid, 
    					  Avparam.audio_type, 
    					  Avparam.pcr_pid );

            Result = CSMPR_SUCCESS;
			break;
		}
		trycnt++;
	}


    if( Result == CSMPR_SUCCESS )
    {
        pProgramInfo->Video_PID = Avparam.video_pid;
        pProgramInfo->Audio_PID = Avparam.audio_pid;
        pProgramInfo->PCR_PID   = 0x1fff;
        pProgramInfo->AudioMode = eCS_AV_STEREO_MODE_STEREO;
        
        switch( Avparam.video_type )
        {
          case 0x10:
          case 0x1b:
            pProgramInfo->VideoType = eCS_AV_VIDEO_STREAM_H264;
          break; //case 0x01: case 0x02:
          default:
            pProgramInfo->VideoType = eCS_AV_VIDEO_STREAM_MPEG2;
        }

        switch( Avparam.audio_type )
        {
          case 0x0f:
            pProgramInfo->AudioType = eCS_AV_AUDIO_STREAM_AAC;
          break;
          case 0x11:
            pProgramInfo->AudioType = eCS_AV_AUDIO_STREAM_LATM;
          break;      
          case 0x06:
          case 0x81:
            pProgramInfo->AudioType = eCS_AV_AUDIO_STREAM_AC3;
          break;
          case 0x82:
          case 0x86:
            pProgramInfo->AudioType = eCS_AV_AUDIO_STREAM_DTS;
          break;
          default:
            pProgramInfo->AudioType = eCS_AV_AUDIO_STREAM_MPEG2;
        }
    
    }
    else 
    {
	    DEBUG_PRINT("[Get_AudioVideo_PID_from_TSfile] No PMT in TS file; %s\n", filename);
    }

    fclose(fp);

return( Result );
}

#if 0
int main()
{
   int       Result;
   AVParam_t avparam;
   
   Result = Get_AudioVideo_PID_from_TSfile( "D:\\Celestial\\TS_Streams\\T_0SPB_000578_000_24128_20060628.ts", &avparam );

return( Result );
}
#endif

