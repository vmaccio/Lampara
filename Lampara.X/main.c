#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include <proc/pic16f886.h>

#pragma config CPD = OFF, BOREN = OFF, IESO = OFF, DEBUG = OFF, FOSC = HS
#pragma config FCMEN = OFF, MCLRE = ON, WDTE = OFF, CP = OFF, LVP = OFF
#pragma config PWRTE = ON, BOR4V = BOR21V, WRT = OFF

#include "i2c-v2.h"

#define _XTAL_FREQ 20000000  // Frecuencia del oscilador externo: 20 MHz

//variables ADC
unsigned short sumaEnvioRuido = 0;
unsigned short sumaSampleResto = 0;
unsigned short resulADClow, resulADChigh;
unsigned short altoRuido = 0;
unsigned short temperatura = 0;
unsigned short humedad = 0;
unsigned short resultado = 0;

//variables i2c
unsigned short luz = 0;
unsigned short lowByteLuz = 0;

unsigned short lowByteCO2 = 0;
unsigned short CO2 = 0;
unsigned short CO2status = 0;

unsigned char UARTcabecera = 170;
unsigned char UARTlongitudMensaje = 0;
unsigned char UARTcomando = 0;
unsigned char UARTdatosRuido = 0;
unsigned short UARTdatosH_T_L_C = 0;
unsigned short UART_CRC = 0;

void putch(char data) {
    while (!TXSTAbits.TRMT);  // Esperar hasta que el buffer esté vacío
    TXREG = data;             // Transmitir el dato
}

void init_uart(void)
{  
    // Configuración del baud rate para 9600 baudios con un oscilador de 20 MHz
    TXSTAbits.BRGH = 0;   // Baja velocidad para el generador de baudios
    BAUDCTLbits.BRG16 = 0; // Generador de baudios de 8 bits

    SPBRG = 32;  // Valor calculado para 9600 baudios con 20 MHz
    
    TXSTAbits.SYNC = 0;  // Modo asíncrono
    TXSTAbits.TX9 = 0;   // Transmisión de 8 bits
    RCSTAbits.RX9 = 0;   // Recepción de 8 bits

    PIE1bits.TXIE = 0;   // Deshabilitar interrupción de transmisión
    PIE1bits.RCIE = 0;   // Deshabilitar interrupción de recepción

    RCSTAbits.SPEN = 1;  // Habilitar puerto serie

    TXSTAbits.TXEN = 0;  // Reiniciar transmisor
    TXSTAbits.TXEN = 1;  // Habilitar transmisor
}

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

void envioRuido(){ //@TODO - tengo que traducir 2 bytes de datos a 1 para pode enviarlo siguiendo el protocolo
    UARTlongitudMensaje = 1;
    UARTcomando = 0;
    UARTdatosRuido = altoRuido; //estoy pasando de 2Byte a 1Byte y eso esta raro
    printf("%do%do%do%do%hu", UARTcabecera, UARTlongitudMensaje, UARTcomando, UARTdatosRuido, UART_CRC);
}

void envioResto(){
    UARTlongitudMensaje = 2;
    
    //UART LUZ
    UARTcomando = 1;
    UARTdatosH_T_L_C = luz;
    printf("%do%do%do%hu%hu", UARTcabecera, UARTlongitudMensaje, UARTcomando, UARTdatosH_T_L_C, UART_CRC);
    
    //UART CO2
    UARTcomando = 2;
    UARTdatosH_T_L_C = CO2;
    printf("%do%do%do%hu%hu", UARTcabecera, UARTlongitudMensaje, UARTcomando, UARTdatosH_T_L_C, UART_CRC);
    
    //UART humedad
    UARTcomando = 3;
    UARTdatosH_T_L_C = humedad;
    printf("%do%do%do%hu%hu", UARTcabecera, UARTlongitudMensaje, UARTcomando, UARTdatosH_T_L_C, UART_CRC);
    
    //UART CO2
    UARTcomando = 2;
    UARTdatosH_T_L_C = CO2;
    printf("%do%do%do%hu%hu", UARTcabecera, UARTlongitudMensaje, UARTcomando, UARTdatosH_T_L_C, UART_CRC);
    
    //UART TEMPERATURA
    //@TODO - tengo que traducir 2 bytes de datos a 1 para pode enviarlo siguiendo el protocolo
    UARTcomando = 6;
    UARTlongitudMensaje = 1;
    UARTdatosH_T_L_C = temperatura;
    printf("%do%do%do%hu%hu", UARTcabecera, UARTlongitudMensaje, UARTcomando, UARTdatosH_T_L_C, UART_CRC);
}

void sampleResto(){
    //************** TEMPERATURA ********************************
    ADCON0bits.CHS = 0b01;
    ADCON0bits.GO_nDONE = 1; //falta sincronizacion creo
    temperatura = resultado;
    
    //************** HUMEDAD ********************************
    ADCON0bits.CHS = 0b10;
    ADCON0bits.GO_nDONE = 1; //falta sincronizacion creo
    humedad = resultado;
    
    //************** LUZ ********************************
    //adress = 0010001
        //SSPCONbits.SSPM = 0b1000; //activo modo master
        //SSPCONbits.SSPEN = 1; //configura los pines
    i2c_start(); //inicio i2c
    while(!i2c_write(17)); //envio direccion del sensor
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
    while(!i2c_write(181));
    lowByteCO2 =i2c_read(0); //leo parte baja del sensor. el 0 indica envio de ack
    CO2 =i2c_read(0); //leo parte alta del sensor. el 0 indica envio de ack
    CO2status =i2c_read(1); //leo estado del sensor. el 1 indica envio de ack
    i2c_stop(); //paro i2c
    if(CO2status == 0){
        CO2 = CO2 << 8; //desplaza high byte a la izquierda
        CO2 = CO2 | lowByteCO2; //añade low byte al resultado
    } else{
        CO2 = 65500;
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
            envioResto();
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
    init_uart();
    return;
}
