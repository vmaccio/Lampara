#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include <proc/pic16f886.h>
#pragma config CPD = OFF, BOREN = OFF, IESO = OFF, DEBUG = OFF, FOSC = HS
#pragma config FCMEN = OFF, MCLRE = ON, WDTE = OFF, CP = OFF, LVP = OFF
#pragma config PWRTE = ON, BOR4V = BOR21V, WRT = OFF

#include "i2c.h"

int leerLuz (){
    int lowByteLuz = 0;
    int luz = 0;
    
    SSPCONbits.SSPM = 0b1000; //activo modo master
    SSPCONbits.SSPEN = 1; //configura los pines
    SSPCON2bits.SEN = 1; //start condition
    SSPBUF = 0b00100001; //direccion de sensor con ultimo bit read
    while(SSPCON2bits.ACKSTAT != 0); //no avanza hasta recibir ACK
    SSPCON2bits.RCEN = 1; //modo recepcion de datos
    while(SSPSTATbits.BF != 1); //espera a llenar el buffer
    lowByteLuz = SSPBUF; //vacia buffer
    SSPCON2bits.ACKDT = 0; //configura ACK
    SSPCON2bits.ACKEN = 1; //envia ACK
    while(SSPSTATbits.BF != 1); //espera a llenar buffer
    luz = SSPBUF; //vacia buffer
    luz = luz << 8; //desplaza high byte a la izquierda
    luz = luz | lowByteLuz; //añade low byte al resultado
    SSPCON2bits.ACKDT = 1; //configuracion ACK
    SSPCON2bits.ACKEN = 1; //envio ACK
    SSPCON2bits.PEN = 1; //stop condition
    return luz;
}

//@TODO - retornar la variable status de alguna manera
int leerCo2(){
    int lowByteCO2 = 0;
    int CO2 = 0;
    int CO2status= 0;
    
    SSPCONbits.SSPM = 0b1000; //activo modo master
    SSPCONbits.SSPEN = 1; //configura los pines
    SSPCON2bits.SEN = 1; //start condition
    SSPBUF = 0b10110101; //direccion de sensor con ultimo bit read
    while(SSPCON2bits.ACKSTAT != 0); //no avanza hasta recibir ACK
    SSPCON2bits.RCEN = 1; //modo recepcion de datos
    while(SSPSTATbits.BF != 1); //espera a llenar el buffer
    lowByteCO2 = SSPBUF; //vacia buffer
    SSPCON2bits.ACKDT = 0; //configura ACK
    SSPCON2bits.ACKEN = 1; //envia ACK
    while(SSPSTATbits.BF != 1); //espera a llenar buffer
    CO2 = SSPBUF; //vacia buffer
    CO2 = CO2 << 8; //desplaza high byte a la izquierda
    CO2 = CO2 | lowByteCO2; //añade low byte al resultado
    SSPCON2bits.ACKDT = 0; //configuracion ACK
    SSPCON2bits.ACKEN = 1; //envio ACK
    while(SSPSTATbits.BF != 1); //espera a llenar buffer
    CO2status = SSPBUF; //vacia buffer
    SSPCON2bits.ACKDT = 0; //configuracion ACK
    SSPCON2bits.ACKEN = 1; //envio ACK
    SSPCON2bits.PEN = 1; //stop condition
    return CO2;
}