/*******************************************************************************
  RGB LED Color control

  Company:
    Microchip Technology Inc.

  File Name:
    rgb_led.c

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

#include "sensors/inc/rgb_led.h"
#include "peripheral/tc/plib_tc2.h"
#include "peripheral/tc/plib_tc3.h"
#include "math.h"

// *****************************************************************************
// *****************************************************************************
// Section: Macros
// *****************************************************************************
// *****************************************************************************
//#define COLOR_XY
#define COLOR_HSV

#define TOP 0xFFFF

#ifdef COLOR_XY

#define NR_OF_PRIMES (3u)    /* Red, Blue and Green */
#define SL_LIGHTLEVEL_RED   0u
#define SL_LIGHTLEVEL_GREEN 1u
#define SL_LIGHTLEVEL_BLUE  2u


#define LED_DATA_RED   {0.64000f,0.33000f,54.0f,0.0f}
#define LED_DATA_GREEN {0.30000f,0.60000f,182.0f,0.0f}
#define LED_DATA_BLUE  {0.15000f,0.06000f,18.4f,0.0f} 
   
// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
// *****************************************************************************
typedef struct
{
    float fx;        // x point
    float fy;        // y point
    float fLm;       // lumen output
    float usDC;      // last DutyCycle divided by 65535 (between 0 and 1)
} LedData_t;
#endif

// *****************************************************************************
// *****************************************************************************
// Section: Functions
// *****************************************************************************
// *****************************************************************************
#ifdef COLOR_XY
static inline void RGB_LED_SetColorXY(void);
static void calcUsDC(float s_setX, float s_setY, float s_setL);
static void XYL2RGB(unsigned int valX, unsigned int valY, int valL, unsigned int *calcR, unsigned int *calcG, unsigned int *calcB);
#endif
#ifdef COLOR_HSV
static inline void RGB_LED_SetColorHSV(void);
void HSV2RGB(unsigned int hue, unsigned int saturation,unsigned int level, unsigned int *calcR, unsigned int *calcG, unsigned int *calcB);
#endif
#if defined (COLOR_XY) || defined (COLOR_HSV)
static void RGB_LED_SetPwmChannelCompareValue(uint16_t r, uint16_t g, uint16_t b);
#endif

// *****************************************************************************
// *****************************************************************************
// Section: Local Variables
// *****************************************************************************
// *****************************************************************************
#ifdef COLOR_XY
static LedData_t s_ledData[NR_OF_PRIMES] = \
        {LED_DATA_RED, LED_DATA_GREEN, LED_DATA_BLUE};
uint16_t currentX, currentY;
#endif
#ifdef COLOR_HSV
uint8_t currentHue, currentSat;
#endif
#if defined (COLOR_XY) || defined(COLOR_HSV)
uint8_t currentL = 0;
bool lastXYColourMode = false;
bool pwmStopped = true;
#endif


#ifdef COLOR_HSV
/**************************************************************************//**
Set the color using Hue and Saturation as parameters
******************************************************************************/
void RGB_LED_SetLedColorHS(uint16_t hue, uint8_t saturation)
{
  currentHue = hue >> 8;
  currentSat = saturation;
  
  lastXYColourMode = false;
  
  RGB_LED_SetColorHSV();
}

/**************************************************************************//**
Set the color using Hue, Saturation and brightness level as parameters
******************************************************************************/
void RGB_LED_SetLedColorHSV(uint8_t hue, uint8_t saturation, uint8_t level)
{
  currentHue = hue;
  currentSat = saturation;
  currentL = level;
  
  RGB_LED_SetColorHSV();
} 


static inline void RGB_LED_SetColorHSV(void)
{
  unsigned int r, g, b;
  
  HSV2RGB(currentHue, currentSat,currentL, &r, &g, &b);
  
  RGB_LED_SetPwmChannelCompareValue(r, g, b);
}

/**************************************************************************//**
Converts R G & B values of the color to corresponding H S V values
******************************************************************************/
static float max(float a, float b, float c) {
   return ((a > b)? (a > c ? a : c) : (b > c ? b : c));
}

static float min(float a, float b, float c) {
   return ((a < b)? (a < c ? a : c) : (b < c ? b : c));
}

