/*******************************************************************************
  BLE Service Change Manager Middleware Source File

  Company:
    Microchip Technology Inc.

  File Name:
    ble_scm.c

  Summary:
    This file contains the BLE Service Change Manager functions and event for application user.

  Description:
    This file contains the BLE Service Change Manager functions and event for application user.
    The "BLE_SCM_Init" function shall be called in the "APP_Initialize" function to 
    initialize the this modules in the system.
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
#include "osal/osal_freertos_extend.h"
#include "mba_error_defs.h"
#include "gatt.h"
#include "ble_smp.h"
#include "ble_util/byte_stream.h"
#include "ble_gcm/ble_scm.h"


// *****************************************************************************
// *****************************************************************************
// Section: Macros
// *****************************************************************************
// *****************************************************************************

#define UUID_SVC_GATT                               0x1801  /**< UUID of GATT Service. */
#define UUID_CHAR_SERVICE_CHANGE                    0x2A05  /**< UUID of Characterisitc Service Change. */
#define UUID_CHAR_CLIENT_SUPPORTED_FEATURES         0x2B29  /**< UUID of Client Supported Features. */

/**@brief The definition of discovered characteristics index of GATT service. */
typedef enum DISC_INDEX_GattSvc_T
{
    DISC_INDEX_GATT_SC_CHAR,                                /**< Service Change Characteristic. */
    DISC_INDEX_GATT_SC_CCCD,                                /**< Service Change Client Characteristic Configuration Descriptor. */
    DISC_INDEX_GATT_CSF_CHAR                                /**< Client Supported Feature Characteristic. */
}DISC_INDEX_GattSvc_T;

/**@brief The definition of configuration procedures in BLE_SCM. */
typedef enum BLE_SCM_CofigProcedure_T
{
    BLE_SCM_CONFIG_NONE,                                    /**< No configuration.*/
    BLE_SCM_CONFIG_GATT_ENABLE_INDICATION,                  /**< Enable Service Change Indication of Remote GATT Service. */
    BLE_SCM_CONFIG_GATT_WRITE_FEATURES,                     /**< Write Client Supported Features to Remote GATT Service. */
    BLE_SCM_CONFIG_END
}BLE_SCM_CofigProcedure_T;

typedef enum BLE_SCM_ConfigStatus_T
{
    BLE_SCM_CONFIG_STATUS_SUCCESS,                          /**< Successfully execute the configuration. */
    BLE_SCM_CONFIG_STATUS_FAIL,                             /**< Fail to execute the configuration. Try another configuration. */
    BLE_SCM_CONFIG_STATUS_ATT_BUSY,                         /**< ATT protocol is busy. */
    BLE_SCM_CONFIG_STATUS_END
}BLE_SCM_ConfigStatus_T;

// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
// *****************************************************************************

/**@brief The structure of connection instance of service change manager module. */
typedef struct BLE_SCM_Conn_T
{
    uint16_t                connHandle;                     /**< Connection handle. */
    uint8_t                 connIndex;                      /**< The index of this connection in connection database array */
    uint8_t                 configProcedure;                /**< Current configure procedure after discovery complete. */
    bool                    isGattSvcFound;                 /**< Record GATT service is found or not of the connection. */
    bool                    isPairingComplete;              /**< Record if paring complete received of the connection. Discovered handle information need to be sent to application when pairing complete. */
    uint8_t                 queueProcedure;                 /**< Record queued procedure by the ATT protocol. */
} BLE_SCM_Conn_T;

/**@brief The database of service change manager module. */
typedef struct BLE_SCM_Ctrl_T
{
    BLE_SCM_Conn_T          conn[BLE_GAP_MAX_LINK_NBR];     /**< Connection database for service change manager module. */
} BLE_SCM_Ctrl_T;

// *****************************************************************************
// *****************************************************************************
// Section: Local Variables
// *****************************************************************************
// *****************************************************************************

BLE_SCM_EventCb_T           s_scmEventCb;                   /* Events callback function. */
BLE_SCM_Ctrl_T              s_scmCtrl;                      /* SCM module database. */

