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

// DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (c) 2013-2014 released Microchip Technology Inc.  All rights reserved.

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

#include "app.h"
#include "i2c_master_noint.h"
#include "ST7735.h"
#include<stdio.h>

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


/* TODO:  Add any necessary local functions.
*/


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

//define some things
char tempmes[30];
char xxlmes[30];
char yxlmes[30];
char zxlmes[30];
#define ADDR 0b1101011
unsigned char b[14];
signed short temp,xg, yg, zg, xxl, yxl, zxl, xxl2,yxl2,zxl2;

void APP_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    appData.state = APP_STATE_INIT;

    TRISBbits.TRISB4 = 1; //make pushbutton pin an input pin
    TRISAbits.TRISA4 = 0; //make LED pin an output
    LATAbits.LATA4 = 1; //make LED pin low to start
    
    

    SPI1_init();
    LCD_init();
    LCD_clearScreen(GREEN);
    initExpander();
    //write to several registers to initialize chip
    writei2c(0x10,0b10000010);  //CTRL1_XL turn on accelerometer, 1.66 kHz 2g, 100Hz
    writei2c(0x11,0b10001000);  //CTRL2_g turn on gyroscope, 1.66 kHz, 1000 dps sensitivity
    writei2c(0x12,0b00000100);  //CTRL3_C make sure IF_INC bit is 1
    
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

    /* Check the application's current state. */
    switch ( appData.state )
    {
        /* Application's initial state. */
        case APP_STATE_INIT:
        {
            bool appInitialized = true;
       
        
            if (appInitialized)
            {
            
                appData.state = APP_STATE_SERVICE_TASKS;
            }
            break;
        }

        case APP_STATE_SERVICE_TASKS:
        {
            
            _CP0_SET_COUNT(0);
         
            while(_CP0_GET_COUNT() < 1200000) { // (2E-3)/(1/24E6) is # core ticks
            LATAbits.LATA4 = 1; //make LED pin high
         
            readi2c_multiple(ADDR, 0x20,b,14);
            sprintf(tempmes,"temp=%d   ",temp);
            sprintf(xxlmes,"xxl=%d    ",xxl2);
            sprintf(yxlmes,"yxl=%d     ",yxl2);
            sprintf(zxlmes,"zxl=%d     ",zxl2);
            LCD_drawString(2,5,tempmes,RED,BLUE);
            LCD_drawString(2,13,xxlmes,RED,BLUE);
            LCD_drawString(2,21,yxlmes,RED,BLUE);
            LCD_drawString(2,29,zxlmes,RED,BLUE);
            
            LCD_drawProgressBar(64,80,xxl2,zxl2,CYAN,MAGENTA);
            }
         
            _CP0_SET_COUNT(0);
         
            while(_CP0_GET_COUNT() < 1200000) { // wait 5 ms again
            LATAbits.LATA4 = 0; //make LED pin low (off)  
        
            readi2c_multiple(ADDR, 0x20,b,14);
            sprintf(tempmes,"temp=%d   ",temp);
            sprintf(xxlmes,"xxl=%d    ",xxl2);
            sprintf(yxlmes,"yxl=%d     ",yxl2);
            sprintf(zxlmes,"zxl=%d     ",zxl2);
            LCD_drawString(2,5,tempmes,RED,BLUE);
            LCD_drawString(2,13,xxlmes,RED,BLUE);
            LCD_drawString(2,21,yxlmes,RED,BLUE);
            LCD_drawString(2,29,zxlmes,RED,BLUE);
    
            LCD_drawProgressBar(64,80,xxl2,zxl2,CYAN,MAGENTA);
  
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
