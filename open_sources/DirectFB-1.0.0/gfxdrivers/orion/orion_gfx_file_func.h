#ifndef _FILE_FUNC_H_
#define _FILE_FUNC_H_
#include <stdio.h>
#include <stdarg.h>

enum 
{
	MAX_DIR_LEN = 1000
};

typedef enum 
{
	DIR_EXIST     =  1,
	DIR_NOT_EXIST =  0,
	UNKOW_ERR     = -1
	
} DIR_ERR_NUM_TYPE;

FILE * FileOpenPlus(const char * FileName, const char *Mode );//Creat a un-exits Path before open a File
//return str1 and str2 not replace Str1
char *StrCat(char *str1, char *str2);

void FileTrace(FILE *stream , char *fmt, ...);
void vFileTrace( FILE *stream,char *fmt, va_list arglist);

#endif
