#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "callback_worker_thread/callback_worker_thread_c.h"

// Global variables for testing
static int g_callback_count = 0;
static int g_last_int_value = 0;
static double g_last_double_value = 0.0;
static char g_last_string_value[256] = {0};
static int g_last_return_value = 0;

// Test callback functions
void test_default_callback(int arg1, double arg2, const char* arg3) {
    g_callback_count++;
    g_last_int_value = arg1;
    g_last_double_value = arg2;
    strncpy(g_last_string_value, arg3, sizeof(g_last_string_value) - 1);
    g_last_string_value[sizeof(g_last_string_value) - 1] = '\0';
}

void test_no_arg_callback(void) {
    g_callback_count++;
}

void test_int_callback(int arg) {
    g_callback_count++;
    g_last_int_value = arg;
}

int test_int_return_callback(int arg1, int arg2) {
    g_callback_count++;
    g_last_return_value = arg1 + arg2;
    return g_last_return_value;
}

void test_string_callback(const char* arg) {
    g_callback_count++;
    strncpy(g_last_string_value, arg, sizeof(g_last_string_value) - 1);
    g_last_string_value[sizeof(g_last_string_value) - 1] = '\0';
}

// Reset test state
void reset_test_state(void) {
    g_callback_count = 0;
    g_last_int_value = 0;
    g_last_double_value = 0.0;
    memset(g_last_string_value, 0, sizeof(g_last_string_value));
    g_last_return_value = 0;
}

// Assertion macros
#define ASSERT_EQ(expected, actual) \
    do { \
        if ((expected) != (actual)) { \
            printf("FAILED: %s:%d - Expected %d, got %d\n", \
                   __FILE__, __LINE__, (int)(expected), (int)(actual)); \
            return 0; \
        } \
    } while(0)

#define ASSERT_DOUBLE_EQ(expected, actual, tolerance) \
    do { \
        double diff = (expected) - (actual); \
        if (diff < 0) diff = -diff; \
        if (diff > (tolerance)) { \
            printf("FAILED: %s:%d - Expected %.6f, got %.6f\n", \
                   __FILE__, __LINE__, (expected), (actual)); \
            return 0; \
        } \
    } while(0)

#define ASSERT_STR_EQ(expected, actual) \
    do { \
        if (strcmp((expected), (actual)) != 0) { \
            printf("FAILED: %s:%d - Expected '%s', got '%s'\n", \
                   __FILE__, __LINE__, (expected), (actual)); \
            return 0; \
        } \
    } while(0)

#define ASSERT_SUCCESS(result) \
    do { \
        if ((result) != CALLBACK_WORKER_SUCCESS) { \
            printf("FAILED: %s:%d - Expected SUCCESS, got %s\n", \
                   __FILE__, __LINE__, callback_worker_result_to_string(result)); \
            return 0; \
        } \
    } while(0)

// Test functions

int test_create_destroy(void) {
    printf("Running test_create_destroy...\n");
    
    CallbackWorkerThreadC* worker = NULL;
    CallbackWorkerResult result;
    
    // Normal case
    result = callback_worker_create(1, &worker);
    ASSERT_SUCCESS(result);
    assert(worker != NULL);
    
    size_t thread_count;
    result = callback_worker_get_thread_count(worker, &thread_count);
    ASSERT_SUCCESS(result);
    ASSERT_EQ(1, thread_count);
    
    result = callback_worker_destroy(worker);
    ASSERT_SUCCESS(result);
    
    // Error cases
    result = callback_worker_create(0, &worker);
    ASSERT_EQ(CALLBACK_WORKER_ERROR_INVALID_PARAM, result);
    
    result = callback_worker_create(1, NULL);
    ASSERT_EQ(CALLBACK_WORKER_ERROR_NULL_POINTER, result);
    
    result = callback_worker_destroy(NULL);
    ASSERT_EQ(CALLBACK_WORKER_ERROR_NULL_POINTER, result);
    
    printf("  PASSED\n");
    return 1;
}

int test_default_callback_execution(void) {
    printf("Running test_default_callback_execution...\n");
    
    reset_test_state();
    
    CallbackWorkerThreadC* worker = NULL;
    CallbackWorkerResult result;
    
    result = callback_worker_create(1, &worker);
    ASSERT_SUCCESS(result);
    
    result = callback_worker_enqueue_default(worker, test_default_callback, 
                                             42, 3.14159, "test message");
    ASSERT_SUCCESS(result);
    
    // Verify that the callback was executed
    ASSERT_EQ(1, g_callback_count);
    ASSERT_EQ(42, g_last_int_value);
    ASSERT_DOUBLE_EQ(3.14159, g_last_double_value, 0.00001);
    ASSERT_STR_EQ("test message", g_last_string_value);
    
    result = callback_worker_destroy(worker);
    ASSERT_SUCCESS(result);
    
    printf("  PASSED\n");
    return 1;
}

