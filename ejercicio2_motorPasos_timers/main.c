#include <msp430.h>

unsigned char secuencia[]={12, 6, 3, 9};  // 1100 0110 0011 1001 (representa el estado de los pines P2.0 a P2.3) (mover stepper)

void delay (unsigned short tiempo){
    unsigned short i;

    for (i=0;i<=tiempo;i++){
        TA0CCR0=TAR+10;                     // Leo que tiempo tiene y luego sumo 10 para que sea decena de microsegundo
        while ((TA0CCTL0 & 1) == 0);        // Espera hasta que la bandera de comparación (bit 0) se active, indicando que se alcanzó el tiempo configurado en TA0CCR0
                                            // Es decir, mientras la bandera no prenda (aún no sea la decena de microsegundo) aqui se queda
        TA0CCTL0 &= ~(1<<0);                // apago bandera (~esto es negador o sea 0000 0001 -> queda 1111 1110 mascara
    }
}

int main(void){
    unsigned char i;
    WDTCTL = WDTPW | WDTHOLD;           // stop watchdog timer

    TA0CTL = (2<<8)+(2<<4);             // Seleccionamos SMCLK, Modo continuo

    P2DIR = 0x0F;                       // 0b0000 1111, BIT3+BIT2+BIT1+BIT0 (se configuran los pines 0, 1, 2, 3 de Puerto 2)



    while(1){
        if ((P1IN & (1<<3)) !=0){       // Botón de usuario P1.3 (si está presionado secuencia es ascendente)
            P2OUT = secuencia[i++%4];   // %4 hace que el valor siempre esté entre 0 y 3
        } else {                        // secuencia descendente
            P2OUT = secuencia[i--%4];
        }
        delay(100);
    }
}
