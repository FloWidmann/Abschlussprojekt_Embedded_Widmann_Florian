#include <main.h>
#include <stm32f0xx.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "dynamic_array.h"
#include "clock_.h"
#include "fifo.h"

#define LOG(msg...) printf(msg)
// Select the Baudrate for the UART
#define BAUDRATE 115200 // Baud rate set to 9600 baud per second


volatile Fifo_t usart_rx_fifo;
const uint8_t USART2_RX_PIN = 3; // PA3 is used as USART2_RX
const uint8_t USART2_TX_PIN = 2; // PA2 is used as USART2_TX


typedef enum State
{
  INIT,
  GETTING_ATTACKED,
  ATTACKING,
  END

}State;

void init_uart()
{
   SystemClock_Config(); // Configure the system clock to 48 MHz
  RCC->AHBENR |= RCC_AHBENR_GPIOAEN;    // Enable GPIOA clock
  RCC->APB1ENR |= RCC_APB1ENR_USART2EN; // Enable USART2 clock
  GPIOA->MODER |= 0b10 << (USART2_TX_PIN * 2);    // Set PA2 to Alternate Function mode
  GPIOA->AFR[0] |= 0b0001 << (4 * USART2_TX_PIN); // Set AF for PA2 (USART2_TX)
  GPIOA->MODER |= 0b10 << (USART2_RX_PIN * 2);    // Set PA3 to Alternate Function mode
  GPIOA->AFR[0] |= 0b0001 << (4 * USART2_RX_PIN); // Set AF for PA3 (USART2_RX)

  USART2->BRR = (APB_FREQ / BAUDRATE); // Set baud rate (requires APB_FREQ to be defined)
  USART2->CR1 |= 0b1 << 2;             // Enable receiver (RE bit)
  USART2->CR1 |= 0b1 << 3;             // Enable transmitter (TE bit)
  USART2->CR1 |= 0b1 << 0;             // Enable USART (UE bit)
  USART2->CR1 |= 0b1 << 5;             // Enable RXNE interrupt (RXNEIE bit)

  NVIC_SetPriorityGrouping(0);                               // Use 4 bits for priority, 0 bits for subpriority
  uint32_t uart_pri_encoding = NVIC_EncodePriority(0, 1, 0); // Encode priority: group 1, subpriority 0
  NVIC_SetPriority(USART2_IRQn, uart_pri_encoding);          // Set USART2 interrupt priority
  NVIC_EnableIRQ(USART2_IRQn);                               // Enable USART2 interrupt

  fifo_init((Fifo_t *)&usart_rx_fifo);                       // Init the FIFO
}


// For supporting printf function we override the _write function to redirect the output to UART
int _write(int handle, char* data, int size) {
  int count = size;  // Make a copy of the size to use in the loop

  while (count--) {
      while (!(USART2->ISR & USART_ISR_TXE)) {
      }
      USART2->TDR = *data++;

  }
  return size;
}


void clean_message(char* inputMessage, int max_size) {
    for(int i = 0; i < max_size; i++) {
        if(inputMessage[i] == '\n') {
            inputMessage[i] = '\0';
            break;        }
    }
}






int main(void)
{
  char messageBuffer[10] = { 0 }; //not const char* as usual because we have different chars
  int diff = 0;
  LOG("Init System");
  init_uart();
  uint32_t bytesReceived = 0;
  State gameState = INIT;
  bool canReceive = false;
  bool canSend = false;

  for (;;)
  { // Infinite loop
    switch (gameState)
    {
    case(INIT):
    {
    uint8_t byte;
    if (fifo_get((Fifo_t *)&usart_rx_fifo, &byte) == 0)
    {
      //for now this works, look for better solution
      if(byte == 72) canReceive = true;
      if(bytesReceived < 10 && canReceive) messageBuffer[bytesReceived] = byte;
      else
      {
        clean_message(messageBuffer, 10);
        if(strcmp(messageBuffer, "HD_START\r") == 0) 
        {
          gameState = ATTACKING;
        }
      }
      if(byte == 10) (canReceive) = false;
      bytesReceived++; // count the Bytes Received by getting Data from the FIFO
      }
      break;
    }
  }
  }
}

void USART2_IRQHandler(void)
{
  static int ret; // You can do some error checking
  if (USART2->ISR & USART_ISR_RXNE)
  {                                              // Check if RXNE flag is set (data received)
    uint8_t c = USART2->RDR;                     // Read received byte from RDR (this automatically clears the RXNE flag)
    ret = fifo_put((Fifo_t *)&usart_rx_fifo, c); // Put incoming Data into the FIFO Buffer for later handling
  }
}