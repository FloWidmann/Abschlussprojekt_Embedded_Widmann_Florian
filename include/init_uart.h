#ifndef INIT_UART_H
#define INIT_UART_H

#include <main.h>
#include <stm32f0xx.h>
#include "clock_.h"
#include "fifo.h"

#define BAUDRATE 115200 // Baud rate set to 9600 baud per second

volatile Fifo_t usart_rx_fifo;
volatile Fifo_t usart_tx_fifo;
const uint8_t USART2_RX_PIN; // PA3 is used as USART2_RX
const uint8_t USART2_TX_PIN; // PA2 is used as USART2_TX


void init_uart();

#endif // !INIT_UART_H
