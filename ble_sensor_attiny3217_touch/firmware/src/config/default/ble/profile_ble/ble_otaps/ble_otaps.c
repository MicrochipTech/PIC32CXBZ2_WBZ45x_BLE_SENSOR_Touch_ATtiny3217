/*******************************************************************************
  BLE OTA Profile Server Source File

  Company:
    Microchip Technology Inc.

  File Name:
    ble_otaps.c

  Summary:
    This file contains the BLE OTA Profile Server functions for application user.

  Description:
    This file contains the BLE OTA Profile Server functions for application user.
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
#include "crypto/crypto.h"
#include "bt_sys.h"
#include "mba_error_defs.h"
#include "ble_gap.h"
#include "gatt.h"
#include "ble_dm/ble_dm_aes.h"
#include "ble_util/byte_stream.h"
#include "ble_util/mw_dfu.h"
#include "ble_otas/ble_otas.h"
#include "ble_otaps/ble_otaps.h"


// *****************************************************************************
// *****************************************************************************
// Section: Macros
// *****************************************************************************
// *****************************************************************************

#define BLE_OTAPS_OP_CODE_RESPONSE            0x01
#define BLE_OTAPS_OP_CODE_UPDATE_REQUEST      0x02
#define BLE_OTAPS_OP_CODE_UPDATE_START        0x03
#define BLE_OTAPS_OP_CODE_UPDATE_COMPLETE     0x04
#define BLE_OTAPS_OP_CODE_RESET_REQUEST       0x05

#define BLE_OTAPS_UPDATE_REQ_SIZE             0x0D
#define BLE_OTAPS_UPDATE_RSP_SIZE             0x0A
#define BLE_OTAPS_UPDATE_START_SIZE           0x01


#define BLE_OTAPS_STATE_CCCD_DISABLED         0x00
#define BLE_OTAPS_STATE_CCCD_ENABLED          0x01
#define BLE_OTAPS_STATE_WAIT_REQ_RSP          0x02
#define BLE_OTAPS_STATE_UPDATE_REQ            0x03
#define BLE_OTAPS_STATE_UPDATE_START          0x04
#define BLE_OTAPS_STATE_UPDATE_ERROR          0x05
#define BLE_OTAPS_STATE_WAIT_COMP_RSP         0x06
#define BLE_OTAPS_STATE_COMPLETE              0x07


#define BLE_OTAPS_RESULT_SUCCESS              0x00
#define BLE_OTAPS_RESULT_INVALID_STATE        0x01
#define BLE_OTAPS_RESULT_NOT_SUPPORTED        0x02
#define BLE_OTAPS_RESULT_OPERATION_FAILED     0x03
#define BLE_OTAPS_RESULT_INVALID_PARA         0x04

#define BLE_OTAPS_ENC_NONE                    0x00
#define BLE_OTAPS_ENC_AES_CBC                 0x01

#define BLE_OTAPS_START_IDX_LEN               0x04

#define BLE_OTAPS_IMG_BUF_SIZE                0x200
#define BLE_OTAPS_MTU_SIZE                    0x400

#define BLE_OTAPS_CCCD_CTRL_ON                (1 << 0)
#define BLE_OTAPS_CCCD_DATA_ON                (1 << 1)

#define BLE_OTAPS_RETRY_TYPE_NONE             0x00
#define BLE_OTAPS_RETRY_TYPE_ERROR            0x01
#define BLE_OTAPS_RETRY_TYPE_READ             0x02
#define BLE_OTAPS_RETRY_TYPE_WRITE            0x03
#define BLE_OTAPS_RETRY_TYPE_NTF              0x04

// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
// *****************************************************************************

typedef struct BLE_OTAPS_RetryList_T
{
    uint16_t connHandle;
    uint8_t *p_pkt;
    GATT_Event_T *p_gattEvt;
    uint8_t  type;
}BLE_OTAPS_RetryList_T;

// *****************************************************************************
// *****************************************************************************
// Section: Local Variables
// *****************************************************************************
// *****************************************************************************

static BLE_OTAPS_EventCb_T sp_otapsCbRoutine;
static uint8_t s_otapsState;
static uint8_t * sp_otapsPktBuf;
static uint8_t s_otapsAesIv[16];
static uint8_t s_otapsAesKey[16];
static uint8_t *sp_otapsPlainBuf;
static uint8_t s_otapsCccdStatus;

static uint8_t s_otapsImgType;
static uint32_t s_otapsUpdateOffset;
static uint16_t s_otapsPktBufIdx;
static uint32_t s_otapsImgReceieved;
static uint32_t s_otapsPktTotal;
static struct {
    uint32_t fwSize;
} s_otapsImgSize;


static uint16_t      s_otapsConnHandle;

static CRYPT_AES_CTX *sp_otapsAes;
static uint8_t       s_otapsEnc;

static BLE_OTAPS_RetryList_T s_otapsRetry[BLE_GAP_MAX_LINK_NBR];
// *****************************************************************************
// *****************************************************************************
// Section: Functions
// *****************************************************************************
// *****************************************************************************

static BLE_OTAPS_RetryList_T * ble_otaps_GetRetryListByHandle(uint16_t connHandle)
{
    uint8_t i;

    for(i=0; i<BLE_GAP_MAX_LINK_NBR;i++)
    {
        if (s_otapsRetry[i].connHandle == connHandle)
        {
            return &s_otapsRetry[i];
        }
    }

    return NULL;
}

static bool ble_otaps_IsProcInProgress()
{
    return (s_otapsState > BLE_OTAPS_STATE_CCCD_ENABLED
        && s_otapsState < BLE_OTAPS_STATE_COMPLETE);
}

static uint8_t ble_otaps_MbaErrToResult(uint16_t mbaErr)
{
    switch (mbaErr)
    {
        case MBA_RES_SUCCESS:
            return BLE_OTAPS_RESULT_SUCCESS;
        case MBA_RES_FAIL:
        case MBA_RES_OOM:
            return BLE_OTAPS_RESULT_OPERATION_FAILED;
        case MBA_RES_INVALID_PARA:
            return BLE_OTAPS_RESULT_INVALID_PARA;
        case MBA_RES_BAD_STATE:
            return BLE_OTAPS_RESULT_INVALID_STATE;
        default:
            return BLE_OTAPS_RESULT_OPERATION_FAILED;
    }
}

static void ble_otaps_FreeBuf()
{
    if (sp_otapsPktBuf)
    {
        OSAL_Free(sp_otapsPktBuf); 
        sp_otapsPktBuf = NULL;
    }

    if (sp_otapsPlainBuf)
    {
        OSAL_Free(sp_otapsPlainBuf);
        sp_otapsPlainBuf = NULL;
    }

    if (sp_otapsAes)
    {
        OSAL_Free(sp_otapsAes);
        sp_otapsAes = NULL;
    }
}

static void ble_otaps_StopProc()
{
    s_otapsState = BLE_OTAPS_STATE_CCCD_DISABLED;
    
    s_otapsCccdStatus = 0x00;
    
    s_otapsConnHandle = 0x00;
}

static void ble_otaps_FreeRetry(BLE_OTAPS_RetryList_T *p_retry)
{
    if (p_retry->p_pkt)
    {
        OSAL_Free(p_retry->p_pkt);
        p_retry->p_pkt = NULL;
    }

    if (p_retry->p_gattEvt)
    {
        OSAL_Free(p_retry->p_gattEvt);
        p_retry->p_gattEvt = NULL;
    }

    p_retry->type = BLE_OTAPS_RETRY_TYPE_NONE;
}

static void ble_otaps_ConveyEvent(uint8_t eventId, uint8_t *p_eventField, uint8_t eventFieldLen)
{
    if (sp_otapsCbRoutine)
    {
        BLE_OTAPS_Event_T evtPara;

        evtPara.eventId = eventId;
        memcpy((uint8_t *)&evtPara.eventField, p_eventField, eventFieldLen);
        sp_otapsCbRoutine(&evtPara);
    }
}

static uint16_t ble_otaps_SendResponse(BLE_OTAPS_RetryList_T *p_retry, uint8_t reqOpcode, uint8_t result)
{
    uint16_t status = MBA_RES_OOM;
    
    p_retry->p_pkt = OSAL_Malloc(sizeof(GATTS_HandleValueParams_T));
    if (p_retry->p_pkt != NULL)
    {
        ((GATTS_HandleValueParams_T *)p_retry->p_pkt)->charHandle = BLE_OTAS_HDL_CTRL_VAL;
        ((GATTS_HandleValueParams_T *)p_retry->p_pkt)->charLength = 0x03;
        ((GATTS_HandleValueParams_T *)p_retry->p_pkt)->charValue[0] = BLE_OTAPS_OP_CODE_RESPONSE;
        ((GATTS_HandleValueParams_T *)p_retry->p_pkt)->charValue[1] = reqOpcode;
        ((GATTS_HandleValueParams_T *)p_retry->p_pkt)->charValue[2] = result;
        ((GATTS_HandleValueParams_T *)p_retry->p_pkt)->sendType = ATT_HANDLE_VALUE_NTF;

        if (result != BLE_OTAPS_RESULT_SUCCESS)
        {
            ble_otaps_FreeBuf();
        }

        status = GATTS_SendHandleValue(p_retry->connHandle, (GATTS_HandleValueParams_T *)p_retry->p_pkt);

        if (status == MBA_RES_SUCCESS)
            ble_otaps_FreeRetry(p_retry);
        else
            p_retry->type = BLE_OTAPS_RETRY_TYPE_NTF;
    }

    return status;
}

static uint16_t ble_otaps_SendDataResponse(BLE_OTAPS_RetryList_T *p_retry, uint8_t result)
{
    uint16_t status = MBA_RES_OOM;

    p_retry->p_pkt = OSAL_Malloc(sizeof(GATTS_HandleValueParams_T));
    if (p_retry->p_pkt != NULL)
    {

        ((GATTS_HandleValueParams_T *)p_retry->p_pkt)->charHandle = BLE_OTAS_HDL_DATA_VAL;
        ((GATTS_HandleValueParams_T *)p_retry->p_pkt)->charLength = 0x01;
        ((GATTS_HandleValueParams_T *)p_retry->p_pkt)->charValue[0] = result;
        ((GATTS_HandleValueParams_T *)p_retry->p_pkt)->sendType = ATT_HANDLE_VALUE_NTF;

        if (result != BLE_OTAPS_RESULT_SUCCESS)
        {
            ble_otaps_FreeBuf();
        }
        
        status = GATTS_SendHandleValue(s_otapsConnHandle, (GATTS_HandleValueParams_T *)p_retry->p_pkt);
   
        if (status == MBA_RES_SUCCESS)
            ble_otaps_FreeRetry(p_retry);
        else
           p_retry->type = BLE_OTAPS_RETRY_TYPE_NTF;
   }
   
   return status;

}

static uint16_t ble_otaps_SendUpdateReqResponse(BLE_OTAPS_RetryList_T *p_retry, uint8_t result, BLE_OTAPS_DevInfo_T * p_devInfo)
{
    uint16_t status = MBA_RES_OOM;
        
    p_retry->p_pkt = OSAL_Malloc(sizeof(GATTS_HandleValueParams_T));
    if (p_retry->p_pkt != NULL)
    {
        ((GATTS_HandleValueParams_T *)p_retry->p_pkt)->charHandle = BLE_OTAS_HDL_CTRL_VAL;

        ((GATTS_HandleValueParams_T *)p_retry->p_pkt)->charLength = 0x03 + BLE_OTAPS_UPDATE_RSP_SIZE;
        ((GATTS_HandleValueParams_T *)p_retry->p_pkt)->charValue[0] = BLE_OTAPS_OP_CODE_RESPONSE;
        ((GATTS_HandleValueParams_T *)p_retry->p_pkt)->charValue[1] = BLE_OTAPS_OP_CODE_UPDATE_REQUEST;
        ((GATTS_HandleValueParams_T *)p_retry->p_pkt)->charValue[2] = result;

        if (result == BLE_OTAPS_RESULT_SUCCESS)
        {
            U16_TO_BUF_LE(&((GATTS_HandleValueParams_T *)p_retry->p_pkt)->charValue[3], BLE_OTAPS_MTU_SIZE);
            memset(&((GATTS_HandleValueParams_T *)p_retry->p_pkt)->charValue[5], 0x00, BLE_OTAPS_START_IDX_LEN);
            U32_TO_BUF_LE(&((GATTS_HandleValueParams_T *)p_retry->p_pkt)->charValue[5 + BLE_OTAPS_START_IDX_LEN], p_devInfo->fwImageVer);
        }
        else
            memset(&((GATTS_HandleValueParams_T *)p_retry->p_pkt)->charValue[3], 0x00, BLE_OTAPS_UPDATE_RSP_SIZE);
        
        ((GATTS_HandleValueParams_T *)p_retry->p_pkt)->sendType = ATT_HANDLE_VALUE_NTF;

        status = GATTS_SendHandleValue(p_retry->connHandle, (GATTS_HandleValueParams_T *)p_retry->p_pkt);

        if (status == MBA_RES_SUCCESS)
            ble_otaps_FreeRetry(p_retry);
        else
            p_retry->type = BLE_OTAPS_RETRY_TYPE_NTF;
    }
    
    return status;

}

static uint16_t ble_otaps_AesCbcDecrypt(uint16_t length, uint8_t *p_plainText, uint8_t *p_chiperText)
{
    CRYPT_AES_CBC_Decrypt(sp_otapsAes, p_plainText, p_chiperText, length);

    return MBA_RES_SUCCESS;
}

static uint16_t ble_otaps_UpdateImage(uint16_t len)
{
    uint16_t err;
    uint8_t *p_buf;
    uint16_t updateLen, pktOffset;

    err = MBA_RES_SUCCESS;
    pktOffset = 0;
    while (pktOffset < len)
    {
        updateLen = (len - pktOffset > BLE_OTAPS_IMG_BUF_SIZE) ? BLE_OTAPS_IMG_BUF_SIZE : len - pktOffset;

        if (s_otapsEnc == BLE_OTAPS_ENC_NONE)
            memcpy(sp_otapsPlainBuf, sp_otapsPktBuf + pktOffset, updateLen);
        else
            err = ble_otaps_AesCbcDecrypt(updateLen, sp_otapsPlainBuf, sp_otapsPktBuf + pktOffset);

        if (err == MBA_RES_SUCCESS)
        {
            p_buf = sp_otapsPlainBuf;

            switch (s_otapsImgType)
            {
                case BLE_OTAPS_IMG_TYPE_FW:
                    err = MW_DFU_FwImageUpdate(updateLen, p_buf);
                    break;
                default:
                    break;
            }


            if (err == MBA_RES_SUCCESS)
            {
                /* read back to sp_otapsPktBuf, sp_otapsPktBuf is decrypted image stored in flash now */
                switch (s_otapsImgType)
                {
                    case BLE_OTAPS_IMG_TYPE_FW:
                        err = MW_DFU_FwImageRead(s_otapsUpdateOffset, updateLen, sp_otapsPktBuf + pktOffset);
                        break;
                    default:
                        break;
                }

                if (err == MBA_RES_SUCCESS)
                {
                    uint16_t offset, validLen;

                    if (s_otapsUpdateOffset  == 0)
                    {
                        offset = 16;
                        validLen = updateLen - 16;
                    }
                    else
                    {
                        offset = 0;
                        validLen = updateLen;
                    }

                    for (; offset < validLen; )
                    {
                        size_t cmpLen;

                        cmpLen = (validLen - offset < SIZE_MAX) ? (validLen - offset) : SIZE_MAX;

                        if (memcmp(p_buf + offset, sp_otapsPktBuf + pktOffset + offset, cmpLen) != 0)
                        {
                            err = MBA_RES_FAIL;

                            break;
                        }

                        offset += cmpLen;
                    }
                }
            }
        } 
        
        if (err == MBA_RES_SUCCESS)
        {
            pktOffset += updateLen;
            s_otapsUpdateOffset += updateLen;
        }
        else
            break;
    }
    
    return err;
}

