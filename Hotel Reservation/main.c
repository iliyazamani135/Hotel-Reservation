#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <direct.h>

#define MAX_ROOMS 50
#define MAX_NAME 50
#define MAX_PHONE 15
#define PASS_FILE "data/admin_pass.txt"
#define PRICE_FILE "data/prices.txt"

// --- ساختار اتاق ---
typedef struct {
    int id;
    int reserved;
    char customerName[MAX_NAME];
    char phone[MAX_PHONE];
    char reservedDate[11];
    char type[20];
    int pricePerNight;
} Room;

Room hotel[MAX_ROOMS];

// --- خالی کردن اتاق‌ها ---
void initRooms() {
    for (int i = 0; i < MAX_ROOMS; i++) {
        hotel[i].id = i + 1;
        hotel[i].reserved = 0;
        strcpy(hotel[i].customerName, "");
        strcpy(hotel[i].phone, "");
        strcpy(hotel[i].reservedDate, "");
        strcpy(hotel[i].type, "");
        hotel[i].pricePerNight = 0;
    }
}

// --- خواندن پسورد ---
int readAdminPass(char* pass) {
    FILE* f = fopen(PASS_FILE, "r");
    if (!f) return 0;
    fscanf(f, "%s", pass);
    fclose(f);
    return 1;
}

// --- تغییر پسورد ---
int changeAdminPass(const char* newPass) {
    FILE* f = fopen(PASS_FILE, "w");
    if (!f) return 0;
    fprintf(f, "%s", newPass);
    fclose(f);
    return 1;
}

// --- خواندن نوع و قیمت اتاق‌ها ---
void loadRoomPrices() {
    FILE* f = fopen(PRICE_FILE, "r");
    if (!f) {
        printf("❌ Could not read prices file!\n");
        return;
    }

    int id, price;
    char type[20];
    while (fscanf(f, "%d %s %d", &id, type, &price) == 3) {
        hotel[id - 1].pricePerNight = price;
        strcpy(hotel[id - 1].type, type);
    }

    fclose(f);
}

// --- منوی مشتری ---
void customerMenu() {
    char name[MAX_NAME], phone[MAX_PHONE];
    int roomId;
    char date[11];

    printf("\n--- Customer Login ---\n");
    printf("Enter your phone: ");
    scanf(" %[^\n]", phone);

    printf("Enter your name: ");
    scanf(" %[^\n]", name);

    printf("Enter reservation date (YYYY-MM-DD): ");
    scanf(" %[^\n]", date);

    printf("\nAvailable rooms on %s:\n", date);
    for (int i = 0; i < MAX_ROOMS; i++) {
        if (strcmp(hotel[i].reservedDate, date) != 0) {
            printf("Room %d %s - $%d\n", hotel[i].id, hotel[i].type, hotel[i].pricePerNight);
        }
    }

    printf("\nEnter room ID to reserve: ");
    scanf("%d", &roomId);

    if (roomId < 1 || roomId > MAX_ROOMS) {
        printf("Invalid room ID!\n");
        return;
    }

    if (strcmp(hotel[roomId - 1].reservedDate, date) == 0) {
        printf("Room already reserved on this date!\n");
        return;
    }

    hotel[roomId - 1].reserved = 1;
    strcpy(hotel[roomId - 1].customerName, name);
    strcpy(hotel[roomId - 1].phone, phone);
    strcpy(hotel[roomId - 1].reservedDate, date);

    printf("\n✅ Room %d successfully reserved for %s on %s!\n", roomId, name, date);
}

