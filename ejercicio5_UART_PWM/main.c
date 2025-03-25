#include <msp430.h>
unsigned char duty_cycle=70;
unsigned char espejo_pin=0;
unsigned char dato;
unsigned char temp;


#pragma vector = TIMER1_A1_VECTOR
__interrupt void TIMER1_A1_ISR (void)
{
    //duty_cycle=dato*10;

    switch(TA1IV)
    {
        case TA1IV_NONE: break; // Vector 0: No interrupt
        case TA1IV_TACCR1: // Vector 2: TACCR1 CCIFG
            if (espejo_pin==0)
            {
                espejo_pin=1;
                TA1CCR1+=duty_cycle*10;
            }
            else
            {
                espejo_pin=0;
                TA1CCR1+=1000-(duty_cycle*10);
            }
            break;
        case TA1IV_TACCR2: // Vector 4: TACCR2 CCIFG
            break;
        case TA1IV_6: break; // Vector 6: Reserved CCIFG
        case TA1IV_8: break; // Vector 8: Reserved CCIFG
        case TA1IV_TAIFG: break; // Vector 10: TAIFG
        default: break;
    }
}

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

    // Pin p2.1 pwm
    P2SEL|=(1<<1);              //P2.1
    P2DIR|=(1<<1);              //P2.1
    P1DIR|=1;                   //P1.0 GPIO, output
    TA1CTL=TASSEL_2+MC_2;
    TA1CCTL1=OUTMOD_4+CCIE;
    TA1CCR1=TAR+1000-(duty_cycle*10);
    TA1CCTL2=CCIE;
    TA1CCR2=TAR+50000;


    // Configurar pines P1.1 y P1.2 para la función de UART (RX y TX)
    P1SEL |= BIT1 + BIT2;
    P1SEL2 |= BIT1 + BIT2;

    // Configuración del módulo UART
    UCA0CTL1 |= UCSSEL_2;           // SMCLK
    UCA0BR0 = 104;              // 104=1MHz/9600
    UCA0BR1 = 0;
    UCA0MCTL = UCBRS0;          // Modulación UCBRSx = 1
    UCA0CTL1 &= ~UCSWRST;       // Salir del estado de reinicio

    // Habilitar la interrupción de recepción
    IE2|=UCA0RXIE;              //Hab local de intr para recepcion

    __bis_SR_register(GIE);     // Global Interrupt Enable
    while (1);
}
