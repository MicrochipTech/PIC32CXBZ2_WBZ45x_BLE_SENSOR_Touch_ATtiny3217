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
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It
    implements the logic of the application's state machine and it may call
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include <string.h>
#include "app.h"
#include "configuration.h"
#include "definitions.h"
#include "app_ble.h"
#include "ble_dis/ble_dis.h"
#include "app_ble_conn_handler.h"
#include "app_ble_sensor.h"
#include "app_adv.h"
#include "system/console/sys_console.h"
#include "ble_otaps/ble_otaps.h"
#include "app_ota/app_ota_handler.h"
#include "sensors/inc/rgb_led.h"


// *****************************************************************************
// *****************************************************************************
// Section: Macros
// *****************************************************************************
#define BLE_SENSOR_VERSION  "1.0.0.0"

#define TOUCH_PREAMBLE      "B_On:0x"

//#define TOUCH_DATA_RED_POS          0 
//#define TOUCH_DATA_GREEN_POS        1
//#define TOUCH_DATA_BLUE_POS         2
//#define TOUCH_DATA_BUTTON_POS       3
//#define TOUCH_DATA_CR_POS           4
//#define TOUCH_DATA_NL_POS           5


#define TOUCH_DATA_RED_POS          1 
#define TOUCH_DATA_GREEN_POS        2
#define TOUCH_DATA_BLUE_POS         3
#define TOUCH_DATA_BUTTON_POS       0
#define TOUCH_DATA_CR_POS           4
#define TOUCH_DATA_NL_POS           5
// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_Initialize function.

    Application strings and buffers are be defined outside this structure.
*/

APP_DATA appData;

SYS_CONSOLE_HANDLE touchDevConsoleHandle;
SYS_CONSOLE_HANDLE uartConsoleHandle;

uint8_t slider_table[9] = {0,0,36,73,109,146,182,219,255};

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

/* TODO:  Add any necessary callback functions.
*/

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************

uint8_t asciiNum_to_hex(uint8_t ascii_val)
{
    uint8_t hex_val = 0xFF;
    if((ascii_val>='0') && (ascii_val<='9'))
    {
        hex_val = ascii_val - 48;
    }
    else if( (ascii_val>='A') && (ascii_val<='F')  )
    {
         hex_val = ascii_val - 55;
    }
    else if ( (ascii_val>='a') && (ascii_val<='f') )
    {
        hex_val = ascii_val - 87;
    }
    else
    {
        hex_val = 0xFF;
    }
    return hex_val;
}

static void touch_read_data(void)                                   //Function to read the touch data transmitted by ATtiny3217+T10
{
    ssize_t nUnreadBytes = 0;
    APP_Msg_T appMsg;
    nUnreadBytes = SYS_CONSOLE_ReadCountGet(touchDevConsoleHandle);
    if(nUnreadBytes >=13)
    {
        appMsg.msgId = APP_TOUCH_USART_READ_MSG;
        appMsg.msgData[0] = (uint8_t)nUnreadBytes;
        OSAL_QUEUE_Send(&appData.appQueue, &appMsg, 0);
    }
    else
    {
        appMsg.msgId = APP_TOUCH_USART_GET_COUNT_MSG;
        OSAL_QUEUE_Send(&appData.appQueue, &appMsg, 0);
    }
}

static inline void ble_send_notification(const uint8_t nfy)
{
    if ( APP_GetBleState() == APP_BLE_STATE_CONNECTED)
    {
        APP_TRPS_SendNotification(APP_TRP_VENDOR_OPCODE_BLE_SENSOR,nfy);
    }
    else
    {
        APP_ADV_Init();
    }
}

void APP_Touch_Handler(char *tDataReadBuf)
{
    char *tReadVal;
    APP_TRPS_SensorData_T *sensorData;
    

    if( strncmp(tDataReadBuf, TOUCH_PREAMBLE, strlen(TOUCH_PREAMBLE)) == 0)
    {
        tReadVal = strtok(tDataReadBuf, (const char *)TOUCH_PREAMBLE);

        if(( *(tReadVal+TOUCH_DATA_CR_POS) == '\r') && (*(tReadVal+TOUCH_DATA_NL_POS) == '\n'))
        {
            sensorData =  APP_TRPS_Get_SensorData();
            
            uint8_t red, green, blue;
            red = slider_table[asciiNum_to_hex(  *(tReadVal + TOUCH_DATA_RED_POS) )];
            green= slider_table[asciiNum_to_hex( *(tReadVal + TOUCH_DATA_GREEN_POS) )];
            blue = slider_table[asciiNum_to_hex( *(tReadVal + TOUCH_DATA_BLUE_POS) )];
            RGB_LED_ConvertRGB2HSV(red, green, blue, &sensorData->RGB_color.Hue, &sensorData->RGB_color.Saturation, &sensorData->RGB_color.Value);

            RGB_LED_SetLedColorHSV(sensorData->RGB_color.Hue,sensorData->RGB_color.Saturation,sensorData->RGB_color.Value);            
            
            uint8_t onOff_button_read = asciiNum_to_hex(*(tReadVal + TOUCH_DATA_BUTTON_POS));
            if(onOff_button_read ==  0x01)      //off command
            {
                if(sensorData->rgbOnOffStatus == LED_ON)
                {
                    RGB_LED_Off();
                    sensorData->rgbOnOffStatus = LED_OFF;
                    ble_send_notification(RGB_ONOFF_STATUS_NFY);
                    SYS_CONSOLE_MESSAGE("[Touch] LED OFF\n\r");
                }
            }
            else
            {
                if(sensorData->rgbOnOffStatus == LED_OFF)
                {
                    RGB_LED_SetLedColorHSV(sensorData->RGB_color.Hue,sensorData->RGB_color.Saturation,sensorData->RGB_color.Value);
                    sensorData->rgbOnOffStatus = LED_ON;
                    ble_send_notification(RGB_ONOFF_STATUS_NFY);
                    SYS_CONSOLE_MESSAGE("[Touch] LED ON\n\r");
                }
            }
            if( (onOff_button_read > 1) && (onOff_button_read < 5))
            {                            
                ble_send_notification(RGB_COLOR_NFY);
                SYS_CONSOLE_MESSAGE("[Touch] COLOR SET\n\r");
            }
        }
    }
}

