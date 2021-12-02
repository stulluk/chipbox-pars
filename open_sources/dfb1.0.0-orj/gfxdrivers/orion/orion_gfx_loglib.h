#ifndef _ORION_GFX_LOGLIB_H_
#define _ORION_GFX_LOGLIB_H_

//Trace Configuration
#define ERR_TRACE_ENABLE             (0)
#if (ERR_TRACE_ENABLE)
	#define PAUSE_WHEN_ERR           (0)
	#define ERR_TRACE_LEVEL          (3)
	#define ERR_TRACE_TO_FILE        (0)
	#if (ERR_TRACE_TO_FILE)
		#define ERR_TRACE_FILE_PATH   "ErrTrace.log"
		#define ERR_TRACE_STEP_FLUSH (1)
	#endif
	#define ERR_TRACE_TO_STD_OUT     (1)
	#define ERR_TRACE_TO_GEN_PRINT_FILE (1)	
#endif

#define GEN_PRINT_ENABLE             (0)
#if (GEN_PRINT_ENABLE)
	#define GEN_PRINT_LEVEL          (3)
	#define GEN_PRINT_TO_FILE        (0)
	#if (GEN_PRINT_TO_FILE)
		#define GEN_PRINT_FILE_PATH   "GenPrint.log"
		#define GEN_PRINT_STEP_FLUSH (1)
	#endif
	#define GEN_PRINT_TO_STD_OUT     (1)
#endif


//Error Trace Function
void ErrTrace(char *fmt, ...);
//General Printf Function
void Print(char *fmt, ...);

//initial Trace
int InitTrace();//Close File
int EndTrace(); //Close File

//Trace Macro define

#if ERR_TRACE_LEVEL > 0
#define ERROR( str )   ErrTrace( (str))
#define ERROR_D1( str, data1 )   ErrTrace( (str), (data1))
#define ERROR_D2( str, data1, data2)   ErrTrace( (str), (data1), (data2))
#define ERROR_D3( str, data1, data2, data3)   ErrTrace( (str), (data1), (data2), (data3))
#else
#define ERROR( str )   
#define ERROR_D1( str, data1 )   
#define ERROR_D2( str, data1, data2)   
#define ERROR_D3( str, data1, data2, data3)   
#endif

#if ERR_TRACE_LEVEL > 1
#define ERROR_L1( str )   ErrTrace( (str))
#define ERROR_D1_L1( str, data1 )   ErrTrace( (str), (data1))
#define ERROR_D2_L1( str, data1, data2)   ErrTrace( (str), (data1), (data2))
#define ERROR_D3_L1( str, data1, data2, data3)   ErrTrace( (str), (data1), (data2), (data3))
#else
#define ERROR_L1( str )   
#define ERROR_D1_L1( str, data1 )   
#define ERROR_D2_L1( str, data1, data2)   
#define ERROR_D3_L1( str, data1, data2, data3)   
#endif

#if ERR_TRACE_LEVEL > 2
#define ERROR_L2( str )   ErrTrace( (str))
#define ERROR_D1_L2( str, data1 )   ErrTrace( (str), (data1))
#define ERROR_D2_L2( str, data1, data2)   ErrTrace( (str), (data1), (data2))
#define ERROR_D3_L2( str, data1, data2, data3)   ErrTrace( (str), (data1), (data2), (data3))
#else
#define ERROR_L2( str )   
#define ERROR_D1_L2( str, data1 )   
#define ERROR_D2_L2( str, data1, data2)   
#define ERROR_D3_L2( str, data1, data2, data3)   
#endif
//Genearl Printf
#if GEN_PRINT_LEVEL > 0
#define PRINT( str )   Print( (str))
#define PRINT_D1( str, data1 )   Print( (str), (data1))
#define PRINT_D2( str, data1, data2)   Print( (str), (data1), (data2))
#define PRINT_D3( str, data1, data2, data3)   Print( (str), (data1), (data2), (data3))
#else
#define PRINT( str )   
#define PRINT_D1( str, data1 )   
#define PRINT_D2( str, data1, data2)   
#define PRINT_D3( str, data1, data2, data3)   
#endif

#if GEN_PRINT_LEVEL > 1
#define PRINT_L1( str )   Print( (str))
#define PRINT_D1_L1( str, data1 )   Print( (str), (data1))
#define PRINT_D2_L1( str, data1, data2)   Print( (str), (data1), (data2))
#define PRINT_D3_L1( str, data1, data2, data3)   Print( (str), (data1), (data2), (data3))
#else
#define PRINT_L1( str )   
#define PRINT_D1_L1( str, data1 )   
#define PRINT_D2_L1( str, data1, data2)   
#define PRINT_D3_L1( str, data1, data2, data3)   
#endif

#if GEN_PRINT_LEVEL > 2
#define PRINT_L2( str )   Print( (str))
#define PRINT_D1_L2( str, data1 )   Print( (str), (data1))
#define PRINT_D2_L2( str, data1, data2)   Print( (str), (data1), (data2))
#define PRINT_D3_L2( str, data1, data2, data3)   Print( (str), (data1), (data2), (data3))
#else
#define PRINT_L2( str )   
#define PRINT_D1_L2( str, data1 )   
#define PRINT_D2_L2( str, data1, data2)   
#define PRINT_D3_L2( str, data1, data2, data3)   
#endif

#endif
