/*******************************************************************************
  Application BLE Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_ble_conn_handler.c

  Summary:
    This file contains the Application BLE functions for this project.

  Description:
    This file contains the Application BLE functions for this project.
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2022 Microchip Technology Inc. and its subsidiaries.
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
#include <stdint.h>
#include "app_ble_conn_handler.h"
#include "ble_dm/ble_dm.h"
#include "app_timer/app_timer.h"
#include "app_adv.h"
#include "app_trps.h"
#include "app_ble_sensor.h"
#include "peripheral/gpio/plib_gpio.h"
#include "system/console/sys_console.h"

// *****************************************************************************
// *****************************************************************************
// Section: Macros
// *****************************************************************************
// *****************************************************************************


// *****************************************************************************
// *****************************************************************************
// Section: Global Variables
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
// *****************************************************************************
// Section: Local Variables
// *****************************************************************************
// *****************************************************************************
static APP_BLE_ConnList_T                   s_bleConnList[BLE_GAP_MAX_LINK_NBR];
static APP_BLE_ConnList_T                   *sp_currentBleLink = NULL;

// *****************************************************************************
// *****************************************************************************
// Section: Functions
// *****************************************************************************
// *****************************************************************************
static void APP_ClearConnListByConnHandle(uint16_t connHandle)
{
    uint8_t i;

    for (i = 0; i < BLE_GAP_MAX_LINK_NBR; i++)
    {
        if (s_bleConnList[i].connData.handle == connHandle)
        {
            memset((uint8_t *)(&s_bleConnList[i]), 0, sizeof(APP_BLE_ConnList_T));

            s_bleConnList[i].linkState= APP_BLE_STATE_STANDBY;
        }
    }
}

static APP_BLE_ConnList_T *APP_GetFreeConnList(void)
{
    uint8_t i;

    for (i = 0; i < BLE_GAP_MAX_LINK_NBR; i++)
    {
        if (s_bleConnList[i].connData.handle == 0)
        {
            return (&s_bleConnList[i]);
        }
    }
    return NULL;
}

APP_BLE_ConnList_T *APP_GetConnInfoByConnHandle(uint16_t connHandle)
{
    uint8_t i;

    for (i = 0; i < BLE_GAP_MAX_LINK_NBR; i++)
    {
        if (s_bleConnList[i].connData.handle == connHandle)
        {
            return (&s_bleConnList[i]);
        }
    }
    return NULL;
}

uint16_t APP_GetConnHandleByIndex(uint8_t index)
{
    if (index < BLE_GAP_MAX_LINK_NBR)
    {
        if ((s_bleConnList[index].connData.handle != 0) && (s_bleConnList[index].linkState != APP_BLE_STATE_STANDBY))
            return s_bleConnList[index].connData.handle;
    }

    return 0xFFFF;
}

void APP_BleGapConnEvtHandler(BLE_GAP_Event_T *p_event)
{
    APP_BLE_ConnList_T *p_bleConn = NULL;    
    switch(p_event->eventId)
    {
        case BLE_GAP_EVT_CONNECTED:
        {
            p_bleConn = APP_GetFreeConnList();

            if (p_bleConn)
            {
                GATTS_UpdateBondingInfo(p_event->eventField.evtConnect.connHandle, NULL, 0, NULL); 

                /* Update the connection parameter */
                p_bleConn->linkState                        = APP_BLE_STATE_CONNECTED;
                p_bleConn->connData.role                    = p_event->eventField.evtConnect.role;        // 0x00: Central, 0x01:Peripheral
                p_bleConn->connData.handle                  = p_event->eventField.evtConnect.connHandle;
                p_bleConn->connData.connInterval            = p_event->eventField.evtConnect.interval; 
                p_bleConn->connData.connLatency             = p_event->eventField.evtConnect.latency;    
                p_bleConn->connData.supervisionTimeout      = p_event->eventField.evtConnect.supervisionTimeout;

                /* Save Remote Device Address */
                p_bleConn->connData.remoteAddr.addrType = p_event->eventField.evtConnect.remoteAddr.addrType;                
                memcpy((uint8_t *)p_bleConn->connData.remoteAddr.addr, (uint8_t *)p_event->eventField.evtConnect.remoteAddr.addr, GAP_MAX_BD_ADDRESS_LEN);

                SYS_CONSOLE_MESSAGE("[BLE] Connected to Peer Device: 0x");
                int8_t idx;
                for(idx=(GAP_MAX_BD_ADDRESS_LEN-1); idx>=0; idx--)
                    SYS_CONSOLE_PRINT("%02x", p_bleConn->connData.remoteAddr.addr[idx]);
                SYS_CONSOLE_PRINT("\n\r[BLE] Connection Handle: %d\n\r",p_bleConn->connData.handle);
                
                APP_TIMER_StopTimer(APP_TIMER_ADV_CTRL);
                USER_LED_Clear();                
                               
            }
        }
        break;

        case BLE_GAP_EVT_DISCONNECTED:
        {
            //Clear connection list
            APP_ClearConnListByConnHandle(p_event->eventField.evtDisconnect.connHandle);
            SYS_CONSOLE_PRINT("[BLE] Disconnected Handle: %d\n\r",p_event->eventField.evtDisconnect.connHandle);                
            APP_TRPS_DiscEvtProc(p_event->eventField.evtDisconnect.connHandle);
            APP_TRPS_Sensor_DiscEvtProc();
            
            /* Restart advertising */
            APP_ADV_Start();
            SYS_CONSOLE_MESSAGE("[BLE] Advertisement Started\n\r");
            
        }
        break;

        case BLE_GAP_EVT_CONN_PARAM_UPDATE:
        {
            /* Update the connection parameter */
            if (p_event->eventField.evtConnParamUpdate.status == 0)
            {

                p_bleConn = APP_GetConnInfoByConnHandle(p_event->eventField.evtConnParamUpdate.connHandle);

                if (p_bleConn)
                {
                    p_bleConn->connData.handle                  = p_event->eventField.evtConnParamUpdate.connHandle;
                    p_bleConn->connData.connInterval            = p_event->eventField.evtConnParamUpdate.connParam.intervalMin;
                    p_bleConn->connData.connLatency             = p_event->eventField.evtConnParamUpdate.connParam.latency;
                    p_bleConn->connData.supervisionTimeout      = p_event->eventField.evtConnParamUpdate.connParam.supervisionTimeout;
                }
            }
        }
        break;

        case BLE_GAP_EVT_ENCRYPT_STATUS:
        {
            p_bleConn = APP_GetConnInfoByConnHandle(p_event->eventField.evtEncryptStatus.connHandle);

            if (p_event->eventField.evtEncryptStatus.status != BLE_GAP_ENCRYPT_SUCCESS)
            {
                BLE_GAP_Disconnect(p_event->eventField.evtEncryptStatus.connHandle, GAP_DISC_REASON_REMOTE_TERMINATE);
            }
        }
        break;

        case BLE_GAP_EVT_ADV_REPORT:
        {
            /* TODO: implement your application code.*/
        }
        break;

        case BLE_GAP_EVT_ENC_INFO_REQUEST:
        {
            /* TODO: implement your application code.*/
        }
        break;

        case BLE_GAP_EVT_REMOTE_CONN_PARAM_REQUEST:
        {
            /* TODO: implement your application code.*/
        }
        break;

        case BLE_GAP_EVT_EXT_ADV_REPORT:
        {
            /* TODO: implement your application code.*/
        }
        break;

        case BLE_GAP_EVT_ADV_TIMEOUT:
        {
            /* TODO: implement your application code.*/
        }
        break;

        case BLE_GAP_EVT_TX_BUF_AVAILABLE:
        {
            /* TODO: implement your application code.*/
        }
        break;

        case BLE_GAP_EVT_DEVICE_NAME_CHANGED:
        {
            /* TODO: implement your application code.*/
        }
        break;

        case BLE_GAP_EVT_AUTH_PAYLOAD_TIMEOUT:
        {
            /* TODO: implement your application code.*/
        }
        break;

        case BLE_GAP_EVT_PHY_UPDATE:
        {
            /* TODO: implement your application code.*/
        }
        break;

        case BLE_GAP_EVT_SCAN_REQ_RECEIVED:
        {
            /* TODO: implement your application code.*/
        }
        break;

        case BLE_GAP_EVT_DIRECT_ADV_REPORT:
        {
            /* TODO: implement your application code.*/
        }
        break;

        case BLE_GAP_EVT_PERI_ADV_SYNC_EST:
        {
            /* TODO: implement your application code.*/
        }
        break;

        case BLE_GAP_EVT_PERI_ADV_REPORT:
        {
            /* TODO: implement your application code.*/
        }
        break;

        case BLE_GAP_EVT_PERI_ADV_SYNC_LOST:
        {
            /* TODO: implement your application code.*/
        }
        break;

        case BLE_GAP_EVT_ADV_SET_TERMINATED:
        {
            /* TODO: implement your application code.*/
        }
        break;

        case BLE_GAP_EVT_SCAN_TIMEOUT:
        {
            /* TODO: implement your application code.*/
        }
        break;

        default:
        break;
    }
}

APP_BLE_LinkState_T APP_GetBleState(void)
{
    return (sp_currentBleLink->linkState);
}

void APP_SetBleState(APP_BLE_LinkState_T state)
{
    sp_currentBleLink->linkState = state;
}

void APP_InitConnList(void)
{
    uint8_t i;

    sp_currentBleLink = &s_bleConnList[0];       
    
    for (i = 0; i < BLE_GAP_MAX_LINK_NBR; i++)
    {
        memset((uint8_t *)(&s_bleConnList[i]), 0, sizeof(APP_BLE_ConnList_T));

        s_bleConnList[i].linkState= APP_BLE_STATE_STANDBY;
    }
}