void RGB_LED_ConvertRGB2HSV(uint8_t red, uint8_t green, uint8_t blue, uint8_t *calcHue, uint8_t *calcSatuartion, uint8_t *calcLevel)
{
   // R, G, B values are divided by 255
   // to change the range from 0..255 to 0..1:
   float h=0, s=0, v=0, r, g, b;
   r = (float)red/255.0;
   g = (float)green/255.0;
   b = (float)blue/255.0;
   float cmax = max(r, g, b); // maximum of r, g, b
   float cmin = min(r, g, b); // minimum of r, g, b
   float diff = cmax-cmin; // diff of cmax and cmin.
   if (cmax == cmin)
      h = 0;
   else if (cmax == r)
      h = fmod((60 * ((g - b) / diff) + 255), 255.0);
   else if (cmax == g)
      h = fmod((60 * ((b - r) / diff) + 85), 255.0);
   else if (cmax == b)
      h = fmod((60 * ((r - g) / diff) + 170), 255.0);
   // if cmax equal zero
    if (cmax == 0)
        s = 0;
    else
        s = (diff / cmax) * 255;
   // compute v
   v = cmax * 255;
   
   *calcHue = (uint8_t)h;
   *calcSatuartion = (uint8_t)s;
   *calcLevel = (uint8_t)v;
}

/**************************************************************************//**
Converts Hue/Saturation/Value values of the color to corresponding R/G/B values
******************************************************************************/
void HSV2RGB(unsigned int hue, unsigned int saturation,unsigned int level, unsigned int *calcR, unsigned int *calcG, unsigned int *calcB)
{
  unsigned int var_i;
  float V,H,S, var_h,var_1,var_2,var_3,var_r,var_g,var_b;
  
  H = (float) hue/255;
  S = (float) saturation/255;
  V = (float) level/255;

  if (saturation == 0)
  {
    var_r = V;
    var_g = V;
    var_b = V;
  }
  else
  {
    var_h = H * 6;
    if ( var_h == 6 ) 
       var_h = 0;
    var_i = (unsigned int)var_h ;
    var_1 = V * ( 1 - S );
    var_2 = V * ( 1 - S * ( var_h - var_i ) );
    var_3 = V * ( 1 - S * ( 1 - ( var_h - var_i ) ) );

    switch (var_i)
    {
        case 0:
        { 
            var_r = V;
            var_g = var_3;
            var_b = var_1;
        }
        break;
            
        case 1:
        {
            var_r = var_2;
            var_g = V; 
            var_b = var_1;
        }
        break;
            
        case 2:
        {
            var_r = var_1 ;
            var_g = V; 
            var_b = var_3;                
        }
        break;
            
        case 3:
        {
            var_r = var_1 ;
            var_g = var_2 ;
            var_b = V ;
        }
        break;
            
        case 4:
        {
            var_r = var_3 ; 
            var_g = var_1 ; 
            var_b = V;                
        }
        break;
            
        default:
        {
            var_r = V     ;
            var_g = var_1 ;
            var_b = var_2 ;               
        }
        break;
    }
  }

   *calcR = var_r * 65535;
   *calcG = var_g * 65535;
   *calcB = var_b * 65535;
}
#endif

#if defined (COLOR_XY) || defined (COLOR_HSV)
/****************************************************************
Turn off RGB LED.
****************************************************************/
void RGB_LED_Off(void)
{
    TC2_CompareStop();
    TC3_CompareStop();
    pwmStopped = true;
}

/**************************************************************************//**
Set brightness level
******************************************************************************/
void RGB_LED_SetBrightnessLevel(uint8_t level)
{
    currentL = level;
   
    if(lastXYColourMode)
    { 
#if defined (COLOR_XY)        
       RGB_LED_SetColorXY();
#endif       
    }   
    else
    {
#if defined (COLOR_HSV)        
       RGB_LED_SetColorHSV();
#endif       
    }   
}

