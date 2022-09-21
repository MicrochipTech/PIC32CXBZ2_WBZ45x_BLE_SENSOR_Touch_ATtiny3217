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

/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    app_ble_conn_handler.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application.  Some of these are required by the system (such as the
    "APP_Initialize" and "APP_Tasks" prototypes) and some of them are only used
    internally by the application (such as the "APP_STATES" definition).  Both
    are defined here for convenience.
*******************************************************************************/

#ifndef _APP_BLE_CONN_HANDLER_H
#define _APP_BLE_CONN_HANDLER_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include <string.h>
#include "gap_defs.h"
#include "ble_gap.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
/**@brief Enumeration type of BLE state. */
typedef enum APP_BLE_LinkState_T
{
    APP_BLE_STATE_STANDBY,                                                /**< Standby state. i.e. not advertising */
    APP_BLE_STATE_ADVERTISING,                                            /**< BLE is advertising */
    APP_BLE_STATE_SCANNING,                                               /**< BLE is scanning (only for BLE_GAP_ROLE_CENTRAL) */
    APP_BLE_STATE_CONNECTED,                                              /**< BLE is connected */
    APP_BLE_STATE_TOTAL
} APP_BLE_LinkState_T;


// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
// *****************************************************************************

/**@brief The structure contains the BLE Connection parameters. */
typedef struct APP_BLE_ConnData_T
{
    uint8_t                role;                                           /**< GAP role, see @ref BLE_GAP_ROLE. */
    uint16_t               handle;                                         /**< Connection handle associated with this connection. */
    BLE_GAP_Addr_T         remoteAddr;                                     /**< See @ref BLE_GAP_Addr_T. */
    uint16_t               connInterval;                                   /**< Connection interval used on this connection. Range should be @ref BLE_GAP_CP_RANGE. */
    uint16_t               connLatency;                                    /**< Slave latency for the connection in terms of number of connection events, see @ref BLE_GAP_CP_RANGE. */
    uint16_t               supervisionTimeout;                             /**< Supervision timeout for the LE Link, see @ref BLE_GAP_CP_RANGE. */
} APP_BLE_ConnData_T;

/**@brief The structure contains the BLE link related information maintained by the application Layer */
typedef struct APP_BLE_ConnList_T
{
    APP_BLE_LinkState_T         linkState;                                              /**< BLE link state. see @ref APP_BLE_LinkState_T */
    APP_BLE_ConnData_T          connData;                                               /**< BLE connection information. See @ref APP_BLE_ConnData_T */
} APP_BLE_ConnList_T;

// *****************************************************************************
// *****************************************************************************
// Section: Global Variables
// *****************************************************************************
// *****************************************************************************

// Section: Type Definitions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_BleGapConnEvtHandler( BLE_GAP_Event_T *p_event )

  Summary:
     BLE Stack application GAP Connection event handler.

  Description:

  Precondition:

  Parameters:
    None.

  Returns:
    None.

*/
void APP_BleGapConnEvtHandler(BLE_GAP_Event_T *p_event);

/*******************************************************************************
  Function:
     void APP_InitConnList(void)

  Summary:
     Initialise connection list.

  Description:

  Precondition:

  Parameters:
    None.

  Returns:
    None.

*/
void APP_InitConnList(void);

/*******************************************************************************
  Function:
     APP_BLE_LinkState_T APP_GetBleState(void)

  Summary:
     Returns BLE link state.

  Description:

  Precondition:

  Parameters:
    None.

  Returns:
    APP_BLE_LinkState_T.

*/
APP_BLE_LinkState_T APP_GetBleState(void);

/*******************************************************************************
  Function:
     void APP_SetBleState(APP_BLE_LinkState_T state)

  Summary:
     Sets BLE link state.

  Description:

  Precondition:

  Parameters:
    APP_BLE_LinkState_T.

  Returns:
    None.

*/
void APP_SetBleState(APP_BLE_LinkState_T state);

/*******************************************************************************
  Function:
     uint16_t APP_GetConnHandleByIndex(uint8_t index)

  Summary:
     Gets the connection handle based on index passed.

  Description:

  Precondition:

  Parameters:
    index.

  Returns:
    connection handle.

*/
uint16_t APP_GetConnHandleByIndex(uint8_t index);

/*******************************************************************************
  Function:
     APP_BLE_ConnList_T *APP_GetConnInfoByConnHandle(uint16_t connHandle)

  Summary:
     Returns the connection information based on connection handle.

  Description:

  Precondition:

  Parameters:
    connHandle.

  Returns:
    APP_BLE_ConnList_T.

*/
APP_BLE_ConnList_T *APP_GetConnInfoByConnHandle(uint16_t connHandle);

#endif /* _APP_BLE_CONN_HANDLER_H */

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

/*******************************************************************************
 End of File
 */