/* Discovery information of GATT Service. */
static const uint8_t        s_svcUuidGatt[ATT_UUID_LENGTH_2] = {UINT16_TO_BYTES(UUID_SVC_GATT)};
static const ATT_Uuid_T     s_discCharSc =          { {UINT16_TO_BYTES(UUID_CHAR_SERVICE_CHANGE)}, ATT_UUID_LENGTH_2 };
static const ATT_Uuid_T     s_discCharScCccd =      { {UINT16_TO_BYTES(UUID_DESCRIPTOR_CLIENT_CHAR_CONFIG)}, ATT_UUID_LENGTH_2 };
static const ATT_Uuid_T     s_discCharCsf    =      { {UINT16_TO_BYTES(UUID_CHAR_CLIENT_SUPPORTED_FEATURES)}, ATT_UUID_LENGTH_2 };
static BLE_DD_DiscChar_T    s_gattSvcChange =       { &s_discCharSc, 0 };
static BLE_DD_DiscChar_T    s_gattSvcChangeCccd =   { &s_discCharScCccd, CHAR_SET_DESCRIPTOR };
static BLE_DD_DiscChar_T    s_gattCliSupFeature =   { &s_discCharCsf, 0 };
static BLE_DD_DiscChar_T    *s_gattDiscCharList[] =
{
    &s_gattSvcChange,           /* GATT Service Change Characteristic */
    &s_gattSvcChangeCccd,       /* GATT Service Change Characteristic CCCD */
    &s_gattCliSupFeature        /* GATT Client Supported Feature Characteristic */
};

/* Database recording discovery response of GATT Service. */
static BLE_DD_CharInfo_T    s_gattCharInfoList[BLE_GAP_MAX_LINK_NBR][BLE_SCM_GATT_DISC_CHAR_NUM];
static BLE_DD_CharList_T    s_gattCharList[BLE_GAP_MAX_LINK_NBR];

/**@brief API returned code mapping between SCM and MBA. */
static const uint16_t       s_scmMbaRetCodeMap[]=
{
    BLE_SCM_CONFIG_STATUS_SUCCESS,          //MBA_RES_SUCCESS
    BLE_SCM_CONFIG_STATUS_FAIL,             //MBA_RES_FAIL
    BLE_SCM_CONFIG_STATUS_FAIL,             //MBA_RES_OOM
    BLE_SCM_CONFIG_STATUS_FAIL,             //MBA_RES_INVALID_PARA
    BLE_SCM_CONFIG_STATUS_FAIL,             //MBA_RES_NO_RESOURCE
    BLE_SCM_CONFIG_STATUS_FAIL,             //MBA_RES_BAD_STATE
    BLE_SCM_CONFIG_STATUS_FAIL,             //MBA_RES_PENDING_DUE_TO_SECURITY
    BLE_SCM_CONFIG_STATUS_ATT_BUSY          //MBA_RES_BUSY
};


// *****************************************************************************
// *****************************************************************************
// Section: Functions
// *****************************************************************************
// *****************************************************************************
static void ble_scm_InitConn(BLE_SCM_Conn_T *p_conn)
{
    memset((uint8_t *)p_conn, 0, sizeof(BLE_SCM_Conn_T));
}

static BLE_SCM_Conn_T *ble_scm_FindConnByHandle(uint16_t connHandle)
{
    uint8_t i;

    for (i=0; i<BLE_GAP_MAX_LINK_NBR; i++)
    {
        if (s_scmCtrl.conn[i].connHandle == connHandle)
        {
            s_scmCtrl.conn[i].connIndex = i;
            return &s_scmCtrl.conn[i];
        }
    }
    return NULL;
}

/** @brief Create connection instance for specific connection. \n
 *         The instance would be created by @ref BLE_SCM_SetBondedCharInfo() or BLE_GAP_EVT_CONNECTED event.
 */
