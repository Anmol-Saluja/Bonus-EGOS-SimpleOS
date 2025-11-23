#include "app.h"
int main() {
    printf("[CPU TEST] Starting CPU-intensive task\n");
    
    // Reduced loop - runs for ~1-2 seconds
    volatile unsigned long long counter = 0;
    for (unsigned long i = 0; i < 100000000; i++) {  // 10M instead of 100M
        counter += i;
        if (i % 1000000 == 0) {
            printf("[CPU TEST] Iteration %d\n", i / 1000000);
        }
    }
    
    printf("[CPU TEST] Completed. Counter = %d\n", counter/1000000);
    return 0;
}
