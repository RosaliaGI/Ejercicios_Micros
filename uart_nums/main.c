#include <msp430.h>
unsigned char dato;
unsigned char temp;


void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;                       // stop watchdog timer
    P1SEL|=(3<<1);
    P1SEL2|=(3<<1);                                 //Con ambas líneas seleccionamos que sean RX y TX de UART
    UCA0CTL1 |= 1<<7;                               // SMCLK
    UCA0BR0 = 104;                                  // 104=1MHz/9600
    UCA0CTL1 &= ~BIT0;                              //
    while(1)
    {
        dato=0;
        do{
            do{}while ((IFG2 & UCA0RXIFG)==0);
            temp = UCA0RXBUF;                       // TX -> RXed character +1
            if ((temp>='0') && (temp<='9')) dato=dato*10+(temp-'0');
        }while (temp!=13);
    }
}
