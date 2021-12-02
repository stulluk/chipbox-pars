include <stdlib.h>
#include <stdio.h>
#include <malloc.h>

    //void copy_file(char *pSrcFileName,char * pDesFileName)
void copy_file(int argc, char *argv[]) 
{
	char scrip[256];
	if (argc < 2) {
		printf("invalid parameters, see help\n");
		return;
	}
	strcpy(scrip, "cp");
	strcat(scrip, " ");
	strcat(scrip, argv[1]);
	strcat(scrip, " ");
	strcat(scrip, argv[2]);
	system(scrip);
	return;
}


//      void del_file(char *pFileName)
void del_file(int argc, char *argv[]) 
{
	char scrip[256];
	if (argc < 2) {
		printf("invalid parameters, see help\n");
		return;
	}
	strcpy(scrip, "rm -f ");
	strcat(scrip, argv[1]);
	system(scrip);
}


//      void list_dir(char *pDirName)
void list_dir(int argc, char *argv[]) 
{
	char scrip[64];
	strcpy(scrip, "ls -a");
	system(scrip);
} 

    //void fs_disk(char *pDiskName)
void fs_disk(int argc, char *argv[]) 
{
	char scrip[64];
	strcpy(scrip, "fdisk -l");
	system(scrip);
} 

//      void fs_info(void)
void fs_info(int argc, char *argv[]) 
{
	printf("sorry! nothing to be display!\n");
} 

//      void mk_dir(char *pDirName)
void mk_dir(int argc, char *argv[]) 
{
	char scrip[256];
	if (argc < 2) {
		printf("invalid parameters, see help\n");
		return;
	}
	strcpy(scrip, "mkdir  ");
	strcat(scrip, argv[1]);
	system(scrip);
}


//      void rename_file(char *pOldName, char *pNewName)
void rename_file(int argc, char *argv[]) 
{
	char scrip[256];
	if (argc < 2) {
		printf("invalid parameters, see help\n");
		return;
	}
	strcpy(scrip, "mv ");
	strcat(scrip, argv[1]);
	strcat(scrip, " ");
	strcat(scrip, argv[2]);
	system(scrip);
}


//      void remove_dir(char *pDirName)         
void remove_dir(int argc, char *argv[]) 
{
	char scrip[256];
	if (argc < 2) {
		printf("invalid parameters, see help\n");
		return;
	}
	strcpy(scrip, "rm -rf ");
	strcat(scrip, argv[1]);
	system(scrip);
}


