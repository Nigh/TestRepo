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
* File Name    : r_cg_dmac.c
* Version      : CodeGenerator for RL78/G13 V2.02.00.02 [11 Feb 2014]
* Device(s)    : R5F101ED
* Tool-Chain   : CA78K0R
* Description  : This file implements device driver for DMAC module.
* Creation Date: 2014/11/22
***********************************************************************************************************************/

/***********************************************************************************************************************
Pragma directive
***********************************************************************************************************************/
/* Start user code for pragma. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
Includes
***********************************************************************************************************************/
#include "r_cg_macrodriver.h"
#include "r_cg_dmac.h"
/* Start user code for include. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */
#include "r_cg_userdefine.h"

/***********************************************************************************************************************
Global variables and functions
***********************************************************************************************************************/
/* Start user code for global. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
* Function Name: R_DMAC1_Create
* Description  : This function initializes the DMA1 transfer.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void R_DMAC1_Create(void)
{
    DRC1 = _80_DMA_OPERATION_ENABLE;
    NOP();
    NOP();
    DMAMK1 = 1U; /* disable INTDMA1 interrupt */
    DMAIF1 = 0U; /* clear INTDMA1 interrupt flag */
    /* Set INTDMA1 low priority */
    DMAPR11 = 1U;
    DMAPR01 = 1U;
    DMC1 = _40_DMA_TRANSFER_DIR_RAM2SFR | _00_DMA_DATA_SIZE_8 | _00_DMA_TRIGGER_SOFTWARE;
    DSA1 = _44_DMA1_SFR_ADDRESS;
    DRA1 = _F300_DMA1_RAM_ADDRESS;
    DBC1 = _0020_DMA1_BYTE_COUNT;
    DEN1 = 0U; /* disable DMA1 operation */
}

/***********************************************************************************************************************
* Function Name: R_DMAC1_Start
* Description  : This function enables DMA1 transfer.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void R_DMAC1_Start(void)
{
    DMAIF1 = 0U; /* clear INTDMA1 interrupt flag */
    DMAMK1 = 0U; /* enable INTDMA1 interrupt */
    DEN1 = 1U;
    DST1 = 1U;
}

/***********************************************************************************************************************
* Function Name: R_DMAC1_Stop
* Description  : This function disables DMA1 transfer.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void R_DMAC1_Stop(void)
{
    if (DST1 != 0U)
    {
        DST1 = 0U;
    }
    
    NOP();
    NOP();
    DEN1 = 0U; /* disable DMA1 operation */
    DMAMK1 = 1U; /* disable INTDMA1 interrupt */
    DMAIF1 = 0U; /* clear INTDMA1 interrupt flag */
}

/***********************************************************************************************************************
* Function Name: R_DMAC1_Set_SoftwareTriggerOn
* Description  : This function sets DMA1 software trigger on.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void R_DMAC1_Set_SoftwareTriggerOn(void)
{
    STG1 = 1U; /* start DMA1 software trigger */
}

/* Start user code for adding. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */
