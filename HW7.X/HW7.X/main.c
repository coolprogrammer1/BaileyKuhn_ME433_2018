#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include "i2c_master_noint.h"
#include "ST7735.h"
#include<stdio.h>

unsigned char r;
#define ADDR 0b1101011

// DEVCFG0
#pragma config DEBUG = OFF // no debugging
#pragma config JTAGEN = OFF // no jtag
#pragma config ICESEL = ICS_PGx1 // use PGED1 and PGEC1
#pragma config PWP = OFF // no write protect
#pragma config BWP = OFF // no boot write protect
#pragma config CP = OFF // no code protect

// DEVCFG1
#pragma config FNOSC = PRIPLL // use primary oscillator with pll
#pragma config FSOSCEN = OFF // turn off secondary oscillator
#pragma config IESO = OFF // no switching clocks
#pragma config POSCMOD = HS // high speed crystal mode
#pragma config OSCIOFNC = OFF // disable secondary osc
#pragma config FPBDIV = DIV_1 // divide sysclk freq by 1 for peripheral bus clock
#pragma config FCKSM = CSDCMD // do not enable clock switch
#pragma config WDTPS = PS1048576 // use slowest wdt - 1048576 ms?
#pragma config WINDIS = OFF // wdt no window mode
#pragma config FWDTEN = OFF // wdt disabled
#pragma config FWDTWINSZ = WINSZ_25 // wdt window at 25%

// DEVCFG2 - get the sysclk clock to 48MHz from the 8MHz crystal
#pragma config FPLLIDIV = DIV_2 // divide input clock to be in range 4-5MHz (8->4)
#pragma config FPLLMUL = MUL_24	 // multiply clock after FPLLIDIV (4->96)
#pragma config FPLLODIV = DIV_2 // divide clock after FPLLMUL to get 48MHz (96->48)
#pragma config UPLLIDIV = DIV_2 // divider for the 8MHz input clock, then multiplied by 12 to get 48MHz for USB
#pragma config UPLLEN = ON // USB clock on

// DEVCFG3
#pragma config USERID = 00000000 // some 16bit userid, doesn't matter what
#pragma config PMDL1WAY = OFF // allow multiple reconfigurations
#pragma config IOL1WAY = OFF // allow multiple reconfigurations
#pragma config FUSBIDIO = ON // USB pins controlled by USB module
#pragma config FVBUSONIO = ON // USB BUSON controlled by USB module

void writei2c(unsigned char reg, unsigned char val){
    i2c_master_start();         // make the start bit
    i2c_master_send(ADDR<<1|0);    // write the address, shifted left by 1, or'ed with a 0 to indicate writing
    i2c_master_send(reg);         // the register to write to
    i2c_master_send(val);       // the value to put in the register
    i2c_master_stop();      // make the stop bit
}

void initExpander(){
    ANSELBbits.ANSB2 = 0;          //making analog pins digital
    ANSELBbits.ANSB3 = 0;
    i2c_master_setup();             //turns on i2c pins
}

unsigned char readi2c(unsigned char address, unsigned char reg){
    i2c_master_start(); // make the start bit
    i2c_master_send(address<<1|0); // write the address, shifted left by 1, or'ed with a 0 to indicate writing
    i2c_master_send(reg); // the register to read from
    i2c_master_restart(); // make the restart bit
    i2c_master_send(address<<1|1); // write the address, shifted left by 1, or'ed with a 1 to indicate reading
    r = i2c_master_recv(); // save the value returned
    i2c_master_ack(1); // make the ack so the slave knows we got it
    i2c_master_stop(); // make the stop bit
    
    return r;
}

unsigned char b[14];
signed short temp,xg, yg, zg, xxl, yxl, zxl;
    
