#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <ctype.h>
#include <pthread.h>
#include "linuxos.h"
#include "csmpr_usb.h"

//#define _TEST_

//=============================================================================
#define USB_MOUNT_DIR			"/mnt/usb/disk"
#define PROC_SCSI_USB_DEV_PATH  "/proc/scsi/usb-storage"
#define PROC_USB_CONN_STATUS    "/proc/usb_conn_status"


//=============================================================================

static UsbStatus_t 		lv_UsbConStatus = USB_STATUS_UNKNOWN;
static pthread_t  		hUsbTaskHandle;
static void 			(*lv_pfNotifier)( UsbStatus_t status ) = NULL;
static int 				mount_fail_cnt=0;
static char				acUsbDevFileName[10][32];
char *MMOUNT = "/proc/mounts";

//=============================================================================

#ifdef _TEST_
static int UsbCon_ListDir( void );
#endif

//=============================================================================

MOUNTP *dfopen(void)
{
	MOUNTP 		*MP;

	// /proc/mounts ??? ??.
	MP = (MOUNTP *)malloc(sizeof(MOUNTP));
	if(!(MP->fp = fopen(MMOUNT, "r")))
	{
		return NULL;
	}
	else
		return MP;
}

MOUNTP *dfget(MOUNTP *MP)
{
	char 			buf[256];
	struct 	statfs 	lstatfs;
	struct 	stat 	lstat; 
	int 			is_root = 0;

    // /proc/mounts? ?? ???? ???? ??? ????.
	while(fgets(buf, 255, MP->fp))
	{
		is_root = 0;
		sscanf(buf, "%s%s%s",MP->devname, MP->mountdir, MP->fstype);
		
		if (strcmp(MP->mountdir,"/") == 0) 
			is_root=1;
		
		if (stat(MP->devname, &lstat) == 0 || is_root)
        {
			if ((strstr(buf, MP->mountdir) && S_ISBLK(lstat.st_mode)) || is_root)
			{
				// ?????? ? ??? ??? ???? ???.
				statfs(MP->mountdir, &lstatfs);
				
				//printf("\n======= %u - %u %d=======\n\n", lstatfs.f_blocks, lstatfs.f_bsize, sizeof(long));
				
				MP->size.blocks = lstatfs.f_blocks * (lstatfs.f_bsize/1024); 
				MP->size.avail  = lstatfs.f_bavail * (lstatfs.f_bsize/1024); 
				return MP;
			}
		}
	}
	rewind(MP->fp);
	return NULL;
}

int dfclose(MOUNTP *MP)
{
	fclose(MP->fp);
	return 0;
}

int MV_Get_USB_Format_Type(int i)
{
	MOUNTP 		*MP;
	
	if ((MP=dfopen()) == NULL)
	{
		perror("error");
		return 1;
	}

	while(dfget(MP))
	{
		if ( strcmp(stPartition_data[i].acPartition_Dir, MP->mountdir) == 0 )
			strncpy(stPartition_data[i].acPartition_Type, MP->fstype, strlen(MP->fstype));
	}
	dfclose(MP);
	return 0;
}
//=============================================================================

int UsbCon_Init( void (*pfNotifier)( UsbStatus_t status ) )
{
	printf( "[USB_CON]:UsbCon_Init().\n");
	printf( "[USB_CON]:Creating usbcon thread.\n");

	lv_pfNotifier = pfNotifier;

	pthread_create( &hUsbTaskHandle, NULL, UsbCon_Task, NULL );

	return( 0 );
}