static BLE_SCM_Conn_T *ble_scm_CreateConn(uint16_t connHandle)
{
    BLE_SCM_Conn_T *p_conn;

    p_conn = ble_scm_FindConnByHandle(0);

    if (p_conn != NULL)
    {
        p_conn->connHandle = connHandle;
        p_conn->queueProcedure = BLE_SCM_CONFIG_NONE;
    }

    return p_conn;
}

static void ble_scm_InitCharList()
{
    uint8_t i, j;

    for(i=0; i<BLE_GAP_MAX_LINK_NBR; i++)
    {
        s_gattCharList[i].connHandle = 0;
        s_gattCharList[i].p_charInfo = (BLE_DD_CharInfo_T *) &(s_gattCharInfoList[i]);

        for(j=0; j<BLE_SCM_GATT_DISC_CHAR_NUM; j++)
        {
            s_gattCharInfoList[i][j].charHandle = 0;
            s_gattCharInfoList[i][j].property = 0;
        }
    }
}

static uint16_t ble_scm_GetScAttrHandle(uint16_t connHandle, uint8_t index)
{
    uint8_t i;

    for (i=0; i<BLE_GAP_MAX_LINK_NBR; i++)
    {
        if (s_gattCharList[i].connHandle == connHandle)
        {
            if (s_gattCharList[i].p_charInfo[index].charHandle == 0)
            {
                break;
            }
            else
            {
                return s_gattCharList[i].p_charInfo[index].charHandle;
            }
        }
    }
    return 0;
}

static void ble_scm_GapEventProcess(BLE_GAP_Event_T *p_event)
{
    switch (p_event->eventId)
    {
        case BLE_GAP_EVT_CONNECTED:
        {
            if ((p_event->eventField.evtConnect.status == GAP_STATUS_SUCCESS))
            {
                /* Create connection instance if it is not created in BLE_SCM_SetBondedCharInfo(). */
                if (ble_scm_FindConnByHandle(p_event->eventField.evtConnect.connHandle) == NULL)
                {
                    ble_scm_CreateConn(p_event->eventField.evtConnect.connHandle);
                }
            }
        }
        break;

        case BLE_GAP_EVT_DISCONNECTED:
        {
            BLE_SCM_Conn_T *p_conn;

            /* Find connection instance */
            p_conn = ble_scm_FindConnByHandle(p_event->eventField.evtDisconnect.connHandle);

            if (p_conn != NULL)
            {
                memset(s_gattCharInfoList[p_conn->connIndex], 0, (sizeof(BLE_DD_CharInfo_T)*BLE_SCM_GATT_DISC_CHAR_NUM));
                ble_scm_InitConn(p_conn);
            }
        }
        break;

        default:
        break;
    }
}

static void ble_scm_ProcServiceChange(uint16_t connHandle, uint16_t affectedStartHandle, uint16_t affectedEndHandle)
{
    if (s_scmEventCb != NULL)
    {
        BLE_SCM_Event_T scmEvent;

        scmEvent.eventId = BLE_SCM_EVT_SVC_CHANGE;
        scmEvent.eventField.evtServiceChange.connHandle = connHandle;
        scmEvent.eventField.evtServiceChange.scStartHandle = affectedStartHandle;
        scmEvent.eventField.evtServiceChange.scEndHandle = affectedEndHandle;
        s_scmEventCb(&scmEvent);
    }
}

static uint16_t ble_scm_SendGattWriteRequest(BLE_SCM_Conn_T *p_conn, uint16_t attrHandle, uint16_t attrLength, uint8_t *p_attrValue)
{
    GATTC_WriteParams_T *p_writeParams;
    uint16_t            result;

    p_writeParams = OSAL_Malloc(sizeof(GATTC_WriteParams_T));
    if (p_writeParams != NULL)
    {
        p_writeParams->charHandle = attrHandle;
        p_writeParams->charLength = attrLength;
        memcpy(p_writeParams->charValue, p_attrValue, attrLength);
        p_writeParams->writeType = ATT_WRITE_REQ;
        p_writeParams->valueOffset = 0;
        p_writeParams->flags = 0;
        result = GATTC_Write(p_conn->connHandle, p_writeParams);
        OSAL_Free(p_writeParams);
    }
    else
    {
        result = MBA_RES_OOM;
    }
    return result;
}