static void ble_otaps_ProcUpdateRequest(BLE_OTAPS_RetryList_T *p_retry, GATT_Event_T *p_event)
{
    if (ble_otaps_IsProcInProgress())
    {
        ble_otaps_SendUpdateReqResponse(p_retry, BLE_OTAPS_RESULT_INVALID_STATE, NULL);
        return;
    }

    if (p_event->eventField.onWrite.writeDataLength - 1 != BLE_OTAPS_UPDATE_REQ_SIZE)
    {
        ble_otaps_SendUpdateReqResponse(p_retry, BLE_OTAPS_RESULT_NOT_SUPPORTED, NULL);
    }
    else
    {
        uint16_t err;
        uint8_t result;
        uint8_t * p_value = p_event->eventField.onWrite.writeValue + 1;
        MW_DFU_Info_T dfu;
        STREAM_LE_TO_U32(&dfu.fwImageSize, &p_value);

        err = MW_DFU_Config(&dfu);
        result = ble_otaps_MbaErrToResult(err);

        if (result == BLE_OTAPS_RESULT_SUCCESS)
        {
            BLE_OTAPS_EvtUpdateReq_T evtUpReq;

            evtUpReq.connHandle = p_event->eventField.onWrite.connHandle;

            evtUpReq.fwImageSize = dfu.fwImageSize;
            STREAM_LE_TO_U32(&evtUpReq.fwImageId, &p_value);
            STREAM_LE_TO_U32(&evtUpReq.fwImageVer, &p_value);
            STREAM_TO_U8(&s_otapsEnc, &p_value);
            
            if (s_otapsEnc > BLE_OTAPS_ENC_AES_CBC)
            {
                ble_otaps_SendUpdateReqResponse(p_retry, BLE_OTAPS_RESULT_NOT_SUPPORTED, NULL);
                return;
            }
            
            s_otapsImgSize.fwSize = dfu.fwImageSize;

            s_otapsState = BLE_OTAPS_STATE_WAIT_REQ_RSP;
            
            ble_otaps_ConveyEvent(BLE_OTAPS_EVT_UPDATE_REQ, (uint8_t *)&evtUpReq, sizeof(evtUpReq));
        }
        else
        {
            err = ble_otaps_SendUpdateReqResponse(p_retry, result, NULL);
        }
    }
}

