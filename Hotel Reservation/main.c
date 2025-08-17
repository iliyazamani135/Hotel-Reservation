#define _CRT_SECURE_NO_WARNINGS 0  // ما از نسخه‌های امن استفاده می‌کنیم، نیازی به این نیست ولی صفر می‌گذاریم تا اشتباهی فعال نشه.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ROOMS 50
#define MAX_NAME 50
#define MAX_PHONE 15
#define PASS_FILE "data/admin_pass.txt"
#define PRICE_FILE "data/prices.txt"

#ifndef _countof
#define _countof(a) (sizeof(a)/sizeof((a)[0]))
#endif

typedef struct {
    int id;
    int reserved;                // 1=رزرو شده برای reservedDate
    int reservationID;
    char customerName[MAX_NAME];
    char phone[MAX_PHONE];
    char reservedDate[11];       // YYYY-MM-DD
    char type[20];               // Single/Double/Suite
    int pricePerNight;
    char features[100];          // مثلا: "WiFi,TV,AC"
} Room;

static Room hotel[MAX_ROOMS];
static int nextReservationID = 1;

/* -------------------- Helpers -------------------- */
static void trim_newline(char* s) {
    if (!s) return;
    size_t n = strlen(s);
    if (n && (s[n - 1] == '\n' || s[n - 1] == '\r')) s[--n] = '\0';
    if (n && (s[n - 1] == '\r')) s[--n] = '\0';
}

static void read_line(const char* prompt, char* buf, size_t bufsz) {
    if (prompt && *prompt) printf("%s", prompt);
    if (fgets(buf, (int)bufsz, stdin) == NULL) { buf[0] = '\0'; return; }
    trim_newline(buf);
}

static int read_int(const char* prompt) {
    char line[64];
    read_line(prompt, line, _countof(line));
    char* end = NULL;
    long v = strtol(line, &end, 10);
    if (end == line) return -1;
    return (int)v;
}

/* -------------------- Initialize rooms -------------------- */
static void initRooms(void) {
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

/* -------------------- Admin password -------------------- */
static int readAdminPass(char* pass, size_t passsz) {
    FILE* f = NULL;
    if (fopen_s(&f, PASS_FILE, "r") != 0 || !f) return 0;
    if (!fgets(pass, (int)passsz, f)) { fclose(f); return 0; }
    fclose(f);
    trim_newline(pass);
    return 1;
}

static int changeAdminPass(const char* newPass) {
    FILE* f = NULL;
    if (fopen_s(&f, PASS_FILE, "w") != 0 || !f) return 0;
    fprintf(f, "%s", newPass);
    fclose(f);
    return 1;
}

/* -------------------- Load room prices (+features) -------------------- */
/* فرمت هر خط در prices.txt:
   id type price features...
   مثال:
   1 Single 100 WiFi,TV
   2 Double 150 WiFi,AC,Fridge
   3 Suite 300 WiFi,TV,AC,Balcony,PrivateBath
*/
static void loadRoomPrices(void) {
    FILE* f = NULL;
    if (fopen_s(&f, PRICE_FILE, "r") != 0 || !f) {
        printf("❌ Could not read prices file!\n");
        return;
    }

    char line[256];
    while (fgets(line, (int)_countof(line), f)) {
        trim_newline(line);
        if (!line[0]) continue;

        int id = 0, price = 0;
        char type[20] = { 0 };
        char features[100] = { 0 };

        // برای %s و %[ نیاز به اندازه با scanf_s داریم
        int matched = sscanf_s(
            line,
            "%d %19s %d %99[^\n]",
            &id,
            type, (unsigned)_countof(type),
            &price,
            features, (unsigned)_countof(features)
        );

        if (matched >= 3) {
            if (id >= 1 && id <= MAX_ROOMS) {
                hotel[id - 1].pricePerNight = price;
                strcpy_s(hotel[id - 1].type, _countof(hotel[id - 1].type), type);
                if (matched == 4) {
                    strcpy_s(hotel[id - 1].features, _countof(hotel[id - 1].features), features);
                }
                else {
                    hotel[id - 1].features[0] = '\0';
                }
            }
        }
    }
    fclose(f);
}

/* -------------------- Sort / Filter / Show -------------------- */
static void sortRoomsByPrice(Room rooms[], int n, int ascending) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = i + 1; j < n; j++) {
            int a = rooms[i].pricePerNight;
            int b = rooms[j].pricePerNight;
            if ((ascending && a > b) || (!ascending && a < b)) {
                Room tmp = rooms[i];
                rooms[i] = rooms[j];
                rooms[j] = tmp;
            }
        }
    }
}

static void showAvailableRooms(const char* date) {
    printf("\nAvailable rooms on %s:\n", date);
    for (int i = 0; i < MAX_ROOMS; i++) {
        if (strcmp(hotel[i].reservedDate, date) != 0) {
            printf("Room %d %s - $%d [%s]\n",
                hotel[i].id,
                hotel[i].type,
                hotel[i].pricePerNight,
                hotel[i].features[0] ? hotel[i].features : "No-Feature");
        }
    }
}

