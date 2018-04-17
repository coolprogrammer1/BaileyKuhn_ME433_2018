#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include <math.h>

/*
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

*/

#define CS LATAbits.LATBA0       // chip select pin

char SPI1_IO(char write){ //function that does generic communication
    SPI1BUF = write;
    while(!SPI1STATbits.SPIRBF) {
       ; //wait to receive the byte
    }
     return SPI1BUF;    
} 

void setVoltage(char channel, int voltage){
}


void initSPI1() { //initialization function
    //set up the chip select pin as an output
    //set CS high
    TRISAbits.TRISA0 = 0;
    CS = 1;
    
    RPA1Rbits.RPA1R = 0011; //set pin A1 as SDO pin
    
    //Master - SPI1 - pins are SDI (A1), SCK1 (B15)
    //we manually control CS (A0) as a digital output
    //since the PIC is just starting, we know that SPI is off. We rely on
    //defaults here
    
    //setup SPI1
    SPI1CON = 0;              // turn off the spi module and reset it
    SPI1BUF;                  // clear the rx buffer by reading from it
    SPI1BRG = 0x3;            // baud rate to 10 MHz [SPI4BRG = (80000000/(2*desired))-1]
    SPI1STATbits.SPIROV = 0;  // clear the overflow bit
    SPI1CONbits.CKE = 1;      // data changes when clock goes from hi to lo (since CKP is 0)
    SPI1CONbits.MSTEN = 1;    // master operation
    SPI1CONbits.ON = 1;       // turn on spi 1

                            // send a ram set status command.
    CS = 0;                   // enable the ram
    SPI1_IO(0x01);             // ram write status
    SPI1_IO(0x41);             // sequential mode (mode = 0b01), hold disabled (hold = 0)
    CS = 1;                   // finish the command
}

int main() {
    initSPI1();             //initialization function
    
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

    while(1) {
	// use _CP0_SET_COUNT(0) and _CP0_GET_COUNT() to test the PIC timing
        // remember the core timer runs at half the sysclk
        
        _CP0_SET_COUNT(0);
        //add code
        setVoltage(1,512);
        setVoltage(0,512/2);
        
        while(_CP0_GET_COUNT() < 12000) { // (5E-3)/(1/24E6) is # core ticks
            ;
        }         
    }
}
                
        

