/*******************************************************************************
  ADCHS Peripheral Library Interface Source File

  Company
    Microchip Technology Inc.

  File Name
    plib_adchs.c

  Summary
    ADCHS peripheral library source.

  Description
    This file implements the ADCHS peripheral library.

*******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2020 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*******************************************************************************/
// DOM-IGNORE-END
#include "device.h"
#include "plib_adchs.h"

#define ADCHS_CHANNEL_32  (32U)

// *****************************************************************************
// *****************************************************************************
// Section: ADCHS Implementation
// *****************************************************************************
// *****************************************************************************




void ADCHS_Initialize(void)
{
    ADCHS_REGS->ADCHS_ADCCON1 = 0;

    ADCHS_REGS->ADCHS_ADCCON1 = 0x600000;
    ADCHS_REGS->ADCHS_ADCCON2 = 0x20001;
    ADCHS_REGS->ADCHS_ADCCON3 = 0x1000000;

    ADCHS_REGS->ADCHS_ADCTRG1 = 0x10200; 
    ADCHS_REGS->ADCHS_ADCTRG2 = 0x0; 
    
    
    
    
    

    ADCHS_REGS->ADCHS_ADCTRGSNS = 0x0;

    ADCHS_REGS->ADCHS_ADCIMCON1 = 0x0;
    
    
    

    /* Input scan */
    ADCHS_REGS->ADCHS_ADCCSS1 = 0x0;
    




    /* Turn ON ADC */
    ADCHS_REGS->ADCHS_ADCCON1 |= ADCHS_ADCCON1_ON_Msk;
    while((ADCHS_REGS->ADCHS_ADCCON2 & ADCHS_ADCCON2_BGVRRDY_Msk) == ADCHS_ADCCON2_BGVRRDY_Msk) 
    {
        // Wait until the reference voltage is ready
    }
    
    while((ADCHS_REGS->ADCHS_ADCCON2 & ADCHS_ADCCON2_REFFLT_Msk) == ADCHS_ADCCON2_REFFLT_Msk) 
    {
        // Wait if there is a fault with the reference voltage
    }

    /* ADC 7 */
    ADCHS_REGS->ADCHS_ADCANCON |= ADCHS_ADCANCON_ANEN7_Msk;      // Enable the clock to analog bias
    while(!((ADCHS_REGS->ADCHS_ADCANCON & ADCHS_ADCANCON_WKRDY7_Msk))); // Wait until ADC is ready
    ADCHS_REGS->ADCHS_ADCCON3 |= ADCHS_ADCCON3_DIGEN7_Msk;       // Enable ADC
}


/* Enable ADCHS channels */
void ADCHS_ModulesEnable (ADCHS_MODULE_MASK moduleMask)
{
    ADCHS_REGS->ADCHS_ADCCON3 |= ((uint32_t)moduleMask << 16U);
}

/* Disable ADCHS channels */
void ADCHS_ModulesDisable (ADCHS_MODULE_MASK moduleMask)
{
    ADCHS_REGS->ADCHS_ADCCON3 &= ~((uint32_t)moduleMask << 16U);
}


void ADCHS_ChannelResultInterruptEnable (ADCHS_CHANNEL_NUM channel)
{
    if ((uint32_t)channel < ADCHS_CHANNEL_32)
    {
        ADCHS_REGS->ADCHS_ADCGIRQEN1 |= 0x01UL << (uint32_t)channel;
    }
}

void ADCHS_ChannelResultInterruptDisable (ADCHS_CHANNEL_NUM channel)
{
    if ((uint32_t)channel < ADCHS_CHANNEL_32)
    {
        ADCHS_REGS->ADCHS_ADCGIRQEN1 &= ~(0x01UL << (uint32_t)channel);
    }
}


void ADCHS_GlobalEdgeConversionStart(void)
{
    ADCHS_REGS->ADCHS_ADCCON3 |= ADCHS_ADCCON3_GSWTRG_Msk;
}

void ADCHS_GlobalLevelConversionStart(void)
{
    ADCHS_REGS->ADCHS_ADCCON3 = ADCHS_ADCCON3_GLSWTRG_Msk;
}

void ADCHS_GlobalLevelConversionStop(void)
{
    ADCHS_REGS->ADCHS_ADCCON3 &= ~ADCHS_ADCCON3_GLSWTRG_Msk;
}

void ADCHS_ChannelConversionStart(ADCHS_CHANNEL_NUM channel)
{
    ADCHS_REGS->ADCHS_ADCCON3 &= ~(ADCHS_ADCCON3_ADINSEL_Msk);
    ADCHS_REGS->ADCHS_ADCCON3 |= (((uint32_t)channel << ADCHS_ADCCON3_ADINSEL_Pos) | ADCHS_ADCCON3_RQCNVRT_Msk);
}


/*Check if conversion result is available */
bool ADCHS_ChannelResultIsReady(ADCHS_CHANNEL_NUM channel)
{
    bool status = false;
    if ((uint32_t)channel < ADCHS_CHANNEL_32)
    {
        status = ((ADCHS_REGS->ADCHS_ADCDSTAT1 >> (uint32_t)channel) & 0x01U) != 0U;
    }
    return status;
}

/* Read the conversion result */
uint16_t ADCHS_ChannelResultGet(ADCHS_CHANNEL_NUM channel)
{
    uint32_t channel_addr = ADCHS_BASE_ADDRESS + ADCHS_ADCDATA0_REG_OFST + ((uint32_t)channel << 4U);
	return (uint16_t)(*(uint32_t*)channel_addr);   
}




bool ADCHS_EOSStatusGet(void)
{
    return (bool)(((ADCHS_REGS->ADCHS_ADCCON2 & ADCHS_ADCCON2_EOSRDY_Msk) 
                    >> ADCHS_ADCCON2_EOSRDY_Pos) != 0U);
}

