#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ROOMS 50
#define MAX_NAME 50
#define MAX_PHONE 15
#define PASS_FILE "admin_pass.txt"
#define ROOM_FILE "room.txt"
#define BOOKING_FILE "booking.txt"

typedef struct {
    int id;
    int reserved;
    int reservationID;
    char customerName[MAX_NAME];
    char phone[MAX_PHONE];
    char reservedDate[11];
    char type[20];
    int pricePerNight;
    char features[100];
} Room;

Room hotel[MAX_ROOMS];
int nextReservationID = 1;

// Helper functions
void trim_newline(char* s) {
    if (!s) return;
    size_t n = strlen(s);
    while (n && (s[n - 1] == '\n' || s[n - 1] == '\r')) s[--n] = '\0';
}

void read_line(const char* prompt, char* buf, size_t bufsz) {
    if (prompt && *prompt) printf("%s", prompt);
    if (fgets(buf, (int)bufsz, stdin) == NULL) { buf[0] = '\0'; return; }
    trim_newline(buf);
}

int read_int(const char* prompt) {
    char line[64];
    read_line(prompt, line, sizeof(line));
    char* end = NULL;
    long v = strtol(line, &end, 10);
    if (end == line) return -1;
    return (int)v;
}

// Initialize rooms
void initRooms() {
    for (int i = 0; i < MAX_ROOMS; i++) {
        hotel[i].id = i + 1;
        hotel[i].reserved = 0;
        hotel[i].reservationID = 0;
        hotel[i].customerName[0] = '\0';
        hotel[i].phone[0] = '\0';
        hotel[i].reservedDate[0] = '\0';
        hotel[i].type[0] = '\0';
        hotel[i].features[0] = '\0';
        hotel[i].pricePerNight = 0;
    }
}

// Admin password
int readAdminPass(char* pass, size_t passsz) {
    FILE* f = fopen(PASS_FILE, "r");
    if (!f) return 0;
    if (!fgets(pass, (int)passsz, f)) { fclose(f); return 0; }
    fclose(f);
    trim_newline(pass);
    return 1;
}

int changeAdminPass(const char* newPass) {
    FILE* f = fopen(PASS_FILE, "w");
    if (!f) return 0;
    fprintf(f, "%s", newPass);
    fclose(f);
    return 1;
}

// Load rooms from file
void loadRooms() {
    FILE* f = fopen(ROOM_FILE, "r");
    if (!f) {
        printf("❌ Could not read rooms file!\n");
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        trim_newline(line);
        if (!line[0]) continue;

        int id = 0, price = 0;
        char type[20] = { 0 };
        char features[100] = { 0 };

        int matched = sscanf(line, "%d %19s %d %99[^\n]", &id, type, &price, features);
        if (matched >= 3 && id >= 1 && id <= MAX_ROOMS) {
            hotel[id - 1].id = id;
            hotel[id - 1].pricePerNight = price;
            strcpy(hotel[id - 1].type, type);
            if (matched == 4) strcpy(hotel[id - 1].features, features);
            else hotel[id - 1].features[0] = '\0';
        }
    }
    fclose(f);
}

// Save booking to file
void saveBookingToFile(Room r) {
    FILE* f = fopen(BOOKING_FILE, "a");
    if (!f) {
        printf("❌ Could not open booking file!\n");
        return;
    }
    fprintf(f, "%d,%d,%s,%s,%s\n",
        r.reservationID, r.id, r.customerName, r.phone, r.reservedDate);
    fclose(f);
}

// Load bookings from file
void loadBookings() {
    FILE* f = fopen(BOOKING_FILE, "r");
    if (!f) return;

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        trim_newline(line);
        if (!line[0]) continue;

        Room r;
        if (sscanf(line, "%d,%d,%49[^,],%14[^,],%10s",
            &r.reservationID, &r.id,
            r.customerName, r.phone, r.reservedDate) == 5) {
            int idx = r.id - 1;
            hotel[idx].reserved = 1;
            hotel[idx].reservationID = r.reservationID;
            strcpy(hotel[idx].customerName, r.customerName);
            strcpy(hotel[idx].phone, r.phone);
            strcpy(hotel[idx].reservedDate, r.reservedDate);

            if (r.reservationID >= nextReservationID)
                nextReservationID = r.reservationID + 1;
        }
    }
    fclose(f);
}

