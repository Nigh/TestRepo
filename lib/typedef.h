
#ifndef _TYPEDEF_H_
#define _TYPEDEF_H_

#include "time.h"
// typedef unsigned long time_t;

typedef unsigned char uchar;
typedef unsigned int uint;
typedef unsigned long ulong;

typedef unsigned char u8;
typedef unsigned int u16;
typedef unsigned long u32;

typedef void (*fFUNC)(void);
typedef int iFUNC(void);

typedef union
{
	time_t lTime;
	struct
	{
		uchar L1;
		uchar L2;
		uchar L3;
		uchar L4;
	}uTime;
} sUTCS;

typedef struct
{
	char sec;
	char min;
	char hour;
} sTIME;

typedef struct
{
	char year;
	char month;
	char day;
} sDATE;

typedef struct
{
	char hour;
	char min;
} sTIMEZONE;

typedef struct
{
	uchar mode;
	uchar submode;
	uchar detail;
	uchar temp;
} sSELF;

typedef struct
{
	uchar connect;	// 0:not connect 1:connected 2:broadcasting 3:reconnect
	uchar count;	// reconnect timer
} sBLE;

// 注意，结构更改，与上版本不同
typedef struct
{
	uchar en:1;
	uchar length:7;
	uchar array[10];
	uchar ptr;
	uchar count;
} sVIBRATE;

#endif
