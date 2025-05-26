#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
    // Windows environment uses Sleep function
    #include <windows.h>
    #define sleep(x) Sleep((x) * 1000)
#else
    // Unix-like environment uses unistd.h
    #include <unistd.h>
#endif

#include "callback_worker_thread/callback_worker_thread_c.h"

// Callback function implementations

// Default callback (int, double, const char*)
void default_callback(int id, double value, const char* message) {
    printf("  Default callback - ID: %d, Value: %.2f, Message: %s\n", 
           id, value, message);
}

// No argument callback
void no_arg_callback(void) {
    printf("  No argument callback executed\n");
}

// Integer argument callback
void int_callback(int arg) {
    printf("  Integer callback - Argument: %d\n", arg);
}

// Integer callback with return value
int int_return_callback(int a, int b) {
    int result = a + b;
    printf("  Calculation callback - %d + %d = %d\n", a, b, result);
    return result;
}

// String argument callback
void string_callback(const char* arg) {
    printf("  String callback - Argument: %s\n", arg);
}

// Error checking macro
#define CHECK_RESULT(result, operation) \
    do { \
        if ((result) != CALLBACK_WORKER_SUCCESS) { \
            printf("Error: %s - %s\n", (operation), \
                   callback_worker_result_to_string((result))); \
            return -1; \
        } \
    } while(0)

int main() {
    printf("C Language Interface Example\n");
    printf("============================\n\n");

    CallbackWorkerThreadC* worker = NULL;
    CallbackWorkerResult result;

    // 1. Create worker thread pool
    printf("1. Creating worker thread pool (3 threads):\n");
    result = callback_worker_create(3, &worker);
    CHECK_RESULT(result, "Worker creation");
    printf("  Worker thread pool created successfully\n\n");

    // 2. Get thread pool information
    printf("2. Thread pool information:\n");
    size_t thread_count, queue_size;
    
    result = callback_worker_get_thread_count(worker, &thread_count);
    CHECK_RESULT(result, "Get thread count");
    
    result = callback_worker_get_queue_size(worker, &queue_size);
    CHECK_RESULT(result, "Get queue size");
    
    printf("  Worker thread count: %zu\n", thread_count);
    printf("  Initial queue size: %zu\n\n", queue_size);

    // 3. Execute default callback
    printf("3. Default callback execution:\n");
    result = callback_worker_enqueue_default(worker, default_callback, 
                                             42, 3.14159, "Hello from C!");
    CHECK_RESULT(result, "Default callback enqueue");
    printf("  Default callback completed\n\n");

    // 4. Execute various callbacks
    printf("4. Various callback execution:\n");
    
    // No arguments
    result = callback_worker_enqueue_no_arg(worker, no_arg_callback);
    CHECK_RESULT(result, "No arg callback enqueue");
    
    // Integer argument
    result = callback_worker_enqueue_int(worker, int_callback, 100);
    CHECK_RESULT(result, "Int callback enqueue");
    
    // String argument
    result = callback_worker_enqueue_string(worker, string_callback, "Call from C");
    CHECK_RESULT(result, "String callback enqueue");
    
    printf("  Various callbacks completed\n\n");

    // 5. Callback with return value (synchronous execution)
    printf("5. Return value callback:\n");
    int calc_result;
    result = callback_worker_enqueue_int_return_sync(worker, int_return_callback, 
                                                     15, 27, &calc_result);
    CHECK_RESULT(result, "Return value callback enqueue");
    printf("  Received result: %d\n\n", calc_result);

    // 6. Execute multiple tasks in parallel
    printf("6. Parallel task execution:\n");
    for (int i = 0; i < 5; ++i) {
        char message[64];
        snprintf(message, sizeof(message), "Parallel task %d", i + 1);
        
        result = callback_worker_enqueue_string(worker, string_callback, message);
        CHECK_RESULT(result, "Parallel task enqueue");
    }
    printf("  Parallel tasks completed\n\n");

    // 7. Check queue size
    result = callback_worker_get_queue_size(worker, &queue_size);
    CHECK_RESULT(result, "Final queue size get");
    printf("7. Final queue size: %zu\n\n", queue_size);

    // 8. Stop and destroy worker thread pool
    printf("8. Worker thread pool stop and destroy:\n");
    result = callback_worker_stop(worker);
    CHECK_RESULT(result, "Worker stop");
    
    result = callback_worker_destroy(worker);
    CHECK_RESULT(result, "Worker destroy");
    
    printf("  Worker thread pool stopped and destroyed successfully\n\n");

    printf("All examples executed successfully.\n");
    return 0;
} 