// Sort and show
void sortRoomsByPrice(Room rooms[], int n, int ascending) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = i + 1; j < n; j++) {
            if ((ascending && rooms[i].pricePerNight > rooms[j].pricePerNight) ||
                (!ascending && rooms[i].pricePerNight < rooms[j].pricePerNight)) {
                Room tmp = rooms[i]; rooms[i] = rooms[j]; rooms[j] = tmp;
            }
        }
    }
}

void showAvailableRooms(const char* date) {
    printf("\nAvailable rooms on %s:\n", date);
    for (int i = 0; i < MAX_ROOMS; i++) {
        if (strcmp(hotel[i].reservedDate, date) != 0) {
            printf("Room %d %s - $%d [%s]\n",
                hotel[i].id, hotel[i].type, hotel[i].pricePerNight,
                hotel[i].features[0] ? hotel[i].features : "No-Feature");
        }
    }
}

void filterRoomsByFeature(Room rooms[], int n, const char* feature, const char* date) {
    printf("\nRooms with \"%s\" on %s:\n", feature, date);
    int found = 0;
    for (int i = 0; i < n; i++) {
        if (strcmp(rooms[i].reservedDate, date) != 0 &&
            rooms[i].features[0] &&
            strstr(rooms[i].features, feature) != NULL) {
            printf("Room %d %s - $%d [%s]\n",
                rooms[i].id, rooms[i].type,
                rooms[i].pricePerNight, rooms[i].features);
            found = 1;
        }
    }
    if (!found) printf("❌ No rooms with %s found.\n", feature);
}

// Customer
void customerMenu() {
    char name[MAX_NAME], phone[MAX_PHONE], date[11];
    read_line("Enter your phone: ", phone, sizeof(phone));
    read_line("Enter your name: ", name, sizeof(name));
    read_line("Enter reservation date (YYYY-MM-DD): ", date, sizeof(date));

    int choice = read_int(
        "\n1. Show all rooms\n2. Sort by Price (Low → High)\n3. Sort by Price (High → Low)\n4. Filter by Feature\nChoose: "
    );

    if (choice == 2) { sortRoomsByPrice(hotel, MAX_ROOMS, 1); showAvailableRooms(date); }
    else if (choice == 3) { sortRoomsByPrice(hotel, MAX_ROOMS, 0); showAvailableRooms(date); }
    else if (choice == 4) {
        char feature[50];
        read_line("Enter feature (e.g., WiFi,TV,AC): ", feature, sizeof(feature));
        filterRoomsByFeature(hotel, MAX_ROOMS, feature, date);
    }
    else { showAvailableRooms(date); }

    int roomId = read_int("\nEnter room ID to reserve: ");
    if (roomId < 1 || roomId > MAX_ROOMS) { printf("Invalid room ID!\n"); return; }
    if (strcmp(hotel[roomId - 1].reservedDate, date) == 0) {
        printf("❌ Room already reserved on this date!\n"); return;
    }

    hotel[roomId - 1].reserved = 1;
    hotel[roomId - 1].reservationID = nextReservationID++;
    strcpy(hotel[roomId - 1].customerName, name);
    strcpy(hotel[roomId - 1].phone, phone);
    strcpy(hotel[roomId - 1].reservedDate, date);

    saveBookingToFile(hotel[roomId - 1]);

    printf("\n✅ Room %d reserved for %s on %s\nYour Reservation ID is: %d\n",
        roomId, name, date, hotel[roomId - 1].reservationID);
}

// View/Cancel
void viewOrCancelReservations() {
    char phone[MAX_PHONE];
    read_line("Enter your phone to view reservations: ", phone, sizeof(phone));

    int found = 0;
    for (int i = 0; i < MAX_ROOMS; i++) {
        if (hotel[i].reserved && strcmp(hotel[i].phone, phone) == 0) {
            found = 1;
            printf("Reservation ID: %d, Room %d %s on %s\n",
                hotel[i].reservationID, hotel[i].id,
                hotel[i].type, hotel[i].reservedDate);
        }
    }
    if (!found) { printf("No reservations found for this phone.\n"); return; }

    int cancelID = read_int("Enter Reservation ID to cancel (0 to skip): ");
    if (cancelID == 0) return;

    FILE* f = fopen(BOOKING_FILE, "r");
    FILE* temp = fopen("temp.txt", "w");

    for (int i = 0; i < MAX_ROOMS; i++) {
        if (hotel[i].reserved && hotel[i].reservationID == cancelID) {
            hotel[i].reserved = 0; hotel[i].reservationID = 0;
            hotel[i].customerName[0] = '\0'; hotel[i].phone[0] = '\0'; hotel[i].reservedDate[0] = '\0';
            printf("✅ Reservation canceled successfully.\n");
        }
        else if (hotel[i].reserved) {
            fprintf(temp, "%d,%d,%s,%s,%s\n",
                hotel[i].reservationID, hotel[i].id,
                hotel[i].customerName, hotel[i].phone, hotel[i].reservedDate);
        }
    }
    if (f) fclose(f);
    if (temp) { fclose(temp); remove(BOOKING_FILE); rename("temp.txt", BOOKING_FILE); }
}

