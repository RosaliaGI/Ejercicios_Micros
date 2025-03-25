#include <msp430.h>
unsigned char dato;


#pragma vector=USCIAB0RX_VECTOR
__interrupt void UART_Rx (void)
{
    temp = UCA0RXBUF; // TX -> RXed character +1
    if ((temp>='0') && (temp<='9')){
        dato=0;
        dato=dato*10+(temp-'0');
    }

}

void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    // Configurar pines P1.1 y P1.2 para la función de UART (RX y TX)
    P1SEL |= BIT1 | BIT2;
    P1SEL2 |= BIT1 | BIT2;

    // Configuración del módulo UART
    UCA0CTL1 |= UCSSEL_2;           // SMCLK
    UCA0BR0 = 104;              // 104=1MHz/9600
    UCA0BR1 = 0;
    UCA0MCTL = UCBRS0;          // Modulación UCBRSx = 1
    UCA0CTL1 &= ~UCSWRST;       // Salir del estado de reinicio

    // Habilitar la interrupción de recepción
    IE2|=UCA0RXIE;              //Hab local de intr para recepcion

    // Habilitar las interrupciones globalmente
    __bis_SR_register(GIE);     // Global interrupt enable
    while(1);
}
