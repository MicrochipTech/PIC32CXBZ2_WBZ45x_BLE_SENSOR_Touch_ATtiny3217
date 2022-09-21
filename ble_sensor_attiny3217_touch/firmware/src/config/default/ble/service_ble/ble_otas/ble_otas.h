/*******************************************************************************
  BLE OTA Service Header File

  Company:
    Microchip Technology Inc.

  File Name:
    ble_otas.h

  Summary:
    This file contains the BLE OTA Service functions for application user.

  Description:
    This file contains the BLE OTA Service functions for application user.
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


/**
 * @addtogroup BLE_OTAS BLE_OTAS
 * @{
 * @brief Header file for the BLE OTA Service library.
 * @note Definitions and prototypes for the BLE OTA Service stack layer application programming interface.
 */
#ifndef BLE_OTAS_H
#define BLE_OTAS_H


// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************


// *****************************************************************************
// *****************************************************************************
// Section: Macros
// *****************************************************************************
// *****************************************************************************

/**@defgroup BLE_OTAS_ASSIGN_HANDLE BLE_OTAS_ASSIGN_HANDLE
 * @brief Assigned attribute handles of BLE OTA Service.
 * @{ */
#define BLE_OTAS_START_HDL                                        0x0100                /**< The start attribute handle of BLE OTA service. */
/** @} */


/**@brief Definition of BLE OTA Service attribute handle */
typedef enum BLE_OTAS_AttributeHandle_T
{
    BLE_OTAS_HDL_SRV = BLE_OTAS_START_HDL,                                              /**< Handle of Primary Service of BLE OTA Service. */
    BLE_OTAS_HDL_FEATURE,                                                               /**< Handle of OTA Feature characteristic. */
    BLE_OTAS_HDL_FEATURE_VAL,                                                           /**< Handle of OTA Feature characteristic value. */
    BLE_OTAS_HDL_CTRL,                                                                  /**< Handle of OTA Control Point characteristic. */
    BLE_OTAS_HDL_CTRL_VAL,                                                              /**< Handle of OTA Control Point characteristic value. */
    BLE_OTAS_HDL_CTRL_CCCD,                                                             /**< Handle of OTA Control Point characteristic CCCD. */
    BLE_OTAS_HDL_DATA,                                                                  /**< Handle of OTA Data characteristic. */
    BLE_OTAS_HDL_DATA_VAL,                                                              /**< Handle of OTA Data characteristic value. */
    BLE_OTAS_HDL_DATA_CCCD                                                              /**< Handle of OTA Data characteristic CCCD. */
}BLE_OTAS_AttributeHandle_T;


/**@defgroup BLE_OTAS_ASSIGN_HANDLE BLE_OTAS_ASSIGN_HANDLE
 * @brief Assigned attribute handles of BLE OTA Service.
 * @{ */
#define BLE_OTAS_END_HDL                                         BLE_OTAS_HDL_DATA_CCCD     /**< The end attribute handle of BLE OTA Service. */
/** @} */


// *****************************************************************************
// *****************************************************************************
// Section: Function Prototypes
// *****************************************************************************
// *****************************************************************************

/**
 *@brief This API is used to register the BLE OTA Service to GATT server.
 *
 *
 *@retval MBA_RES_SUCCESS       Successfully registering the service
 *@retval MBA_RES_FAIL          Fail to register the service. The assigned attribute handles in the service conflict or \n
 *                              the start handle of the service is smaller than @ref GATTS_APP_SVC_START_HDL.
 */
uint16_t BLE_OTAS_Add(void);

#endif

/**
  @}
 */