/**************************************************************************//**
\brief Set compare value for PWM channels
******************************************************************************/
static void RGB_LED_SetPwmChannelCompareValue(uint16_t r, uint16_t g, uint16_t b)
{
  r = r * TOP / 65535;
  g = g * TOP / 65535;
  b = b * TOP / 65535;

  if(pwmStopped)
  {
       TC2_CompareStart();
       TC3_CompareStart();
       pwmStopped = false;
  }
  
  TC2_Compare16bitMatch0Set(r);

  TC2_Compare16bitMatch1Set(g);
  
  // Errata Workaround
  TC2_CompareStop(); 
  TC2_REGS->COUNT16.TC_CTRLA |=  ( TC_CTRLA_CAPTEN0_Msk | TC_CTRLA_COPEN0_Msk | TC_CTRLA_CAPTEN1_Msk| TC_CTRLA_COPEN1_Msk);
  TC2_CompareStart();

  TC3_Compare16bitMatch0Set(b);
  
  // Errata Workaround
  TC3_CompareStop();
  TC3_REGS->COUNT16.TC_CTRLA |= ( TC_CTRLA_CAPTEN0_Msk | TC_CTRLA_COPEN0_Msk );
  TC3_CompareStart();
  
}
#endif

void RGB_LED_SetPwmChannel(uint16_t r, uint16_t g, uint16_t b)
{
    RGB_LED_SetPwmChannelCompareValue(r,g,b);
}

#ifdef COLOR_XY
/**************************************************************************//**
\brief Set the color using X and Y as parameters
******************************************************************************/
void RGB_LED_SetLedColorXY(uint16_t x, uint16_t y)
{
  currentX = x;
  currentY = y;
  lastXYColourMode = true;
  RGB_LED_SetColorXY();
}

/**************************************************************************//**
\brief Convert X, Y parameters to RGB and set the color
******************************************************************************/
static inline void RGB_LED_SetColorXY(void)
{
  unsigned int r, g, b;
  
  XYL2RGB(currentX, currentY, currentL, &r, &g, &b);

  RGB_LED_SetPwmChannelCompareValue(r, g, b);
}

/**************************************************************************//**
\brief Calculate PWM duty cycle X,Y, L values
******************************************************************************/
static void calcUsDC(float s_setX, float s_setY, float s_setL)
{
    float fraction;
    float rbg = 0.0F;
    float bbg = 0.0F;
    float rxMSetx = (s_ledData[SL_LIGHTLEVEL_RED].fx - s_setX);
    float ryMSety = (s_ledData[SL_LIGHTLEVEL_RED].fy - s_setY);
    float gxMSetx = (s_ledData[SL_LIGHTLEVEL_GREEN].fx - s_setX);
    float gyMSety = (s_ledData[SL_LIGHTLEVEL_GREEN].fy - s_setY);
    float bxMSetx = (s_ledData[SL_LIGHTLEVEL_BLUE].fx - s_setX);
    float byMSety = (s_ledData[SL_LIGHTLEVEL_BLUE].fy - s_setY);

    // matrix inversion
    float temp = s_ledData[SL_LIGHTLEVEL_GREEN].fy * ((rxMSetx * byMSety) - (bxMSetx * ryMSety));
    if (temp != 0.0F)
    { //robustness to assure that divider != 0
        rbg = (-s_ledData[SL_LIGHTLEVEL_RED].fy * ((gxMSetx * byMSety) - (bxMSetx * gyMSety)))/ temp;
        temp = s_ledData[SL_LIGHTLEVEL_GREEN].fy * ((bxMSetx * ryMSety) - (rxMSetx * byMSety));
        if (temp != 0.0F)
        { //robustness to assure that divider != 0
            bbg = (-s_ledData[SL_LIGHTLEVEL_BLUE].fy * ((gxMSetx * ryMSety) - (rxMSetx * gyMSety)))/ temp;
            temp = rbg + 1.0f + bbg;
            if (temp != 0.0F)
            { //robustness to assure that divider != 0
                temp = s_setL / temp;
            }
        }
    }

    // introduce the lumen output in the fraction

    // calculate the red portion of the mixed light
    if (s_ledData[SL_LIGHTLEVEL_RED].fLm != 0.0F)
    {
        fraction = (temp * rbg) / s_ledData[SL_LIGHTLEVEL_RED].fLm;
        if (fraction > 0.0f)
        {
            s_ledData[SL_LIGHTLEVEL_RED].usDC =  fraction;
        }
    }

    // calculate the green portion of the mixed light
    if (s_ledData[SL_LIGHTLEVEL_GREEN].fLm != 0.0F)
    {
        fraction = temp / s_ledData[SL_LIGHTLEVEL_GREEN].fLm;
        if (fraction > 0.0f)
        {
            s_ledData[SL_LIGHTLEVEL_GREEN].usDC =  fraction;
        }
    }

    // calculate the blue portion of the mixed light
    if (s_ledData[SL_LIGHTLEVEL_BLUE].fLm != 0.0F)
    {
        fraction = (temp * bbg) / s_ledData[SL_LIGHTLEVEL_BLUE].fLm;
        if (fraction > 0.0f)
        {
            s_ledData[SL_LIGHTLEVEL_BLUE].usDC = fraction;
        }
    }
    
    if ( s_ledData[SL_LIGHTLEVEL_RED].usDC > 0.0031308 ) 
        s_ledData[SL_LIGHTLEVEL_RED].usDC = (1.055 * ( pow(s_ledData[SL_LIGHTLEVEL_RED].usDC, ( 1 / 2.4 )) )) - 0.055 ;
    else
        s_ledData[SL_LIGHTLEVEL_RED].usDC = 12.92 * s_ledData[SL_LIGHTLEVEL_RED].usDC;
    
    if ( s_ledData[SL_LIGHTLEVEL_GREEN].usDC > 0.0031308 )
        s_ledData[SL_LIGHTLEVEL_GREEN].usDC = (1.055 * ( pow(s_ledData[SL_LIGHTLEVEL_GREEN].usDC,( 1 / 2.4 )) )) - 0.055;
    else
        s_ledData[SL_LIGHTLEVEL_GREEN].usDC = 12.92 * s_ledData[SL_LIGHTLEVEL_GREEN].usDC;
    
    if ( s_ledData[SL_LIGHTLEVEL_BLUE].usDC > 0.0031308 )
        s_ledData[SL_LIGHTLEVEL_BLUE].usDC = (1.055 * ( pow(s_ledData[SL_LIGHTLEVEL_BLUE].usDC,( 1 / 2.4 )) )) - 0.055;
    else
        s_ledData[SL_LIGHTLEVEL_BLUE].usDC = 12.92 * s_ledData[SL_LIGHTLEVEL_BLUE].usDC;
}

