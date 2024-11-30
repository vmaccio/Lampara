/*
 * File:   pruebaTimer0.c
 * Author: valentin
 *
 * Created on 27 de noviembre de 2024, 16:05
 * 
 * Objetivo de la prueba : comprobar si timer 0 cumple con las plazos esperados
 *  de interrupcion para recoger y enviar datos
 * 
 * Forma de realizarla : mediante la herramienta stopwatch de MPLAB y usando 
 *  breakpoints podemos comprobar el tiempo entre cada parada de ejecucion.
 *  Esto nos permite comprobar cada cuanto interrumpe timer0.
 *  Serian necesarios breakpoint en las lineas 35, 40 y 46. Hay que ejecutarlos
 *  de manera individual para medir bien el tiempo.
 * 
 * Resultado esperado : Para cada breakpoint se espera un tiempo diferente por
 *  por lo tanto para el de la linea 35 el tiempo entre paradas deberia ser 
 *  10ms, para la linea 40 deberia ser 1s y para la linea 46 deberia ser 5s.
 *  Si esto se cumple podemos asegurar que la variable TMR0 esta en el valor
 *  correcto.
 */

#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include <proc/pic16f886.h>

#pragma config CPD = OFF, BOREN = OFF, IESO = OFF, DEBUG = OFF, FOSC = HS
#pragma config FCMEN = OFF, MCLRE = ON, WDTE = OFF, CP = OFF, LVP = OFF
#pragma config PWRTE = ON, BOR4V = BOR21V, WRT = OFF

#define _XTAL_FREQ 20000000  // Frecuencia del oscilador externo: 20 MHz

unsigned short altoRuido = 0;
unsigned short resultado = 0;
unsigned short temperatura = 0;
unsigned short humedad = 0;
unsigned short resulADClow, resulADChigh;

void init_PORTA() {
    TRISAbits.TRISA0 = 1;
    TRISAbits.TRISA1 = 1;
    TRISAbits.TRISA2 = 1;
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

void sampleTodos(){
    //************** RUIDO ********************************
    ADCON0bits.CHS = 0b00;
    ADCON0bits.GO_nDONE = 1;
    altoRuido = resultado;
    
    //************** TEMPERATURA ********************************
    ADCON0bits.CHS = 0b01;
    ADCON0bits.GO_nDONE = 1;
    temperatura = resultado;
    
    //************** HUMEDAD ********************************
    ADCON0bits.CHS = 0b10;
    ADCON0bits.GO_nDONE = 1;
    humedad = resultado;
}

void __interrupt()   INT_CONTROLADO(void)
{
    if(PIR1bits.ADIF){
        PIR1bits.ADIF = 0;
        resulADClow = ADRESL;
        resulADChigh = ADRESH;
        resultado = resulADChigh << 8;
        resultado |= resulADClow;
    }
}

void main(void) {
    INTCONbits.GIE = 1;  // habilita interrupciones de forma general
    INTCONbits.PEIE = 1; // habilita interrupciones de los periféricos
    init_ADC();
    init_PORTA();
    sampleTodos();
    printf("Ruido: %hu /Temperatura: %hu /Humedad: %hu", altoRuido, temperatura, humedad);
    
    return;
}