void *UsbCon_Task( void *param )
{
	while(1)
	{
		switch( lv_UsbConStatus )
		{

			case USB_STATUS_UNMOUNTED:
			case USB_STATUS_UNMOUNT_FAILED:
				if( UsbCon_DetectConnection() == 0 )
				{
					lv_UsbConStatus = USB_STATUS_DISCONNECT;
//					lv_pfNotifier( USB_STATUS_DISCONNECT );
				}
				break;

			case USB_STATUS_UNKNOWN:
			case USB_STATUS_DISCONNECT:
			case USB_STATUS_MOUNT_FAILED:
			//Detect USB connection:
				if( UsbCon_DetectConnection() == 1 )
				{
					usleep( 200*1000 );
					
					if( UsbCon_DetectConnection() == 1 )
					{
						// new connected:
						//printf( "[USB_CON]:Usb connected.\n" );

						//printf( "[USB_CON]:wait 2 second.\n" );
						usleep( 2*1000*1000 );

						if( UsbCon_DetectConnection() == 1 )
						{
							//printf( "[USB_CON]:Usb connected. Second \n" );
							
							lv_UsbConStatus = USB_STATUS_CONNECTED;
							//SEND_MESSAE( USB_STATUS_CONNECTED );
							if( lv_pfNotifier != NULL )
							{ 
								lv_pfNotifier( USB_STATUS_CONNECTED );
							}
														
							if( UsbCon_Mount() == 0 )
							{  
								// Mount success !!!
								lv_UsbConStatus = USB_STATUS_MOUNTED;
								if( lv_pfNotifier != NULL )
								{ 
									lv_pfNotifier( USB_STATUS_MOUNTED );
								}
							} 
							else 
							{
								// Mount Failed !!!
								lv_UsbConStatus = USB_STATUS_MOUNT_FAILED;
								mount_fail_cnt++;
								if( mount_fail_cnt>=3 )
								{
									mount_fail_cnt = 0;
									if( lv_pfNotifier != NULL )
									{ 
										lv_pfNotifier( USB_STATUS_MOUNT_FAILED );
									}
								}
							}
						}
					}
				}
				break;
			case USB_STATUS_CONNECTED:
			case USB_STATUS_MOUNTED:
				//Detect USB connection:
				if( UsbCon_DetectConnection() == 0 )
				{
					usleep( 200*1000 );
					if( UsbCon_DetectConnection() == 0 )
					{
						// new connected:
						//printf( "[USB_CON]:Usb disconnected.\n" );	
						lv_UsbConStatus = USB_STATUS_DISCONNECT;

						//printf( "[USB_CON]:wait 2 second.\n" );
						usleep( 2*1000*1000 );

						UsbCon_Umount();

						if( lv_pfNotifier != NULL )
						{ 
							lv_pfNotifier( USB_STATUS_DISCONNECT );
						}
					}

				}

				break;	
		}
		usleep( 2*1000*1000 );
	};

	return( NULL );
}

//-----------------------------------------------------------------------------
// 	FUNCTION:   UsbCon_DetectConnection()
//	DESRIPTION: Detect if usb device is connected.
// 	RETURN:     1, if connected;
// 	            0, if disconnected.
//	            -1,if error encountered.
//-----------------------------------------------------------------------------
int UsbCon_DetectConnection( void )
{	
	FILE *fp1 = NULL;
	FILE *fp2 = NULL;
	FILE *fp3 = NULL;
	FILE *fp4 = NULL;
	FILE *fp5 = NULL;
	
	//char ConnectStatus[2]; Ben comment out ettim...
	
	//printf( "[USB_CON]:UsbCon_DetectConnection().\n" );
	
	if( (fp1=fopen( "/dev/sda", "rb" )) == NULL && (fp2=fopen( "/dev/sdb", "rb" )) == NULL && (fp3=fopen( "/dev/sdc", "rb" )) == NULL && (fp4=fopen( "/dev/sdd", "rb" )) == NULL && (fp5=fopen( "/dev/sde", "rb" )) == NULL )
	{   
	  //printf("Hepsi NULL dondurdu...\nreturn 0\n");
	  return 0; //disconnected
	}
	else{
	  if ( fp1 ){
	    //printf("fp1 NULL degil, close edildi!!\n");
	    fclose( fp1 );
	  }
	  if ( fp2 ){
	    //printf("fp2 NULL degil, close edildi!!\n");
	    fclose( fp2 );
	  }
	  if ( fp3 ){
	    //printf("fp3 NULL degil, close edildi!!\n");
	    fclose( fp3 );
	  }
	  if ( fp4 ){
	    //printf("fp4 NULL degil, close edildi!!\n");
	    fclose( fp4 );
	  }
	  if ( fp5 ){
	    //printf("fp5 NULL degil, close edildi!!\n");
	    fclose( fp5 );
	  }

	  return 1;
	}
	
	
	
	/* Buradan itibaren de?i?tirdim...
	
	if( (fp=fopen( PROC_USB_CONN_STATUS, "r" )) == NULL )
	{   return( -1 ); //Error
	}
	
	
	
	if( fread( &ConnectStatus, 1, 1, fp ) != 1 )
	{   fclose( fp );		
		return( -1 ); //Error
	}	
	
	fclose( fp );
		
	if( ConnectStatus[0] == '1' )
	{   return( 1 );
	}
	else
	{   return( 0 );
	}
	
	******/
}

