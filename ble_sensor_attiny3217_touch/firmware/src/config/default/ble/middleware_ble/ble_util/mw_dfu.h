/*******************************************************************************
  Middleware Device Firmware Udpate Header File

  Company:
    Microchip Technology Inc.

  File Name:
    mw_dfu.h

  Summary:
    This file contains the BLE Device Firmware Udpate functions for application user.

  Description:
    This file contains the BLE Device Firmware Udpate functions for application user.
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
 * @addtogroup MW_DFU
 * @{
 * @brief Header file for the Middleware Device Firmware Update library.
 * @note Definitions and prototypes for the Middleware Device Firmware Update stack layer application programming interface.
 */

#ifndef MW_DFU_H
#define MW_DFU_H

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
/**@addtogroup MW_DFU_DEFINES Defines
 * @{ */


/**@defgroup MW_DFU_MAX_IMAGE_SIZE Maximum image size
 * @brief The definition of DFU maximum image size.
 * @{ */
#define MW_DFU_MAX_SIZE_FW_IMAGE              507904           /**< Maximum size of firmware image in bytes */
/** @} */


/**@defgroup MW_DFU_MAX_BLOCK_LEN Maximum block len
 * @brief The definition of maximum block length.
 * @{ */
#define MW_DFU_MAX_BLOCK_LEN                   0x400           /**< Maximum block length */
/** @} */

/**@} */ //MW_DFU_DEFINES


// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
// *****************************************************************************
/**@addtogroup MW_DFU_STRUCTS Structures
 * @{ */

/**@brief Structure of DFU info which is used to indicate the actual image size: @ref MW_DFU_MAX_IMAGE_SIZE . */
typedef struct MW_DFU_Info_T
{
    uint32_t fwImageSize;                                      /**< Firmware image size. It must be 16-bytes aligned. */
} MW_DFU_Info_T;

/**@} */ //MW_DFU_STRUCTS

// *****************************************************************************
// *****************************************************************************
// Section: Function Prototypes
// *****************************************************************************
// *****************************************************************************
/**@addtogroup MW_DFU_FUNS Functions
 * @{ */

/**
 *@brief The API is used to configure the device information for DFU process.
 *
 *@param[in] p_dfuInfo            Pointer to the structure of the image size for DFU. Refer to @ref MW_DFU_Info_T for detail structure info.
 *
 *
 *@retval MBA_RES_SUCCESS         Configure successfully.
 *@retval MBA_RES_INVALID_PARA    Invalid parameters. The size exceeds the maximum category size or size is not 4-bytes aligned.
 */
uint16_t MW_DFU_Config(MW_DFU_Info_T *p_dfuInfo);

/**
 *@brief The API is used to start or restart firmware image update procedure. 
 *       The state machine and parameters of this module would be reset after this API is called. 
 *
 *
 *@retval MBA_RES_SUCCESS         Start or restart firmware image update procedure successfully.
 *@retval MBA_RES_BAD_STATE       This API cannot be executed in current DFU process state.
 */
uint16_t MW_DFU_FwImageStart();

/**
 *@brief The API is used to update the fragment of firmware image to flash. 
 *       The API should be called multiple times to udpate all fragments of firmware image to flash.
 *
 *@param[in] length               The length of image fragment to update, unit: byte. It must be 16-bytes aligned.
 *@param[in] p_content            Pointer to the image fragment.
 *
 *
 *@retval MBA_RES_SUCCESS         Update the fragment of firmware image successfully.
 *@retval MBA_RES_INVALID_PARA    Invalid parameters. The length exceeds the image size or length exceeds @ref MW_DFU_MAX_BLOCK_LEN or length is not 16-bytes aligned. 
 *@retval MBA_RES_BAD_STATE       This API cannot be executed in current DFU process state.
 *@retval MBA_RES_OOM             No available memory.
 *@retval MBA_RES_FAIL            Fail to updpate fragment to flash.
 */
uint16_t MW_DFU_FwImageUpdate(uint16_t length, uint8_t *p_content);

/**
 *@brief The API is used to read the fragment of firmware image from flash. 
 *@note  Before full firmware image is updated, the first 16 bytes image fragment read from flash is invalid.
 *
 *
 *@param[in] offset               The flash offset to start reading image fragment.
 *@param[in] length               The length of image fragment to read, unit: byte.
 *@param[in] p_content            Pointer to the image fragment buffer.
 *
 *
 *@retval MBA_RES_SUCCESS         Read the fragment of firmware image successfully.
 *@retval MBA_RES_INVALID_PARA    Invalid parameters. The offset + length exceed @ref MW_DFU_MAX_SIZE_FW_IMAGE or length exceed @ref MW_DFU_MAX_BLOCK_LEN. 
 */
uint16_t MW_DFU_FwImageRead(uint32_t offset, uint16_t length, uint8_t *p_content);

/**@} */ //MW_DFU_FUNS

#endif
/**
 @}
*/

