/*******************************************************************************
  Application Advertising Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_adv.c

  Summary:
    This file contains the Application advertising functions for this project.

  Description:
    This file contains the Application advertising functions for this project.
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
#include "app.h"
#include "app_ble_conn_handler.h"
#include "app_adv.h"
#include "app_error_defs.h"
#include "peripheral/gpio/plib_gpio.h"
#include "app_ble_sensor.h"
#include "app_timer/app_timer.h"


// *****************************************************************************
// *****************************************************************************
// Section: Macros
// *****************************************************************************
// *****************************************************************************
#define APP_BLE_NUM_ADDR_IN_DEV_NAME    2    /**< The number of bytes of device address included in the device name. */



// *****************************************************************************
// *****************************************************************************
// Section: Local Variables
// *****************************************************************************
// *****************************************************************************
//static APP_BLE_AdvParams_T                  s_bleAdvParam;
static BLE_GAP_AdvDataParams_T   			  s_bleAdvData;
static BLE_GAP_AdvDataParams_T                s_bleScanRspData;



// *****************************************************************************
// *****************************************************************************
// Section: Functions
// *****************************************************************************
// *****************************************************************************

static uint8_t APP_ADV_CalculateDataLength(uint8_t *advData)
{
    /* Caculate Total Length of Adv Data /Scan Response Data Elements  */
    uint8_t len = 0, i = 0;

    while (1)
    {
        if (advData[i] != 0x00)   // Check the length is Zero or not
        {
            len++;               // Add Length field size
            len += advData[i];   // Add this Element Data Size

            if (len >= BLE_GAP_ADV_MAX_LENGTH)
            {
                len = BLE_GAP_ADV_MAX_LENGTH;
                break;
            }
            else
            {
                i = len;
            }
        }
        else
        {
            break;
        }
    }
    return len;
}


void APP_ADV_UpdateAdvData(void)
{
    uint8_t idx = 0;

    memset(&s_bleAdvData.advData[0], 0x00, BLE_GAP_ADV_MAX_LENGTH);

    //Flags
    s_bleAdvData.advData[idx++] = (APP_ADV_TYPE_LEN + 1); //length
    s_bleAdvData.advData[idx++] = APP_ADV_TYPE_FLAGS;     //AD Type: flags
    s_bleAdvData.advData[idx++] = APP_ADV_FLAG_LE_GEN_DISCOV | APP_ADV_FLAG_BREDR_NOT_SUPPORTED;

    //Service Data
    s_bleAdvData.advData[idx++] = (APP_ADV_TYPE_LEN + APP_ADV_SRV_DATA_LEN); //length
    s_bleAdvData.advData[idx++] = APP_ADV_TYPE_SRV_DATA_16BIT_UUID;              //AD Type: Service Data
    s_bleAdvData.advData[idx++] = (uint8_t)APP_ADV_SERVICE_UUID_MCHP;
    s_bleAdvData.advData[idx++] = (uint8_t)(APP_ADV_SERVICE_UUID_MCHP >> 8);
    
    APP_TRPS_Sensor_Beacon(&s_bleAdvData.advData[idx]);

    s_bleAdvData.advLen = APP_ADV_CalculateDataLength(&s_bleAdvData.advData[0]);
}

void APP_ADV_UpdateScanRspData(void)
{
    uint8_t devNameLen;

    memset(&s_bleScanRspData.advData[0], 0x00, BLE_GAP_ADV_MAX_LENGTH);

    BLE_GAP_GetDeviceName(&devNameLen, &s_bleScanRspData.advData[2]);

    //Local Name
    s_bleScanRspData.advData[0] = (APP_ADV_TYPE_LEN + devNameLen); //length
    s_bleScanRspData.advData[1] = APP_ADV_TYPE_COMPLETE_LOCAL_NAME;

    s_bleScanRspData.advLen = APP_ADV_CalculateDataLength(&s_bleScanRspData.advData[0]);
}


uint16_t APP_ADV_Ctrl(uint8_t enable)
{
    uint16_t result = APP_RES_BAD_STATE;
    
    if (enable)
    {
        if (APP_GetBleState() == APP_BLE_STATE_STANDBY)
        {
                result = BLE_GAP_SetAdvEnable(true, 0);

                if (result == APP_RES_SUCCESS)
                {
                    APP_TIMER_SetTimer(APP_TIMER_ADV_CTRL, APP_TIMER_500MS, true); 
                    APP_SetBleState(APP_BLE_STATE_ADVERTISING);
                }
        }
    }
    else
    {
        if(APP_GetBleState() == APP_BLE_STATE_ADVERTISING)
        {
            result = BLE_GAP_SetAdvEnable(false, 0);
            if(result == APP_RES_SUCCESS)
            {
                APP_TIMER_StopTimer(APP_TIMER_ADV_CTRL);                
                APP_SetBleState(APP_BLE_STATE_STANDBY);
            }
        }
    }

    return result;
}

void APP_ADV_Start(void)
{
    APP_ADV_UpdateAdvData();
    BLE_GAP_SetAdvData(&s_bleAdvData);

    APP_ADV_UpdateScanRspData();
    BLE_GAP_SetScanRspData(&s_bleScanRspData);

    APP_ADV_Ctrl(true);
}

void APP_ADV_Stop(void)
{
    APP_ADV_Ctrl(false);
}

void APP_ADV_Init(void)
{
    APP_ADV_Start();
}

/* User LED control for advertising indication */
void APP_BLE_Adv_TimerHandler(void)
{

    if ( APP_GetBleState() == APP_BLE_STATE_ADVERTISING)
    {
        USER_LED_Toggle();
    }
      
}

static void APP_HexToAscii(uint8_t byteNum, uint8_t *p_hex, uint8_t *p_ascii)
{
    uint8_t i, j, c;
    uint8_t digitNum = byteNum * 2;

    if (p_hex == NULL || p_ascii == NULL)
        return;

    for (i = 0; i < digitNum; i++)
    {
        j = i / 2;
        c = p_hex[j] & 0x0F;

        if (c >= 0x00 && c <= 0x09)
        {
            p_ascii[digitNum - i - 1] = c + 0x30;
        }
        else if (c >= 0x0A && c <= 0x0F)
        {
            p_ascii[digitNum - i - 1] = c - 0x0A + 'A';
        }

        p_hex[j] /= 16;
    }
}

void APP_UpdateLocalName(uint8_t devNameLen, uint8_t *p_devName)
{
    uint8_t localName[GAP_MAX_DEVICE_NAME_LEN] = {0};
    uint8_t localNameLen = 0;

    if (p_devName == NULL || devNameLen == 0)
    {
        BLE_GAP_Addr_T addrPara;
        uint8_t addrAscii[APP_BLE_NUM_ADDR_IN_DEV_NAME * 2];
        uint8_t digitNum = APP_BLE_NUM_ADDR_IN_DEV_NAME * 2;
        
        BLE_GAP_GetDeviceName(&localNameLen, &localName[0]);
        localName[localNameLen++] = '_';

        BLE_GAP_GetDeviceAddr(&addrPara);

        APP_HexToAscii(APP_BLE_NUM_ADDR_IN_DEV_NAME, addrPara.addr, addrAscii);

        memcpy(&localName[localNameLen], &addrAscii[0], digitNum);

        localNameLen += digitNum;

        BLE_GAP_SetDeviceName(localNameLen, localName);
    }
    else
    {
        BLE_GAP_SetDeviceName(devNameLen, p_devName);
    }
}