static uint16_t ble_scm_EnableServiceChangeIndication(BLE_SCM_Conn_T *p_conn)
{
    uint16_t    scCccdHandle, apiStatus, result;
    uint8_t     cccdValue[] = {UINT16_TO_BYTES(INDICATION)};

    scCccdHandle = ble_scm_GetScAttrHandle(p_conn->connHandle, DISC_INDEX_GATT_SC_CCCD);

    if (scCccdHandle != 0)
    {
        apiStatus = ble_scm_SendGattWriteRequest(p_conn, scCccdHandle, sizeof(cccdValue), cccdValue);
        result = s_scmMbaRetCodeMap[apiStatus];
    }
    else
    {
        result = BLE_SCM_CONFIG_STATUS_FAIL;
    }
    return result;
}

static uint16_t ble_scm_WriteClientSupportFeatures(BLE_SCM_Conn_T *p_conn)
{
    uint16_t    csfHandle, apiStatus, result;
    uint8_t     feature[] = {GATT_ROBUST_CACHING};

    csfHandle = ble_scm_GetScAttrHandle(p_conn->connHandle, DISC_INDEX_GATT_CSF_CHAR);

    if (csfHandle != 0)
    {
        apiStatus = ble_scm_SendGattWriteRequest(p_conn, csfHandle, sizeof(feature), feature);
        result = s_scmMbaRetCodeMap[apiStatus];
    }
    else
    {
        result = BLE_SCM_CONFIG_STATUS_FAIL;
    }
    return result;
}

static void ble_scm_ConfigProcedureStateMachine(BLE_SCM_Conn_T *p_conn, uint8_t procedure)
{
    if (procedure == BLE_SCM_CONFIG_GATT_ENABLE_INDICATION)
    {
        uint16_t result;

        result = ble_scm_EnableServiceChangeIndication(p_conn);

        if (result == BLE_SCM_CONFIG_STATUS_SUCCESS)
        {
            p_conn->configProcedure = procedure;
        }
        else if (result == BLE_SCM_CONFIG_STATUS_ATT_BUSY)
        {
            p_conn->queueProcedure = procedure;
        }
        else
        {
            procedure += 1;
        }
    }

    if (procedure == BLE_SCM_CONFIG_GATT_WRITE_FEATURES)
    {
        uint16_t result;

        result = ble_scm_WriteClientSupportFeatures(p_conn);

        if (result == BLE_SCM_CONFIG_STATUS_SUCCESS)
        {
            p_conn->configProcedure = procedure;
        }
        else if (result == BLE_SCM_CONFIG_STATUS_ATT_BUSY)
        {
            p_conn->queueProcedure = procedure;
        }
        else
        {
            procedure += 1;
        }
    }

    if (procedure == BLE_SCM_CONFIG_END)
    {
        p_conn->configProcedure = BLE_SCM_CONFIG_NONE;
        if (s_scmEventCb != NULL)
        {
            BLE_SCM_Event_T scmEvent;

            scmEvent.eventId = BLE_SCM_EVT_CONFIGURED;
            scmEvent.eventField.evtConfigured.connHandle = p_conn->connHandle;
            s_scmEventCb(&scmEvent);
        }
    }
}

