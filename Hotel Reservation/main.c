#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- تعریف ثابت‌ها ---
#define MAX_ROOMS 50
#define MAX_NAME 50
#define MAX_PHONE 15
#define ADMIN_PASS "admin123"

// --- ساختار اتاق ---
typedef struct {
    int id;
    int reserved;   // 0 = free, 1 = reserved
    char customerName[MAX_NAME];
    char phone[MAX_PHONE];
} Room;

// --- آرایه برای ذخیره اتاق‌ها ---
Room hotel[MAX_ROOMS];

// --- تابع خالی کردن اتاق‌ها ---
void initRooms() {
    for (int i = 0; i < MAX_ROOMS; i++) {
        hotel[i].id = i + 1;
        hotel[i].reserved = 0;
        strcpy(hotel[i].customerName, "");
        strcpy(hotel[i].phone, "");
    }
}

// --- منوی مشتری ---
void customerMenu() {
    char name[MAX_NAME];
    char phone[MAX_PHONE];
    int roomId;

    printf("\n--- Customer Login ---\n");
    printf("Enter your name: ");
    scanf(" %[^\n]", name);

    printf("Enter your phone: ");
    scanf(" %[^\n]", phone);

    printf("\nAvailable rooms:\n");
    for (int i = 0; i < MAX_ROOMS; i++) {
        if (!hotel[i].reserved) {
            printf("Room %d (Free)\n", hotel[i].id);
        }
    }

    printf("\nEnter room ID to reserve: ");
    scanf("%d", &roomId);

    if (roomId < 1 || roomId > MAX_ROOMS) {
        printf("Invalid room ID!\n");
        return;
    }

    if (hotel[roomId - 1].reserved) {
        printf("Room already reserved!\n");
        return;
    }

    hotel[roomId - 1].reserved = 1;
    strcpy(hotel[roomId - 1].customerName, name);
    strcpy(hotel[roomId - 1].phone, phone);

    printf("\n✅ Room %d successfully reserved for %s!\n", roomId, name);
}

// --- منوی ادمین ---
void adminMenu() {
    char pass[20];
    printf("\n--- Admin Login ---\n");
    printf("Enter password: ");
    scanf(" %[^\n]", pass);

    if (strcmp(pass, ADMIN_PASS) != 0) {
        printf("❌ Wrong password!\n");
        return;
    }

    printf("\n--- Reservations List ---\n");
    for (int i = 0; i < MAX_ROOMS; i++) {
        if (hotel[i].reserved) {
            printf("Room %d -> %s (%s)\n", hotel[i].id, hotel[i].customerName, hotel[i].phone);
        }
    }
}

// --- تابع اصلی ---
int main() {
    int choice;
    initRooms();

    while (1) {
        printf("\n=== Hotel Reservation System ===\n");
        printf("1. Customer Login\n");
        printf("2. Admin Login\n");
        printf("3. Exit\n");
        printf("Choose option: ");
        scanf("%d", &choice);

        switch (choice) {
        case 1:
            customerMenu();
            break;
        case 2:
            adminMenu();
            break;
        case 3:
            printf("Goodbye!\n");
            exit(0);
        default:
            printf("Invalid choice!\n");
        }
    }

    return 0;
}