static void ble_otaps_ProcUpdateStart(BLE_OTAPS_RetryList_T *p_retry, GATT_Event_T *p_event)
{
    uint16_t err;
    uint8_t result = BLE_OTAPS_RESULT_NOT_SUPPORTED;

    if (s_otapsState !=  BLE_OTAPS_STATE_UPDATE_REQ)
    {
        ble_otaps_SendResponse(p_retry, BLE_OTAPS_OP_CODE_UPDATE_START, BLE_OTAPS_RESULT_INVALID_STATE);
        return;
    }
        
    if (p_event->eventField.onWrite.writeDataLength - 1 != BLE_OTAPS_UPDATE_START_SIZE)
    {
        ble_otaps_SendResponse(p_retry, BLE_OTAPS_OP_CODE_UPDATE_START, BLE_OTAPS_RESULT_NOT_SUPPORTED);
        return;
    }

    s_otapsImgType = p_event->eventField.onWrite.writeValue[1];

    switch (s_otapsImgType)
    {
        case BLE_OTAPS_IMG_TYPE_FW:
        {
            err = MW_DFU_FwImageStart();
            s_otapsPktTotal = s_otapsImgSize.fwSize;
            result = ble_otaps_MbaErrToResult(err);
        }
        break;

        default:
            break;
    }

    if (result == BLE_OTAPS_RESULT_SUCCESS)
    {
        sp_otapsPktBuf = OSAL_Malloc(BLE_OTAPS_MTU_SIZE);
        sp_otapsPlainBuf = OSAL_Malloc(BLE_OTAPS_IMG_BUF_SIZE);
        if (s_otapsEnc == BLE_OTAPS_ENC_AES_CBC)
            sp_otapsAes = OSAL_Malloc(sizeof(CRYPT_AES_CTX));
        if (sp_otapsPktBuf == NULL || sp_otapsPlainBuf == NULL
            || (s_otapsEnc == BLE_OTAPS_ENC_AES_CBC && sp_otapsAes == NULL)) 
        {
            result = BLE_OTAPS_RESULT_OPERATION_FAILED;
        }
        else
        {
            BLE_OTAPS_EvtStartInd_T evtInit;

            evtInit.imageType = s_otapsImgType;

            s_otapsState = BLE_OTAPS_STATE_UPDATE_START;
            s_otapsUpdateOffset = 0;
            s_otapsPktBufIdx = 0;
            s_otapsImgReceieved = 0;

            if (s_otapsEnc == BLE_OTAPS_ENC_AES_CBC)
                CRYPT_AES_KeySet(sp_otapsAes, s_otapsAesKey, 16, s_otapsAesIv, CRYPT_AES_DECRYPTION);

            ble_otaps_ConveyEvent(BLE_OTAPS_EVT_START_IND, (uint8_t *)&evtInit , sizeof(evtInit));
        }
    }

    err = ble_otaps_SendResponse(p_retry, BLE_OTAPS_OP_CODE_UPDATE_START, result);
}


