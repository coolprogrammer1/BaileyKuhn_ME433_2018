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
#include <stdio.h>
#include <xc.h>
#include "i2c_master_noint.h"
#include "ST7735.h"


// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

uint8_t APP_MAKE_BUFFER_DMA_READY dataOut[APP_READ_BUFFER_SIZE];
uint8_t APP_MAKE_BUFFER_DMA_READY readBuffer[APP_READ_BUFFER_SIZE];
int len1, i = 0;
int startTime = 0;

//define some things
char tempmes[30];
char xgmes[30];
char ygmes[30];
char zgmes[30];
char xxlmes[30];
char yxlmes[30];
char zxlmes[30];
#define ADDR 0b1101011
unsigned char b[14];
signed short temp,xg, yg, zg, xxl, yxl, zxl, xxl2,yxl2,zxl2;
int set = 0;
int ii = 0;
int j,k = 0;
int avg = 4;
int rawData[100];
float MAFfilteredData[100];
float FIRfilteredData[100];
float IIRfilteredData[100];
int buffer[8];
float A = 0.8;
float B = 0.2;
//Bvec Vector from MATLAB

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

/*******************************************************
 * USB CDC Device Events - Application Event Handler
 *******************************************************/

USB_DEVICE_CDC_EVENT_RESPONSE APP_USBDeviceCDCEventHandler
(
        USB_DEVICE_CDC_INDEX index,
        USB_DEVICE_CDC_EVENT event,
        void * pData,
        uintptr_t userData
        ) {
    APP_DATA * appDataObject;
    appDataObject = (APP_DATA *) userData;
    USB_CDC_CONTROL_LINE_STATE * controlLineStateData;

    switch (event) {
        case USB_DEVICE_CDC_EVENT_GET_LINE_CODING:

            /* This means the host wants to know the current line
             * coding. This is a control transfer request. Use the
             * USB_DEVICE_ControlSend() function to send the data to
             * host.  */

            USB_DEVICE_ControlSend(appDataObject->deviceHandle,
                    &appDataObject->getLineCodingData, sizeof (USB_CDC_LINE_CODING));

            break;

        case USB_DEVICE_CDC_EVENT_SET_LINE_CODING:

            /* This means the host wants to set the line coding.
             * This is a control transfer request. Use the
             * USB_DEVICE_ControlReceive() function to receive the
             * data from the host */

            USB_DEVICE_ControlReceive(appDataObject->deviceHandle,
                    &appDataObject->setLineCodingData, sizeof (USB_CDC_LINE_CODING));

            break;

        case USB_DEVICE_CDC_EVENT_SET_CONTROL_LINE_STATE:

            /* This means the host is setting the control line state.
             * Read the control line state. We will accept this request
             * for now. */

            controlLineStateData = (USB_CDC_CONTROL_LINE_STATE *) pData;
            appDataObject->controlLineStateData.dtr = controlLineStateData->dtr;
            appDataObject->controlLineStateData.carrier = controlLineStateData->carrier;

            USB_DEVICE_ControlStatus(appDataObject->deviceHandle, USB_DEVICE_CONTROL_STATUS_OK);

            break;

        case USB_DEVICE_CDC_EVENT_SEND_BREAK:

            /* This means that the host is requesting that a break of the
             * specified duration be sent. Read the break duration */

            appDataObject->breakData = ((USB_DEVICE_CDC_EVENT_DATA_SEND_BREAK *) pData)->breakDuration;

            /* Complete the control transfer by sending a ZLP  */
            USB_DEVICE_ControlStatus(appDataObject->deviceHandle, USB_DEVICE_CONTROL_STATUS_OK);

            break;

        case USB_DEVICE_CDC_EVENT_READ_COMPLETE:

            /* This means that the host has sent some data*/
            appDataObject->isReadComplete = true;
            break;

        case USB_DEVICE_CDC_EVENT_CONTROL_TRANSFER_DATA_RECEIVED:

            /* The data stage of the last control transfer is
             * complete. For now we accept all the data */

            USB_DEVICE_ControlStatus(appDataObject->deviceHandle, USB_DEVICE_CONTROL_STATUS_OK);
            break;

        case USB_DEVICE_CDC_EVENT_CONTROL_TRANSFER_DATA_SENT:

            /* This means the GET LINE CODING function data is valid. We dont
             * do much with this data in this demo. */
            break;

        case USB_DEVICE_CDC_EVENT_WRITE_COMPLETE:

            /* This means that the data write got completed. We can schedule
             * the next read. */

            appDataObject->isWriteComplete = true;
            break;

        default:
            break;
    }

    return USB_DEVICE_CDC_EVENT_RESPONSE_NONE;
}

/***********************************************
 * Application USB Device Layer Event Handler.
 ***********************************************/
