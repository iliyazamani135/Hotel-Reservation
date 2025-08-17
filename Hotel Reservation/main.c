#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ROOMS 50
#define MAX_NAME 50
#define MAX_PHONE 15
#define PASS_FILE "data/admin_pass.txt"
#define PRICE_FILE "data/prices.txt"

typedef struct {
    int id;
    int reserved;
    int reservationID;
    char customerName[MAX_NAME];
    char phone[MAX_PHONE];
    char reservedDate[11];
    char type[20];
    int pricePerNight;
} Room;

Room hotel[MAX_ROOMS];
int nextReservationID = 1;

// --- Initialize rooms ---
void initRooms() {
    for (int i = 0; i < MAX_ROOMS; i++) {
        hotel[i].id = i + 1;
        hotel[i].reserved = 0;
        hotel[i].reservationID = 0;
        strcpy(hotel[i].customerName, "");
        strcpy(hotel[i].phone, "");
        strcpy(hotel[i].reservedDate, "");
        strcpy(hotel[i].type, "");
        hotel[i].pricePerNight = 0;
    }
}

// --- Admin password ---
int readAdminPass(char* pass) {
    FILE* f = fopen(PASS_FILE, "r");
    if (!f) return 0;
    fscanf(f, "%s", pass);
    fclose(f);
    return 1;
}

int changeAdminPass(const char* newPass) {
    FILE* f = fopen(PASS_FILE, "w");
    if (!f) return 0;
    fprintf(f, "%s", newPass);
    fclose(f);
    return 1;
}

// --- Load room prices ---
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

// --- Customer Reserve ---
void customerMenu() {
    char name[MAX_NAME], phone[MAX_PHONE], date[11];
    int roomId;

    printf("\n--- Customer Login ---\n");
    printf("Enter your phone: ");
    scanf(" %[^\n]", phone);
    printf("Enter your name: ");
    scanf(" %[^\n]", name);
    printf("Enter reservation date (YYYY-MM-DD): ");
    scanf(" %[^\n]", date);

    printf("\nAvailable rooms on %s:\n", date);
    for (int i = 0; i < MAX_ROOMS; i++) {
        if (strcmp(hotel[i].reservedDate, date) != 0)
            printf("Room %d %s - $%d\n", hotel[i].id, hotel[i].type, hotel[i].pricePerNight);
    }

    printf("\nEnter room ID to reserve: ");
    scanf("%d", &roomId);

    if (roomId < 1 || roomId > MAX_ROOMS) {
        printf("Invalid room ID!\n");
        return;
    }

    if (strcmp(hotel[roomId - 1].reservedDate, date) == 0) {
        printf("❌ Room already reserved on this date!\n");
        return;
    }

    hotel[roomId - 1].reserved = 1;
    hotel[roomId - 1].reservationID = nextReservationID++;
    strcpy(hotel[roomId - 1].customerName, name);
    strcpy(hotel[roomId - 1].phone, phone);
    strcpy(hotel[roomId - 1].reservedDate, date);

    printf("\n✅ Room %d reserved for %s on %s\n", roomId, name, date);
    printf("Your Reservation ID is: %d\n", hotel[roomId - 1].reservationID);
}

// --- View/Cancel Reservations ---
void viewOrCancelReservations() {
    char phone[MAX_PHONE];
    printf("Enter your phone to view reservations: ");
    scanf(" %[^\n]", phone);

    int found = 0;
    for (int i = 0; i < MAX_ROOMS; i++) {
        if (hotel[i].reserved && strcmp(hotel[i].phone, phone) == 0) {
            found = 1;
            printf("Reservation ID: %d, Room %d %s on %s\n",
                hotel[i].reservationID, hotel[i].id,
                hotel[i].type, hotel[i].reservedDate);
        }
    }
    if (!found) {
        printf("No reservations found for this phone.\n");
        return;
    }

    int cancelID;
    printf("Enter Reservation ID to cancel (0 to skip): ");
    scanf("%d", &cancelID);
    if (cancelID == 0) return;

    for (int i = 0; i < MAX_ROOMS; i++) {
        if (hotel[i].reserved && hotel[i].reservationID == cancelID) {
            hotel[i].reserved = 0;
            hotel[i].reservationID = 0;
            strcpy(hotel[i].customerName, "");
            strcpy(hotel[i].phone, "");
            strcpy(hotel[i].reservedDate, "");
            printf("✅ Reservation canceled successfully.\n");
            return;
        }
    }
    printf("Reservation ID not found.\n");
}