static void ble_otaps_OnCtrlPtWrite(BLE_OTAPS_RetryList_T *p_retry, GATT_Event_T *p_event)
{
    switch (p_event->eventField.onWrite.writeValue[0])
    {
        case BLE_OTAPS_OP_CODE_UPDATE_REQUEST:
            ble_otaps_ProcUpdateRequest(p_retry, p_event);
            break;
        case BLE_OTAPS_OP_CODE_UPDATE_START:
            ble_otaps_ProcUpdateStart(p_retry, p_event);
            break;
        case BLE_OTAPS_OP_CODE_UPDATE_COMPLETE:
        {
            BLE_OTAPS_EvtCompleteInd_T evtComplete;

            if (s_otapsState == BLE_OTAPS_STATE_COMPLETE)
            {
                ble_otaps_SendResponse(p_retry, BLE_OTAPS_OP_CODE_UPDATE_COMPLETE, BLE_OTAPS_RESULT_INVALID_STATE);
                return;
            }

            if (s_otapsState >= BLE_OTAPS_STATE_UPDATE_START 
                && s_otapsImgReceieved >= s_otapsPktTotal)
            {
                evtComplete.errStatus = false;

                s_otapsState = BLE_OTAPS_STATE_WAIT_COMP_RSP;
            }
            else
            {
                ble_otaps_FreeBuf(p_retry);
            
                evtComplete.errStatus = true;
                
                ble_otaps_SendResponse(p_retry, BLE_OTAPS_OP_CODE_UPDATE_COMPLETE, BLE_OTAPS_RESULT_SUCCESS);

                s_otapsState = BLE_OTAPS_STATE_COMPLETE;
            }

            ble_otaps_ConveyEvent(BLE_OTAPS_EVT_COMPLETE_IND, (uint8_t *)&evtComplete , sizeof(evtComplete));
        }
        break;
        case BLE_OTAPS_OP_CODE_RESET_REQUEST:
            ble_otaps_ConveyEvent(BLE_OTAPS_EVT_RESET_IND, NULL, 0);
            break;
        default:
            break;
    }
}

