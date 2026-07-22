#include "qosa_def.h"

#define QOS_LOG_TAG  LOG_TAG_COMPONENT

// Random number API declaration
int qosa_rand(void);

// Generate a random number from 0 to 99
int get_random_less_than_100(void) {
    int rand_val = qosa_rand();
    int result = rand_val % 100;  // Use modulo to get 0-99
    QLOGD("raw random number: %d, converted value: %d", rand_val, result);
    return result;
}

// Generate a random number from 1000 to 10000
int get_random_1000_to_10000(void) {
    int rand_val = qosa_rand();
    // Use modulo to get 0-9000 first, then add 1000
    int result = (rand_val % 9001) + 1000;  // 9001 = 10000 - 1000 + 1
    QLOGD("raw random number: %d, converted value: %d", rand_val, result);
    return result;
}

// Main demo function
void demo_random(void) {
    // Initialize log output
    qosa_log_control_set(QOSA_LOG_BIT_MASTER_ENABLE | QOSA_LOG_BIT_USB);
    
    QLOGI("===== random number generation demo =====");
    
    // Generate 10 random numbers less than 100
    QLOGI("generate 10 random numbers less than 100:");
    for (int i = 0; i < 10; i++) {
        int num = get_random_less_than_100();
        QLOGI("random number %d: %d", i + 1, num);
    }
    
    // Generate 10 random numbers from 1000 to 10000
    QLOGI("\ngenerate 10 random numbers from 1000 to 10000:");
    for (int i = 0; i < 10; i++) {
        int num = get_random_1000_to_10000();
        QLOGI("random number %d: %d", i + 1, num);
    }
    
    QLOGI("===== random number demo end =====");
}