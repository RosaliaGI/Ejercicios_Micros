#include <msp430.h>

unsigned char duty_cycle=70;
unsigned char espejo_pin=0;

unsigned int last_time = 0;             // Último valor del temporizador
unsigned int current_time = 0;          // Valor actual del temporizador
unsigned int time_difference = 0;       // Diferencia de tiempo (período)
unsigned int speed = 0;                 // Velocidad calculada
volatile unsigned int n_vueltas = 0;    // Variable para contar los desbordamientos
unsigned char interrupted_port2 = 0;
unsigned char buffer[10];               // Para almacenar el número convertido itoa

void itoa(unsigned int num)
{
    int i = 0;

    // Si el número es 0, enviamos '0'
    if (num == 0) {
        while (!(IFG2 & UCA0TXIFG));    // Esperar que el buffer esté libre
        UCA0TXBUF = '0';                // Enviar '0'
        while (!(IFG2 & UCA0TXIFG));    // Esperar que el buffer esté libre
        UCA0TXBUF = '\n';               // Enviar delimitador de nueva línea
        return;
    }

    // Convertir el número a caracteres, dígito por dígito
    while (num > 0)
    {
        buffer[i++] = (num % 10) + '0';  // Convertir el dígito a carácter
        num /= 10;                       // Eliminar el último dígito
    }

    // Los dígitos se almacenan en orden inverso, entonces los enviamos en orden correcto
    while (i > 0) {
        while (!(IFG2 & UCA0TXIFG));    // Esperar que el buffer esté libre
        UCA0TXBUF = buffer[--i];        // Enviar cada carácter, de mayor a menor
    }

    // Enviar un delimitador de nueva línea ('\n') al final
    while (!(IFG2 & UCA0TXIFG));    // Esperar que el buffer esté libre
    UCA0TXBUF = '\n';
}

#pragma vector=TIMER0_A1_VECTOR
__interrupt void TIMER0_A1_ISR(void)
{
    switch (TA0IV)
    {
        case TA0IV_TAIFG:    // Se desbordó el temporizador TA0
            n_vueltas++;     // Incrementar el contador de desbordamientos
            break;
        default: break;
    }
}

#pragma vector = TIMER1_A1_VECTOR
__interrupt void TIMER1_A1_ISR (void)
{
    switch(TA1IV)
    {
        case TA1IV_NONE: break;         // Vector 0: No interrupt
        case TA1IV_TACCR1:              // Vector 2: TACCR1 CCIFG
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
        case TA1IV_TACCR2:              // Vector 4: TACCR2 CCIFG
            break;
        case TA1IV_6: break;            // Vector 6: Reserved CCIFG
        case TA1IV_8: break;            // Vector 8: Reserved CCIFG
        case TA1IV_TAIFG: break;        // Vector 10: TAIFG
        default: break;
    }
}

#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void)
{
    interrupted_port2 = 1;

    if (P2IFG & BIT4){
        // Aquí calculamos el tiemo entre interrupciones
        current_time = TA0R;            // Se guarda tiempo actual


        if (n_vueltas > 0) {            // Si ocurrieron desbordamientos,
            // La diferencia de tiempo incluye los desbordamientos
            time_difference = (0xFFFF - last_time) + (n_vueltas - 1) * 0xFFFF + current_time;
            n_vueltas = 0;              // Reiniciar el contador de desbordamientos
        } else {
            // No hubo desbordamientos, cálculo normal
            time_difference = current_time - last_time;//tiempo entre interrupciones
        }

        last_time = current_time;       //tiempo actual como el último para la próxima interrupción

        if (time_difference > 0) {
            speed = 125000 / time_difference;//Calculamos la velocidad (vueltas por segundo)
        } else {
            speed = 0;
        }

        itoa(speed);                    // Invoco func itoa con speed
        P2IFG &=~BIT4;                  // Limpiar flag de interrupción
    }
}

void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;           // stop watchdog timer

    // P2.4 Config sensor de herradura
    P2SEL &= ~BIT4;  // Asegurarse que P2.4 está en modo I/O (no en modo especial)
    P2SEL2 &= ~BIT4; // Asegurarse que P2.4 está en modo I/O

    P2DIR &= ~BIT4;                     // P2.4 como entrada (0)
    P2REN |= BIT4;                      // Habilitar resistencia interna
    P2OUT |= BIT4;                      // Configurar como pull-up (1 lógico)
    // Interrupciones para el sensor
    P2IFG &=~BIT4;                      // Limpiar flag de interrup
    P2IE |= BIT4;                       // Habilitar interrup para P2.4
    P2IES |= BIT4;                      // Interrup en flanco de bajada

    // Pin p2.1 pwm
    P2SEL|=(1<<1);                      //P2.1
    P2DIR|=(1<<1);                      //P2.1

    // Configurar el temporizador TA1 para PWM
    TA1CTL=TASSEL_2 + MC_2;               // SMCLK, modo continuo
    TA1CCTL1=OUTMOD_4+CCIE;             // Modo toggle
    TA1CCR1=TAR+1000-(duty_cycle*10);
    TA1CCTL2=CCIE;
    TA1CCR2=TAR+50000;

    // Configurar pin P1.2 para la función de UART (TX)
    P1SEL |= BIT2;
    P1SEL2 |= BIT2;
    // Configuración del módulo UART
    UCA0CTL1 |= UCSSEL_2;               // SMCLK
    UCA0BR0 = 104;                      // 104=1MHz/9600
    UCA0BR1 = 0;
    UCA0MCTL = UCBRS0;                  // Modulación UCBRSx = 1
    UCA0CTL1 &= ~UCSWRST;               // Salir del estado de reinicio

    // Config timer TA0 para contar tiempo que pasa entre interrupciones
    TA0CCR0 = 0xFFFF;                   // Config valor máx del temporizador
    TA0CTL = TASSEL_2 + MC_2 + ID_3;    // SMCLK, modo continuo, divisor 8
    TA0CTL |= TAIE;                     // Habilitar interrupciones por desbordamiento

    __bis_SR_register(GIE);             // Global Interrupt Enable
    while (1);
}
