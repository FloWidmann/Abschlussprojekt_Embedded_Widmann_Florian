#include <main.h>
#include <stm32f0xx.h>
#include "stm32f091xc.h"

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdbool.h>
#include <time.h>
#include "clock_.h"
#include "fifo.h"
//#include "init_timer.h"
#include "init_uart.h"
#include "init_enums_structs.h"
#include "init_playing_field.h"

#define LOG(msg...) printf(msg)
// Select the Baudrate for the UART

#define MESSAGEBUFFERSIZE FIFO_SIZE



//global variables with prefix g for easier debugging
char gMessageBuffer[MESSAGEBUFFERSIZE] = { 0 }; //not const char* as usual because we have different chars
uint32_t gBytesReceived = 0;  
State gGameState = RECEIVING;
bool gCanReceive = false;
  

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
            break;        
          }
    }
}

int sum_of_check_sum(const char* message) 
{
    int sum = 0;
    for (int i = 6; i < 16; i++) 
    { 
        if (message[i] >= '0' && message[i] <= '9') 
        {
            sum += message[i] - '0'; //message[i] return int value of ascii symbol, subtract '0' to get actual value
        }
    }
    return sum;
}

void set_back_variables(bool* canReceive)
{
  *canReceive = false;
  gBytesReceived = 0;
  memset(gMessageBuffer, 0, MESSAGEBUFFERSIZE);            
}





int main(void)
{
  init_uart();
//  init_timer15();
  gBytesReceived = 0;
  gGameState = RECEIVING;
  bool canReceive = false;
  //Enums are defined in Header init_enums_structs
  int playingField[ROWS * COLUMNS] = { EMPTY_FIELD };
  srand(TIM15->CNT);  // Zufälliger Startwert vom Timerzähler
  fill_playing_field(playingField);

  for (;;)
  { // Infinite loop
     uint8_t byte;
    switch (gGameState)
    {
      case(RECEIVING):
      {
        if (fifo_get((Fifo_t *)&usart_rx_fifo, &byte) == 0)
        {
          
          if(gBytesReceived < MESSAGEBUFFERSIZE && canReceive)
          {
              gMessageBuffer[gBytesReceived] = byte;
          }

          gBytesReceived++; // count the Bytes Received by getting Data from the FIFO 
          
          if(strcmp(gMessageBuffer, "HD_START") == 0)
          {
              gGameState = SENDING_START;
              set_back_variables(&canReceive);
          }

          if(strncmp(gMessageBuffer, "HD_CS_", strlen("HD_CS_")) == 0 && gBytesReceived >= strlen("HD_CS_0123456789"))
          {
              if(sum_of_check_sum(gMessageBuffer) <= 30)
              { 
                gGameState = SENDING_CHECKSUM;
              }
              else
              {
                gGameState = SENDING_VICTORY;
              }
              set_back_variables(&canReceive);
          }

          if(strncmp(gMessageBuffer, "HD_BOOM_", strlen("HD_BOOM_")) == 0 && gBytesReceived >= strlen("HD_BOOM_0_0"))
          {
              gGameState = SENDING_BOOM;
              set_back_variables(&canReceive);
          
          }
          
          if(strcmp(gMessageBuffer, "HD_SF") == 0)
          {
              gGameState = SENDING_DEFEAT;
              set_back_variables(&canReceive);
          }

          
          if(byte == '\n') 
          {
            canReceive = true;
            gBytesReceived = 0;
          }

          }

          break;
        
      }     
      
      case(SENDING_START):
      {
          LOG("DH_START_FLORIAN\r\n");
          gGameState = RECEIVING;
          break;
      }
      
      case(SENDING_CHECKSUM):
      {
        LOG("DH_CS_012345689\r\n");
        gGameState = RECEIVING;
        break;
      }

      case(SENDING_BOOM):
      {
        LOG("DH_BOOM_9_9");
        gGameState = RECEIVING;
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