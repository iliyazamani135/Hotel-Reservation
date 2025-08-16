#include <stdio.h>
#include "../BLL/ReservationService.c"

void customerMenu() {
    char name[MAX_NAME];
    char phone[MAX_PHONE];
    int roomId;

    printf("\nEnter your name: ");
    scanf(" %[^\n]", name);
    printf("Enter your phone: ");
    scanf(" %[^\n]", phone);

    printf("\nEnter room ID to reserve: ");
    scanf("%d", &roomId);

    int result = reserveRoom(roomId, name, phone);

    if (result == 1) printf("✅ Room reserved!\n");
    else if (result == -1) printf("❌ Already reserved!\n");
    else printf("❌ Invalid room ID!\n");
}
