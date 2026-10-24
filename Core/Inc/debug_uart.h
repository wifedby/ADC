#ifndef __DEBUG_UART_H__
#define __DEBUG_UART_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/* USART2_TX: PA2, 115200 baud, 8 data bits, no parity, 1 stop bit. */
void Debug_UART_Init(void);
void Debug_UART_WriteString(const char *text);
void Debug_UART_WriteUInt(uint32_t value);

#ifdef __cplusplus
}
#endif

#endif /* __DEBUG_UART_H__ */