void APP_USBDeviceEventHandler(USB_DEVICE_EVENT event, void * eventData, uintptr_t context) {
    USB_DEVICE_EVENT_DATA_CONFIGURED *configuredEventData;

    switch (event) {
        case USB_DEVICE_EVENT_SOF:

            /* This event is used for switch debounce. This flag is reset
             * by the switch process routine. */
            appData.sofEventHasOccurred = true;
            break;

        case USB_DEVICE_EVENT_RESET:

            /* Update LED to show reset state */

            appData.isConfigured = false;

            break;

        case USB_DEVICE_EVENT_CONFIGURED:

            /* Check the configuratio. We only support configuration 1 */
            configuredEventData = (USB_DEVICE_EVENT_DATA_CONFIGURED*) eventData;
            if (configuredEventData->configurationValue == 1) {
                /* Update LED to show configured state */

                /* Register the CDC Device application event handler here.
                 * Note how the appData object pointer is passed as the
                 * user data */

                USB_DEVICE_CDC_EventHandlerSet(USB_DEVICE_CDC_INDEX_0, APP_USBDeviceCDCEventHandler, (uintptr_t) & appData);

                /* Mark that the device is now configured */
                appData.isConfigured = true;

            }
            break;

        case USB_DEVICE_EVENT_POWER_DETECTED:

            /* VBUS was detected. We can attach the device */
            USB_DEVICE_Attach(appData.deviceHandle);
            break;

        case USB_DEVICE_EVENT_POWER_REMOVED:

            /* VBUS is not available any more. Detach the device. */
            USB_DEVICE_Detach(appData.deviceHandle);
            break;

        case USB_DEVICE_EVENT_SUSPENDED:

            /* Switch LED to show suspended state */
            break;

        case USB_DEVICE_EVENT_RESUMED:
        case USB_DEVICE_EVENT_ERROR:
        default:
            break;
    }
}

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************

/*****************************************************
 * This function is called in every step of the
 * application state machine.
 *****************************************************/