static void APP_Init(void)
{
   
    APP_BleStackInit();

    /* Add BLE Service */
    BLE_DIS_Add();

    APP_UpdateLocalName(0, NULL);
    APP_InitConnList();
    APP_ADV_Init();
    
    SYS_CONSOLE_MESSAGE("BLE Sensor Application: Version ");
    SYS_CONSOLE_PRINT(BLE_SENSOR_VERSION);
    SYS_CONSOLE_MESSAGE("\n\r[BLE} Advertisement Started\n\r");    

    APP_TRPS_Sensor_Init();
    
    APP_OTA_HDL_Init();
}


// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_Initialize ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    appData.state = APP_STATE_INIT;


    appData.appQueue = xQueueCreate( 64, sizeof(APP_Msg_T) );
    /* TODO: Initialize your application's state machine and other
     * parameters.
     */
}

/******************************************************************************
  Function:
    void APP_Tasks ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Tasks ( void )
{
    APP_Msg_T    appMsg[1];
    APP_Msg_T   *p_appMsg;
    p_appMsg=appMsg;
    
    char touchDataReadBuffer[15];
    /* Check the application's current state. */
    switch ( appData.state )
    {
        /* Application's initial state. */
        case APP_STATE_INIT:
        {
            bool appInitialized = true;
            //appData.appQueue = xQueueCreate( APP_QUEUE_LENGTH, sizeof(APP_Msg_T) );

            APP_Init();
            RTC_Timer32Start();
            touchDevConsoleHandle = SYS_CONSOLE_HandleGet(SYS_CONSOLE_INDEX_1);
            uartConsoleHandle = SYS_CONSOLE_HandleGet(SYS_CONSOLE_INDEX_0);
            touch_read_data();
            if (appInitialized)
            {
                APP_Msg_T appMsg;
                appMsg.msgId = APP_TOUCH_USART_WRITE_MSG;
                OSAL_QUEUE_Send(&appData.appQueue, &appMsg, 0);
                appData.state = APP_STATE_SERVICE_TASKS;
            }
            break;
        }

        case APP_STATE_SERVICE_TASKS:
        {
            if (OSAL_QUEUE_Receive(&appData.appQueue, &appMsg, OSAL_WAIT_FOREVER))
            {
                if(p_appMsg->msgId==APP_MSG_BLE_STACK_EVT)
                {
                    // Pass BLE Stack Event Message to User Application for handling
                    APP_BleStackEvtHandler((STACK_Event_T *)p_appMsg->msgData);
                }
                else if(p_appMsg->msgId== APP_TIMER_ADV_CTRL_MSG)
                {
                    APP_BLE_Adv_TimerHandler();
                }
                else if(p_appMsg->msgId== APP_TIMER_BLE_SENSOR_MSG)
                {
                    APP_TRPS_Sensor_TimerHandler();
                }
                else if(p_appMsg->msgId== APP_MSG_TRS_BLE_SENSOR_INT)
                {
                    APP_TRPS_Sensor_Button_Handler();
                }               
                else if(p_appMsg->msgId == APP_TIMER_OTA_TIMEOUT_MSG)
                {
                    APP_OTA_Timeout_Handler(); 
                }
                else if(p_appMsg->msgId== APP_TIMER_OTA_REBOOT_MSG)
                {
                    APP_OTA_Reboot_Handler(); 
                }
                else if(p_appMsg->msgId== APP_TOUCH_USART_READ_MSG)
                {
                    memset(touchDataReadBuffer, 0, sizeof(touchDataReadBuffer));
                    SYS_CONSOLE_Read( touchDevConsoleHandle, touchDataReadBuffer, p_appMsg->msgData[0] );
                    SYS_CONSOLE_Write( uartConsoleHandle, touchDataReadBuffer, strlen(touchDataReadBuffer) );
                    
                    APP_Touch_Handler(touchDataReadBuffer);
                    
                    p_appMsg->msgId = APP_TOUCH_USART_GET_COUNT_MSG;
                    OSAL_QUEUE_Send(&appData.appQueue, p_appMsg, 0);
                }
                else if(p_appMsg->msgId== APP_TOUCH_USART_GET_COUNT_MSG)
                {
                    touch_read_data();
                }
                else if(p_appMsg->msgId== APP_TOUCH_USART_WRITE_MSG)
                {
                    update_touch_data();
                }
            }
            break;
        }

        /* TODO: implement your application state machine.*/


        /* The default state should never be executed. */
        default:
        {
            /* TODO: Handle error in application's state machine. */
            break;
        }
    }
}


/*******************************************************************************
 End of File
 */