// --- مدیریت اتاق‌ها بر اساس تاریخ ---
void manageRoomsByDate() {
    char date[11];
    printf("Enter date to manage rooms (YYYY-MM-DD): ");
    scanf(" %[^\n]", date);

    printf("\n--- Rooms on %s ---\n", date);
    for (int i = 0; i < MAX_ROOMS; i++) {
        if (strcmp(hotel[i].reservedDate, date) == 0) {
            printf("Room %d %s -> %s (%s) - $%d [Reserved]\n",
                hotel[i].id, hotel[i].type,
                hotel[i].customerName, hotel[i].phone,
                hotel[i].pricePerNight);
        }
        else {
            printf("Room %d %s - $%d [Free]\n",
                hotel[i].id, hotel[i].type, hotel[i].pricePerNight);
        }
    }

    int roomId, action;
    printf("\nEnter Room ID to modify (0 to cancel): ");
    scanf("%d", &roomId);
    if (roomId == 0) return;
    if (roomId < 1 || roomId > MAX_ROOMS) {
        printf("Invalid Room ID!\n");
        return;
    }

    printf("Choose action: 1. Mark as Free  2. Mark as Reserved\n");
    scanf("%d", &action);
    if (action == 1) {
        hotel[roomId - 1].reserved = 0;
        strcpy(hotel[roomId - 1].customerName, "");
        strcpy(hotel[roomId - 1].phone, "");
        strcpy(hotel[roomId - 1].reservedDate, "");
        printf("Room %d is now Free.\n", roomId);
    }
    else if (action == 2) {
        hotel[roomId - 1].reserved = 1;
        printf("Room %d is now Reserved (no customer info yet).\n", roomId);
    }
    else {
        printf("Invalid action!\n");
    }
}

// --- منوی ادمین ---
void adminMenu() {
    char pass[50], newPass[50];
    printf("\n--- Admin Login ---\n");
    printf("Enter password: ");
    scanf(" %[^\n]", pass);

    char correctPass[50];
    if (!readAdminPass(correctPass)) {
        printf("❌ Could not read password file!\n");
        return;
    }

    if (strcmp(pass, correctPass) != 0) {
        printf("❌ Wrong password!\n");
        return;
    }

    int choice;
    while (1) {
        printf("\n--- Admin Menu ---\n");
        printf("1. View Reservations\n");
        printf("2. Change Password\n");
        printf("3. Manage Rooms by Date\n");
        printf("4. Logout\n");
        printf("Choose: ");
        scanf("%d", &choice);

        switch (choice) {
        case 1:
            printf("\n--- Reservations List ---\n");
            for (int i = 0; i < MAX_ROOMS; i++) {
                if (hotel[i].reserved) {
                    printf("Room %d %s on %s -> %s (%s) - $%d\n",
                        hotel[i].id, hotel[i].type, hotel[i].reservedDate,
                        hotel[i].customerName, hotel[i].phone, hotel[i].pricePerNight);
                }
            }
            break;
        case 2:
            printf("Enter new password: ");
            scanf(" %[^\n]", newPass);
            if (changeAdminPass(newPass))
                printf("✅ Password changed successfully!\n");
            else
                printf("❌ Could not write to file!\n");
            break;
        case 3:
            manageRoomsByDate();
            break;
        case 4:
            return;
        default:
            printf("Invalid choice!\n");
        }
    }
}

// --- تابع اصلی ---
int main() {
    _mkdir("data");

    FILE* f = fopen(PASS_FILE, "r");
    if (!f) {
        f = fopen(PASS_FILE, "w");
        if (f) {
            fprintf(f, "admin123");
            fclose(f);
            printf("Admin password file created with default password: admin123\n");
        }
        else {
            printf("❌ Could not create password file!\n");
            return 1;
        }
    }
    else {
        fclose(f);
    }

    initRooms();
    loadRoomPrices();

    int choice;
    while (1) {
        printf("\n=== Hotel Reservation System ===\n");
        printf("1. Customer Login\n");
        printf("2. Admin Login\n");
        printf("3. Exit\n");
        printf("Choose option: ");
        scanf("%d", &choice);

        switch (choice) {
        case 1: customerMenu(); break;
        case 2: adminMenu(); break;
        case 3: exit(0);
        default: printf("Invalid choice!\n");
        }
    }

    return 0;
}
