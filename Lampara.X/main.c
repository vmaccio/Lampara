#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include <proc/pic16f886.h>

#pragma config CPD = OFF, BOREN = OFF, IESO = OFF, DEBUG = OFF, FOSC = HS
#pragma config FCMEN = OFF, MCLRE = ON, WDTE = OFF, CP = OFF, LVP = OFF
#pragma config PWRTE = ON, BOR4V = BOR21V, WRT = OFF

#include <xc.h>
#include "i2c-v2.h"

//variables ADC
int sumaEnvioRuido = 0;
int sumaSampleResto = 0;
int resulADClow, resulADChigh;
int altoRuido = 0;
int temperatura = 0;
int humedad = 0;
int resultado = 0;

//variables i2c
int luz = 0;
int lowByteLuz = 0;

int lowByteCO2 = 0;
int CO2 = 0;
int CO2status = 0;


void init_ADC(){
    PIE1bits.ADIE = 0;
    ADCON0bits.ADCS = 0b10; //A/D conversion clock
    ADCON1bits.VCFG0 = 0;
    ADCON1bits.VCFG1 = 0;
    ADCON0bits.CHS = 0b00; //Varia
    ADCON1bits.ADFM = 1;
    ADCON0bits.ADON = 1;
}

void init_timer0(){
    INTCONbits.T0IE = 1;
    OPTION_REGbits.T0CS = 0;
    OPTION_REGbits.PSA = 0;
    OPTION_REGbits.PS = 0b111;
    TMR0 = 60; //10ms (puede estar mal)
}

void sampleRuido(){
    ADCON0bits.CHS = 0b00;
    ADCON0bits.GO_nDONE = 1;
    if(resultado > altoRuido){
        altoRuido = resultado;
    }
}

void envioRuido(){
    
}

void sampleResto(){
    //************** TEMPERATURA ********************************
    ADCON0bits.CHS = 0b01;
    ADCON0bits.GO_nDONE = 1;
    temperatura = resultado;
    
    //************** HUMEDAD ********************************
    ADCON0bits.CHS = 0b10;
    ADCON0bits.GO_nDONE = 1;
    humedad = resultado;
    
    //************** LUZ ********************************
    //adress = 0010001
        //SSPCONbits.SSPM = 0b1000; //activo modo master
        //SSPCONbits.SSPEN = 1; //configura los pines
    i2c_start(); //inicio i2c
    while(!i2c_write("0010001")); //envio direccion del sensor
    //no estoy seguro del while este pero creo que funciona
    lowByteLuz =i2c_read(0); //leo parte baja del sensor. el 0 indica envio de ack
    luz =i2c_read(1); //leo parte alta sensor. el 1 indica NO envio ack
    i2c_stop(); //paro i2c
    luz = luz << 8; //desplaza high byte a la izquierda
    luz = luz | lowByteLuz; //añade low byte al resultado
    
    //************** CO2 ********************************
    //adress = 10110101
        //SSPCONbits.SSPM = 0b1000; //activo modo master
        //SSPCONbits.SSPEN = 1; //configura los pines
    i2c_start();
    while(!i2c_write("10110101"));
    lowByteCO2 =i2c_read(0); //leo parte baja del sensor. el 0 indica envio de ack
    CO2 =i2c_read(0); //leo parte alta del sensor. el 0 indica envio de ack
    CO2status =i2c_read(1); //leo estado del sensor. el 1 indica envio de ack
    i2c_stop(); //paro i2c
    if(CO2status == 0){
        CO2 = CO2 << 8; //desplaza high byte a la izquierda
        CO2 = CO2 | lowByteCO2; //añade low byte al resultado
    } else{
        CO2 = -1;
    }
}

void __interrupt()   INT_CONTROLADO(void)
{
    if (INTCONbits.T0IF == 1){
        TMR0 = 60;
        sampleRuido();
        sumaEnvioRuido ++;
        if (sumaEnvioRuido == 100){
            envioRuido();
            sumaSampleResto ++;
            sumaEnvioRuido = 0;
            altoRuido = 0;
        }
        if (sumaSampleResto == 5){
            sampleResto();
            sumaSampleResto = 0;
        }
        INTCONbits.T0IF = 0;
    }
    
    if(PIR1bits.ADIF){
        PIR1bits.ADIF = 0;
        resulADClow = ADRESL;
        resulADChigh = ADRESH;
        resultado = resulADChigh << 8;
        resultado |= resulADClow;
    }
}

void main(void) {
    INTCONbits.GIE = 1;
    init_ADC();
    init_timer0();
    return;
}
