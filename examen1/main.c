#include <msp430.h>

unsigned char secuencia[] = {12, 6, 3, 9};  // 1100 0110 0011 1001 (para mover el motor, los pasos)
unsigned char i_derecha = 0;
unsigned char i_izquierda = 0;
unsigned short speed_derecha = 0;
unsigned short speed_izquierda = 0;
unsigned short x = 0;
unsigned short y = 0;
volatile unsigned short ADC_samples[2];     // x = ADC_samples[0];     y = ADC_samples[1];

#pragma vector=ADC10_VECTOR
__interrupt void ADC_ISR (void)
{
    x = ADC_samples[0];
    y = ADC_samples[1];
    ADC10CTL0 &= ~ADC10IFG;             // Limpiar la bandera de interrupción
    ADC10SA = (unsigned short)&ADC_samples;// Reiniciar el DTC
}

// Llanta derecha
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
    TA0CCTL0 &= ~CCIFG;                 // Apagamos bandera de interrup, 0 en bit menos significativo

    // Comparar valores de joystick y que las llantas se muevan acorde
    // No moverse x = 512, y = 512
    if (x == 512 & y == 512){
        // detener llanta derecha
        speed_derecha = 0;
    } else if (x < 512) {
        // Disminuir velocidad de la llanta derecha
        speed_derecha = (512 - x) / 4;  // Ajuste de velocidad basado en x
    }

    TA0CCR0 += speed_derecha;

    P2OUT = secuencia[i_derecha++%4];           // Mover motor llanta derecha

}

#pragma vector=TIMER0_A1_VECTOR
__interrupt void Timer_A1 (void){
    TA0CCTL1 &= ~CCIFG;                 // Apagamos bandera

    // Comparar valores de joystick y que las llantas se muevan acorde
    // No moverse x = 512, y = 512
    if (x == 512 & y == 512){
            // detener llanta izquierda
            speed_izquierda = 0;
    } else if (x < 512) {
        // Aumentar velocidad de la llanta izquierda (máx 180 grados)
        speed_izquierda = (512 - x) / 4;  // Ajuste de velocidad basado en x
    } else if (x > 512) {
        // Disminuir velocidad de la llanta izquierda
        speed_izquierda = (x - 512) / 4;  // Ajuste de velocidad basado en x
    }

    TA0CCR1 += speed_izquierda;
    P1OUT = (P1OUT & 0xC3) | (secuencia[i_izquierda++ % 4] << 2); // Mover motor llanta izquierda
                                        // (deja los otros pines como estaban y recorre para que sea con pines correctos)
}


int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;           // stop watchdog timer
// GPIO
    // Llanta derecha
    P2DIR = BIT5 + BIT4 + BIT3 + BIT2;  // Ponemos los pines como salida: P2.5, 2.4, 2.3, 2.2
    // Llanta izquierda
    P1DIR = BIT7 + BIT6 + BIT5 + BIT4;  // Pines en salida: P1.7, 1.6, 1.5, 1.6

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
    ADC10AE0|= BIT0 + BIT1;             // habilita el pin P1.1 y P1.0 como entrada analógica (A1 y A0)
    ADC10CTL1 =INCH_3 + CONSEQ_3;       // A3 primer canal, que repetir secuencia de canales
    ADC10CTL0|= ADC10ON + ADC10IE + MSC;// ADC10ON enciende, ADC10IE habilita interrupdel ADC, Multiple Sample Conver, Start Conv
    ADC10DTC1 = 0x02;                   // 2 conversions
    ADC10SA = (unsigned short)&ADC_samples;// Start address para las conversiones
    ADC10CTL0|=ENC + ADC10SC;           // ENC=1 (enable conversion)

    __bis_SR_register(GIE);             // Global interrupt enable (biS prender biC apagar)

    while(1);
}