//-----------------------------------------------------------------------------
// 	FUNCTION:   UsbCon_Mount()
//	DESRIPTION: Mount the USB storage device
// 	RETURN:     0 success.
//			    -1 failed.
//-----------------------------------------------------------------------------
int UsbCon_Mount( void )
{
	int  	nTry;
	int		Success_Count = 0;
	int  	nUsbDevID;
	char 	ShellCommand[64];
	int  	Result; //temp

	printf( "[USB_CON]:UsbCon_Mount().\n" );	

/*	
	for( nTry=1; nTry<=3; nTry++ )
	{
	    nUsbDevID = UsbCon_GetUsbDevID();

		if( nUsbDevID >= 0 )
			break;
		else	
			printf( "[USB_CON]:Error, Failed to get USB device id. nTry=%d\n", nTry );					

	    sleep( 1 );	    
	}
		
	if( nUsbDevID < 0 )
	{
		printf( "[USB_CON]:Error, Failed to get USB device id.nTry=%d\n", nTry );		
		return( -1 );
	}
*/	
	//wait USB enumation
	printf( "[USB_CON]:wait 3 second.\n" );
	sleep( 3 );
	
	for( nTry=1; nTry<=3; nTry++ )
	{
		printf( "[USB_CON]:Mounting USB Try count=%d.\n", nTry );					
		printf("-###---nUsbDevID = %d---------\n",nUsbDevID);
        // 1021

		memset(&acUsbDevFileName, 0x00, 32*10 );
		memset(&stPartition_data, 0x00, sizeof(stPartition) * PARTITION_NUM );
		acUsbDevCount = 0;
		
		UsbGetDevFileName();

		if( acUsbDevCount != 0 )
		{
			int		j = 0;
			char	Temp_Str[100];

			for( j = 0 ; j < acUsbDevCount ; j++ )
			{				
				sprintf(Temp_Str, "%s%d", USB_MOUNT_DIR, j+1 );
				if( access( Temp_Str, 0 ) != 0 )	
				{
				 	sprintf( ShellCommand, "mkdir %s", Temp_Str );	
			 		//printf( "[USB_CON]: %s.\n", ShellCommand );
				 	Result = system( ShellCommand );	
				}

				sprintf( ShellCommand, "ntfs-3g /dev/%s %s", acUsbDevFileName[j], Temp_Str );//New  kernel mount
					
		   		printf( "[USB_CON] NTFS : %s\n", ShellCommand );
		   		
		   		if( system( ShellCommand ) == 0 )
				{
					printf( "  [USB_CON] NTFS : mount success. TryCount=%d.\n", nTry );
						
					sprintf(stPartition_data[j].acPartition_Data, "%s", acUsbDevFileName[j]);
					sprintf(stPartition_data[j].acPartition_Dir, "%s", Temp_Str);
					MV_Get_USB_Format_Type(j);
					
					Success_Count++;

					if ( Success_Count == acUsbDevCount )
			   			return( 0 );
				} else {
					sprintf( ShellCommand, "mount -t vfat /dev/%s %s", acUsbDevFileName[j], Temp_Str );//New  kernel mount
					
			   		printf( "[USB_CON] VFAT : %s\n", ShellCommand );
			   		
			   		if( system( ShellCommand ) == 0 )
			   		{
			   			printf( "  [USB_CON] VFAT : mount success. TryCount=%d.\n", nTry );
						
	//					sprintf(stPartition_data[j].acPartition_Type , "Fat or NTFS");
						sprintf(stPartition_data[j].acPartition_Data, "%s", acUsbDevFileName[j]);
						sprintf(stPartition_data[j].acPartition_Dir, "%s", Temp_Str);
						MV_Get_USB_Format_Type(j);
						
						Success_Count++;

						if ( Success_Count == acUsbDevCount )
				   			return( 0 );
			   		}
			   		else 
			   		{
						printf( "  [USB_CON] VFAT : Try again with no parameter.\n" );
						
						sprintf( ShellCommand, "mount /dev/%s %s", acUsbDevFileName[j], Temp_Str );//New  kernel mount
						
						printf( "[USB_CON] EXT : %s\n", ShellCommand );

						if( system( ShellCommand ) == 0 )
				   		{
							printf( "  [USB_CON] EXT : mount success. TryCount=%d.\n", nTry );

	//						sprintf(stPartition_data[j].acPartition_Type , "Ext2");
							sprintf(stPartition_data[j].acPartition_Data, "%s", acUsbDevFileName[j]);
							sprintf(stPartition_data[j].acPartition_Dir, "%s", Temp_Str);
							MV_Get_USB_Format_Type(j);
							
							Success_Count++;

							if ( Success_Count == acUsbDevCount )
				   			   return( 0 );
			   			} else {
			   				printf( "  [USB_CON] EXT : Try again with no parameter.\n" );
			   			}
			   		}
				}
			}
		}
		printf( "[USB_CON]:wait 1 second.\n" );
		sleep( 1 );			
	}
	
	UsbCon_Umount();
	return( - 1);
}	

