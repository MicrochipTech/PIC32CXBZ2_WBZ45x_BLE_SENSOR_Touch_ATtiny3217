/*******************************************************************************
  Resets (RCON) PLIB

  Company
    Microchip Technology Inc.

  File Name
    plib_rcon.h

  Summary
    RCON PLIB Header File.

  Description
    This file defines the interface to the RCON peripheral library.
    This library provides access to and control of the associated Resets.

  Remarks:
    None.

*******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2019 Microchip Technology Inc. and its subsidiaries.
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

#ifndef PLIB_RCON_H      // Guards against multiple inclusion
#define PLIB_RCON_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdbool.h>
#include <stddef.h>
#include "device.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus // Provide C++ Compatibility

    extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
// *****************************************************************************

    #define    RCON_RESET_CAUSE_POR   RCON_RCON_POR_Msk
    #define    RCON_RESET_CAUSE_BOR   RCON_RCON_BOR_Msk
    #define    RCON_RESET_CAUSE_IDLE   RCON_RCON_IDLE_Msk
    #define    RCON_RESET_CAUSE_SLEEP   RCON_RCON_SLEEP_Msk
    #define    RCON_RESET_CAUSE_WDTO   RCON_RCON_WDTO_Msk
    #define    RCON_RESET_CAUSE_DMTO   RCON_RCON_DMTO_Msk
    #define    RCON_RESET_CAUSE_SWR   RCON_RCON_SWR_Msk
    #define    RCON_RESET_CAUSE_EXTR   RCON_RCON_EXTR_Msk
    #define    RCON_RESET_CAUSE_CMR   RCON_RCON_CMR_Msk
    #define    RCON_RESET_CAUSE_DPSLP   RCON_RCON_DPSLP_Msk
    #define    RCON_RESET_CAUSE_NVMEOL   RCON_RCON_NVMEOL_Msk
    #define    RCON_RESET_CAUSE_NVMLTA   RCON_RCON_NVMLTA_Msk
    #define    RCON_RESET_CAUSE_BCFGFAIL   RCON_RCON_BCFGFAIL_Msk
    #define    RCON_RESET_CAUSE_BCFGERR   RCON_RCON_BCFGERR_Msk
    #define    RCON_RESET_CAUSE_PORCORE   RCON_RCON_PORCORE_Msk
    #define    RCON_RESET_CAUSE_PORIO   RCON_RCON_PORIO_Msk
typedef uint32_t RCON_RESET_CAUSE;

// *****************************************************************************
// *****************************************************************************
// Section: Interface
// *****************************************************************************
// *****************************************************************************

RCON_RESET_CAUSE RCON_ResetCauseGet( void );

void RCON_ResetCauseClear( RCON_RESET_CAUSE cause );

void RCON_SoftwareReset( void );

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    }

#endif
// DOM-IGNORE-END

#endif /* PLIB_RCON_H */