// --- Admin functions ---
void manageRoomsByDate() {
    char date[11];
    printf("Enter date (YYYY-MM-DD): ");
    scanf(" %[^\n]", date);

    printf("\n--- Rooms on %s ---\n", date);
    for (int i = 0; i < MAX_ROOMS; i++) {
        if (strcmp(hotel[i].reservedDate, date) == 0)
            printf("Room %d %s -> %s (%s) - $%d [Reserved]\n",
                hotel[i].id, hotel[i].type, hotel[i].customerName,
                hotel[i].phone, hotel[i].pricePerNight);
        else
            printf("Room %d %s - $%d [Free]\n", hotel[i].id, hotel[i].type, hotel[i].pricePerNight);
    }
}

// --- Search Reservations ---
void searchReservations() {
    int choice;
    char name[MAX_NAME], phone[MAX_PHONE], date[11];
    printf("\nSearch by:\n1. Customer Name\n2. Phone\n3. Date\nChoose: ");
    scanf("%d", &choice);

    int found = 0;
    if (choice == 1) {
        printf("Enter customer name: "); scanf(" %[^\n]", name);
        for (int i = 0; i < MAX_ROOMS; i++)
            if (hotel[i].reserved && strcmp(hotel[i].customerName, name) == 0) {
                printf("ID:%d Room:%d %s Date:%s Price:$%d\n",
                    hotel[i].reservationID, hotel[i].id,
                    hotel[i].type, hotel[i].reservedDate, hotel[i].pricePerNight);
                found = 1;
            }
    }
    else if (choice == 2) {
        printf("Enter phone: "); scanf(" %[^\n]", phone);
        for (int i = 0; i < MAX_ROOMS; i++)
            if (hotel[i].reserved && strcmp(hotel[i].phone, phone) == 0) {
                printf("ID:%d Room:%d %s Date:%s Price:$%d\n",
                    hotel[i].reservationID, hotel[i].id,
                    hotel[i].type, hotel[i].reservedDate, hotel[i].pricePerNight);
                found = 1;
            }
    }
    else if (choice == 3) {
        printf("Enter date (YYYY-MM-DD): "); scanf(" %[^\n]", date);
        for (int i = 0; i < MAX_ROOMS; i++)
            if (hotel[i].reserved && strcmp(hotel[i].reservedDate, date) == 0) {
                printf("ID:%d Room:%d %s Customer:%s Phone:%s Price:$%d\n",
                    hotel[i].reservationID, hotel[i].id,
                    hotel[i].type, hotel[i].customerName, hotel[i].phone, hotel[i].pricePerNight);
                found = 1;
            }
    }
    if (!found) printf("No reservations found.\n");
}

// --- Revenue Report ---
void revenueReport() {
    char date[11];
    printf("Enter date for daily revenue (YYYY-MM-DD): ");
    scanf(" %[^\n]", date);

    int total = 0;
    for (int i = 0; i < MAX_ROOMS; i++)
        if (hotel[i].reserved && strcmp(hotel[i].reservedDate, date) == 0)
            total += hotel[i].pricePerNight;

    printf("Total revenue on %s: $%d\n", date, total);
}

// --- Admin Menu ---
void adminMenu() {
    char pass[50]; if (!readAdminPass(pass)) { printf("❌ Cannot read admin pass!\n"); return; }
    char input[50]; printf("Enter admin password: "); scanf(" %[^\n]", input);
    if (strcmp(input, pass) != 0) { printf("❌ Wrong password!\n"); return; }

    int choice;
    do {
        printf("\n--- Admin Menu ---\n");
        printf("1. Manage rooms by date\n2. Search Reservations\n3. Revenue Report\n4. Change Admin Pass\n5. Exit\nChoose: ");
        scanf("%d", &choice);
        switch (choice) {
        case 1: manageRoomsByDate(); break;
        case 2: searchReservations(); break;
        case 3: revenueReport(); break;
        case 4: {
            char newPass[50]; printf("Enter new admin pass: "); scanf(" %[^\n]", newPass);
            if (changeAdminPass(newPass)) printf("✅ Password changed!\n"); else printf("❌ Failed!\n");
        } break;
        case 5: break;
        default: printf("Invalid choice!\n");
        }
    } while (choice != 5);
}

// --- Main Menu ---
int main() {
    initRooms();
    loadRoomPrices();
    int choice;
    do {
        printf("\n=== Hotel Reservation System ===\n");
        printf("1. Customer Login\n2. View/Cancel Reservations\n3. Admin Login\n4. Exit\nChoose: ");
        scanf("%d", &choice);
        switch (choice) {
        case 1: customerMenu(); break;
        case 2: viewOrCancelReservations(); break;
        case 3: adminMenu(); break;
        case 4: printf("Goodbye!\n"); break;
        default: printf("Invalid choice!\n");
        }
    } while (choice != 4);
    return 0;
}
