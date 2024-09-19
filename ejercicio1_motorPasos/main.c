#include <msp430.h>

unsigned char secuencia[]={12, 6, 3, 9};  // 1100 0110 0011 1001 (representa el estado de los pines P2.0 a P2.3) (mover stepper)
unsigned char i;

void delay (){
    volatile unsigned long u32Count;
    for (u32Count=0;u32Count<=250;u32Count++);   // Cuenta hasta 500
}

int main(void){
    WDTCTL = WDTPW | WDTHOLD;           // stop watchdog timer

    P2DIR = 0x0F;                       // 0b0000 1111, 15 (decimal), BIT3+BIT2+BIT1+BIT0 (se configuran los pines 0, 1, 2, 3 de Puerto 2)

    while(1){
        if ((P1IN & (1<<3)) !=0){       // Botón de usuario P1.3 (si está presionado secuencia es ascendente)
            P2OUT = secuencia[i++%4];   // %4 hace que el valor siempre esté entre 0 y 3
        } else {                        // secuencia descendente
            P2OUT = secuencia[i--%4];
        }
        delay();
    }
}
