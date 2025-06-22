#ifndef INIT_PLAYING_FIELD_H
#define INIT_PLAYING_FIELD_H

#include <stdbool.h>
#include "init_enums_structs.h"

#define ROWS 10
#define COLUMNS 10


void fill_playing_field(int* playingField);
void placeShip(int* field, int startX, int startY, int shipLength, int directionX, int directionY);
char* return_checksum_message(int* field);
bool canPlaceShip(int* playingField, int startX, int startY, int shipLength, int directionX, int directionY);


#endif // !INIT_PLAYING_FIELD_H