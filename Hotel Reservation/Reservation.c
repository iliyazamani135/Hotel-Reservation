#include <stdio.h>
#include <string.h>
#include "../Entities/Room.h"
#include "../DAL/RoomRepository.c"

int reserveRoom(int roomId, char* name, char* phone) {
    if (roomId < 1 || roomId > MAX_ROOMS) return 0;

    if (hotel[roomId - 1].reserved) return -1;

    hotel[roomId - 1].reserved = 1;
    strcpy(hotel[roomId - 1].customerName, name);
    strcpy(hotel[roomId - 1].phone, phone);

    return 1;
}
