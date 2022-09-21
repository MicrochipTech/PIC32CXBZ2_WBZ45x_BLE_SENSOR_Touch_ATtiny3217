/*******************************************************************************
  Application Transparent Service  Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_trps.c

  Summary:
    This file contains the Application Transparent Server Role functions for this project.

  Description:
    This file contains the Application Transparent Server Role functions for this project.
    The implementation of demo mode is included in this file.
 *******************************************************************************/

//DOM-IGNORE-BEGIN
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
//DOM-IGNORE-END


// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include <string.h>
#include <stdint.h>
#include "stack_mgr.h"
#include "ble_trsps/ble_trsps.h"
#include "app_trps.h"
#include "app_error_defs.h"


// *****************************************************************************
// *****************************************************************************
// Section: Local Variables
// *****************************************************************************
// *****************************************************************************
static APP_TRPS_ConnList_T       s_trpsConnList_t;
static APP_TRPS_Ctrl_T  s_trpsCtrl[APP_TRPS_CTRL_LST_SIZE];

// *****************************************************************************
// *****************************************************************************
// Section: Global Variables
// *****************************************************************************
// *****************************************************************************


// *****************************************************************************
// *****************************************************************************
// Section: Functions
// *****************************************************************************
// *****************************************************************************
static uint16_t APP_TRPS_NotifyHandler(APP_TRPS_NotifyData_T* p_notify, uint8_t opcode);
static uint16_t APP_TRPS_SendCmdRsp(APP_TRPS_CmdResp_T* p_resp, uint8_t opcode, uint8_t status);
static void APP_ClearCtrlList(uint8_t opCode);
static APP_TRPS_Ctrl_T *APP_GetFreeCtrlList(void);
static APP_TRPS_Ctrl_T *APP_GetCtrlListByOpcode(uint8_t opCode);
static uint16_t APP_TRPS_SendErrorRsp(uint8_t opcode, uint8_t RspID, uint8_t status);

static void APP_ClearCtrlList(uint8_t opCode)
{
    uint8_t i;

    for (i = 0; i < APP_TRPS_CTRL_LST_SIZE; i++)
    {
        if (s_trpsCtrl[i].opcode == opCode)
        {
            memset((uint8_t *)(&s_trpsCtrl[i]), 0, sizeof(APP_TRPS_Ctrl_T));

            s_trpsCtrl[i].opcode= 0;
        }
    }
}

static APP_TRPS_Ctrl_T *APP_GetFreeCtrlList(void)
{
    uint8_t i;

    for (i = 0; i < APP_TRPS_CTRL_LST_SIZE; i++)
    {
        if (s_trpsCtrl[i].opcode == 0)
        {
            return (&s_trpsCtrl[i]);
        }
    }
    return NULL;
}

static APP_TRPS_Ctrl_T *APP_GetCtrlListByOpcode(uint8_t opCode)
{
    uint8_t i;

    for (i = 0; i < APP_TRPS_CTRL_LST_SIZE; i++)
    {
        if (s_trpsCtrl[i].opcode == opCode && opCode != 0)
        {
            return (&s_trpsCtrl[i]);
        }
    }
    return NULL;
}

/* Init TRPS data structure */
uint16_t APP_TRPS_Init(uint8_t opcode, APP_TRPS_CmdResp_T *p_cmd, APP_TRPS_NotifyData_T *p_ntfy,uint8_t cmdRspSize,uint8_t ntfySize)
{
    APP_TRPS_Ctrl_T *p_trpsCtrl = NULL;
    
    p_trpsCtrl = APP_GetFreeCtrlList();
    
    if(p_trpsCtrl != NULL)
    {
        p_trpsCtrl->appTrpsCmdResp = p_cmd;
        p_trpsCtrl->appTrpsNotify = p_ntfy;
        p_trpsCtrl->opcode = opcode;
        p_trpsCtrl->cmdRspSize = cmdRspSize;
        p_trpsCtrl->ntfySize = ntfySize;
        return APP_RES_SUCCESS;
    }
    return APP_RES_FAIL;
}

/* deInit TRPS data structure */
void APP_TRPS_deInit(uint8_t opCode)
{
    APP_ClearCtrlList(opCode);
}


/* Do the BLE Sensor specific on connection  */
void APP_TRPS_ConnEvtProc(BLE_GAP_Event_T *p_event)
{
   // s_trpsConnList_t.connHandle = p_event->eventField.evtConnect.connHandle;
}

/* Do the BLE Sensor specific on disconnection  */
void APP_TRPS_DiscEvtProc(uint16_t connHandle)
{
    if (s_trpsConnList_t.connHandle == connHandle)
    {
        s_trpsConnList_t.connHandle = 0;
    }
}

