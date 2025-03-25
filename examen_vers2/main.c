#include <msp430.h>

volatile unsigned short x, y;
volatile unsigned char chanel=4;
unsigned char secuencia[] = {12, 6, 3, 9};  // 1100 0110 0011 1001 (para mover el motor, los pasos)
unsigned char i_derecha = 0;
unsigned char i_izquierda = 0;
unsigned short speed_derecha = 5000;
unsigned short speed_izquierda = 5000;

#pragma vector=ADC10_VECTOR
__interrupt void ADC_ISR (void)
{
    ADC10CTL0 &= ~ENC;              // Desactivar ENC antes de cambiar el canal
    if(chanel == 4){
        x = ADC10MEM;           // Almacenar el resultado de la conversión en x
        ADC10CTL1 = INCH_5;         // Mux (multiplexor) Analógico tome entrada A5
        chanel--;
    } else {
        y = ADC10MEM;           //  Almacenar el resultado de la conversión en y
        ADC10CTL1 = INCH_4;         // Mux (multiplexor) Analógico tome entrada A4
        chanel++;
    }

    ADC10CTL0 |= ENC;               // Reactivar ENC después de cambiar el canal
    ADC10CTL0 |= ADC10SC;           // Empieza conversión
}

// Llanta izquierda
#pragma vector=TIMER0_A1_VECTOR
__interrupt void Timer_A1 (void){
    TA0CCTL1 &= ~CCIFG;                 // Apagamos bandera

    /*
    // Comparar valores de joystick y que las llantas se muevan acorde
    // No moverse x = 714, y = 723
    if (x == 714 && y == 723){
        speed_izquierda = 0;
    } else if (y > 723) {
        // Aumenta velocidad en llanta izquierda si el joystick se mueve hacia adelante
        speed_izquierda = (y - 723) * 3;
    } else if (y < 723) {
        // Disminuye velocidad en llanta izquierda si el joystick se mueve hacia atrás
        speed_izquierda = (723 - y) * 3;
    }
    */
    TA0CCR1 += 5000;
    P1OUT = secuencia[i_izquierda++ % 4]; // Mover motor llanta izquierda
}

// Llanta derecha
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
    TA0CCTL0 &= ~CCIFG;                 // Apagamos bandera de interrup, 0 en bit menos significativo

    /*
    // Comparar valores de joystick y que las llantas se muevan acorde
    // No moverse x = 714, y = 723
    if (x == 714 && y == 723){
        speed_derecha = 0;
    } else if (y > 723) {
        // Aumenta velocidad en llanta derecha si el joystick se mueve hacia adelante
        speed_derecha = (y - 723) * 3;
    } else if (y < 723) {
        // Disminuye velocidad en llanta derecha si el joystick se mueve hacia atrás
        speed_derecha = (723 - y) * 3;
    }
    */

    TA0CCR0 += 5000;
    P2OUT = secuencia[i_derecha++%4];           // Mover motor llanta derecha
}

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;       // stop watchdog timer
// GPIO
    // Llanta izquierda
    P1DIR = BIT5 + BIT4 + BIT1 + BIT0;  // Pines en salida: P1.5, 1.4, 1.1, 1.0
    // Llanta derecha
    P2DIR = BIT3 + BIT2 + BIT1 + BIT0;  // Ponemos los pines como salida: P2.3, 2.2, 2.1, 2.0

// TIMERS
    // Config timer
    TA0CTL = TASSEL_2 +MC_2;             // Seleccionamos SMCLK,  Modo continuo

    // Llanta derecha
    TA0CCTL0 |= (1<<4);                 // Habilitador local para CCR0 la interrupción
    TA0CCR0 = TAR + 2000;

    // Llanta izquierda
    TA0CCTL1 |= (1<<4);                 // Habilitador local para CCR1 la interrupción
    TA0CCR1 = TAR + 2000;

// ADC
    ADC10AE0 |= BIT4 + BIT5;        // P1.4 y P1.5 entrada analógica A4 y A5
    ADC10CTL1 = INCH_4;             // Mux (multiplexor) Analógico tome entrada A4
    ADC10CTL0 |= ADC10ON + ADC10IE; // ADC10 on y ADC10IE (Prender ADC y Enable interrupciones locales del ADC)
    ADC10CTL0 |= ENC;               // Enc = 1

    __bis_SR_register(GIE);         // Globar interrup enable (biS prender biC apagar)

    ADC10CTL0 |= ADC10SC;           // Empieza conversión

    while(1);
}
