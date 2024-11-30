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

unsigned short sumaEnvioRuido = 0;
unsigned short sumaSampleResto = 0;

 
void __interrupt()   INT_CONTROLADO(void)
{
    if (INTCONbits.T0IF == 1){
        TMR0 = 217;
        printf("Sample Ruido");
        sumaEnvioRuido ++;
        if (sumaEnvioRuido == 100){
            printf("Envio Ruido");
            sumaSampleResto ++;
            sumaEnvioRuido = 0;
            printf("Reinicio max ruido");
        }
        if (sumaSampleResto == 5){
            printf("Sample Resto");
            printf("Envio Resto");
            sumaSampleResto = 0;
        }
        INTCONbits.T0IF = 0;
    }
}

void init_timer0(){
    INTCONbits.T0IE = 1;
    OPTION_REGbits.T0CS = 0;
    OPTION_REGbits.PSA = 0;
    OPTION_REGbits.PS = 0b111;
    TMR0 = 217; //10ms (puede estar mal)
}

void main(void) {
    init_timer0();
    INTCONbits.GIE = 1;  // habilita interrupciones de forma general
    INTCONbits.PEIE = 1; // habilita interrupciones de los periféricos
    while(1);
    return;
}