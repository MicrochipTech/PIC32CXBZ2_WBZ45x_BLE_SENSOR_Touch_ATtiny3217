/*******************************************************************************
  RGB LED Color control Header file

  Company:
    Microchip Technology Inc.

  File Name:
    rgb_led.h

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

#ifndef _RGB_LED_H
#define _RGB_LED_H

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
/****************************************************************
Turn off RGB LED.
****************************************************************/
void RGB_LED_Off(void);

/**************************************************************************//**
Set brightness level
******************************************************************************/
void RGB_LED_SetBrightnessLevel(uint8_t level);

/**************************************************************************//**
Set the color using Hue and Saturation as parameters
******************************************************************************/
void RGB_LED_SetLedColorHS(uint16_t hue, uint8_t saturation);

/**************************************************************************//**
Set the color using Hue, Saturation and brightness level as parameters
******************************************************************************/
void RGB_LED_SetLedColorHSV(uint8_t hue, uint8_t saturation, uint8_t level);

/**************************************************************************//**
\brief Set the color using X and Y as parameters
******************************************************************************/
void RGB_LED_SetLedColorXY(uint16_t x, uint16_t y);

void RGB_LED_ConvertRGB2HSV(uint8_t red, uint8_t green, uint8_t blue, uint8_t *calcHue, uint8_t *calcSatuartion, uint8_t *calcLevel);

void HSV2RGB(unsigned int hue, unsigned int saturation,unsigned int level, unsigned int *calcR, unsigned int *calcG, unsigned int *calcB);
#endif