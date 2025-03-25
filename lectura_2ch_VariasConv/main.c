#include <msp430.h>

unsigned short ADC_samples[2];
unsigned short pot1;
unsigned short pot2;

#pragma vector=TIMER1_A0_VECTOR         // Interrupción del timer1
__interrupt void Timer1_A (void)
{
    TA1CCTL0 &= ~CCIFG;                 // Limpiar bandera del timer
    ADC10SA = (unsigned short)ADC_samples; // Dirección del buffer de datos
    ADC10CTL0 |= ENC + ADC10SC;         // Iniciar conversión ADC
    __bis_SR_register(CPUOFF + GIE);    // Entrar en modo bajo consumo y habilitar interrupciones globales
}

// ADC10 interrupt service routine
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void)
{
    ADC10CTL0 &= ~ENC;                  // Desactivar ADC
    pot1 = ADC_samples[0];              // Leer potenciómetro en P1.2 (A2)
    pot2 = ADC_samples[1];              // Leer potenciómetro en P1.3 (A3)
    __bic_SR_register_on_exit(CPUOFF);  // Salir del modo bajo consumo
    TA1CCR0 = TAR+50000;
}



int main(void)
{
    WDTCTL = WDTPW + WDTHOLD;

    // Configuración del ADC
    ADC10CTL1 = INCH_3 + CONSEQ_1;
    ADC10CTL0 = ADC10SHT_2 + MSC + ADC10ON + ADC10IE;
    ADC10DTC1 = 0x02;                   // 2 conversiones (una para cada potenciómetro)
    ADC10AE0 |= 0x0C;                   // Habilitar canales ADC para P1.2 (A2) y P1.3 (A3)

    TA1CTL = TASSEL_2 + MC_2;
    TA1CCTL0 |= CCIE;
    TA1CCR0 = TAR+50000;                // Configurar el intervalo del temporizador para 50 ms (ajuste del tiempo)

    __bis_SR_register(GIE);             // Habilitar interrupciones globales

    while(1);
   }