unsigned char readi2c_multiple(unsigned char address, unsigned char reg, unsigned char * data, int length){
    i2c_master_start(); // make the start bit
    i2c_master_send(address<<1|0); // write the address, shifted left by 1, or'ed with a 0 to indicate writing
    i2c_master_send(reg); // the register to read from
    i2c_master_restart(); // make the restart bit
    i2c_master_send(address<<1|1); // write the address, shifted left by 1, or'ed with a 1 to indicate reading
    int i;
    for(i = 0; i <length; i++){
        b[i] = i2c_master_recv();
        if(i<length-1){
        i2c_master_ack(0);
        }
        else {
        i2c_master_ack(1);
        }
    }
    i2c_master_stop(); // make the stop bit
    temp = b[0] | (b[1] << 8);
    xg = b[2] | (b[3] << 8);
    yg = b[4] | (b[5] << 8);
    zg = b[6] | (b[7] << 8);
    xxl = b[8] | (b[9] << 8);
    yxl = b[10] | (b[11] << 8);
    zxl = b[12] | (b[13] << 8);
}

int main() {

    __builtin_disable_interrupts();

    // set the CP0 CONFIG register to indicate that kseg0 is cacheable (0x3)
    __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);

    // 0 data RAM access wait states
    BMXCONbits.BMXWSDRM = 0x0;

    // enable multi vector interrupts
    INTCONbits.MVEC = 0x1;

    // disable JTAG to get pins back
    DDPCONbits.JTAGEN = 0;

    // do your TRIS and LAT commands here
    TRISBbits.TRISB4 = 1; //make pushbutton pin an input pin
   
    TRISAbits.TRISA4 = 0; //make LED pin an output
    LATAbits.LATA4 = 0; //make LED pin low to start
    
    __builtin_enable_interrupts();
    SPI1_init();
    LCD_init();
    LCD_clearScreen(GREEN);
    
    
    initExpander();
    //write to several registers to initialize chip
    writei2c(0x10,0b10000010);  //CTRL1_XL turn on accelerometer, 1.66 kHz 2g, 100Hz
    writei2c(0x11,0b10001000);  //CTRL2_g turn on gyroscope, 1.66 kHz, 1000 dps sensitivity
    writei2c(0x12,0b00000100);  //CTRL3_C make sure IF_INC bit is 1
    
    
    
    while(1) {
	// use _CP0_SET_COUNT(0) and _CP0_GET_COUNT() to test the PIC timing
        // remember the core timer runs at half the sysclk

            /*
            writei2c(0x0A, 0b00000001);
                
            _CP0_SET_COUNT(0);
            LATAbits.LATA4 = 1; //make LED pin high
            while(_CP0_GET_COUNT() < 1200000) { // (2E-3)/(1/24E6) is # core ticks
            ;}
            
            _CP0_SET_COUNT(0);
            LATAbits.LATA4 = 0; //make LED pin low (off)  
            while(_CP0_GET_COUNT() < 1200000) { // wait 5 ms again
            ;}
*/      
        
        _CP0_SET_COUNT(0);
        
        
         LATAbits.LATA4 = 1; //make LED pin high
         while(_CP0_GET_COUNT() < 1200000) { // (2E-3)/(1/24E6) is # core ticks
            ;}
         
         _CP0_SET_COUNT(0);
         LATAbits.LATA4 = 0; //make LED pin low (off)  
         while(_CP0_GET_COUNT() < 1200000) { // wait 5 ms again
            ;}
        /*
        r = readi2c(ADDR, 0x0F);
        char message[30];
        sprintf(message,"r=%d",r);
        LCD_drawString(28,32, message, RED, BLUE);
         */
         
         readi2c_multiple(ADDR, 0x20,b,14);
         char tempmes[30];
         char xxlmes[30];
         char yxlmes[30];
         sprintf(tempmes,"temp=%d",temp);
         sprintf(xxlmes,"xxl=%d",xxl);
         sprintf(yxlmes,"yxl=%d",yxl);
         
         /*
         char xgmes[30];
         char ygmes[30];
         char zgmes[30];
         char zxlmes[30];
          */
         
         /*
         sprintf(xgmes,"xg=%d",xg);
         sprintf(ygmes,"yg=%d",yg);
         sprintf(zgmes,"zg=%d",zg);   
         sprintf(zxlmes,"zxl=%d",zxl);
         */
         
         
         LCD_drawString(28,5,tempmes,RED,BLUE);
         LCD_drawString(28,13,xxlmes,RED,BLUE);
         LCD_drawString(28,21,yxlmes,RED,BLUE);
    
         LCD_drawProgressBar(64,80,xxl,CYAN,MAGENTA);
    }
}