static void ble_scm_GattEventProcess(GATT_Event_T *p_event)
{
    switch (p_event->eventId)
    {
        case GATTC_EVT_ERROR_RESP:
        {
            BLE_SCM_Conn_T *p_conn;

            p_conn = ble_scm_FindConnByHandle(p_event->eventField.onError.connHandle);
            if (p_conn != NULL)
            {
                if (p_event->eventField.onError.errCode == ATT_ERRCODE_DATABASE_OUT_OF_SYNC)
                {
                    p_conn->configProcedure = BLE_SCM_CONFIG_NONE;
                    p_conn->isGattSvcFound = false;
                    ble_scm_ProcServiceChange(p_conn->connHandle, 0x0001, 0xFFFF);
                }
                else
                {
                    if (p_conn->configProcedure)
                    {
                        ble_scm_ConfigProcedureStateMachine(p_conn, (p_conn->configProcedure+1));
                    }
                }
            }
        }
        break;

        case GATTC_EVT_WRITE_RESP:
        {
            BLE_SCM_Conn_T *p_conn;

            p_conn = ble_scm_FindConnByHandle(p_event->eventField.onWriteResp.connHandle);

            if ((p_conn != NULL) && (p_conn->configProcedure))
            {
                ble_scm_ConfigProcedureStateMachine(p_conn, (p_conn->configProcedure+1));
            }
        }
        break;

        case GATTC_EVT_HV_INDICATE:
        {
            if (p_event->eventField.onIndication.charHandle == ble_scm_GetScAttrHandle(p_event->eventField.onIndication.connHandle, DISC_INDEX_GATT_SC_CHAR))
            {
                uint16_t        affectedStartHandle, affectedEndHandle;
                BLE_SCM_Conn_T  *p_conn;

                p_conn = ble_scm_FindConnByHandle(p_event->eventField.onIndication.connHandle);
                if (p_conn != NULL)
                {
                    p_conn->isGattSvcFound = false;
                }

                BUF_LE_TO_U16(&affectedStartHandle, &p_event->eventField.onIndication.receivedValue[0]);
                BUF_LE_TO_U16(&affectedEndHandle, &p_event->eventField.onIndication.receivedValue[2]);
                ble_scm_ProcServiceChange(p_event->eventField.onIndication.connHandle, affectedStartHandle, affectedEndHandle);
            }
        }
        break;

        case GATTC_EVT_PROTOCOL_AVAILABLE:
        {
            BLE_SCM_Conn_T *p_conn;

            p_conn = ble_scm_FindConnByHandle(p_event->eventField.onClientProtocolAvailable.connHandle);

            if ((p_conn != NULL) && (p_conn->queueProcedure))
            {
                ble_scm_ConfigProcedureStateMachine(p_conn, p_conn->queueProcedure);
                p_conn->queueProcedure = 0;
            }
        }
        break;

        default:
        break;
    }
}

static void ble_scm_SmpEventProcess(BLE_SMP_Event_T *p_event)
{
    switch (p_event->eventId)
    {
        case BLE_SMP_EVT_PAIRING_COMPLETE:
        {
            BLE_SCM_Conn_T *p_conn;

            p_conn = ble_scm_FindConnByHandle(p_event->eventField.evtPairingComplete.connHandle);

            if (p_conn != NULL)
            {
                if (p_event->eventField.evtPairingComplete.status == BLE_SMP_PAIRING_SUCCESS)
                {
                    p_conn->isPairingComplete = true;
                    if (p_conn->isGattSvcFound)
                    {
                        /* Send handle information to application when connection paired and characteristics discovered. */
                        if (s_scmEventCb != NULL)
                        {
                            BLE_SCM_Event_T scmEvent;

                            scmEvent.eventId = BLE_SCM_EVT_BONDED_CHAR_INFO;
                            scmEvent.eventField.evtBondedCharInfo.connHandle = p_event->eventField.evtPairingComplete.connHandle;
                            memcpy(scmEvent.eventField.evtBondedCharInfo.charInfo, s_gattCharInfoList[p_conn->connIndex], sizeof(BLE_DD_CharInfo_T)*BLE_SCM_GATT_DISC_CHAR_NUM);
                            s_scmEventCb(&scmEvent);
                        }
                    }
                }
            }
        }
        break;

        default:
        break;
    }
}

