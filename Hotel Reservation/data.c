#include <stdio.h>
#include <string.h>
#include "../headers/login.h"   // هدر لاگین رو میاریم
#include "login.h"

// تابع لاگین ادمین
int adminLogin() {
    char pass[50];
    printf("Enter admin password: ");
    scanf("%s", pass);

    if (strcmp(pass, ADMIN_PASSWORD) == 0) {
        printf("Login successful!\n");
        return 1; // موفق
    }
    else {
        printf("Wrong password!\n");
        return 0; // ناموفق
    }
}