//-----------------------------------------------------------------------------
// 	FUNCTION:   UsbCon_Umount()
//	DESRIPTION: umount the USB storage device
// 	RETURN:     0 success.
//			    -1 failed.
//-----------------------------------------------------------------------------
int UsbCon_Umount( void )
{
	char 	ShellCommand[64];
	int  	Result = 0; 
	int		i;

	printf( "[USB_CON]:UsbCon_Umount().\n" );	
	//	umount
	//	Result = system( "pwd" );	
	//	Result = system( "cd /" );

	for ( i = 0 ; i < acUsbDevCount ; i++ )
	{
	 	sprintf( ShellCommand, "umount %s", stPartition_data[i].acPartition_Dir);
	 	printf( "%s\n",ShellCommand);
	 	Result = system( ShellCommand );
		sprintf( ShellCommand, "rm -rf %s", stPartition_data[i].acPartition_Dir );
		Result = system( ShellCommand );
	}

	if ( Result == 0 )
		lv_UsbConStatus = USB_STATUS_UNMOUNTED;

	mount_fail_cnt = 0;
	
	memset(&acUsbDevFileName, 0x00, 32*10 );
	memset(&stPartition_data, 0x00, sizeof(stPartition) * PARTITION_NUM );
	
	return( Result );
}	


//-----------------------------------------------------------------------------
// 	FUNCTION:   UsbCon_GetUsbDevID()
//	DESRIPTION: Get the USB device id
// 	RETURN:     return the USB device id, -1 for error.
//				
//-----------------------------------------------------------------------------
int UsbCon_GetUsbDevID( void )
{
	int  nUsbDevID;
	DIR  *pDir = NULL;
	struct dirent *pDirEnt;

	printf( "[USB_CON]:UsbCon_GetUsbDevID().\n" );

	if( (pDir=opendir( PROC_SCSI_USB_DEV_PATH )) == NULL )
	{   printf( "[USB_CON]:Failed to open dir.\n" );
		return( -1 );
	}

	while( (pDirEnt=readdir( pDir )) != NULL )
	{
		if( strcmp( pDirEnt->d_name, "." ) == 0  ||
		    strcmp( pDirEnt->d_name, "..") == 0 )
			continue ;
		else
			break;
	}

	if( pDirEnt == NULL )
	{   printf( "[USB_CON]:Empty dir?\n" );
		closedir( pDir );
		return( -1 );
	}
	
	nUsbDevID = atoi( pDirEnt->d_name );	
	closedir( pDir );	
	printf( "[USB_CON]:USB device ID =%d.\n", nUsbDevID );
	return( nUsbDevID );	
}