// Admin
void manageRoomsByDate() {
    char date[11];
    read_line("Enter date (YYYY-MM-DD): ", date, sizeof(date));

    printf("\n--- Rooms on %s ---\n", date);
    for (int i = 0; i < MAX_ROOMS; i++) {
        if (strcmp(hotel[i].reservedDate, date) == 0)
            printf("Room %d %s -> %s (%s) - $%d [%s] [Reserved]\n",
                hotel[i].id, hotel[i].type, hotel[i].customerName,
                hotel[i].phone, hotel[i].pricePerNight,
                hotel[i].features[0] ? hotel[i].features : "No-Feature");
        else
            printf("Room %d %s - $%d [%s] [Free]\n",
                hotel[i].id, hotel[i].type, hotel[i].pricePerNight,
                hotel[i].features[0] ? hotel[i].features : "No-Feature");
    }
}

void searchReservations() {
    int choice = read_int("\nSearch by:\n1. Customer Name\n2. Phone\n3. Date\nChoose: ");
    int found = 0;
    if (choice == 1) {
        char name[MAX_NAME]; read_line("Enter customer name: ", name, sizeof(name));
        for (int i = 0; i < MAX_ROOMS; i++)
            if (hotel[i].reserved && strcmp(hotel[i].customerName, name) == 0) {
                printf("ID:%d Room:%d %s Date:%s Price:$%d\n",
                    hotel[i].reservationID, hotel[i].id,
                    hotel[i].type, hotel[i].reservedDate, hotel[i].pricePerNight);
                found = 1;
            }
    }
    else if (choice == 2) {
        char phone[MAX_PHONE]; read_line("Enter phone: ", phone, sizeof(phone));
        for (int i = 0; i < MAX_ROOMS; i++)
            if (hotel[i].reserved && strcmp(hotel[i].phone, phone) == 0) {
                printf("ID:%d Room:%d %s Date:%s Price:$%d\n",
                    hotel[i].reservationID, hotel[i].id,
                    hotel[i].type, hotel[i].reservedDate, hotel[i].pricePerNight);
                found = 1;
            }
    }
    else if (choice == 3) {
        char date[11]; read_line("Enter date (YYYY-MM-DD): ", date, sizeof(date));
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

void revenueReport() {
    char date[11];
    read_line("Enter date for daily revenue (YYYY-MM-DD): ", date, sizeof(date));
    int total = 0;
    for (int i = 0; i < MAX_ROOMS; i++)
        if (hotel[i].reserved && strcmp(hotel[i].reservedDate, date) == 0)
            total += hotel[i].pricePerNight;
    printf("Total revenue on %s: $%d\n", date, total);
}

void adminMenu() {
    char pass[64];
    if (!readAdminPass(pass, sizeof(pass))) { printf("❌ Cannot read admin pass!\n"); return; }

    char input[64]; read_line("Enter admin password: ", input, sizeof(input));
    if (strcmp(input, pass) != 0) { printf("❌ Wrong password!\n"); return; }

    int choice;
    do {
        choice = read_int(
            "\n--- Admin Menu ---\n1. Manage rooms by date\n2. Search Reservations\n3. Revenue Report\n4. Change Admin Pass\n5. Exit\nChoose: "
        );
        switch (choice) {
        case 1: manageRoomsByDate(); break;
        case 2: searchReservations(); break;
        case 3: revenueReport(); break;
        case 4: {
            char newPass[64]; read_line("Enter new admin pass: ", newPass, sizeof(newPass));
            if (changeAdminPass(newPass)) printf("✅ Password changed!\n"); else printf("❌ Failed!\n");
        } break;
        case 5: break;
        default: printf("Invalid choice!\n");
        }
    } while (choice != 5);
}

// Main
int main() {
    initRooms();
    loadRooms();
    loadBookings();

    int choice;
    do {
        choice = read_int(
            "\n=== Hotel Reservation System ===\n"
            "1. Customer Login\n2. View/Cancel Reservations\n3. Admin Login\n4. Exit\nChoose: "
        );
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