static void ble_otaps_PacketHandler(BLE_OTAPS_RetryList_T *p_retry, GATT_Event_T *p_event)
{
    uint16_t err;
    uint8_t result;
    uint16_t copyLen, remainLen;
    uint8_t *p_copyData;
    BLE_OTAPS_EvtUpdatingInd_T evtUpdate;

    if (s_otapsState != BLE_OTAPS_STATE_UPDATE_START)
    {
        ble_otaps_SendDataResponse(p_retry, BLE_OTAPS_RESULT_INVALID_STATE);
        return;
    }

    p_copyData = p_event->eventField.onWrite.writeValue;
    remainLen = p_event->eventField.onWrite.writeDataLength;
    evtUpdate.length = 0;

    while (remainLen + s_otapsPktBufIdx >= BLE_OTAPS_MTU_SIZE)
    {
        copyLen = BLE_OTAPS_MTU_SIZE - s_otapsPktBufIdx;

        memcpy(sp_otapsPktBuf + s_otapsPktBufIdx, p_copyData, copyLen);
        s_otapsPktBufIdx = 0;
        p_copyData += copyLen;

        err = ble_otaps_UpdateImage(BLE_OTAPS_MTU_SIZE);

        result = ble_otaps_MbaErrToResult(err);

        if (result != BLE_OTAPS_RESULT_SUCCESS)
        {
            s_otapsState = BLE_OTAPS_STATE_UPDATE_ERROR;
            err = ble_otaps_SendDataResponse(p_retry, result);
            return;
        }
        evtUpdate.length += BLE_OTAPS_MTU_SIZE;

        remainLen -= copyLen;
        
    }

    if (remainLen)
    {
        memcpy(sp_otapsPktBuf + s_otapsPktBufIdx, p_copyData, remainLen);
        s_otapsPktBufIdx += remainLen;
    }

    if (p_event->eventField.onWrite.writeDataLength + s_otapsImgReceieved >= s_otapsPktTotal && s_otapsPktBufIdx)
    {
        err = ble_otaps_UpdateImage(s_otapsPktBufIdx);

        result = ble_otaps_MbaErrToResult(err);

        if (result != BLE_OTAPS_RESULT_SUCCESS)
        {
            s_otapsState = BLE_OTAPS_STATE_UPDATE_ERROR;
            err = ble_otaps_SendDataResponse(p_retry, result);
            return;
        }

        evtUpdate.length += s_otapsPktBufIdx;
    }

    if (evtUpdate.length)
    {
        ble_otaps_ConveyEvent(BLE_OTAPS_EVT_UPDATING_IND, (uint8_t *)&evtUpdate , sizeof(evtUpdate));
        err = ble_otaps_SendDataResponse(p_retry, result);
    }

    s_otapsImgReceieved += p_event->eventField.onWrite.writeDataLength;
}

static void ble_otaps_Disconnect()
{
    if (s_otapsState > BLE_OTAPS_STATE_CCCD_DISABLED)
    {
        ble_otaps_StopProc();
        
        ble_otaps_FreeBuf();
    }
}