/**************************************************************************//**
\brief Converts X/Y/L values of the color to corresponding R/G/B values. 
******************************************************************************/
static void XYL2RGB(unsigned int valX, unsigned int valY, int valL, unsigned int *calcR, unsigned int *calcG, unsigned int *calcB)
{
  float s_setX, s_setY, s_setL;  
  float s_brightnessTemperatureFactor = 1.0f;

  s_setX = (float)valX/65535.0f;
  s_setY = (float)valY/65535.0f;
  s_setL = valL;


  for (unsigned int cnt = 0u; cnt < NR_OF_PRIMES; cnt++)
  {
      s_ledData[cnt].usDC = 0.0f;
  }

  if ((s_setX != 0.0f) && (s_setY != 0.0f) && (s_setL != 0.0f))
  {

      calcUsDC(s_setX, s_setY, s_setL);

      /* check for duty cycle > 1.0 */
      float maxFactor = s_ledData[SL_LIGHTLEVEL_RED].usDC;
      if (maxFactor < s_ledData[SL_LIGHTLEVEL_GREEN].usDC)
      {
          maxFactor = s_ledData[SL_LIGHTLEVEL_GREEN].usDC;
      }
      if (maxFactor < s_ledData[SL_LIGHTLEVEL_BLUE].usDC)
      {
          maxFactor = s_ledData[SL_LIGHTLEVEL_BLUE].usDC;
      }

      if (maxFactor > 1.0f)
      {
          maxFactor = s_brightnessTemperatureFactor/maxFactor;
      }
      else
      {
          maxFactor = s_brightnessTemperatureFactor;
      }

      if (maxFactor != 1.0f)
      {
          s_ledData[SL_LIGHTLEVEL_RED].usDC *= maxFactor;
          s_ledData[SL_LIGHTLEVEL_GREEN].usDC *= maxFactor;
          s_ledData[SL_LIGHTLEVEL_BLUE].usDC *= maxFactor;
      }
  }

  float red = s_ledData[SL_LIGHTLEVEL_RED].usDC * 65535.0f;
  float green = s_ledData[SL_LIGHTLEVEL_GREEN].usDC * 65535.0f;
  float blue = s_ledData[SL_LIGHTLEVEL_BLUE].usDC * 65535.0f;

  *calcR = (unsigned int) red;
  *calcG = (unsigned int) green;
  *calcB = (unsigned int) blue;
}

#endif