static void filterRoomsByFeature(Room rooms[], int n, const char* feature, const char* date) {
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

/* -------------------- Customer -------------------- */
static void customerMenu(void) {
    char name[MAX_NAME];
    char phone[MAX_PHONE];
    char date[11];

    printf("\n--- Customer Login ---\n");
    read_line("Enter your phone: ", phone, _countof(phone));
    read_line("Enter your name: ", name, _countof(name));
    read_line("Enter reservation date (YYYY-MM-DD): ", date, _countof(date));

    int choice = read_int(
        "\n1. Show all rooms\n"
        "2. Sort by Price (Low → High)\n"
        "3. Sort by Price (High → Low)\n"
        "4. Filter by Feature\n"
        "Choose: "
    );

    if (choice == 2) {
        sortRoomsByPrice(hotel, MAX_ROOMS, 1);
        showAvailableRooms(date);
    }
    else if (choice == 3) {
        sortRoomsByPrice(hotel, MAX_ROOMS, 0);
        showAvailableRooms(date);
    }
    else if (choice == 4) {
        char feature[50];
        read_line("Enter feature (e.g., WiFi,TV,AC): ", feature, _countof(feature));
        filterRoomsByFeature(hotel, MAX_ROOMS, feature, date);
    }
    else {
        showAvailableRooms(date);
    }

    int roomId = read_int("\nEnter room ID to reserve: ");
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
    strcpy_s(hotel[roomId - 1].customerName, _countof(hotel[roomId - 1].customerName), name);
    strcpy_s(hotel[roomId - 1].phone, _countof(hotel[roomId - 1].phone), phone);
    strcpy_s(hotel[roomId - 1].reservedDate, _countof(hotel[roomId - 1].reservedDate), date);

    printf("\n✅ Room %d reserved for %s on %s\n", roomId, name, date);
    printf("Your Reservation ID is: %d\n", hotel[roomId - 1].reservationID);
}

/* -------------------- View/Cancel -------------------- */
static void viewOrCancelReservations(void) {
    char phone[MAX_PHONE];
    read_line("Enter your phone to view reservations: ", phone, _countof(phone));

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

    int cancelID = read_int("Enter Reservation ID to cancel (0 to skip): ");
    if (cancelID == 0) return;

    for (int i = 0; i < MAX_ROOMS; i++) {
        if (hotel[i].reserved && hotel[i].reservationID == cancelID) {
            hotel[i].reserved = 0;
            hotel[i].reservationID = 0;
            hotel[i].customerName[0] = '\0';
            hotel[i].phone[0] = '\0';
            hotel[i].reservedDate[0] = '\0';
            printf("✅ Reservation canceled successfully.\n");
            return;
        }
    }
    printf("Reservation ID not found.\n");
}

/* -------------------- Admin -------------------- */
static void manageRoomsByDate(void) {
    char date[11];
    read_line("Enter date (YYYY-MM-DD): ", date, _countof(date));

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

static void searchReservations(void) {
    int choice = read_int("\nSearch by:\n1. Customer Name\n2. Phone\n3. Date\nChoose: ");

    int found = 0;
    if (choice == 1) {
        char name[MAX_NAME];
        read_line("Enter customer name: ", name, _countof(name));
        for (int i = 0; i < MAX_ROOMS; i++)
            if (hotel[i].reserved && strcmp(hotel[i].customerName, name) == 0) {
                printf("ID:%d Room:%d %s Date:%s Price:$%d\n",
                    hotel[i].reservationID, hotel[i].id,
                    hotel[i].type, hotel[i].reservedDate, hotel[i].pricePerNight);
                found = 1;
            }
    }
    else if (choice == 2) {
        char phone[MAX_PHONE];
        read_line("Enter phone: ", phone, _countof(phone));
        for (int i = 0; i < MAX_ROOMS; i++)
            if (hotel[i].reserved && strcmp(hotel[i].phone, phone) == 0) {
                printf("ID:%d Room:%d %s Date:%s Price:$%d\n",
                    hotel[i].reservationID, hotel[i].id,
                    hotel[i].type, hotel[i].reservedDate, hotel[i].pricePerNight);
                found = 1;
            }
    }
    else if (choice == 3) {
        char date[11];
        read_line("Enter date (YYYY-MM-DD): ", date, _countof(date));
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

static void revenueReport(void) {
    char date[11];
    read_line("Enter date for daily revenue (YYYY-MM-DD): ", date, _countof(date));

    int total = 0;
    for (int i = 0; i < MAX_ROOMS; i++)
        if (hotel[i].reserved && strcmp(hotel[i].reservedDate, date) == 0)
            total += hotel[i].pricePerNight;

    printf("Total revenue on %s: $%d\n", date, total);
}

static void adminMenu(void) {
    char pass[64];
    if (!readAdminPass(pass, _countof(pass))) { printf("❌ Cannot read admin pass!\n"); return; }

    char input[64];
    read_line("Enter admin password: ", input, _countof(input));
    if (strcmp(input, pass) != 0) { printf("❌ Wrong password!\n"); return; }

    int choice;
    do {
        choice = read_int(
            "\n--- Admin Menu ---\n"
            "1. Manage rooms by date\n"
            "2. Search Reservations\n"
            "3. Revenue Report\n"
            "4. Change Admin Pass\n"
            "5. Exit\n"
            "Choose: "
        );
        switch (choice) {
        case 1: manageRoomsByDate(); break;
        case 2: searchReservations(); break;
        case 3: revenueReport(); break;
        case 4: {
            char newPass[64];
            read_line("Enter new admin pass: ", newPass, _countof(newPass));
            if (changeAdminPass(newPass)) printf("✅ Password changed!\n"); else printf("❌ Failed!\n");
        } break;
        case 5: break;
        default: printf("Invalid choice!\n");
        }
    } while (choice != 5);
}

/* -------------------- Main -------------------- */
int main(void) {
    initRooms();
    loadRoomPrices();

    int choice;
    do {
        choice = read_int(
            "\n=== Hotel Reservation System ===\n"
            "1. Customer Login\n"
            "2. View/Cancel Reservations\n"
            "3. Admin Login\n"
            "4. Exit\n"
            "Choose: "
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