void BLE_SCM_Init()
{
    BLE_DD_DiscSvc_T gattDisc;

    memset((uint8_t *)&s_scmCtrl, 0, sizeof(BLE_SCM_Ctrl_T));

    ble_scm_InitCharList();

    /* Register GATT service discovery */
    gattDisc.svcUuid.uuidLength = ATT_UUID_LENGTH_2;
    memcpy(gattDisc.svcUuid.uuid, s_svcUuidGatt, ATT_UUID_LENGTH_2);
    gattDisc.p_discChars = s_gattDiscCharList;
    gattDisc.p_charList = s_gattCharList;
    gattDisc.discCharsNum = BLE_SCM_GATT_DISC_CHAR_NUM;
    BLE_DD_ServiceDiscoveryRegister(&gattDisc);
}

void BLE_SCM_EventRegister(BLE_SCM_EventCb_T eventCb)
{
    s_scmEventCb = eventCb;
}

void BLE_SCM_BleEventHandler(STACK_Event_T *p_stackEvent)
{
    switch (p_stackEvent->groupId)
    {
        case STACK_GRP_BLE_GAP:
        {
            ble_scm_GapEventProcess((BLE_GAP_Event_T *)p_stackEvent->p_event);
        }
        break;

        case STACK_GRP_GATT:
        {
            ble_scm_GattEventProcess((GATT_Event_T *)p_stackEvent->p_event);
        }
        break;

        case STACK_GRP_BLE_SMP:
        {
            ble_scm_SmpEventProcess((BLE_SMP_Event_T *)p_stackEvent->p_event);
        }
        break;

        default:
        break;
    }
}

void BLE_SCM_BleDdEventHandler(BLE_DD_Event_T *p_event)
{
    switch (p_event->eventId)
    {
        case BLE_DD_EVT_DISC_COMPLETE:
        {
            BLE_SCM_Conn_T *p_conn;

            p_conn = ble_scm_FindConnByHandle(p_event->eventField.evtDiscResult.connHandle);

            /* Check if GATT service is found of the connection */
            if (p_conn != NULL)
            {
                /* By checking the discovered handles exist or not. */
                if (ble_scm_GetScAttrHandle(p_event->eventField.evtDiscResult.connHandle, DISC_INDEX_GATT_SC_CHAR) != 0)
                {
                    p_conn->isGattSvcFound = true;

                    if (s_scmEventCb != NULL)
                    {
                        BLE_SCM_Event_T scmEvent;

                        scmEvent.eventId = BLE_SCM_EVT_DISC_COMPLETE;
                        scmEvent.eventField.evtDiscComplete.connHandle = p_event->eventField.evtDiscResult.connHandle;
                        s_scmEventCb(&scmEvent);

                        if (p_conn->isPairingComplete)
                        {
                            scmEvent.eventId = BLE_SCM_EVT_BONDED_CHAR_INFO;
                            scmEvent.eventField.evtBondedCharInfo.connHandle = p_event->eventField.evtDiscResult.connHandle;
                            memcpy(scmEvent.eventField.evtBondedCharInfo.charInfo, s_gattCharInfoList[p_conn->connIndex], sizeof(BLE_DD_CharInfo_T)*BLE_SCM_GATT_DISC_CHAR_NUM);
                            s_scmEventCb(&scmEvent);
                        }
                    }

                    /* Start configuration procedure when discovery complete and GATT svc is found. */
                    ble_scm_ConfigProcedureStateMachine(p_conn, BLE_SCM_CONFIG_GATT_ENABLE_INDICATION);
                }
                else
                {
                    p_conn->isGattSvcFound = false;
                }
            }
        }
        break;

        default:
        break;
    }
}

void BLE_SCM_SetBondedCharInfo(uint16_t connHandle, BLE_DD_CharInfo_T *p_charInfo)
{
    BLE_SCM_Conn_T *p_conn;

    p_conn = ble_scm_FindConnByHandle(connHandle);

    if (p_conn == NULL)
    {
        p_conn = ble_scm_CreateConn(connHandle);
    }

    if (p_conn != NULL)
    {
        memcpy(s_gattCharInfoList[p_conn->connIndex], p_charInfo, (sizeof(BLE_DD_CharInfo_T)*BLE_SCM_GATT_DISC_CHAR_NUM));
        p_conn->isGattSvcFound = true;
    }
}