/* TRPS Event handler called from BLE Stack */
void APP_TRPS_EventHandler(BLE_TRSPS_Event_T *p_event)
{ 
    uint8_t idx=0, status = SUCCESS;
    bool cmdFound = false;
    APP_TRPS_CtrlCmd_T *p_Cmd = (APP_TRPS_CtrlCmd_T*)p_event->eventField.onVendorCmd.p_payLoad;
    APP_TRPS_Ctrl_T *p_trpsCtrl = NULL;
    
    switch(p_event->eventId)
    {
        case BLE_TRSPS_EVT_CTRL_STATUS:
        {
            s_trpsConnList_t.connHandle = p_event->eventField.onCtrlStatus.connHandle;
        }
        break;
        
        case BLE_TRSPS_EVT_VENDOR_CMD:
        {
            p_trpsCtrl = APP_GetCtrlListByOpcode(p_Cmd->Opcode);
            if ((s_trpsConnList_t.connHandle == p_event->eventField.onVendorCmd.connHandle)
                && (p_trpsCtrl != NULL))
            {
                s_trpsConnList_t.connHandle = p_event->eventField.onVendorCmd.connHandle;
                
                for(idx =0; idx<p_trpsCtrl->cmdRspSize; idx++)
                {
                    if(p_trpsCtrl->appTrpsCmdResp[idx].CmdId == p_Cmd->ctrlID)
                    {
                        cmdFound = true;
                        break;
                    }
                }
                
                if(cmdFound)
                {
                    if(p_trpsCtrl->appTrpsCmdResp[idx].fnPtr)
                        status = p_trpsCtrl->appTrpsCmdResp[idx].fnPtr(p_event->eventField.onVendorCmd.p_payLoad);
                    APP_TRPS_SendCmdRsp(&p_trpsCtrl->appTrpsCmdResp[idx],p_trpsCtrl->opcode,status);                    
                }
                else
                {
                    APP_TRPS_SendErrorRsp(p_trpsCtrl->opcode,p_Cmd->ctrlID,INVALID_PARAMETER);
                }

            }
            else
            {
                APP_TRPS_SendErrorRsp(p_Cmd->Opcode,0x00,OPCODE_NOT_SUPPORTED);
            }
        }
        break;

        default:
            break;
    }
}

/* Send Control notify data through TRPS control service */
uint16_t APP_TRPS_SendNotification(uint8_t opcode, uint8_t ntfyId)
{
    uint16_t result = APP_RES_FAIL;
    APP_TRPS_Ctrl_T *p_trpsCtrl = NULL;
    uint8_t idx;
    
    p_trpsCtrl = APP_GetCtrlListByOpcode(opcode);
    
    if(p_trpsCtrl != NULL){
        for(idx =0; idx<p_trpsCtrl->ntfySize; idx++)
        {
            if(p_trpsCtrl->appTrpsNotify[idx].NtfID == ntfyId)
            {
                result = APP_TRPS_NotifyHandler(&p_trpsCtrl->appTrpsNotify[idx],opcode);            
                break;
            }
        }
    }
    return result;
}

/* Send Control command response through TRPS control service */
static uint16_t APP_TRPS_SendCmdRsp(APP_TRPS_CmdResp_T* p_resp, uint8_t opcode, uint8_t status)
{
    uint8_t resp[20];
    uint16_t result;

    resp[0] = p_resp->Length+APP_TRPS_CTRL_RSP_ID_STATUS_LEN;  // Length byte includes size of RspID and Status
    resp[1] = p_resp->RspId;
    resp[2] = status;
    
    if(p_resp->Length != 0)
        memcpy(&resp[3],p_resp->p_Payload,p_resp->Length);

    result = BLE_TRSPS_SendVendorCommand(s_trpsConnList_t.connHandle, opcode, resp[0]+1, resp);      

    return result;
}

/* Send Control command error response through TRPS control service */
static uint16_t APP_TRPS_SendErrorRsp(uint8_t opcode, uint8_t RspID, uint8_t status)
{
    uint8_t resp[3];
    uint16_t result;

    resp[0] = APP_TRPS_CTRL_RSP_ID_STATUS_LEN;  // Length byte includes size of RspID and Status
    resp[1] = RspID;
    resp[2] = status;

    result = BLE_TRSPS_SendVendorCommand(s_trpsConnList_t.connHandle, opcode, resp[0]+1, resp);

    return result;
}

/* Send Control notify data through TRPS control service */
static uint16_t APP_TRPS_NotifyHandler(APP_TRPS_NotifyData_T* p_notify, uint8_t opcode)
{
    uint8_t resp[20];
    uint16_t result;

    resp[0] = p_notify->Length+1; // Length byte includes size of NtyID
    resp[1] = p_notify->NtfID;

    if(p_notify->Length != 0)
        memcpy(&resp[2],p_notify->p_Payload,p_notify->Length);

    result = BLE_TRSPS_SendVendorCommand(s_trpsConnList_t.connHandle, opcode, resp[0]+1, resp);

    return result;
}