/*******************************************************************************
  Middleware Device Firmware Udpate Source File

  Company:
    Microchip Technology Inc.

  File Name:
    mw_dfu.c

  Summary:
    This file contains the Middleware Device Firmware Udpate functions for application user.

  Description:
    This file contains the Middleware Device Firmware Udpate functions for application user.
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2018 Microchip Technology Inc. and its subsidiaries.
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

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include <string.h>
#include "osal/osal_freertos.h"
#include "peripheral/nvm/plib_nvm.h"
#include "mba_error_defs.h"
#include "mw_dfu.h"


#define MW_DFU_FW_START_ADDR                   0x01080000
#define MW_DFU_FW_PAGE_SIZE                    0x1000
#define MW_DFU_FW_QUAD_WORD_SIZE               0x10

enum
{
    MW_DFU_STATE_IDLE,
    MW_DFU_STATE_CONFIG,
    MW_DFU_STATE_FW_START,
    MW_DFU_STATE_FW_UPDATE
};


static uint8_t s_dfuState = MW_DFU_STATE_IDLE;
static MW_DFU_Info_T s_dfuSizeInfo;
static uint32_t s_dfuAddr;
static uint32_t *sp_dfuIdent = NULL;

uint16_t MW_DFU_Config(MW_DFU_Info_T * p_dfuInfo)
{
    if ((p_dfuInfo->fwImageSize > MW_DFU_MAX_SIZE_FW_IMAGE) || (p_dfuInfo->fwImageSize == 0)
        || (p_dfuInfo->fwImageSize & (MW_DFU_FW_QUAD_WORD_SIZE-1)))
        return MBA_RES_INVALID_PARA;

    s_dfuSizeInfo = *p_dfuInfo;

    s_dfuState = MW_DFU_STATE_CONFIG;

    return MBA_RES_SUCCESS;
}

uint16_t MW_DFU_FwImageStart()
{
    if (s_dfuState == MW_DFU_STATE_IDLE)
        return MBA_RES_BAD_STATE;

    s_dfuAddr = MW_DFU_FW_START_ADDR;
    s_dfuState = MW_DFU_STATE_FW_START;

    return MBA_RES_SUCCESS;
}

uint16_t MW_DFU_FwImageUpdate(uint16_t length, uint8_t *p_content)
{
    uint32_t * p_data;
    uint32_t addr;

    if ((s_dfuState != MW_DFU_STATE_FW_START) && (s_dfuState != MW_DFU_STATE_FW_UPDATE))
        return MBA_RES_BAD_STATE;

    /* Check if image content length and offset are legal */
    if ((s_dfuAddr + length > MW_DFU_FW_START_ADDR + s_dfuSizeInfo.fwImageSize) || (length > MW_DFU_MAX_BLOCK_LEN)
        || (length & (MW_DFU_FW_QUAD_WORD_SIZE-1)) || (length == 0))
        return MBA_RES_INVALID_PARA;

    /* make sure nvm is not busy now */
    while(NVM_IsBusy());

    /* if start from page boundary or write cross page, erase new page */
    if ((s_dfuAddr & (MW_DFU_FW_PAGE_SIZE - 1)) == 0 
    || (s_dfuAddr & ~(MW_DFU_FW_PAGE_SIZE - 1)) < ((s_dfuAddr + length - 1)  & ~(MW_DFU_FW_PAGE_SIZE - 1)))
    {
        if (!NVM_PageErase((s_dfuAddr + length - 1) & ~(MW_DFU_FW_PAGE_SIZE - 1)))
            return MBA_RES_FAIL;

        while(NVM_IsBusy());
        
        if (NVM_ErrorGet() != NVM_ERROR_NONE)
            return MBA_RES_FAIL;
    }

    
    p_data = (uint32_t *)p_content;
    addr = s_dfuAddr; 

    /* backup first 16 bytes */
    if (s_dfuAddr == MW_DFU_FW_START_ADDR)
    {
        if (sp_dfuIdent == NULL)
        {
            sp_dfuIdent = OSAL_Malloc(16);
            if (sp_dfuIdent == NULL)
                return MBA_RES_OOM;
        }
        
        memcpy(sp_dfuIdent, p_data, 16);

        addr += MW_DFU_FW_QUAD_WORD_SIZE;
        p_data += 4;
    }
    
    for (;addr < s_dfuAddr + length; addr += MW_DFU_FW_QUAD_WORD_SIZE)
    {
        if(!NVM_QuadWordWrite(p_data, addr))
            return MBA_RES_FAIL;

        while(NVM_IsBusy());

        if (NVM_ErrorGet() != NVM_ERROR_NONE)
            return MBA_RES_FAIL;

        p_data += 4;
    }

    s_dfuAddr = addr;

    s_dfuState = MW_DFU_STATE_FW_UPDATE;

    /* update complete, write first 16 bytes */
    if (s_dfuAddr == MW_DFU_FW_START_ADDR + s_dfuSizeInfo.fwImageSize)
    {
        if(!NVM_QuadWordWrite(sp_dfuIdent, MW_DFU_FW_START_ADDR))
            return MBA_RES_FAIL;

        while(NVM_IsBusy());

        if (NVM_ErrorGet() != NVM_ERROR_NONE)
            return MBA_RES_FAIL;

        OSAL_Free(sp_dfuIdent);
        sp_dfuIdent = NULL;
    }

    return MBA_RES_SUCCESS;
}

uint16_t MW_DFU_FwImageRead(uint32_t offset, uint16_t length, uint8_t *p_content)
{
    //Check read range 
    if ((offset + length > MW_DFU_MAX_SIZE_FW_IMAGE) || (length > MW_DFU_MAX_BLOCK_LEN))
        return MBA_RES_INVALID_PARA;

    while(NVM_IsBusy());

    if (!NVM_Read((uint32_t *)p_content, length, MW_DFU_FW_START_ADDR + offset))
        return MBA_RES_FAIL;
    
    return MBA_RES_SUCCESS;
}
