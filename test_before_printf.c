#include "include/malloc.h"
#include <unistd.h>

// Don't use printf until we check for leaks!

int main(void) {
    // Check leaks BEFORE any printf
    int leaks_before_printf = check_malloc_leaks();
    
    // Now we can use printf
    write(STDOUT_FILENO, "=== Testing leak before printf ===\n", 36);
    write(STDOUT_FILENO, "Leaks BEFORE printf: ", 21);
    
    // Print the number (manually convert to string)
    char buf[20];
    int i = 0;
    int n = leaks_before_printf;
    if (n == 0) {
        buf[i++] = '0';
    } else {
        while (n > 0) {
            buf[i++] = '0' + (n % 10);
            n /= 10;
        }
        // Reverse
        for (int j = 0; j < i / 2; j++) {
            char tmp = buf[j];
            buf[j] = buf[i - 1 - j];
            buf[i - 1 - j] = tmp;
        }
    }
    buf[i++] = '\n';
    write(STDOUT_FILENO, buf, i);
    
    // NOW use printf for the first time
    write(STDOUT_FILENO, "\nNow calling printf for first time...\n", 39);
    
    // Use printf
    #include <stdio.h>
    printf("Hello from printf!\n");
    
    // Check leaks AFTER printf
    int leaks_after_printf = check_malloc_leaks();
    write(STDOUT_FILENO, "Leaks AFTER printf: ", 20);
    
    i = 0;
    n = leaks_after_printf;
    if (n == 0) {
        buf[i++] = '0';
    } else {
        while (n > 0) {
            buf[i++] = '0' + (n % 10);
            n /= 10;
        }
        for (int j = 0; j < i / 2; j++) {
            char tmp = buf[j];
            buf[j] = buf[i - 1 - j];
            buf[i - 1 - j] = tmp;
        }
    }
    buf[i++] = '\n';
    write(STDOUT_FILENO, buf, i);
    
    write(STDOUT_FILENO, "\n=== RESULT ===\n", 16);
    if (leaks_before_printf == 0 && leaks_after_printf > 0) {
        write(STDOUT_FILENO, "CONFIRMED: printf() causes the leak!\n", 38);
    } else if (leaks_before_printf > 0) {
        write(STDOUT_FILENO, "Leak exists BEFORE printf - something else causes it\n", 54);
    } else {
        write(STDOUT_FILENO, "No leaks detected at all\n", 26);
    }
    
    return 0;
}
