/*******************************************************************************
  Application OTA Handler Header File

  Company:
    Microchip Technology Inc.

  File Name:
    app_OTA_handler.h

  Summary:
    This file contains the Application OTA Handler header file for this project.

  Description:
    This file contains the Application OTA Handler header file for this project.
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

#ifndef APP_OTA_HANDLER_H
#define APP_OTA_HANDLER_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include "ble_otaps/ble_otaps.h"

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
typedef enum APP_OTA_HDL_Mode_T
{
    APP_OTA_MODE_IDLE,
    APP_OTA_MODE_OTA,
} APP_OTA_HDL_Mode_T;

// *****************************************************************************
// *****************************************************************************
// Section: Global Variables
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
// *****************************************************************************
// Section: Function Prototypes
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
     void APP_OTA_HDL_Prepare(uint16_t OTAHandle)

  Summary:
     Preparation for OTA procedure.

  Description:

  Precondition:

  Parameters:
    OTAHandle.

  Returns:
    None.

*/
void APP_OTA_HDL_Prepare(uint16_t OTAHandle);

/*******************************************************************************
  Function:
     void APP_OTA_HDL_Start(void);

  Summary:
     Starts the OTA timeout timer.

  Description:

  Precondition:

  Parameters:
    None.

  Returns:
    None.

*/
void APP_OTA_HDL_Start(void);

/*******************************************************************************
  Function:
     void APP_OTA_HDL_Updating(void);

  Summary:
     Starts the OTA timeout timer while OTA update is in progress

  Description:

  Precondition:

  Parameters:
    None.

  Returns:
    None.

*/
void APP_OTA_HDL_Updating(void);

/*******************************************************************************
  Function:
     void APP_OTA_HDL_Complete(void);

  Summary:
     Stops the OTA timeout timer as OTA update is completed

  Description:

  Precondition:

  Parameters:
    None.

  Returns:
    None.

*/
void APP_OTA_HDL_Complete(void);

/*******************************************************************************
  Function:
     void APP_OTA_HDL_ErrorHandle(uint16_t OTAHandle)

  Summary:
     Handles OTA error

  Description:

  Precondition:

  Parameters:
    OTAHandle.

  Returns:
    None.

*/
void APP_OTA_HDL_ErrorHandle(uint16_t OTAHandle);

/*******************************************************************************
  Function:
     void APP_OTA_HDL_Reset(void);

  Summary:
     Starts the OTA reboot timer to reset the device

  Description:

  Precondition:

  Parameters:
    None.

  Returns:
    None.

*/
void APP_OTA_HDL_Reset(void);

/*******************************************************************************
  Function:
     void APP_OTA_HDL_SetOTAMode(APP_OTA_HDL_Mode_T mode)

  Summary:
     Sets OTA mode

  Description:

  Precondition:

  Parameters:
    mode.

  Returns:
    None.

*/
void APP_OTA_HDL_SetOTAMode(APP_OTA_HDL_Mode_T mode);

/*******************************************************************************
  Function:
     APP_OTA_HDL_Mode_T APP_OTA_HDL_GetOTAMode(void);

  Summary:
     Gets OTA mode

  Description:

  Precondition:

  Parameters:
    None.

  Returns:
    APP_OTA_HDL_Mode_T.

*/
APP_OTA_HDL_Mode_T APP_OTA_HDL_GetOTAMode(void);

/*******************************************************************************
  Function:
     void APP_OTA_Timeout_Handler(void)

  Summary:
     OTA timeout handler

  Description:

  Precondition:

  Parameters:
    None.

  Returns:
    None.

*/
void APP_OTA_Timeout_Handler(void);

/*******************************************************************************
  Function:
     void APP_OTA_Reboot_Handler(void)

  Summary:
     OTA reboot timer handler

  Description:

  Precondition:

  Parameters:
    None.

  Returns:
    None.

*/
void APP_OTA_Reboot_Handler(void);

/*******************************************************************************
  Function:
     void APP_OTA_EvtHandler(BLE_OTAPS_Event_T *p_event)

  Summary:
     OTA handler called from OTA profile from middleware

  Description:

  Precondition:

  Parameters:
    p_event.

  Returns:
    None.

*/
void APP_OTA_EvtHandler(BLE_OTAPS_Event_T *p_event);

/*******************************************************************************
  Function:
     void APP_OTA_HDL_Init(void)

  Summary:
     OTA initialisation API to init mode and encryption API

  Description:

  Precondition:

  Parameters:
    None.

  Returns:
    None.

*/
void APP_OTA_HDL_Init(void);

#endif  // End of APP_OTA_HANDLER_H
/*******************************************************************************
 End of File
 */

