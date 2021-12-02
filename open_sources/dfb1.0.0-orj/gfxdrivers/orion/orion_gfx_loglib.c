/*////////////////////////////////////////////////////////////////////////
// Copyright (C) 2006 Celestial Semiconductor Inc.
// All rights reserved
// ---------------------------------------------------------------------------
// FILE NAME        : orion_gfx_loglib.c
// MODULE NAME      : basic Logsystem Libary
// AUTHOR           : Jiasheng Chen
// ---------------------------------------------------------------------------
// [RELEASE HISTORY]                           Last Modified : 06-10-25
// VERSION  DATE       AUTHOR                  DESCRIPTION
// 0.2      06-10-25   jiasheng Chen           Original
// ---------------------------------------------------------------------------
// [DESCRIPTION]
// 
// ---------------------------------------------------------------------------
// $Id: 
///////////////////////////////////////////////////////////////////////*/

#include "orion_gfx_loglib.h"
#include "orion_gfx_file_func.h"
#include <stdarg.h>
#include <stdio.h>
//initial Trace

#if GEN_PRINT_TO_FILE
static FILE *fpGeneralPrint = NULL;
#endif
#if ERR_TRACE_TO_FILE
static FILE *fpErrTrace = NULL;
#endif

int InitTrace()//Open File //Boolean Type Return
{
#if GEN_PRINT_TO_FILE
	fpGeneralPrint = FileOpenPlus(GEN_PRINT_FILE_PATH, "w+");
	if (fpGeneralPrint == NULL)
	{
		return 1;
	}
#endif

#if ERR_TRACE_TO_FILE
#if ERR_TRACE_TO_GEN_PRINT_FILE
	fpErrTrace = fpGeneralPrint;
#else
	fpErrTrace = FileOpenPlus(ERR_TRACE_FILE_PATH, "w+");
#endif
	if (fpErrTrace == NULL)
	{
		return 1;
	}

#endif
	return 0;
}
int EndTrace() //Close File
{
#if GEN_PRINT_TO_FILE
	if (fpGeneralPrint != NULL)
	{
		fclose(fpGeneralPrint);
	}
#endif

#if ERR_TRACE_TO_FILE
	if (fpErrTrace != NULL)
	{
		fclose(fpErrTrace);
	}	
#endif
	return 0;
	
}

//Error Trace Function

void ErrTrace(char *fmt, ...)
{
	va_list arglist;
	va_start(arglist, fmt);
#if ERR_TRACE_TO_FILE
	if (fpErrTrace != NULL)
	{
		vFileTrace(fpErrTrace, fmt, arglist);	
	}
	#if ERR_TRACE_STEP_FLUSH
	fflush(fpErrTrace);
	#endif
#endif
#if ERR_TRACE_TO_STD_OUT
	vFileTrace(stdout, fmt, arglist);	
#endif
#if (PAUSE_WHEN_ERR)
	printf("\npress any key to continue...\n");
	getchar();
	
#endif
	va_end(arglist);	
}
//General Printf Function
void Print(char *fmt, ...)
{
	va_list arglist;
	va_start(arglist, fmt);
#if GEN_PRINT_TO_FILE
	if (fpGeneralPrint!= NULL)
	{
		vFileTrace(fpGeneralPrint, fmt, arglist);	
	}
	#if GEN_PRINT_STEP_FLUSH
	fflush(fpGeneralPrint);
	#endif
#endif
#if GEN_PRINT_TO_STD_OUT
	vFileTrace(stdout, fmt, arglist);	
#endif
	va_end(arglist);		
}

