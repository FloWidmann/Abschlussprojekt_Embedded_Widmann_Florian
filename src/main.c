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
#include "init_adc.h"
#include "init_uart.h"
#include "init_enums_structs.h"
#include "init_playing_field.h"

#define LOG(msg...) printf(msg)
// Select the Baudrate for the UART
#define TIMEOUT 100000
#define MESSAGEBUFFERSIZE FIFO_SIZE



//global variables with prefix g for easier debugging
char gMessageBuffer[MESSAGEBUFFERSIZE] = { 0 }; //not const char* as usual because we have different chars
uint32_t gBytesReceived = 0;  
State gGameState = RECEIVING;
bool gCanReceive = false;
int gMyShipParts = 30;
int gEnemyShipParts = 30;  
int gShootingPosition = 0;
int gMyPlayingField[ROWS * COLUMNS] = { EMPTY_FIELD };
int allShots[200];

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

void set_back_playing_variables(bool* canReceive)
{
  *canReceive = false;
  gBytesReceived = 0;
  memset(gMessageBuffer, 0, MESSAGEBUFFERSIZE);            
}

void set_back_all_variables(bool* canReceive)
{


}


uint32_t return_adc_seed()
{
  uint32_t seed = 0;                                  // Variable to store the ADC converted value
  ADC1->CR |= 0b1 << 2;                                  // Start ADC conversion by setting ADSTART bit
  while ((ADC1->ISR & ADC_ISR_EOC) == 0) {           // Wait for End Of Conversion flag (EOC)
            if (timeout(TIMEOUT)){                         // Check for timeout during wait
                error();                                   // Handle timeout error
            }
        }   
  seed = ADC1->DR;   
  return seed;
}

bool was_hit()
{
  int posY = gMessageBuffer[8] - '0';
  int posX = gMessageBuffer[10] - '0';
  if(gMyPlayingField[posY * ROWS + posX] != 0) 
  {
    return true;
  }
  else
  {
    return false;
  }
}

char* return_boom_message()
{
    static char message[13] = "DH_BOOM_";	//has to be static because the pointer is deleted after function -> dangling pointer
    //static variables are stored in .data segment and exist during run-time. they are only visible inside the scope of function
    int posY = gShootingPosition / ROWS;
    int posX = gShootingPosition % ROWS;
    message[8] = posX + '0';
    message[9] = '_';
    message[10] = posY + '0';
    message[11] = '\r';
    message[12] = '\n';
    message[13] = '\0';
    gShootingPosition++;
    return message;
}

char* return_defeat_message(int index)
{
  static char message[19] = "DH_SF";
  message[5] = index + '0';
  message[6] = 'D';
  
  for(int i = 7; i < 17; i++)
  {
      message[i] = gMyPlayingField[index * ROWS + (i-7)] + '0'; 
  }
  message[17] = '\r';
  message[18] = '\n';
  message[19] = '\0';
  return message;
}



int main(void)
{
  init_uart();
  init_adc();
  gBytesReceived = 0;
  gGameState = RECEIVING;
  bool canReceive = false;
  int iteration = 0;
  unsigned int index = 0;
  bool sendBoom = false;
 
  //Enums are defined in Header init_enums_structs
 
  int enemyPlayingField[ROWS * COLUMNS] = { EMPTY_FIELD };
  uint32_t seed = return_adc_seed();
  srand(0);
  fill_playing_field(gMyPlayingField);         
           
  for (;;)
  { // Infinite loop
    uint8_t byte;
    switch (gGameState)
    {
      case(RECEIVING):
{
    if (fifo_get((Fifo_t *)&usart_rx_fifo, &byte) == 0)
    {
        
        if (gBytesReceived < MESSAGEBUFFERSIZE)
        {
            gMessageBuffer[gBytesReceived++] = byte;
        }

        if (byte == '\n')
        {
            if (strncmp(gMessageBuffer, "HD_START", strlen("HD_START")) == 0)
            {
                gGameState = SENDING_START;
            }
            else if (strncmp(gMessageBuffer, "HD_CS_", strlen("HD_CS_")) == 0 &&
                     strlen(gMessageBuffer) >= strlen("HD_CS_0123456789"))
            {
                if (sum_of_check_sum(gMessageBuffer) <= 30)
                {
                    gGameState = SENDING_CHECKSUM;
                }
                else
                {
                    gGameState = SENDING_VICTORY;
                }
            }
            else if (strncmp(gMessageBuffer, "HD_BOOM_", strlen("HD_BOOM_")) == 0 &&
                     strlen(gMessageBuffer) >= strlen("HD_BOOM_0_0"))
            {
                int x = gMessageBuffer[10] - '0';
                int y = gMessageBuffer[8] - '0';
                
                //Added this if condition to avoid a weird bug
                if(x >= 0 && x < 10 && y >= 0 && y < 10)
                {
                allShots[index++] = x;
                allShots[index++] = y;

                if (gMyPlayingField[y * 10 + x] != 0 && gMyShipParts > 1)
                {
                    gGameState = SENDING_HIT;
                }
                else if (!was_hit() && gMyShipParts > 1)
                {
                    gGameState = SENDING_MISS;
                }
                else
                {
                    gGameState = SENDING_DEFEAT;
                }
                }
              }
            
          

            // Nach Verarbeitung: Variablen zurücksetzen
            set_back_playing_variables(&canReceive);
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
        char* msg = return_checksum_message(gMyPlayingField);
        LOG(msg);
        gGameState = RECEIVING;
        break;
      }

      case(SENDING_HIT):
      {
        gMyShipParts--;
        LOG("DH_BOOM_H\r\n");
        gGameState = SENDING_BOOM;
        break;
      }

      case(SENDING_MISS):
      {
        gMyShipParts--;
        LOG("DH_BOOM_M\r\n");
        gGameState = SENDING_BOOM;
        break;
      }

      case(SENDING_BOOM):
      {
        //evaluate_boom_position(enemyPlayingField);
        char* msg = return_boom_message();

        LOG("DH_BOOM_9_9\r\n");
        gGameState = RECEIVING;
        break;
      }

      case(SENDING_DEFEAT):
      {

        for(int i = 0; i < 10; i++)
        {
          char* msg = return_defeat_message(i);
          LOG(return_defeat_message(i));
        }
       
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

