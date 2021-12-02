/*////////////////////////////////////////////////////////////////////////
// Copyright (C) 2006 Celestial Semiconductor Inc.
// All rights reserved
// ---------------------------------------------------------------------------
// FILE NAME        : file_func.c
// MODULE NAME      : file func lib
// AUTHOR           : Jiasheng Chen
// ---------------------------------------------------------------------------
// [RELEASE HISTORY]                           Last Modified : 06-10-25
// VERSION  DATE       AUTHOR                  DESCRIPTION
// 0.2      06-10-25   jiasheng Chen           Original
// ---------------------------------------------------------------------------
// [DESCRIPTION]
// Enhanced File Operation Funtion support Win32 and Linux
// ---------------------------------------------------------------------------
// $Id: 
///////////////////////////////////////////////////////////////////////*/



#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#ifdef WIN32
	#include <direct.h>
#else //linux
	#include <fcntl.h>
	#include <unistd.h>
	#include <sys/stat.h>
	#include <sys/types.h>
#endif
#include <errno.h>

#include "orion_gfx_file_func.h"


int CreatDir( const char *Dir)
{
#ifdef WIN32	
	if (_mkdir( Dir) == 0)
#else
	if (mkdir( Dir, S_IRWXU|S_IRWXG|S_IRWXO ) == 0)
#endif
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

int DirExist(const char *Dir) //need add More detail Erro Define here
{
	char* buffer;

	char *(*GetCwd)(char *, int );
//	char *(*GetCwd)(char *, unsigned int );

	int (*ChangeDir)( const char * );
#ifdef WIN32
	GetCwd    = _getcwd;
	ChangeDir = _chdir; 
#else
	GetCwd = getcwd;
	ChangeDir = chdir;
#endif
    // Get the current working directory:
	if( (buffer = GetCwd( NULL, 0 )) == NULL )
	{
		printf( "Unable to Get the Current Working Directory\n" );
		return UNKOW_ERR;
	}
	else
	{
		if( ChangeDir( Dir ) )
		{
			if (errno == ENOENT)
			{
				printf( "Unable to locate the directory, should be Created: %s\n", Dir );
				return DIR_NOT_EXIST;
			}
		}
		if( ChangeDir( buffer ) )
		{
			if (errno == ENOENT)
			{
				printf( "Unable to locate the directory: %s\n", buffer );
				return UNKOW_ERR;
			}
		}
		return DIR_EXIST;
	}	         
}


int PreProcessUpDir( const char * FileName, int FileNameLen) // Create the UP directory of the FileName
{
	char UpDir[MAX_DIR_LEN];
	int i = 0, LastExitDirPos = -1;
	
	if ( FileNameLen > MAX_DIR_LEN)
	{
		printf("\nERR:FileName %s too long\n", FileName);
		return 0;
	}
	else
	{
		strncpy(UpDir, FileName, FileNameLen);
		UpDir[FileNameLen]= 0;
	}
	
	for (i = FileNameLen -1; i >=0 ; i --)
	{
		char c = UpDir[i]; 
		if (( c == '/') || ( c == '\\'))
		{	
			int ErroNum ;
			UpDir[i] = 0;
			ErroNum = DirExist(UpDir);
			if ( ErroNum == DIR_NOT_EXIST) // return 1 means dir Exist
			{
				UpDir[i] = c;
			}
			else if (ErroNum == DIR_EXIST)
			{
				UpDir[i] = c;
				LastExitDirPos = i;
				break;
			}
			else 
			{
				//printf("Invalid Input FileName %s\n", UpDir);
				return 0;
			}
		}
	}
	if ( LastExitDirPos > 0 )  //"\\FileName" or "/FileName" is Invalid Path
	{
		for ( i = LastExitDirPos + 1; i < FileNameLen; i++)
		{
			char c = UpDir[i]; 
			if (( c == '/') || ( c == '\\'))
			{
				UpDir[i] = 0;
				if (CreatDir(UpDir))
				{
					UpDir[i] = c;				
				}
				else
				{
					printf("Failed to Creat Dir %s", UpDir);
					return 0;
				}
			}			
		}
	}
	else if ( i > -1)  //i = -1 : No \\ or / exist, pure FileName
	{
		printf("Invalid Input FileName %s\n", FileName);
		return 0;
	}
	return 1;
	
}

FILE * FileOpenPlus(const char * FileName, const char *Mode )
{
	if (PreProcessUpDir(FileName, strlen(FileName)))
	{
		return fopen( FileName, Mode);
	}
	else
	{
		return NULL;
	}
}
//return str1 and str2 not replace Str1
char *StrCat(char *str1, char *str2)
{
	static char str[1000];
	int len = 1000, len1 = 0, len2 =0;
	len1 = strlen(str1);
	len2 = strlen(str2);
	strncpy(str, str1, len-1 );
	strncpy(str+len1, str2, ((len-1) > len1 )? (len -len1-1): 0);
	str[len-1] = 0;
	return str;
}


void FileTrace(FILE *stream , char *fmt, ...)
{

	va_list arglist;
	va_start(arglist, fmt);
	if (stream != NULL)
	{
		vFileTrace(stream,fmt, arglist);
	}
	va_end(arglist);
	
}

void vFileTrace( FILE *stream,char *fmt, va_list arglist)
{
	if (stream != NULL)
	{
		vfprintf(stream,fmt, arglist);
		fflush(stream);
	}
}