int test_various_callbacks(void) {
    printf("Running test_various_callbacks...\n");
    
    reset_test_state();
    
    CallbackWorkerThreadC* worker = NULL;
    CallbackWorkerResult result;
    
    result = callback_worker_create(2, &worker);
    ASSERT_SUCCESS(result);
    
    // No argument callback
    result = callback_worker_enqueue_no_arg(worker, test_no_arg_callback);
    ASSERT_SUCCESS(result);
    ASSERT_EQ(1, g_callback_count);
    
    // Integer callback
    result = callback_worker_enqueue_int(worker, test_int_callback, 100);
    ASSERT_SUCCESS(result);
    ASSERT_EQ(2, g_callback_count);
    ASSERT_EQ(100, g_last_int_value);
    
    // String callback
    result = callback_worker_enqueue_string(worker, test_string_callback, "hello");
    ASSERT_SUCCESS(result);
    ASSERT_EQ(3, g_callback_count);
    ASSERT_STR_EQ("hello", g_last_string_value);
    
    result = callback_worker_destroy(worker);
    ASSERT_SUCCESS(result);
    
    printf("  PASSED\n");
    return 1;
}

int test_return_value_callback(void) {
    printf("Running test_return_value_callback...\n");
    
    reset_test_state();
    
    CallbackWorkerThreadC* worker = NULL;
    CallbackWorkerResult result;
    
    result = callback_worker_create(1, &worker);
    ASSERT_SUCCESS(result);
    
    int calc_result;
    result = callback_worker_enqueue_int_return_sync(worker, test_int_return_callback, 
                                                     10, 20, &calc_result);
    ASSERT_SUCCESS(result);
    ASSERT_EQ(1, g_callback_count);
    ASSERT_EQ(30, calc_result);
    ASSERT_EQ(30, g_last_return_value);
    
    result = callback_worker_destroy(worker);
    ASSERT_SUCCESS(result);
    
    printf("  PASSED\n");
    return 1;
}

int test_queue_size(void) {
    printf("Running test_queue_size...\n");
    
    CallbackWorkerThreadC* worker = NULL;
    CallbackWorkerResult result;
    
    result = callback_worker_create(1, &worker);
    ASSERT_SUCCESS(result);
    
    size_t queue_size;
    result = callback_worker_get_queue_size(worker, &queue_size);
    ASSERT_SUCCESS(result);
    ASSERT_EQ(0, queue_size);
    
    result = callback_worker_destroy(worker);
    ASSERT_SUCCESS(result);
    
    printf("  PASSED\n");
    return 1;
}

int test_error_handling(void) {
    printf("Running test_error_handling...\n");
    
    CallbackWorkerThreadC* worker = NULL;
    CallbackWorkerResult result;
    
    result = callback_worker_create(1, &worker);
    ASSERT_SUCCESS(result);
    
    // NULL pointer test
    result = callback_worker_enqueue_default(NULL, test_default_callback, 1, 1.0, "test");
    ASSERT_EQ(CALLBACK_WORKER_ERROR_NULL_POINTER, result);
    
    result = callback_worker_enqueue_default(worker, NULL, 1, 1.0, "test");
    ASSERT_EQ(CALLBACK_WORKER_ERROR_NULL_POINTER, result);
    
    result = callback_worker_enqueue_default(worker, test_default_callback, 1, 1.0, NULL);
    ASSERT_EQ(CALLBACK_WORKER_ERROR_NULL_POINTER, result);
    
    size_t count;
    result = callback_worker_get_thread_count(NULL, &count);
    ASSERT_EQ(CALLBACK_WORKER_ERROR_NULL_POINTER, result);
    
    result = callback_worker_get_thread_count(worker, NULL);
    ASSERT_EQ(CALLBACK_WORKER_ERROR_NULL_POINTER, result);
    
    result = callback_worker_destroy(worker);
    ASSERT_SUCCESS(result);
    
    printf("  PASSED\n");
    return 1;
}

int test_error_string_conversion(void) {
    printf("Running test_error_string_conversion...\n");
    
    const char* msg;
    
    msg = callback_worker_result_to_string(CALLBACK_WORKER_SUCCESS);
    assert(msg != NULL);
    assert(strlen(msg) > 0);
    
    msg = callback_worker_result_to_string(CALLBACK_WORKER_ERROR_NULL_POINTER);
    assert(msg != NULL);
    assert(strlen(msg) > 0);
    
    msg = callback_worker_result_to_string(CALLBACK_WORKER_ERROR_INVALID_PARAM);
    assert(msg != NULL);
    assert(strlen(msg) > 0);
    
    printf("  PASSED\n");
    return 1;
}

int main(void) {
    printf("Starting C Interface Tests\n");
    printf("============================\n\n");
    
    int passed = 0;
    int total = 0;
    
    total++; if (test_create_destroy()) passed++;
    total++; if (test_default_callback_execution()) passed++;
    total++; if (test_various_callbacks()) passed++;
    total++; if (test_return_value_callback()) passed++;
    total++; if (test_queue_size()) passed++;
    total++; if (test_error_handling()) passed++;
    total++; if (test_error_string_conversion()) passed++;
    
    printf("\n============================\n");
    printf("Test Results: %d/%d passed\n", passed, total);
    
    if (passed == total) {
        printf("All tests passed successfully!\n");
        return 0;
    } else {
        printf("Some tests failed.\n");
        return 1;
    }
} 