bool APP_StateReset(void) {
    /* This function returns true if the device
     * was reset  */

    bool retVal;

    if (appData.isConfigured == false) {
        appData.state = APP_STATE_WAIT_FOR_CONFIGURATION;
        appData.readTransferHandle = USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;
        appData.writeTransferHandle = USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;
        appData.isReadComplete = true;
        appData.isWriteComplete = true;
        retVal = true;
    } else {
        retVal = false;
    }

    return (retVal);
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

void APP_Initialize(void) {
    /* Place the App state machine in its initial state. */
    appData.state = APP_STATE_INIT;

    /* Device Layer Handle  */
    appData.deviceHandle = USB_DEVICE_HANDLE_INVALID;

    /* Device configured status */
    appData.isConfigured = false;

    /* Initial get line coding state */
    appData.getLineCodingData.dwDTERate = 9600;
    appData.getLineCodingData.bParityType = 0;
    appData.getLineCodingData.bParityType = 0;
    appData.getLineCodingData.bDataBits = 8;

    /* Read Transfer Handle */
    appData.readTransferHandle = USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;

    /* Write Transfer Handle */
    appData.writeTransferHandle = USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;

    /* Intialize the read complete flag */
    appData.isReadComplete = true;

    /*Initialize the write complete flag*/
    appData.isWriteComplete = true;

    /* Reset other flags */
    appData.sofEventHasOccurred = false;
    //appData.isSwitchPressed = false;

    /* Set up the read buffer */
    appData.readBuffer = &readBuffer[0];
    
    appData.state = APP_STATE_INIT;
    
    
//MY STUFF
    TRISBbits.TRISB4 = 1; //make pushbutton pin an input pin
    TRISAbits.TRISA4 = 0; //make LED pin an output
    LATAbits.LATA4 = 1; //make LED pin low to start
    
   //initializing other stuff
    SPI1_init();
    initExpander();
    //write to several registers to initialize chip
    writei2c(0x10,0b10000010);  //CTRL1_XL turn on accelerometer, 1.66 kHz 2g, 100Hz
    writei2c(0x11,0b10001000);  //CTRL2_g turn on gyroscope, 1.66 kHz, 1000 dps sensitivity
    writei2c(0x12,0b00000100);  //CTRL3_C make sure IF_INC bit is 1

    startTime = _CP0_GET_COUNT();
}

/******************************************************************************
  Function:
    void APP_Tasks ( void )
  Remarks:
    See prototype in app.h.
 */

void APP_Tasks(void) {
    /* Update the application state machine based
     * on the current state */

    switch (appData.state) {
        case APP_STATE_INIT:

            /* Open the device layer */
            appData.deviceHandle = USB_DEVICE_Open(USB_DEVICE_INDEX_0, DRV_IO_INTENT_READWRITE);

            if (appData.deviceHandle != USB_DEVICE_HANDLE_INVALID) {
                /* Register a callback with device layer to get event notification (for end point 0) */
                USB_DEVICE_EventHandlerSet(appData.deviceHandle, APP_USBDeviceEventHandler, 0);

                appData.state = APP_STATE_WAIT_FOR_CONFIGURATION;
            } else {
                /* The Device Layer is not ready to be opened. We should try
                 * again later. */
            }

            break;

        case APP_STATE_WAIT_FOR_CONFIGURATION:

            /* Check if the device was configured */
            if (appData.isConfigured) {
                /* If the device is configured then lets start reading */
                appData.state = APP_STATE_SCHEDULE_READ;
            }
            break;

        case APP_STATE_SCHEDULE_READ:

            if (APP_StateReset()) {
                break;
            }

            /* If a read is complete, then schedule a read
             * else wait for the current read to complete */

            appData.state = APP_STATE_WAIT_FOR_READ_COMPLETE;
            if (appData.isReadComplete == true) {
                appData.isReadComplete = false;
                appData.readTransferHandle = USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;

                USB_DEVICE_CDC_Read(USB_DEVICE_CDC_INDEX_0,
                        &appData.readTransferHandle, appData.readBuffer,
                        APP_READ_BUFFER_SIZE);
                
                if(appData.readBuffer[0]=='r'){
                    set = 1;
                    i=0;
                }

                if (appData.readTransferHandle == USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID) {
                    appData.state = APP_STATE_ERROR;
                    break;
                }
            }

            break;

        case APP_STATE_WAIT_FOR_READ_COMPLETE:
        case APP_STATE_CHECK_TIMER:

            if (APP_StateReset()) {
                break;
            }

            /* Check if a character was received or a switch was pressed.
             * The isReadComplete flag gets updated in the CDC event handler. */

            if (appData.isReadComplete || _CP0_GET_COUNT() - startTime > (48000000 / 2 / 100)) {
                appData.state = APP_STATE_SCHEDULE_WRITE;
            }

            break;


        case APP_STATE_SCHEDULE_WRITE:

            if (APP_StateReset()) {
                break;
            }

            /* Setup the write */
           
            appData.writeTransferHandle = USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;
            appData.isWriteComplete = false;
            appData.state = APP_STATE_WAIT_FOR_WRITE_COMPLETE;

            
            readi2c_multiple(ADDR, 0x20,b,14);
            
            //MAF filter 
            
            for(ii=0;ii<=100;ii++){ //set all values of rawData = 0
                rawData[ii]=0;
                MAFfilteredData[ii]=0;
            }
            rawData[i]=zxl;
            
            if(i==0){
            MAFfilteredData[i] = (rawData[i])/avg;
            }
            else if(i==1){
            MAFfilteredData[i] = (rawData[i-1]+rawData[i])/2;
            }
            else if(i==2){
            MAFfilteredData[i] = (rawData[i-2]+rawData[i-1]+rawData[i])/3;
            }
            else{
            MAFfilteredData[i] = (rawData[i-3]+rawData[i-2]+rawData[i-1]+rawData[i])/4;
            }
            
            
            //FIR
            for(k=0;k<=7;k++){
                if(k==0){
                    buffer[0] = rawData[i];
                    //FIRfilteredData[i]=rawData[0]*Bvec[0]+rawData[1]*Bvec[1]+rawData[2]*Bvec[2]+rawData[3]*Bvec[3]+rawData[4]*Bvec[4]+rawData[5]*Bvec[5]+rawData[6]*Bvec[6]+rawData[7]*Bvec[7];
                }
                else if(k==1){
                    buffer[k-1]=buffer[k];
                    buffer[0]=rawData[i];
                    //FIRfilteredData[i]=rawData[0]*Bvec[0]+rawData[1]*Bvec[1]+rawData[2]*Bvec[2]+rawData[3]*Bvec[3]+rawData[4]*Bvec[4]+rawData[5]*Bvec[5]+rawData[6]*Bvec[6]+rawData[7]*Bvec[7];
                }
                if(k==2){
                    buffer[k-1]=buffer[k];
                    buffer[k-2]=buffer[k-1];
                    buffer[0]=rawData[i];
                    //FIRfilteredData[i]=rawData[0]*Bvec[0]+rawData[1]*Bvec[1]+rawData[2]*Bvec[2]+rawData[3]*Bvec[3]+rawData[4]*Bvec[4]+rawData[5]*Bvec[5]+rawData[6]*Bvec[6]+rawData[7]*Bvec[7];
                }
                else if(k==3){
                    buffer[k-1]=buffer[k];
                    buffer[k-2]=buffer[k-1];
                    buffer[k-3]=buffer[k-2];
                    buffer[0]=rawData[i];
                    //FIRfilteredData[i]=rawData[0]*Bvec[0]+rawData[1]*Bvec[1]+rawData[2]*Bvec[2]+rawData[3]*Bvec[3]+rawData[4]*Bvec[4]+rawData[5]*Bvec[5]+rawData[6]*Bvec[6]+rawData[7]*Bvec[7];
                }
                else if(k==4){
                    buffer[k-1]=buffer[k];
                    buffer[k-2]=buffer[k-1];
                    buffer[k-3]=buffer[k-2];
                    buffer[k-4]=buffer[k-3];
                    buffer[0]=rawData[i];
                    //FIRfilteredData[i]=rawData[0]*Bvec[0]+rawData[1]*Bvec[1]+rawData[2]*Bvec[2]+rawData[3]*Bvec[3]+rawData[4]*Bvec[4]+rawData[5]*Bvec[5]+rawData[6]*Bvec[6]+rawData[7]*Bvec[7];
                }
                else if(k==5){
                    buffer[k-1]=buffer[k];
                    buffer[k-2]=buffer[k-1];
                    buffer[k-3]=buffer[k-2];
                    buffer[k-4]=buffer[k-3];
                    buffer[k-5]=buffer[k-4];
                    buffer[0]=rawData[i];
                    //FIRfilteredData[i]=rawData[0]*Bvec[0]+rawData[1]*Bvec[1]+rawData[2]*Bvec[2]+rawData[3]*Bvec[3]+rawData[4]*Bvec[4]+rawData[5]*Bvec[5]+rawData[6]*Bvec[6]+rawData[7]*Bvec[7];
                }
                else if(k==6){
                    buffer[k-1]=buffer[k];
                    buffer[k-2]=buffer[k-1];
                    buffer[k-3]=buffer[k-2];
                    buffer[k-4]=buffer[k-3];
                    buffer[k-5]=buffer[k-4];
                    buffer[k-6]=buffer[k-5];
                    buffer[0]=rawData[i];
                    //FIRfilteredData[i]=rawData[0]*Bvec[0]+rawData[1]*Bvec[1]+rawData[2]*Bvec[2]+rawData[3]*Bvec[3]+rawData[4]*Bvec[4]+rawData[5]*Bvec[5]+rawData[6]*Bvec[6]+rawData[7]*Bvec[7];
                }
                else {
                    buffer[k-1]=buffer[k];
                    buffer[k-2]=buffer[k-1];
                    buffer[k-3]=buffer[k-2];
                    buffer[k-4]=buffer[k-3];
                    buffer[k-5]=buffer[k-4];
                    buffer[k-6]=buffer[k-5];
                    buffer[k-7]=buffer[k-6];
                    buffer[0]=rawData[i];
                    //FIRfilteredData[i]=rawData[0]*Bvec[0]+rawData[1]*Bvec[1]+rawData[2]*Bvec[2]+rawData[3]*Bvec[3]+rawData[4]*Bvec[4]+rawData[5]*Bvec[5]+rawData[6]*Bvec[6]+rawData[7]*Bvec[7];
                }
                }
                    
            
            //IIR
            if(i==0){
                IIRfilteredData[i]=rawData[i];
            }
            else{
                IIRfilteredData[i]=A*IIRfilteredData[i-1]+B*rawData[i];
            }
            
            
            //other loop
            if (appData.isReadComplete) {
                ;
            }
            
            if(set==1 && i<100){
            
            len1 = sprintf(dataOut, "%d       %d       %d       %d       %d       \r\n", i,zxl);
   
            
                USB_DEVICE_CDC_Write(USB_DEVICE_CDC_INDEX_0,
                        &appData.writeTransferHandle, dataOut, len1,
                        USB_DEVICE_CDC_TRANSFER_FLAGS_DATA_COMPLETE);
                i++;
               
                startTime = _CP0_GET_COUNT();
        
            }
            
            else {
                dataOut[0]=0;
                len1 = 1;
                USB_DEVICE_CDC_Write(USB_DEVICE_CDC_INDEX_0,
                        &appData.writeTransferHandle, dataOut, len1,
                        USB_DEVICE_CDC_TRANSFER_FLAGS_DATA_COMPLETE);

                //startTime = _CP0_GET_COUNT();
            
            }
           
            
            
            
            break;
            
        case APP_STATE_WAIT_FOR_WRITE_COMPLETE:

            if (APP_StateReset()) {
                break;
            }

            /* Check if a character was sent. The isWriteComplete
             * flag gets updated in the CDC event handler */

            if (appData.isWriteComplete == true) {
                appData.state = APP_STATE_SCHEDULE_READ;
            }

            break;

        case APP_STATE_ERROR:
            break;
        default:
            break;
    }
}



/*******************************************************************************
 End of File
 */