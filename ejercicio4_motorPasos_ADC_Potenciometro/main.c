#include <msp430.h>

unsigned char secuencia[] = {12, 6, 3, 9};  // 1100 0110 0011 1001 (para mover el motor, los pasos)
unsigned char i = 0;
#define TIMER_INCREMENT 6250                // 50,000us / 8us = 6250 -> 50ms
volatile unsigned short ADC_result = 0;

#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
    TA0CCTL0 &=~CCIFG;                      // Apagamos bandera de interrup, 0 en bit menos significativo
    TA0CCR0 = TAR + ADC_result;

    if((P1IN & (1<<3))!= 0)             // revisamos si estamos presionando el bot�n bot�n est� en P1.3
        P2OUT = secuencia[i++%4];
    else
        P2OUT = secuencia[i--%4];

}

#pragma vector=TIMER0_A1_VECTOR
__interrupt void Timer_A1 (void){
    TA0CCR1 = TAR + TIMER_INCREMENT;  // Restart CCR1 50ms
    ADC10CTL0 |= (1<<0);              // Start of conversion
}

#pragma vector=ADC10_VECTOR
__interrupt void ADC_ISR (void)
{
    ADC_result = ADC10MEM;            //ADC10MEM resultado de la conversion del ADC y se apaga la flag,
                                        //se guarda el restultado en la variable ADC_result
}

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;           // stop watchdog timer
// GPIO
    P2DIR = 0x0F;                       // 0b0000 1111, 15, BIT3 + BIT2 + BIT1 + BIT0  Ponemos los pines como salida

// TIMERS
    // Config timer
    TA0CTL = (2<<8)+(2<<4)+(3<<6);      // Seleccionamos SMCLK,  Modo continuo, divider 8 (cada ciclo toma 8us (microsegs))

    // El del movimiento
    TA0CCTL0 |= (1<<4);                 // Habilitador local para CCR0 la interrupci�n
    TA0CCR0 = TAR + 2500;

    // El de las conversiones de ADC
    TA0CCTL1 |= (1<<4);                 // Habilitador local para CCR1 la interrupci�n
    TA0CCR1 = TAR + TIMER_INCREMENT;    // 50 ms para que empiece la conversi�n

// ADC
    ADC10AE0|=(1<<1);                   // A1 ser� entrada anal�gica
    ADC10CTL1 =INCH_1 ;                 // (1<<12) Mux Anal�gico tome la entrada A1
    ADC10CTL0|= ADC10ON + ADC10IE;      // ADC10ON enciende, ADC10IE habilita interrupci�n del ADC
    ADC10CTL0|=ENC;                     // ENC=1 (enable conversion)

    __bis_SR_register(GIE);             // Global interrupt enable (biS prender biC apagar)

    while(1);
}
