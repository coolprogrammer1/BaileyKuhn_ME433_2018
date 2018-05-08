#include<xc.h>
#include "i2c_master_noint.h"

// I2C Master utilities, 100 kHz, using polling rather than interrupts
// The functions must be callled in the correct order as per the I2C protocol
// Change I2C1 to the I2C channel you are using
// I2C pins need pull-up resistors, 2k-10k


unsigned char r;
#define ADDR 0b1101011


void i2c_master_setup(void) {
  I2C2BRG = 53;            // I2CBRG = [1/(2*Fsck) - PGD]*Pblck - 2
                            //Fsck = 400kHz, PGD  100 ns, Pbclk - 48MHz
                                    // look up PGD for your PIC32
  I2C2CONbits.ON = 1;               // turn on the I2C1 module
}

// Start a transmission on the I2C bus
void i2c_master_start(void) {
    I2C2CONbits.SEN = 1;            // send the start bit
    while(I2C2CONbits.SEN) { ; }    // wait for the start bit to be sent
}

void i2c_master_restart(void) {
    I2C2CONbits.RSEN = 1;           // send a restart
    while(I2C2CONbits.RSEN) { ; }   // wait for the restart to clear
}

void i2c_master_send(unsigned char byte) { // send a byte to slave
  I2C2TRN = byte;                   // if an address, bit 0 = 0 for write, 1 for read
  while(I2C2STATbits.TRSTAT) { ; }  // wait for the transmission to finish
  if(I2C2STATbits.ACKSTAT) {        // if this is high, slave has not acknowledged
    // ("I2C2 Master: failed to receive ACK\r\n");
  }
}

unsigned char i2c_master_recv(void) { // receive a byte from the slave
    I2C2CONbits.RCEN = 1;             // start receiving data
    while(!I2C2STATbits.RBF) { ; }    // wait to receive the data
    return I2C2RCV;                   // read and return the data
}

void i2c_master_ack(int val) {        // sends ACK = 0 (slave should send another byte)
                                      // or NACK = 1 (no more bytes requested from slave)
    I2C2CONbits.ACKDT = val;          // store ACK/NACK in ACKDT
    I2C2CONbits.ACKEN = 1;            // send ACKDT
    while(I2C2CONbits.ACKEN) { ; }    // wait for ACK/NACK to be sent
}

void i2c_master_stop(void) {          // send a STOP:
  I2C2CONbits.PEN = 1;                // comm is complete and master relinquishes bus
  while(I2C2CONbits.PEN) { ; }        // wait for STOP to complete
}


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
signed short temp,xg, yg, zg, xxl, yxl, zxl, xxl2,yxl2,zxl2;
    
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
    
    xxl2 = -1.0*((xxl)/16000.0)*60.0;
    yxl2 = ((yxl)/16000.0)*60.0;
    zxl2 = ((zxl)/16000.0)*60.0;
}