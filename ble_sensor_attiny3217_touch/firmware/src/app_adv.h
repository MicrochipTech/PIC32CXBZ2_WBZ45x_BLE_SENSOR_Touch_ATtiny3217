/*******************************************************************************
  Application Advertising Header File

  Company:
    Microchip Technology Inc.

  File Name:
    app_adv.h

  Summary:
    This file contains the Application Advertising functions for this project.

  Description:
    This file contains the Application Advertising functions for this project.
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


#ifndef APP_ADV_H
#define APP_ADV_H


// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include <stdint.h>



// *****************************************************************************
// *****************************************************************************
// Section: Macros
// *****************************************************************************
// *****************************************************************************
/**@defgroup APP_ADV_DEFAULT_INTERVAL APP_ADV_DEFAULT_INTERVAL
 * @brief The default value for the BLE Advertising interval. Unit: 0.625 ms. Default: 0x0200 (320 milliseconds)
 * @{ */
#define APP_ADV_DEFAULT_INTERVAL                                        0x0200
/** @} */

/**@defgroup APP_ADV_TYPE APP_ADV_TYPE
* @brief The definition of the advertising type
* @{ */
#define APP_ADV_TYPE_FLAGS                                              0x01       /**< Flags. */
#define APP_ADV_TYPE_INCOMPLETE_16BIT_SRV_UUID                          0x02       /**< Incomplete List of 16-bit Service Class UUIDs. */
#define APP_ADV_TYPE_COMPLETE_16BIT_SRV_UUID                            0x03       /**< Complete List of 16-bit Service Class UUIDs. */
#define APP_ADV_TYPE_SHORTENED_LOCAL_NAME                               0x08       /**< Shortened Local Name. */
#define APP_ADV_TYPE_COMPLETE_LOCAL_NAME                                0x09       /**< Complete Local Name. */
#define APP_ADV_TYPE_TX_POWER                                           0x0A       /**< Tx Power Level. */
#define APP_ADV_TYPE_SRV_DATA_16BIT_UUID                                0x16       /**< Service Data - 16-bit UUID. */
#define APP_ADV_TYPE_MANU_DATA                                          0xFF       /**< Manufacture Specific Data. */
/** @} */

/** @brief Advertising data size. */
#define APP_ADV_TYPE_LEN                                                0x01       /**< Length of advertising data type. */
#define APP_ADV_SRV_DATA_LEN                                            0x09       /**< Length of service data. */
#define APP_ADV_SRV_UUID_LEN                                            0x02       /**< Length of service UUID. */

/**@defgroup APP_ADV_FLAG APP_ADV_FLAG
* @brief The definition of the mask setting for the advertising type
* @{ */
#define APP_ADV_FLAG_LE_LIMITED_DISCOV                                              (1 << 0)       /**< LE Limited Discoverable Mpde. */
#define APP_ADV_FLAG_LE_GEN_DISCOV                                                  (1 << 1)       /**< LE General Discoverable Mpde. */
#define APP_ADV_FLAG_BREDR_NOT_SUPPORTED                                            (1 << 2)       /**< BR/EDR Not Supported. */
#define APP_ADV_FLAG_SIMULTANEOUS_LE_BREDR_TO_SAME_DEVICE_CAP_CONTROLLER            (1 << 3)       /**< Simultaneous LE and BR/EDR to Same Device Capable (Controller). */
#define APP_ADV_FLAG_SIMULTANEOUS_LE_BREDR_TO_SAME_DEVICE_CAP_HOST                  (1 << 4)       /**< Simultaneous LE and BR/EDR to Same Device Capable (Host). */
/** @} */

#define APP_ADV_COMPANY_ID_MCHP                                         0x00CD
#define APP_ADV_SERVICE_UUID_MCHP                                       0xFEDA
#define APP_ADV_ADD_DATA_CLASS_BYTE                                     0xFF

/**@defgroup APP_ADV_PROD_TYPE APP_ADV_PROD_TYPE
* @brief The definition of the product type in the advertising data
* @{ */
#define APP_ADV_PROD_TYPE_BLE_UART                                                  0x01           /**< Product Type: BLE UART */
#define APP_ADV_PROD_TYPE_BLE_SENSOR                                                0x02           /**< Product Type: BLE Sesnor */
/** @} */

/**@defgroup APP_ADV_FEATURE_SET APP_ADV_FEATURE_SET
* @brief The definition of the mask setting for the feature set in the advertising data
* @{ */
#define APP_ADV_FEATURE_SET_FEATURE1                                                (1 << 0)       /**< Feature 1. */
#define APP_ADV_FEATURE_SET_FEATURE2                                                (1 << 1)       /**< Feature 2. */
/** @} */


// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
// *****************************************************************************


// *****************************************************************************
// *****************************************************************************
// Section: Function Prototypes
// *****************************************************************************
// *****************************************************************************
void APP_ADV_Init(void);
void APP_ADV_Start(void);
void APP_BLE_Adv_TimerHandler(void);
void APP_ADV_Stop(void);
void APP_UpdateLocalName(uint8_t devNameLen, uint8_t *p_devName);
#endif