static void ble_otaps_GapEventProcess(BLE_GAP_Event_T *p_event)
{
    BLE_OTAPS_RetryList_T    *p_retry;

    switch(p_event->eventId)
    {
        case BLE_GAP_EVT_CONNECTED:
        {
            if ((p_event->eventField.evtConnect.status == GAP_STATUS_SUCCESS))
            {
                p_retry=ble_otaps_GetRetryListByHandle(0);
                if(p_retry==NULL)
                {
                    ble_otaps_ConveyEvent(BLE_OTAPS_EVT_ERR_UNSPECIFIED_IND, NULL, 0);
                    return;
                }

                p_retry->connHandle=p_event->eventField.evtConnect.connHandle;
            }
        }
        break;
        case BLE_GAP_EVT_DISCONNECTED:
        {
            if (s_otapsConnHandle == p_event->eventField.evtDisconnect.connHandle)
                ble_otaps_Disconnect();

            p_retry=ble_otaps_GetRetryListByHandle(p_event->eventField.evtDisconnect.connHandle);
            if(p_retry!=NULL)
            {
                ble_otaps_FreeRetry(p_retry);
                p_retry->connHandle = 0;
            }
        }
        break;
        case BLE_GAP_EVT_TX_BUF_AVAILABLE:
        {
            p_retry=ble_otaps_GetRetryListByHandle(p_event->eventField.evtTxBufAvailable.connHandle);
        
            if (p_retry && p_retry->p_pkt)
            {
                switch (p_retry->type)
                {
                    case BLE_OTAPS_RETRY_TYPE_ERROR:
                    {
                        if (MBA_RES_SUCCESS == GATTS_SendErrorResponse(p_retry->connHandle, (GATTS_SendErrRespParams_T *)p_retry->p_pkt))
                            ble_otaps_FreeRetry(p_retry);
                    }
                    break;
                    case BLE_OTAPS_RETRY_TYPE_READ:
                    {
                        if (MBA_RES_SUCCESS == GATTS_SendReadResponse(p_retry->connHandle, (GATTS_SendReadRespParams_T *)p_retry->p_pkt))
                            ble_otaps_FreeRetry(p_retry);
                    }
                    break;
                    case BLE_OTAPS_RETRY_TYPE_WRITE:
                    {
                        if (MBA_RES_SUCCESS == GATTS_SendWriteResponse(p_retry->connHandle, (GATTS_SendWriteRespParams_T *)p_retry->p_pkt))
                        {
                            OSAL_Free(p_retry->p_pkt);
                            p_retry->p_pkt = NULL;
                            p_retry->type = BLE_OTAPS_RETRY_TYPE_NONE;

                            if (p_retry->p_gattEvt)
                            {
                                ble_otaps_OnCtrlPtWrite(p_retry, p_retry->p_gattEvt);

                                OSAL_Free(p_retry->p_gattEvt);
                                p_retry->p_gattEvt = NULL;
                            }
                        }
                    }
                    break;
                    case BLE_OTAPS_RETRY_TYPE_NTF:
                    {
                        if (MBA_RES_SUCCESS == GATTS_SendHandleValue(p_retry->connHandle, (GATTS_HandleValueParams_T *)p_retry->p_pkt))
                            ble_otaps_FreeRetry(p_retry);
                    }
                    break;

                    default:
                    break;

                }
            }
        }
        break;
        default:
            break;
    }
}

