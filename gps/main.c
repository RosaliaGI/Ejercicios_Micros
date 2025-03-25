#include <msp430.h> 
#define TRUE        1
#define FALSE       0
unsigned int temp=0,longitud_good=0,latitud_good=0,entero=0,decimal=0;
unsigned long int longitud,longitud_temp,latitud,latitud_temp,copia_longitud,copia_latitud;
unsigned char dato,longi[5],lati[5];
unsigned int i=0,flag;

#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
         dato=UCA0RXBUF;
         if(dato=='$'){
             temp=TRUE;
             return;
         }

         if((dato=='L')&&temp){
             longitud_good=TRUE; entero=TRUE;
             temp=FALSE;
             return;
         }
         if(longitud_good){
             if((dato>='0')&&(dato<='9')){
                 longitud_temp=(longitud_temp*10)+(dato-0x30);
             }
             else if((dato=='N')||dato=='S'){
                 longitud=longitud_temp;
                 longitud_temp=0;
                 longitud_good=FALSE;
                 latitud_good=TRUE;
             }
         }
         if(latitud_good){
             if(((dato>='0')&&(dato<='9'))){
                 latitud_temp=(latitud_temp*10)+(dato-0x30);
             }
             else if((dato=='W')||dato=='E'){
                 latitud=latitud_temp;
                 latitud_temp=0;
                 latitud_good=FALSE;
                 decimal=FALSE;
             }
         }
         if(dato==13){
             copia_longitud=longitud;
             copia_latitud=latitud;
             copia_longitud/=10;
             longi[3]=copia_longitud%100;
             copia_longitud/=100;
             longi[2]=copia_longitud%100;
             copia_longitud/=100;
             longi[1]=copia_longitud%100;
             copia_longitud/=100;
             longi[0]=copia_longitud%100;
             copia_latitud/=10;
             lati[3]=copia_latitud%100;
             copia_latitud/=100;
             lati[2]=copia_latitud%100;
             copia_latitud/=100;
             lati[1]=copia_latitud%100;
             copia_latitud/=100;
             lati[0]=copia_latitud%1000;
             flag=1;
             IE2|=BIT1;
         }
}
#pragma vector=USCIAB0TX_VECTOR
__interrupt void USCI0TX_ISR(void)
{
    if(flag==1){
        UCA0TXBUF=longi[i++];
        if (longi[i]==5)
        {
            flag=2;
            i=0;
            return; }
    }
    if(flag==2){
        UCA0TXBUF=lati[i++];
        if (lati[i]==5)
        {
            flag=3;
            i=0;
            IE2&=~BIT1;
        }
    }
}

void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    P1SEL = BIT1 + BIT2 ;                     // P1.1 = RXD, P1.2=TXD
    P1SEL2 = BIT1 + BIT2;                    // P1.1 = RXD, P1.2=TXD
    UCA0CTL1 |= UCSSEL_2;                     // SMCLK
    UCA0BR0 = 104;                            // 1MHz 9600
    UCA0BR1 = 0;                              // 1MHz 9600
    UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine
    IE2|=BIT0;                              //Hab Intr Rx
  __bis_SR_register(GIE);                   // interrupts enabled

  while (1);
}
