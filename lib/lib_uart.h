#ifndef E93EA26B_14F2_4343_980B_E6AC51E8676F
#define E93EA26B_14F2_4343_980B_E6AC51E8676F
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#define UART_WORD_5 0x00
#define UART_WORD_6 0x01
#define UART_WORD_7 0x02
#define UART_WORD_8 0x03
#define UART_2_STOP 0x04
#define UART_PR_ODD 0x08
#define UART_PR_EVN 0x18
#define UART_PR_HGH 0x28
#define UART_PR_LOW 0x38

void UART1_IRQHandler(void);
void uart_init(size_t speed, uint8_t mode);
void uart_write(const void *buffer, size_t n);
size_t uart_read(void *buffer, size_t n);

void uart_puts(const char *s);
void uart_printf(const char *fmt, ...);
void uart_vprintf(const char *fmt, va_list ap);

#endif /* E93EA26B_14F2_4343_980B_E6AC51E8676F */
