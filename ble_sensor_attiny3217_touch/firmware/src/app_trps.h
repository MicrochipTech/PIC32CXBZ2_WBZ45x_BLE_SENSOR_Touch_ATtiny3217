/*******************************************************************************
  Application Transparent Service Header File

  Company:
    Microchip Technology Inc.

  File Name:
    app_trps.h

  Summary:
    This file contains the Application Transparent Server Role functions for this project.

  Description:
    This file contains the Application Transparent Server Role functions for this project.
    The implementation of demo mode is included in this file.
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


#ifndef APP_TRPS_H
#define APP_TRPS_H


// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
//#include "compiler.h"
#include "ble_gap.h"
#include "ble_trsps/ble_trsps.h"

// *****************************************************************************
// *****************************************************************************
// Section: Macros
// *****************************************************************************
// *****************************************************************************
#define APP_TRPS_CTRL_LST_SIZE   1

#define APP_TRPS_CTRL_RSP_ID_STATUS_LEN 2

// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
// *****************************************************************************
enum APP_TRPS_CTRL_STATUS_CODE_T
{
    SUCCESS = 0x00,
    OPCODE_NOT_SUPPORTED = 0x01,
    INVALID_PARAMETER = 0x02,
    OPERATION_FAILED = 0x03
};

/**@brief The structure contains information about APP transparent connection parameters for recording connection information. */
typedef struct APP_TRPS_ConnList_T
{
    uint16_t                connHandle;         /**< Connection handle associated with this connection. */
} APP_TRPS_ConnList_T;


/**@brief The structure contains the information about control command format. */
typedef struct __attribute__ ((packed))
{
    uint8_t    Opcode;        /**< Control Opcode */    
    uint8_t    Length;        /**< Control command length */
    uint8_t    ctrlID;        /**< Control control ID */
    uint8_t*    p_Payload;     /**< Control command data pointer. */   
} APP_TRPS_CtrlCmd_T;

/**@brief The structure contains the information about control command/Resp format. */
typedef struct __attribute__ ((packed))
{ 
    uint8_t   CmdId;
    uint8_t   RspId;
    uint8_t   Length;
    uint8_t*   p_Payload;
    uint8_t  (*fnPtr) (uint8_t*);
} APP_TRPS_CmdResp_T;

/**@brief The structure contains the information about control Notify format. */
typedef struct __attribute__ ((packed))
{
    uint8_t    NtfID;        /**< Control Notify ID */
    uint8_t    Length;        /**< Control Notify length */    
    uint8_t    *p_Payload;     /**< Control Notify data pointer. */   
} APP_TRPS_NotifyData_T;

/**@brief The structure contains the information about control Cmd,resp,notify structures. */
typedef struct
{
    uint8_t    opcode;     /**<  opcode */
    uint8_t    cmdRspSize;  /**<  Size of cmd Resp array */
    uint8_t    ntfySize;    /**<  Size of Notfy array */
    APP_TRPS_CmdResp_T *appTrpsCmdResp;
    APP_TRPS_NotifyData_T *appTrpsNotify;  
} APP_TRPS_Ctrl_T;

// *****************************************************************************
// *****************************************************************************
// Section: Function Prototypes
// *****************************************************************************
// *****************************************************************************
uint16_t APP_TRPS_Init(uint8_t opcode, APP_TRPS_CmdResp_T *p_cmd, APP_TRPS_NotifyData_T *p_ntfy,uint8_t cmdRspSize,uint8_t ntfySize);
void APP_TRPS_deInit(uint8_t opCode);
void APP_TRPS_ConnEvtProc(BLE_GAP_Event_T *p_event);
void APP_TRPS_DiscEvtProc(uint16_t connHandle);
void APP_TRPS_EventHandler(BLE_TRSPS_Event_T *p_event);
uint16_t APP_TRPS_SendNotification(uint8_t opcode, uint8_t idx);
#endif
