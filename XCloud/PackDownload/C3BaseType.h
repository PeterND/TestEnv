#ifndef __C3_BASE_TYPE_H__
#define __C3_BASE_TYPE_H__

#ifdef _WIN32
#pragma warning(disable : 4786 4244 4018 4996)
#define _CRT_SECURE_NO_WARNINGS        1
#define _CRT_NON_CONFORMING_SWPRINTFS  1
#include <Windows.h>
#include <float.h>
#include <MMSystem.h>
#if _MSC_VER >= 1600
#include <stdint.h>
#else
typedef int int32_t;
typedef unsigned int uint32_t;
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
#endif
#else// unix
#include <ctype.h>
#include <inttypes.h>
#include <float.h>
typedef unsigned short WORD;
typedef unsigned int   DWORD;
typedef unsigned char  BYTE;
typedef int            BOOL;
#ifndef NULL
#define NULL 0
#endif//NULL
#ifndef FALSE
#define FALSE               0
#endif
#ifndef TRUE
#define TRUE                1
#endif

#endif//_WIN32

#endif//__C3_BASE_TYPE_H__