//-----------------------------------------------------------------------------
// 	FUNCTION:   UsbCon_GetStatus()
//	DESRIPTION: 
// 	RETURN:     
//				
//-----------------------------------------------------------------------------
UsbStatus_t UsbCon_GetStatus( void )
{
   return( lv_UsbConStatus );
}


//-----------------------------------------------------------------------------
// 	FUNCTION:   GetUsbDevFileName()
//	DESRIPTION: 
// 	RETURN:     
//				
//-----------------------------------------------------------------------------
char *UsbGetDevFileName( void )
{
	DIR    *pDir = NULL;
	struct dirent *pDirEnt;
	//static char    acUsbDevFileName[32];
	unsigned char  bGotDevFileName = 0;

	printf( "[USB_CON]:GetUsbDevFileName().\n" );

	if( (pDir=opendir( "/dev" )) == NULL )
	{
		printf( "[USB_CON]:GetUsbDevFileName(): Failed to open dir.\n" );
		return( NULL );
	}

	acUsbDevFileName[acUsbDevCount][0] = '\0';

	while( (pDirEnt=readdir( pDir )) != NULL )
	{
		if( pDirEnt->d_name[0] == 's' && pDirEnt->d_name[1] == 'd' )
		{
			bGotDevFileName = 1;
			if( pDirEnt->d_name[3] > '0' && pDirEnt->d_name[3] < '9' )
			{
//				printf("===== %s =======\n", pDirEnt->d_name);
				sprintf(acUsbDevFileName[acUsbDevCount], "%s", pDirEnt->d_name);
				acUsbDevCount++;
			}
		}
	}

	closedir( pDir );	

	if( bGotDevFileName )
	{
		printf( "[USB_CON]:GetUsbDevFileName(): Success. %d File => 1st DevFileName:%s.\n", acUsbDevCount, acUsbDevFileName[0] );
		//return( acUsbDevFileName );
		return( NULL );
	} else {   
		printf( "[USB_CON]:GetUsbDevFileName(): Failed to get dev file name.\n" );
		return( NULL );
	}
}


#ifdef _TEST_
//-----------------------------------------------------------------------------
// 	FUNCTION:   UsbCon_ListDir()
//	DESRIPTION: for test
// 	RETURN:     
//				
//-----------------------------------------------------------------------------
int UsbCon_ListDir( void )
{
	DIR    *pDir = NULL;
	struct dirent *pDirEnt;

	printf( "[USB_CON]:UsbCon_ListDir().\n" );

	if( (pDir=opendir( USB_MOUNT_DIR )) == NULL )
	{   printf( "[USB_CON]:UsbCon_ListDir(): Failed to open dir.\n" );
		return( -1 );
	}

	while( (pDirEnt=readdir( pDir )) != NULL )
	{
		if( strcmp( pDirEnt->d_name, "." ) != 0 &&
		    strcmp( pDirEnt->d_name, "..") != 0 )
		{
			printf( "-->%s\n", pDirEnt->d_name );
		}	
	}

	closedir( pDir );	
	return( 0 );
}
#endif


// _TEST_
#ifdef _TEST_
int main( void )
{
	printf( "[USB_CON]:USB auto mount test.\n" );
	
	UsbCon_Init();
	
	while( 1 )
	{
		usleep( 100*1000*1000 );
	}
}
#endif

