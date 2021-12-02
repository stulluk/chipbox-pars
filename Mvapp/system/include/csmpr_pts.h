
#ifndef _PTS_H_
#define _PTS_H_

typedef struct TS_Info_s{
	int transport_error_indicator;
	int payload_unit_start_indicator;
	int transport_priority;
	int PID;
	int transport_scrambling_control;
	int adaptation_field;
	int payload_field;
	int continuity_counter;	
}ts_info_t;

unsigned char PVR_parse_pts( char *file_path, int vid_pid, unsigned int *first_pts, unsigned int *last_pts);
unsigned int  PVR_get_file_duration(char *file_path, int vid_pid ) ;
//unsigned char PVR_seek_position(int pts_percentnum, int file_fd, int vpid, unsigned int duration );
unsigned char PVR_seek_PTS_position(int pts_target,unsigned int first_pts,unsigned int last_pts, int file_fd,int vpid);
unsigned char PVR_seek_position(int pts_percentnum,int file_fd,int vpid,unsigned int temp_total_pts,unsigned int first_pts);
unsigned char PVR_get_current_pts(int file_fd,int vpid, unsigned int *p_current_pts);

#endif