static void ble_otaps_GattEventHandler(GATT_Event_T *p_event)
{
    BLE_OTAPS_RetryList_T    *p_retry;

    switch (p_event->eventId)
    {
        case GATTS_EVT_WRITE:
        {
            uint8_t errCode = 0;

            p_retry=ble_otaps_GetRetryListByHandle(p_event->eventField.onWrite.connHandle);

            if (p_retry == NULL || p_retry->p_pkt != NULL)
                break;

            if (p_event->eventField.onWrite.attrHandle == BLE_OTAS_HDL_DATA_VAL)
            {
                /* No need to send response. */
                if (p_event->eventField.onWrite.connHandle == s_otapsConnHandle)
                    ble_otaps_PacketHandler(p_retry, p_event);
                return;
            }

            if ((p_event->eventField.onWrite.attrHandle != BLE_OTAS_HDL_CTRL_CCCD) &&
                (p_event->eventField.onWrite.attrHandle != BLE_OTAS_HDL_CTRL_VAL) &&
                (p_event->eventField.onWrite.attrHandle != BLE_OTAS_HDL_DATA_CCCD))
            {
                /* Only process write operations on SETTING_MANUAL_WRITE_RSP in OTA. */
                return;
            }


            if (sp_otapsCbRoutine == NULL)
            {
                errCode = ATT_ERRCODE_APPLICATION_ERROR;
            }
            else if (p_event->eventField.onWrite.writeType == ATT_PREPARE_WRITE_REQ)
            {
                errCode = ATT_ERRCODE_REQUEST_NOT_SUPPORT;
            }

            if (!errCode)
            {
                if (p_event->eventField.onWrite.attrHandle == BLE_OTAS_HDL_CTRL_CCCD ||
                    p_event->eventField.onWrite.attrHandle == BLE_OTAS_HDL_DATA_CCCD)
                {
                    uint16_t cccd;

                    BUF_LE_TO_U16(&cccd, p_event->eventField.onWrite.writeValue);

                    if (cccd == NOTIFICATION)
                    {
                        if (p_event->eventField.onWrite.attrHandle == BLE_OTAS_HDL_CTRL_CCCD)
                        {
                            s_otapsCccdStatus |= (BLE_OTAPS_CCCD_CTRL_ON);
                        }
                        else
                        {
                            s_otapsCccdStatus |= (BLE_OTAPS_CCCD_DATA_ON);
                        }

                        if ((s_otapsCccdStatus & (BLE_OTAPS_CCCD_CTRL_ON|BLE_OTAPS_CCCD_CTRL_ON))
                            == (BLE_OTAPS_CCCD_CTRL_ON|BLE_OTAPS_CCCD_CTRL_ON))
                            s_otapsState = BLE_OTAPS_STATE_CCCD_ENABLED;

                        errCode = 0x00;
                    }
                    else if (cccd == 0x0000)
                    {
                        if (s_otapsConnHandle == 0 || p_event->eventField.onWrite.connHandle == s_otapsConnHandle)
                        {
                            if (p_event->eventField.onWrite.attrHandle == BLE_OTAS_HDL_CTRL_CCCD)
                                s_otapsCccdStatus &= ~(BLE_OTAPS_CCCD_CTRL_ON);
                            else
                                s_otapsCccdStatus &= ~(BLE_OTAPS_CCCD_DATA_ON);
                        
                            s_otapsState = BLE_OTAPS_STATE_CCCD_DISABLED;
                            errCode = 0x00;
                        }
                        else
                            errCode = ATT_ERRCODE_WRITE_NOT_PERMITTED;
                    }
                    else
                    {
                        errCode = ATT_ERRCODE_APPLICATION_ERROR;
                    }
                }
                else if (p_event->eventField.onWrite.attrHandle == BLE_OTAS_HDL_CTRL_VAL)
                {
                    if (s_otapsState == BLE_OTAPS_STATE_CCCD_DISABLED
                        || (s_otapsConnHandle != 0 && p_event->eventField.onWrite.connHandle != s_otapsConnHandle)
                        || (s_otapsConnHandle == 0 && p_event->eventField.onWrite.writeValue[0] != BLE_OTAPS_OP_CODE_UPDATE_REQUEST))
                        errCode = ATT_ERRCODE_WRITE_NOT_PERMITTED;
                }
            }

            if (!errCode)
            {
                p_retry->p_pkt = OSAL_Malloc(sizeof(GATTS_SendWriteRespParams_T));
                if (p_retry->p_pkt != NULL)
                {
                    ((GATTS_SendWriteRespParams_T *)p_retry->p_pkt)->responseType = ATT_WRITE_RSP;
                    if (GATTS_SendWriteResponse(p_event->eventField.onWrite.connHandle, (GATTS_SendWriteRespParams_T *)p_retry->p_pkt) == MBA_RES_SUCCESS)
                    {
                        ble_otaps_FreeRetry(p_retry);

                        /* It will take long time to validate so we must send write response before calling BLE_OTAPS_OnCtrlPtWrite */
                        if (p_event->eventField.onWrite.attrHandle == BLE_OTAS_HDL_CTRL_VAL)
                        {
                            ble_otaps_OnCtrlPtWrite(p_retry, p_event);
                        }
                    }
                    else if (p_event->eventField.onWrite.attrHandle == BLE_OTAS_HDL_CTRL_VAL)
                    {
                        uint16_t validEvtLen;

                        validEvtLen = (uint32_t)p_event->eventField.onWrite.writeValue - (uint32_t)p_event + p_event->eventField.onWrite.writeDataLength;
                    
                        p_retry->p_gattEvt = OSAL_Malloc(validEvtLen);
                        if (p_retry->p_gattEvt)
                        {
                            memcpy(p_retry->p_gattEvt, p_event, validEvtLen);
                        }

                        p_retry->type = BLE_OTAPS_RETRY_TYPE_WRITE;
                    }
                }


            }
            else
            {
                p_retry->p_pkt = OSAL_Malloc(sizeof(GATTS_SendErrRespParams_T));
                if (p_retry->p_pkt != NULL)
                {
                    ((GATTS_SendErrRespParams_T *)p_retry->p_pkt)->reqOpcode = p_event->eventField.onWrite.writeType;
                    ((GATTS_SendErrRespParams_T *)p_retry->p_pkt)->attrHandle = p_event->eventField.onWrite.attrHandle;
                    ((GATTS_SendErrRespParams_T *)p_retry->p_pkt)->errorCode = errCode;
                    if (GATTS_SendErrorResponse(p_event->eventField.onWrite.connHandle, (GATTS_SendErrRespParams_T *)p_retry->p_pkt) == MBA_RES_SUCCESS)
                        ble_otaps_FreeRetry(p_retry);
                    else
                        p_retry->type = BLE_OTAPS_RETRY_TYPE_ERROR;
                }
            }
        }
        break;

        case GATTS_EVT_READ:
        {
            if (p_event->eventField.onRead.attrHandle == BLE_OTAS_HDL_FEATURE_VAL)
            {
                uint8_t errCode = 0;

                p_retry=ble_otaps_GetRetryListByHandle(p_event->eventField.onRead.connHandle);
                
                if (p_retry == NULL || p_retry->p_pkt != NULL)
                    break;

                if (sp_otapsCbRoutine == NULL)
                {
                    errCode = ATT_ERRCODE_APPLICATION_ERROR;
                }
                else if (s_otapsState == BLE_OTAPS_STATE_CCCD_DISABLED)
                {
                    errCode = ATT_ERRCODE_READ_NOT_PERMITTED;
                }
                else if (p_event->eventField.onRead.readType != ATT_READ_REQ)
                {
                    errCode = ATT_ERRCODE_REQUEST_NOT_SUPPORT;
                }


                if (!errCode)
                {
                    p_retry->p_pkt = OSAL_Malloc(sizeof(GATTS_SendReadRespParams_T));
                    if (p_retry->p_pkt != NULL)
                    {
                        ((GATTS_SendReadRespParams_T *)p_retry->p_pkt)->attrLength = 0x01;
                        ((GATTS_SendReadRespParams_T *)p_retry->p_pkt)->responseType = ATT_READ_RSP;
                        ((GATTS_SendReadRespParams_T *)p_retry->p_pkt)->attrValue[0] = BLE_OTAPS_FEATURE_SUPP_IMG_TYPE;
                        if (GATTS_SendReadResponse(p_event->eventField.onRead.connHandle, (GATTS_SendReadRespParams_T *)p_retry->p_pkt) == MBA_RES_SUCCESS)
                            ble_otaps_FreeRetry(p_retry);
                        else
                            p_retry->type = BLE_OTAPS_RETRY_TYPE_READ;
                    }
                }
                else
                {
                    p_retry->p_pkt = OSAL_Malloc(sizeof(GATTS_SendErrRespParams_T));
                    if (p_retry->p_pkt != NULL)
                    {
                        ((GATTS_SendErrRespParams_T *)p_retry->p_pkt)->reqOpcode = p_event->eventField.onRead.readType;
                        ((GATTS_SendErrRespParams_T *)p_retry->p_pkt)->attrHandle = p_event->eventField.onRead.attrHandle;
                        ((GATTS_SendErrRespParams_T *)p_retry->p_pkt)->errorCode = errCode;
                        if (GATTS_SendErrorResponse(p_event->eventField.onWrite.connHandle, (GATTS_SendErrRespParams_T *)p_retry->p_pkt) == MBA_RES_SUCCESS)
                            ble_otaps_FreeRetry(p_retry);
                        else
                            p_retry->type = BLE_OTAPS_RETRY_TYPE_ERROR;
                    }
                }
            }
        }
        break;

        default:
            break;
    }
}

