/****************************************************************
*
* FILENAME
*	mvutil.h
*
* PURPOSE 
*	Header for MV Utility function APIs
*
* AUTHOR
*	KB Kim
*
* HISTORY
*  Status                            Date              Author
*  Create                         2010.08.19           KB
*
****************************************************************/
#ifndef MV_UTIL_H
#define MV_UTIL_H

/****************************************************************
 *                       Include files                          *
 ****************************************************************/

/****************************************************************
*	                    Define Values                           *
*****************************************************************/
#ifndef ABS
#define ABS(X) ((X)<0 ? (-1*(X)) : (X))
#endif

#ifndef MAX
#define MAX(X,Y) ((X)>=(Y) ? (X) : (Y))
#endif

#ifndef MIN
#define MIN(X,Y) ((X)<=(Y) ? (X) : (Y)) 
#endif

#define INRANGE(X,Y,Z) ((((X)<=(Y)) && ((Y)<=(Z)))||(((Z)<=(Y)) && ((Y)<=(X))) ? 1 : 0)

#ifndef MAKEWORD
	#undef MAKEWORD
	#define MAKEWORD(X,Y) (( (X) <<8)+(Y))
#endif

#define LSB(X) ( ( (X) & 0xFF ) )
#define MSB(Y) ( ( (Y)>>8  ) & 0xFF )
#define MMSB(Y)( ( (Y)>>16 ) & 0xFF )

/****************************************************************
 *                       Type define                            *
 ****************************************************************/

/****************************************************************
 *                      Global Variable                         *
 ****************************************************************/

/****************************************************************
 *                      Extern Variable                         *
 ****************************************************************/

/****************************************************************
 *                     Function Prototype                       *
 ****************************************************************/
extern void StrNcpy(char *destination, const char *source, int num);
extern void Str2Lower(U8 *src, U8 *dest);
extern BOOL Char2Hex(U8 data, U8 *value);
extern BOOL Bcd2Dec(U8 data, U8 *value);
extern BOOL Str2Hex(U8 *data, U8 *value);
extern U32 Array2Word(U8 *data, U8 len);
extern void Word2Array(U8 *buff, U32 data, U32 size);
extern U32 Bcd2Word(U32 bcdValue);
extern U32 BcdArray2Word(U8 *data, U8 len);
extern U32 BcdStr2Word(U8 *data);
extern S32 XtoPowerY(S32 number, U32 power);

#endif /* #ifndef MV_UTIL_H */
