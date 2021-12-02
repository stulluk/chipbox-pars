/****************************************************************************
 * Copyright (c) 2007 Celestial Corporation  All Rights Reserved.
 *
 * Module:      mwpublic.h
 *
 * Authors:     River<jin.jiang@celestialsemi.com>
 *
 * Ver:           1.0(06.17.2007)
 *
 * Description: middleware application public definitions.
 *
 * Notes:
 *
 ***************************************************************************/


#ifndef _CS_MW_PUBLIC_H_
#define _CS_MW_PUBLIC_H_


#ifdef __cplusplus
extern "C"
{
#endif/*__cplusplus*/

/* Maximum of programs in one channel  */
#define CS_MW_MAXIMUM_PROGRAMS          50

/* Maximum of program name Length */
#define CS_MW_PROGRAM_NAMELENG          16

/* Maximum of Mutil Audio Language Type in one program */
#define CS_MW_MULTI_AUDIO_LANGUAGE      10

/* Maximum of Middleware Information Pipe Number */
#define CS_MW_MAXIMUM_PIPE_NUMBER        5



#define CS_MW_MemAlloc          malloc
#define CS_MW_MemFree           free
#define CS_MW_MemSet            memset
#define CS_MW_DbgPrintf         printf
#define CS_MW_Delayms           CSOS_DelayTaskMs


typedef enum
{
    CS_MW_OK,
    CS_MW_ERROR_NULL_POINTER,
    CS_MW_FAILED
    
}eCS_MW_RESULT;

typedef enum
{
    CS_MW_MSG_UNKOWN,
    CS_MW_MSG_SERVICEINFO,
    CS_MW_MSG_COMPLETE,
    CS_MW_MSG_VIDEO_FORMAT_CHANGE,
    CS_MW_MSG_MAXIMUM_TYPE

} eCS_MW_MSG_TYPE;



typedef struct
{
    eCS_MW_MSG_TYPE MsgType;
    U32 WParam;
    U32 LParam;
}tCS_MW_MSG_INFO;

typedef struct
{
  S32 Left;
  S32 Top;
  U32 Width;
  U32 Height;
} tCS_MW_RECT;


typedef struct
{

    BOOL        bVideoScalar;
    tCS_MW_RECT tVideoScalarRect;

}tCS_MW_VIDEO_SCALAR;


typedef struct
{
    char    year[5];
    char    month[3];
    char    day[3];

    char    hour[3];
    char    minute[3];
    char    second[3];

} tCS_MW_TIME;


#ifdef __cplusplus
}
#endif/*__cplusplus*/


#endif/*_CS_MW_PUBLIC_H_*/



