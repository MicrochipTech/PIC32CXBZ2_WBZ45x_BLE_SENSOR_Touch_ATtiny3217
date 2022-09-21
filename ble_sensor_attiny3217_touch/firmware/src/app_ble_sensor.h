/*******************************************************************************
  Application BLE Sensor Header File

  Company:
    Microchip Technology Inc.

  File Name:
    app_ble_sensor.h

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


#ifndef APP_BLE_SENSOR_H
#define APP_BLE_SENSOR_H


// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include "app_trps.h"

// *****************************************************************************
// *****************************************************************************
// Section: Macros
// *****************************************************************************
// *****************************************************************************
#define GREEN_H  85  // 240deg
#define GREEN_S  255  // 100%
#define GREEN_V  255  //100%

#define BUTTON_1  EIC_PIN_0

// Define for BLE Sensor Ctrl Commands

#define APP_TRP_VENDOR_OPCODE_BLE_SENSOR  0x8A
//  Defines BLE Sensor Control Command Set APP_TRPS_CTRL_CMD
#define    RGB_ONOFF_SET_CMD    0x10
#define    RGB_ONOFF_GET_CMD    0x11    
#define    RGB_COLOR_SET_CMD    0x12
#define    RGB_COLOR_GET_CMD    0x13


//  Defines BLE Sensor Response Command Set APP_TRPS_CTRL_RSP
#define    RGB_ONOFF_SET_RSP    0x20
#define    RGB_ONOFF_GET_RSP    0x21    
#define    RGB_COLOR_SET_RSP    0x22
#define    RGB_COLOR_GET_RSP    0x23


//  Defines BLE Sensor Response Command length APP_TRPS_CTRL_RSP_LENGTH
#define    RGB_ONOFF_SET_RSP_LEN 0x0
#define    RGB_ONOFF_GET_RSP_LEN 0x1    
#define    RGB_COLOR_SET_RSP_LEN 0x0
#define    RGB_COLOR_GET_RSP_LEN 0x3


//  Defines BLE Sensor Notify Command Set APP_TRPS_CTRL_NOTIFY
#define    RGB_ONOFF_STATUS_NFY  0x40
#define    TEMP_SENSOR_NFY       0x41
#define    RGB_COLOR_NFY         0x42


//  Defines BLE Sensor Notify Command length APP_TRPS_CTRL_NOTIFY_LENGTH
#define    RGB_ONOFF_STATUS_NFY_LEN   0x1
#define    TEMP_SENSOR_NFY_LEN        0x2
#define    RGB_COLOR_NFY_LEN          0x3

// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
// *****************************************************************************
enum APP_TRPS_LED_STATUS_T
{
    LED_OFF = 0x00,
    LED_ON = 0x01,
};

/**@brief The structure contains the information about BLE sensor data */
typedef struct
{
    uint8_t    rgbOnOffStatus;        /**< RGB LED On/Off Status */
    struct __attribute__ ((packed))
    {
        uint8_t    Hue;         /**The array contains the information about RGB colour value in HSV format. */
        uint8_t    Saturation;
        uint8_t    Value;
    }RGB_color;
    struct __attribute__ ((packed))
    {
        uint8_t    msb;
        uint8_t    lsb;
    }tempSens;
} APP_TRPS_SensorData_T;

#define BLE_SENSOR_CMD_RESP_LST_SIZE   4
#define BLE_SENSOR_DEFINE_CTRL_CMD_RESP()                \
        { RGB_ONOFF_SET_CMD, RGB_ONOFF_SET_RSP, RGB_ONOFF_SET_RSP_LEN, NULL , APP_TRPS_Sensor_LED_Ctrl},      \
        { RGB_ONOFF_GET_CMD, RGB_ONOFF_GET_RSP, RGB_ONOFF_GET_RSP_LEN, (uint8_t *)&bleSensorData.rgbOnOffStatus , NULL},       \
        { RGB_COLOR_SET_CMD, RGB_COLOR_SET_RSP, RGB_COLOR_SET_RSP_LEN, NULL , APP_TRPS_Sensor_Color_Ctrl},      \
        { RGB_COLOR_GET_CMD, RGB_COLOR_GET_RSP, RGB_COLOR_GET_RSP_LEN, (uint8_t *)&bleSensorData.RGB_color , NULL}

#define BLE_SENSOR_NOTIFY_LST_SIZE   3
#define BLE_SENSOR_DEFINE_CTRL_NOTIFY()                   \
        { RGB_ONOFF_STATUS_NFY, RGB_ONOFF_STATUS_NFY_LEN, (uint8_t *)&bleSensorData.rgbOnOffStatus},      \
        { TEMP_SENSOR_NFY, TEMP_SENSOR_NFY_LEN, (uint8_t *)&bleSensorData.tempSens},       \
        { RGB_COLOR_NFY, RGB_COLOR_NFY_LEN, (uint8_t *)&bleSensorData.RGB_color},   
// *****************************************************************************
// *****************************************************************************
// Section: Function Prototypes
// *****************************************************************************
// *****************************************************************************
void APP_TRPS_Sensor_Init(void);

void APP_TRPS_Sensor_DiscEvtProc(void);

void APP_TRPS_Sensor_TimerHandler(void);

void APP_TRPS_Sensor_Button_Handler(void);

void APP_TRPS_Sensor_Beacon(uint8_t* ptr_data);

void APP_TRPS_Sensor_BLE_light_Color_Sync(uint8_t hue, uint8_t sat);

void APP_TRPS_Sensor_BLE_light_onOff_Sync(bool lightState);

APP_TRPS_SensorData_T* APP_TRPS_Get_SensorData();

void update_touch_data(void);
#endif
