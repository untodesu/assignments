#include <lpc17xx.h>
#include <string.h>
#include "lib_uart.h"

#define IER_RBR		0x01
#define IER_THRE	0x02
#define IER_RLS		0x04

#define IIR_PEND	0x01
#define IIR_RLS		0x03
#define IIR_RDA		0x02
#define IIR_CTI		0x06
#define IIR_THRE	0x01

#define LSR_RDR		0x01
#define LSR_OE		0x02
#define LSR_PE		0x04
#define LSR_FE		0x08
#define LSR_BI		0x10
#define LSR_THRE	0x20
#define LSR_TEMT	0x40
#define LSR_RXFE	0x80

#define UART_BUFSIZE 64
static uint8_t uart_buffer[UART_BUFSIZE];
static size_t uart_buffer_size = 0;
static int uart_can_write = 1;

void UART1_IRQHandler(void) 
{
    uint8_t iir, lsr;
    uint8_t abyss = abyss;
	
    iir = LPC_UART1->IIR;
    iir >>= 1;
    iir &= 0x07;

    /* receive line status */
    if(iir == IIR_RLS) {
        lsr = LPC_UART1->LSR;
        if (lsr & (LSR_OE | LSR_PE | LSR_FE | LSR_RXFE | LSR_BI)) {
            abyss = LPC_UART1->RBR;
            return;
        }

        if(lsr & LSR_RDR) {
            uart_buffer[uart_buffer_size] = LPC_UART1->RBR;
            uart_buffer_size++;
            if (uart_buffer_size >= UART_BUFSIZE)
                uart_buffer_size = 0;
            return;
        }
        
        return;
    }

    /* receive data available */
    if(iir == IIR_RDA) {
        uart_buffer[uart_buffer_size] = LPC_UART1->RBR;
        uart_buffer_size++;
        if (uart_buffer_size == UART_BUFSIZE)
            uart_buffer_size = 0;
        return;
    }

    /* transmit holding register empty */
    if (iir == IIR_THRE) {
        uart_can_write = ((LPC_UART1->LSR & LSR_THRE) ? 1 : 0);
        return;
    }
}

extern uint32_t SystemFrequency;
void uart_init(size_t speed, uint8_t mode)
{
    uint8_t lcr;
    uint32_t divisor, pclk;

	LPC_PINCON->PINSEL4 &= ~0x0000000F;
	LPC_PINCON->PINSEL4 |= 0x0000000A;

	switch ((LPC_SC->PCLKSEL0 >> 8) & 0x03) {
	    case 0x01:
            pclk = SystemFrequency / 1;
            break; 
	    case 0x02:
            pclk = SystemFrequency / 2;
            break; 
	    case 0x03:
            pclk = SystemFrequency / 8;
            break;
	    default:
            pclk = SystemFrequency / 4;
            break;
	}
    
	divisor = pclk / 16 / speed;

    LPC_UART1->LCR = mode | 0x80;
    LPC_UART1->DLM = divisor / 256;
    LPC_UART1->DLL = divisor % 256;
	LPC_UART1->LCR = mode;
    LPC_UART1->FCR = 0x07;

    NVIC_EnableIRQ(UART1_IRQn);

    LPC_UART1->IER = IER_THRE | IER_RLS | IER_RBR;
}

void uart_write(const void *s, size_t n)
{
    const uint8_t *sp = s;
    
    LPC_UART1->IER &= ~IER_RBR;

    while(n--) {
        while(!uart_can_write);
        LPC_UART1->THR = *sp++;
        uart_can_write = 0;
    }

    LPC_UART1->IER |= IER_RBR;
}

size_t uart_read(void *s, size_t n)
{
    if(uart_buffer_size) {
        if(n > uart_buffer_size)
            n = uart_buffer_size;
        memcpy(s, uart_buffer, n);
        uart_buffer_size = 0;
        return n;
    }
    
    return 0;
}