uint16_t BLE_OTAPS_Init()
{
	sp_otapsCbRoutine = NULL;
    sp_otapsPktBuf = NULL;
    sp_otapsPlainBuf = NULL;
    sp_otapsAes = NULL;

    s_otapsState = BLE_OTAPS_STATE_CCCD_DISABLED;

    s_otapsConnHandle = 0x00;

    s_otapsCccdStatus = 0x00;

    memset(s_otapsRetry, 0x00, sizeof(s_otapsRetry));

    return BLE_OTAS_Add();
}

void BLE_OTAPS_EventRegister(BLE_OTAPS_EventCb_T bleOtapsRoutine)
{
    sp_otapsCbRoutine = bleOtapsRoutine;
}

uint16_t BLE_OTAPS_UpdateStop()
{
    if (ble_otaps_IsProcInProgress())
    {
        s_otapsState = BLE_OTAPS_STATE_CCCD_ENABLED;

        ble_otaps_FreeBuf();
    }

    return MBA_RES_SUCCESS;
}


uint16_t BLE_OTAPS_SetEncrytionInfo(uint8_t * p_iv, uint8_t * p_key)
{
    if (ble_otaps_IsProcInProgress())
        return MBA_RES_BAD_STATE;

    memcpy(s_otapsAesIv, p_iv, 16);
    memcpy(s_otapsAesKey, p_key, 16);

    return MBA_RES_SUCCESS;
}

uint16_t BLE_OTAPS_UpdateResponse(uint16_t connHandle, bool isAllow, BLE_OTAPS_DevInfo_T * p_devInfo)
{
    uint16_t status;
    BLE_OTAPS_RetryList_T *p_retry;

    p_retry = ble_otaps_GetRetryListByHandle(connHandle);

    if (p_retry == NULL)
        return MBA_RES_INVALID_PARA;
    
    if (s_otapsState != BLE_OTAPS_STATE_WAIT_REQ_RSP
        || p_retry->p_pkt)
        return MBA_RES_BAD_STATE;

    if (isAllow)
    {
        s_otapsState = BLE_OTAPS_STATE_UPDATE_REQ;

        s_otapsConnHandle = connHandle;
    }

    status = ble_otaps_SendUpdateReqResponse(p_retry, (isAllow) ? BLE_OTAPS_RESULT_SUCCESS : BLE_OTAPS_RESULT_NOT_SUPPORTED, p_devInfo);

    ble_otaps_FreeRetry(p_retry);

    return status;
}

uint16_t BLE_OTAPS_CompleteResponse(bool success)
{
    uint16_t status;
    BLE_OTAPS_RetryList_T *p_retry;

    p_retry = ble_otaps_GetRetryListByHandle(s_otapsConnHandle);

    if (s_otapsState != BLE_OTAPS_STATE_WAIT_COMP_RSP
        || p_retry == NULL || p_retry->p_pkt)
        return MBA_RES_BAD_STATE;

    s_otapsState = BLE_OTAPS_STATE_COMPLETE;

    status = ble_otaps_SendResponse(p_retry, BLE_OTAPS_OP_CODE_UPDATE_COMPLETE, (success) ? BLE_OTAPS_RESULT_SUCCESS : BLE_OTAPS_RESULT_OPERATION_FAILED);

    ble_otaps_FreeRetry(p_retry);

    return status;
}


void BLE_OTAPS_BleEventHandler(STACK_Event_T *p_stackEvent)
{
    switch (p_stackEvent->groupId)
    {
        case STACK_GRP_BLE_GAP:
        {
            ble_otaps_GapEventProcess((BLE_GAP_Event_T *)p_stackEvent->p_event);
        }
        break;

        case STACK_GRP_GATT:
        {
            ble_otaps_GattEventHandler((GATT_Event_T *)p_stackEvent->p_event);
        }
        break;

        default:
        break;
    }
}

