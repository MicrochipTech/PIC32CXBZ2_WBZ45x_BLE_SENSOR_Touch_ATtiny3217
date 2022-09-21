/*******************************************************************************
  Application OTA Handler Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_ota_handler.c

  Summary:
    This file contains the Application OTA handler for this project.

  Description:
    This file contains the Application OTA handler for this project.
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
#include <stdio.h>
#include <stdlib.h>
#include "ble_gap.h"
#include "app_ota_handler.h"
#include "app_ble_conn_handler.h"
#include "app_error_defs.h"
#include "app_timer/app_timer.h"
#include "app_ble/app_otaps_handler.h"
#include "driver/pds/include/pds.h"
#include "peripheral/rcon/plib_rcon.h"
#include "ble_dis/ble_dis.h"
#include "app_ble/app_ble_utility.h"
#include "ble_dm/ble_dm.h"

// *****************************************************************************
// *****************************************************************************
// Section: Macros
// *****************************************************************************
// *****************************************************************************


// *****************************************************************************
// *****************************************************************************
// Section: Data Types
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
static uint8_t s_OTAMode;
static APP_BLE_ConnList_T *s_pOTAConnLink = NULL;
static uint16_t s_connHandle;


// *****************************************************************************
// *****************************************************************************
// Section: Function Prototypes
// *****************************************************************************
// *****************************************************************************

void APP_OTA_HDL_SetOTAMode(APP_OTA_HDL_Mode_T mode)
{
    s_OTAMode = mode;
}

APP_OTA_HDL_Mode_T APP_OTA_HDL_GetOTAMode(void)
{
    return s_OTAMode;
}

void APP_OTA_HDL_Prepare(uint16_t OTAHandle)
{
    uint8_t i;
    uint16_t connHandle;
    APP_BLE_ConnList_T *p_connList;
    BLE_DM_ConnParamUpdate_T params;
	
	BLE_GAP_SetAdvEnable(false, 0);

    s_pOTAConnLink = APP_GetConnInfoByConnHandle(OTAHandle);

    if (s_pOTAConnLink == NULL)
        return;

    //Disconnect all others except proceeding OTA one
    for (i=0; i< BLE_GAP_MAX_LINK_NBR; i++)
    {
        connHandle = APP_GetConnHandleByIndex(i);
        if (connHandle == 0xFFFF)
            continue;
        
        p_connList = APP_GetConnInfoByConnHandle(connHandle);
        if (p_connList == NULL)
            continue;
    
        if (p_connList->linkState >= APP_BLE_STATE_CONNECTED && 
            OTAHandle != connHandle)
        {
            BLE_GAP_Disconnect(connHandle, GAP_DISC_REASON_REMOTE_TERMINATE);
        }
    }

    if (s_pOTAConnLink->connData.handle)
    {
        //speed up transmission
        params.intervalMin = 0x08;  //10ms
        params.intervalMax = 0x10;  //20ms
        params.latency = 0;
        params.timeout = 0xC8;//2000ms
        BLE_DM_ConnectionParameterUpdate(s_pOTAConnLink->connData.handle, &params);
    }

    APP_TIMER_SetTimer(APP_TIMER_OTA_TIMEOUT, APP_TIMER_5S, false);
}

void APP_OTA_HDL_Start(void)
{
    APP_TIMER_StopTimer(APP_TIMER_OTA_TIMEOUT);
    APP_TIMER_SetTimer(APP_TIMER_OTA_TIMEOUT, APP_TIMER_5S, false);
}

void APP_OTA_HDL_Updating(void)
{
    APP_TIMER_StopTimer(APP_TIMER_OTA_TIMEOUT);
    APP_TIMER_SetTimer(APP_TIMER_OTA_TIMEOUT, APP_TIMER_5S, false);
}

void APP_OTA_HDL_Complete(void)
{
    APP_TIMER_StopTimer(APP_TIMER_OTA_TIMEOUT);
}

void APP_OTA_HDL_ErrorHandle(uint16_t OTAHandle)
{
    BLE_DM_ConnParamUpdate_T params;
    APP_BLE_ConnList_T *OTAConnLink = NULL;


    APP_TIMER_StopTimer(APP_TIMER_OTA_TIMEOUT);
    APP_OTA_HDL_SetOTAMode(APP_OTA_MODE_IDLE);
    
    if (OTAHandle)
    {
        
        OTAConnLink = APP_GetConnInfoByConnHandle(OTAHandle);

        if (OTAConnLink == NULL)
           return; 
        //restore connection parameter
        params.intervalMin = 0x10;  //20ms
        params.intervalMax = 0x10;  //20ms
        params.latency = 0;
        params.timeout = 0x48;//720ms
        BLE_DM_ConnectionParameterUpdate(OTAHandle, &params);
    }
}

void APP_OTA_HDL_Reset(void)
{
    APP_TIMER_SetTimer(APP_TIMER_OTA_REBOOT, APP_TIMER_500MS, false);
}

void APP_OTA_Timeout_Handler(void)
{
    if (APP_OTA_HDL_GetOTAMode() == APP_OTA_MODE_OTA)
        {
            APP_OTA_HDL_ErrorHandle(s_pOTAConnLink->connData.handle);
        }

}
 

void APP_OTA_Reboot_Handler(void)
{
    uint8_t count = PDS_GetPendingItemsCount();

    if ((!count) && (APP_OTA_HDL_GetOTAMode() == APP_OTA_MODE_OTA))
    {
        RCON_SoftwareReset();
    }
}

void APP_OTA_EvtHandler(BLE_OTAPS_Event_T *p_event)
{
    switch(p_event->eventId)
    {
        case BLE_OTAPS_EVT_UPDATE_REQ:
        {
            /* TODO: implement your application code.*/
            uint8_t appVerison[BLE_ATT_DEFAULT_MTU_LEN] = {'\0'};
            uint16_t result = APP_RES_FAIL, appVerisonLength = BLE_ATT_DEFAULT_MTU_LEN;			
            BLE_OTAPS_DevInfo_T devInfo = {0};

            result = GATTS_GetHandleValue(DIS_HDL_CHARVAL_FW_REV, &appVerison[0], &appVerisonLength);
            if (result == APP_RES_SUCCESS)
            {
                devInfo.fwImageVer = (appVerison[0]-'0') << 24 | (appVerison[2]-'0') << 16 | (appVerison[4]-'0') << 8 | (appVerison[6]-'0');
            }
            
            s_connHandle = p_event->eventField.evtUpdateReq.connHandle;
			
            {
                APP_OTA_HDL_SetOTAMode(APP_OTA_MODE_OTA);
                BLE_OTAPS_UpdateResponse(s_connHandle, true, &devInfo);
                APP_OTA_HDL_Prepare(s_connHandle);
            }            
        }
        break;
        
        case BLE_OTAPS_EVT_START_IND:
        {
            /* TODO: implement your application code.*/
            APP_OTA_HDL_Start();            
        }
        break;

        case BLE_OTAPS_EVT_UPDATING_IND:
        {
            /* TODO: implement your application code.*/
            APP_OTA_HDL_Updating();            
        }
        break;
        
        case BLE_OTAPS_EVT_COMPLETE_IND:
        {
            /* TODO: implement your application code.*/
            if (p_event->eventField.evtCompleteInd.errStatus == false)
            {
                APP_OTA_HDL_Complete();
                if (APP_ImageValidation() == true)
                {
                    BLE_OTAPS_CompleteResponse(true);
                }
                else
                {
                    BLE_OTAPS_CompleteResponse(false);
                    APP_OTA_HDL_ErrorHandle(s_connHandle);
                }
            }
            else
            {
                APP_OTA_HDL_ErrorHandle(s_connHandle);
            }            
        }
        break;
        
        case BLE_OTAPS_EVT_RESET_IND:
        {
            /* TODO: implement your application code.*/
            APP_OTA_HDL_Reset();            
        }
        break;

        default:
        break;
    }
}

void APP_OTA_HDL_Init(void)
{

    uint8_t initVector[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    
    uint8_t key[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99};

    BLE_OTAPS_SetEncrytionInfo(initVector, key);
					

    APP_OTA_HDL_SetOTAMode(APP_OTA_MODE_IDLE);

}	
/*******************************************************************************
 End of File
 */
