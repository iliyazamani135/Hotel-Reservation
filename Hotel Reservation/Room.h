#ifndef ROOM_H
#define ROOM_H

#define MAX_NAME 50
#define MAX_PHONE 15

typedef struct {
    int id;
    int reserved;
    char customerName[MAX_NAME];
    char phone[MAX_PHONE];
} Room;

#endif
