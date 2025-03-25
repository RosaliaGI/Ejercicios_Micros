#include <msp430.h>
#include <math.h>
#include <stdint.h>  // Incluir esta librería para usar int32_t, int16_t

#define IIC_slave_ID 0x68       // Dirección del MPU-6050 con AD0 a GND
#define IIC_data_buffer_in_size 7 // 6 bytes: xh, xl, yh, yl, zh, zl

unsigned char *PTxData;
unsigned char TXByteCtr;
unsigned char RXByteCtr;
//unsigned char Command[] = {0x75}; // Who_am_i
 unsigned char Command[] = {0x3B};  // Dirección inicial de ACCEL_XOUT_H
unsigned char Datos[IIC_data_buffer_in_size];
int32_t ang_x, ang_y;

#pragma vector = USCIAB0TX_VECTOR
__interrupt void USCIAB0TX_ISR(void)
{
    if (TXByteCtr)
    {
        UCB0TXBUF = *PTxData++;
        TXByteCtr--;
    }
    else
    {
        UCB0CTL1 |= UCTXSTP;
        IFG2 &= ~UCB0TXIFG;
        __bic_SR_register_on_exit(CPUOFF);
    }
}

// DEBUG
void UART_debug_rx ()
{
    while (!(IFG2 & UCA0RXIFG)); // USCI_A0 RX wait for a data
    (void) UCA0RXBUF;   // Trash received data
}

void UART_debug_tx (unsigned char dato)
{
    while (!(IFG2 & UCA0TXIFG)); // USCI_A0 TX buffer ready?
    UCA0TXBUF = dato; // TX -> RXed character
}

// Función para inicializar IIC
void IIC_init()
{
    P1SEL |= BIT6 + BIT7;
    P1SEL2 |= BIT6 + BIT7;
    UCB0CTL1 |= UCSWRST;
    UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;
    UCB0CTL1 = UCSSEL_2 + UCSWRST;
    UCB0BR0 = 20;
    UCB0BR1 = 0;
    UCB0I2CSA = IIC_slave_ID;
    UCB0CTL1 &= ~UCSWRST;
    IE2 |= UCB0TXIE;
}

// Función para inicializar UART
void UART_debug_init ()
{
    P1SEL |= BIT1 + BIT2;
    P1SEL2 |= BIT1 + BIT2;
    UCA0CTL1 |= UCSSEL_2;
    UCA0BR0 = 104;
    UCA0BR1 = 0;
    UCA0CTL1 &= ~UCSWRST;
}

// Inicialización del MPU-6050
void MPU6050_init()
{
    IIC_init();
    unsigned char initCmd[] = {0x6B, 0x00}; // PWR_MGMT_1, despertar el MPU-6050
    PTxData = initCmd;
    TXByteCtr = 2;  // Cambiar a 2 ya que estamos enviando 2 bytes
    while (UCB0CTL1 & UCTXSTP);
    UCB0CTL1 |= UCTR + UCTXSTT;
    __bis_SR_register(CPUOFF + GIE); // Esperar interrupción
    __delay_cycles(1000);  // Añadir un pequeño retraso para estabilización del sensor
}

// Función para leer datos del acelerómetro
void IIC_read(unsigned char *Address, unsigned char size)
{
    unsigned char i;
    PTxData = Address;
    TXByteCtr = size;
    while (UCB0CTL1 & UCTXSTP);
    UCB0CTL1 |= UCTR + UCTXSTT;
    __bis_SR_register(CPUOFF + GIE);

    UCB0CTL1 &= ~UCTR;
    UCB0CTL1 |= UCTXSTT;
    while ((UCB0CTL1 & UCTXSTT) != 0);

    for (i = 0; i < size; i++)
    {
        while ((IFG2 & UCB0RXIFG) == 0);
        Datos[i] = UCB0RXBUF;
        IFG2 &= ~UCB0RXIFG;
    }
    UCB0CTL1 |= UCTXSTP;
    while ((UCB0CTL1 & UCTXSTP) != 0);
}

// Función para enviar datos del acelerómetro por UART
void UART_buffer_send()
{
    // Enviar el valor de ang_x y ang_y por UART
    UART_debug_tx((unsigned char)(ang_x >> 8));  // Enviar el byte alto de ang_x
    UART_debug_tx((unsigned char)(ang_x & 0xFF)); // Enviar el byte bajo de ang_x
    UART_debug_tx((unsigned char)(ang_y >> 8));  // Enviar el byte alto de ang_y
    UART_debug_tx((unsigned char)(ang_y & 0xFF)); // Enviar el byte bajo de ang_y
}

void calcular_angulos()
{
    int16_t x = (Datos[0] << 8) | Datos[1]; // Combinar X_high y X_low
    int16_t y = (Datos[2] << 8) | Datos[3]; // Combinar Y_high y Y_low
    int16_t z = (Datos[4] << 8) | Datos[5]; // Combinar Z_high y Z_low

    // Aproximación para atan y sqrt en enteros
    int32_t x_sq = x * x;
    int32_t y_sq = y * y;
    int32_t z_sq = z * z;

    int32_t denom = sqrt(x_sq + y_sq + z_sq); // Necesitarías una función para sqrt en enteros
    if (denom == 0) return; // Evitar división por cero

    ang_x = (x * 18000) / (denom * 314); // Esto es una versión simplificada de atan en enteros
    ang_y = (y * 18000) / (denom * 314); // Versión simplificada de atan en enteros
}

// Función principal
void main(void)
{
    WDTCTL = WDTPW + WDTHOLD;
    UART_debug_init();
    MPU6050_init(); // Inicializar el MPU-6050
    while (1)
    {
        IIC_read((unsigned char *) Command, IIC_data_buffer_in_size);
        calcular_angulos();
        UART_buffer_send();
        __delay_cycles(10000); // Retraso para evitar lecturas continuas
    }
}
