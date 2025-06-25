#include "init_playing_field.h"

bool canPlaceShip(int* gMyPlayingField, int startX, int startY, int shipLength, int directionX, int directionY)
{
	for (int i = 0; i < shipLength; i++)
	{
		if (gMyPlayingField[(startY + directionY * i) * ROWS + (startX + directionX * i)] == 0 && startX + directionX * i >= 0 && startX + directionX * i < ROWS && startY + directionY * i >= 0 && startY + directionY * i < COLUMNS)
		{}
		else
		{
			return false;
		}
	}
	return true;
}

//void placeShip(int* gMyPlayingField, int startX, int startY, int shipLength, int directionX, int directionY)
void placeShip(int* field, int startX, int startY, int shipLength, int directionX, int directionY) {

	//place ships, the length of ship equal it´s type
	for (int i = 0; i < shipLength; i++) {
		int x = startX + directionX * i;
		int y = startY + directionY * i;
		field[y * ROWS + x] = shipLength;
	}

	//mark surrounding fields as 9 - ships are not allowed to be placed there
	for (int i = 0; i < shipLength; i++) 
	{
		int currentX = startX + directionX * i;
		int currentY = startY + directionY * i;

		for (int offsetX = -1; offsetX <= 1; offsetX++) {
			for (int offsetY = -1; offsetY <= 1; offsetY++) {
				//iterate through the ship and check all directions including edges 
				int neighborX = currentX + offsetX;
				int neighborY = currentY + offsetY;

				if (neighborX >= 0 && neighborX < COLUMNS && neighborY >= 0 && neighborY < ROWS) {
					int index = neighborY * ROWS + neighborX;

					//Only if 0, if it´s already 1 or 9 then leave it be
					if (field[index] == 0) {
						field[index] = 9;
					}
				}
			}
		}
	}
}

void clean_up_array(int* gMyPlayingField)
{
	//remove 9s and replaces them with 0
	for(int i = 0; i < ROWS * COLUMNS; i++)
	{
		if(gMyPlayingField[i] == 9) gMyPlayingField[i] = 0;
	}
}


void fill_playing_field(int* gMyPlayingField)
{
	const int directions[4][2] = { {0,-1}, {0,1}, {-1,0}, {1,0} };
	IntTuple amountOfShips[] = { {SCHLACHTSCHIFF, 1}, {KREUZER, 2}, {ZERSTOERER, 3}, {U_BOOT, 4} };
	int shipIndex = 0;
	int currentAmountOfShips = 0;

	while (true)
	{
		if (shipIndex >= sizeof(amountOfShips) / sizeof(amountOfShips[0])) break;

		int randPosX = rand() % COLUMNS;
		int randPosY = rand() % ROWS;
		int shipDirection = rand() % 4;

		if (canPlaceShip(gMyPlayingField, randPosX, randPosY, amountOfShips[shipIndex].typeShip, directions[shipDirection][0], directions[shipDirection][1]))
		{
			placeShip(gMyPlayingField, randPosX, randPosY, amountOfShips[shipIndex].typeShip, directions[shipDirection][0], directions[shipDirection][1]);
			currentAmountOfShips++;

			if (currentAmountOfShips >= amountOfShips[shipIndex].amountShips)
			{
				currentAmountOfShips = 0;
				shipIndex++;
			}
		}
	}
	clean_up_array(gMyPlayingField);
}



char* return_checksum_message(int* gMyPlayingField)
{
    static char message[20] = "DH_CS_";	//has to be static because the pointer is deleted after function -> dangling pointer
    //static variables are stored in .data segment and exist during run-time. they are only visible inside the scope of function
	int index = 6; // after "DH_CS_"

    for (int y = 0; y < ROWS; y++) 
    {
        int sum = 0;
        for (int x = 0; x < COLUMNS; x++) 
        {
            int value = gMyPlayingField[y * COLUMNS + x];
            if (value != 0) 
            {
                sum++;
            }
        }
        message[index++] = '0' + sum;  // Voraussetzung: sum ist 0–9
    }

    message[index++] = '\r';
    message[index++] = '\n';
    message[index] = '\0';

    return message;
}

