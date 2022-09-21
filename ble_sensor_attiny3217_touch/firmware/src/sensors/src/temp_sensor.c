/*******************************************************************************
  Temperature sensor measurement

  Company:
    Microchip Technology Inc.

  File Name:
    temp_sensor.c

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


// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include "math.h"
#include "peripheral/adchs/plib_adchs.h"

// *****************************************************************************
// *****************************************************************************
// Section: Macros
// *****************************************************************************
// *****************************************************************************
#define ADC_VREF  (3.00)

#define ADC_MAX    4096

#define MCP9700_V0C  (0.5)

#define MCP9700_TC   (0.01)

// *****************************************************************************
// *****************************************************************************
// Section: Local Variables
// *****************************************************************************
// *****************************************************************************


// *****************************************************************************
// *****************************************************************************
// Section: Functions
// *****************************************************************************
// *****************************************************************************

/**************************************************************************//**
\brief Reads the ADC input and converts to temperature
******************************************************************************/
float MCP9700_Temp_Celsius(void)
{
	uint16_t adc_read=0, adc_read1=0;
    volatile float temperature = 0, vout;
    uint8_t i=0;
    
    /* Take 4 Samples and take the average*/
	for(i=0; i<5; i++)
    {
        ADCHS_GlobalEdgeConversionStart();
    
        while(!ADCHS_ChannelResultIsReady(ADCHS_CH2));
        
        adc_read1 = ADCHS_ChannelResultGet(ADCHS_CH2);
        
        if(i!=0)
            adc_read = adc_read + adc_read1;
    }
    adc_read = adc_read/4;
    
    vout =  ((float)adc_read)/((float) ADC_MAX);
        
    vout = vout * ADC_VREF;  // Convert to voltage with 3.25 V is reference
              
    // TA = (VOUT - V0°C )/TC  TC = 0.01, V0°C = 0.5v as per MCP9700/9700A datasheet
	temperature = (vout - MCP9700_V0C)/MCP9700_TC;
	
	return (temperature);
}


/**************************************************************************//**
\brief Reads the ADC input and converts to temperature in Fahrenheit
******************************************************************************/
float MCP9700_Temp_Fahrenheit(void)
{
    volatile float temperature = 0;
	
    temperature = MCP9700_Temp_Celsius();
	
    temperature = ((temperature * 9/5)) + 32;
	
	return (temperature);
}