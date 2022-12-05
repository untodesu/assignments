#ifndef LIB_UART_H_ 
#define LIB_UART_H_ 1
#include <lpc17xx.h>
#include <stddef.h>

#define UART_NOPAR 0
#define UART_ODDPAR 1
#define UART_EVENPAR 2

void UART1_IRQHandler(void);
void uart_init(size_t speed, unsigned int wordsize, int parity, int doublestop);
size_t uart_write(const void *s, size_t n);
size_t uart_read(void *s, size_t n);

#endif
