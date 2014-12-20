/***********************************************************************************************************************
* DISCLAIMER
* This software is supplied by Renesas Electronics Corporation and is only 
* intended for use with Renesas products. No other uses are authorized. This 
* software is owned by Renesas Electronics Corporation and is protected under 
* all applicable laws, including copyright laws.
* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING 
* THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT 
* LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE 
* AND NON-INFRINGEMENT.  ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED.
* TO THE MAXIMUM EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS 
* ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES SHALL BE LIABLE 
* FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR 
* ANY REASON RELATED TO THIS SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE 
* BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* Renesas reserves the right, without notice, to make changes to this software 
* and to discontinue the availability of this software.  By using this software, 
* you agree to the additional terms and conditions found by accessing the 
* following link:
* http://www.renesas.com/disclaimer
*
* Copyright (C) 2011, 2014 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/

/***********************************************************************************************************************
* File Name    : r_cg_userdefine.h
* Version      : CodeGenerator for RL78/G13 V2.02.00.02 [11 Feb 2014]
* Device(s)    : R5F101ED
* Tool-Chain   : CA78K0R
* Description  : This file includes user definition.
* Creation Date: 2014/11/11
***********************************************************************************************************************/

#ifndef _USER_DEF_H
#define _USER_DEF_H

/***********************************************************************************************************************
User definitions
***********************************************************************************************************************/

/* Start user code for function. Do not edit comment generated here */
#include "r_cg_macrodriver.h"
#include "r_cg_timer.h"
#include "r_cg_serial.h"
#include "r_cg_dmac.h"
#include "r_cg_it.h"
#include "apptimer.h"
#include "typedef.h"
#include "led.h"
#include "event2.0.h"
#include "Clickpro.h"
#include "spiapp.h"
#include "uartapp.h"
#include "angle.h"
#include "lis3dh.h"
#include "main.h"
#include "battery.h"
#include "flashapp.h"
#include "r_cg_intc.h"
#include "step.h"
#include "project.h"

#define and &&
#define or ||

#define M_NORMAL 1
#define M_CHARGE 2

#define BAT_NORMAL (0x0)
#define BAT_CHARGE (0x1)
#define BAT_FULL (0x2)

#define UPLOAD_IDLE (0)
#define UPLOAD_NECK (1)
#define UPLOAD_STEP (2)

#define VibrateOn() P5.0=1
#define VibrateOff() P5.0=0

#define SYS_ACTIVE (0)
#define SYS_SLEEP (1)
#define SYS_TEST (2)
#define SYS_OAD (3)

extern sSELF sSelf;
extern sUTCS sUtcs,sAlarmTime;
extern sUPLOAD sUpload;

extern uchar directGEn;

extern uchar isTimeSync;
extern uint step;

extern uchar batteryStatu;
extern sSTEPLONGLOG currentStepLog;

extern void _nop_Ex(void);
extern void _halt_Ex(void);
extern void startHClk(void);
extern void stopHClk(void);

extern uchar neckUnhealthCount;

extern uchar BLE_Connect_Timeout;

#define HEALTHNECKMOVE (30)

/* End user code. Do not edit comment generated here */
#endif
