/*******************************************************************************
 System Interrupts File

  File Name:
    system_interrupt.c

  Summary:
    Raw ISR definitions.

  Description:
    This file contains a definitions of the raw ISRs required to support the
    interrupt sub-system.

  Summary:
    This file contains source code for the interrupt vector functions in the
    system.

  Description:
    This file contains source code for the interrupt vector functions in the
    system.  It implements the system and part specific vector "stub" functions
    from which the individual "Tasks" functions are called for any modules
    executing interrupt-driven in the MPLAB Harmony system.

  Remarks:
    This file requires access to the systemObjects global data structure that
    contains the object handles to all MPLAB Harmony module objects executing
    interrupt-driven in the system.  These handles are passed into the individual
    module "Tasks" functions to identify the instance of the module to maintain.
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (c) 2011-2014 released Microchip Technology Inc.  All rights reserved.

Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).

You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.

SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
(INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
 *******************************************************************************/
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "system/common/sys_common.h"
#include "app.h"
#include "system_definitions.h"

//float RightWheel, LeftWheel;
float kiR, kpR, kiL, kpL;
float outRight, outLeft;
float RightWheel, LeftWheel;
float RightWheelD, LeftWheelD;
// *****************************************************************************
// *****************************************************************************
// Section: System Interrupt Vector Functions
// *****************************************************************************
// *****************************************************************************

void __ISR(_TIMER_4_VECTOR, IPL4SOFT) Timer4ISR(void) {
  // code for PI control goes here
    /*
    //find wheel velocity 
    RightWheelD = (5*1000000)*right + 1*1000000000; 
    LeftWheelD = (5*1000000)*left + 1*1000000000; 
    RightWheel = TMR3;  //rev/s 
    LeftWheel = TMR5; //rev/s        
    
    //set variables such that they are only declared once 
    static float intRight = 0;
    static float intLeft = 0;
    static float errorRight = 0;
    static float errorLeft = 0;             
    
    //PI controller
    errorRight = RightWheel - RightWheelD; //difference between actual and desired
    intRight = intRight + (errorRight*RightWheel); //integral value
    outRight = (kpR * errorRight + kiR * intRight)/100.0; //set output based on gains
    
    errorLeft = LeftWheel - LeftWheelD; //difference between actual and desired
    intLeft = intLeft + (errorLeft*LeftWheel); //integral
    outLeft = (kpL * errorLeft + kiL * intLeft)/100.0; //set output based on gains
    
    if(outRight > 1){ //cap output 
        outRight = 1;
    }
    else if(outRight < -1){
        outRight = -1;
    }
    
    if(outLeft > 1){ //cap output 
        outLeft = 1;
    }
    else if(outLeft < -1){
        outLeft = -1;
    }
    
    
    OC1RS = outLeft*MAX_DUTY; //set OC1RS right wheel
    OC4RS = outRight*MAX_DUTY; //set OC4RS left wheel  
   
    TMR3 = 0;  //set timer 3 = 0
    TMR5 = 0;  //set timer 5 = 0
    */
  IFS0bits.T4IF = 0; // clear interrupt flag, last line
}

void __ISR(_USB_1_VECTOR, ipl4AUTO) _IntHandlerUSBInstance0(void)
{
    DRV_USBFS_Tasks_ISR(sysObj.drvUSBObject);
}

/*******************************************************************************
 End of File
*/
