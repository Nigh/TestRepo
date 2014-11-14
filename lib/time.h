/*******************************************************************************
*
* definitions/declarations for time routines
*
* time.h
* $Date: 2007/05/31 12:42:39 $
* $Revision: 1.4 $
*
* Copyright(C) 1999(2000-2004). Renesas Electronics Corp.
* and Renesas Solutions Corp., All rights reserved.
*
*
*******************************************************************************/

#ifndef NC30
#define NC30
#endif
#if defined(NC30)		/* NCxx */
	/* dummy */
#elif defined(NC77)
	#error "NC77 not supported"
#elif defined(NC79)
	#error "NC79 not supported"
#else
	#error "NC30, NC77, NC79 not defined"
#endif				/* NCxx */

#ifdef NEED_SJMP_FOR_LIB
_asm("	.SJMP	OFF");
#endif

#ifndef __TIME_H
#define __TIME_H

#define CLOCKS_PER_SEC	1000
#ifndef NULL
#define NULL	0
#endif
#ifndef _CLOCK_T_DEF
typedef long clock_t;
#define _CLOCK_T_DEF
#endif
#ifndef _SIZE_T_DEF
#ifdef __SIZET_16__
typedef unsigned int size_t;
#else
typedef unsigned long size_t;
#endif
#define _SIZE_T_DEF
#endif
#ifndef _TIME_T_DEF
typedef long time_t;
#define _TIME_T_DEF
#endif
#ifndef _TM_DEF
struct tm {
	int tm_sec;	/* seconds after the minute - [0,59] */
	int tm_min;	/* minutes after the hour - [0,59] */
	int tm_hour;	/* hours since midnight - [0,23] */
	int tm_mday;	/* day of the month - [1,31] */
	int tm_mon;	/* months since January - [0,11] */
	int tm_year;	/* years since 1900 */
	int tm_wday;	/* days since Sunday - [0,6] */
	int tm_yday;	/* days since January 1 - [0,365] */
	int tm_isdst;	/* daylight savings time flag */
};
#define _TM_DEF
#endif

#endif
/*******************************************************************************
*
* time.h
*
* Copyright(C) 1999(2000-2004). Renesas Electronics Corp.
* and Renesas Solutions Corp., All rights reserved.
*
*
*******************************************************************************/
