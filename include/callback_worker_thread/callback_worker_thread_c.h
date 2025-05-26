#ifndef CALLBACK_WORKER_THREAD_C_H_
#define CALLBACK_WORKER_THREAD_C_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

/**
 * @file callback_worker_thread_c.h
 * @brief C language interface for CallbackWorkerThread
 * 
 * This file provides a C language interface for the CallbackWorkerThread library implemented in C++.
 */

/// Opaque pointer type
typedef struct CallbackWorkerThreadC CallbackWorkerThreadC;

/// Return status codes
typedef enum {
    CALLBACK_WORKER_SUCCESS = 0,           ///< Success
    CALLBACK_WORKER_ERROR_INVALID_PARAM,   ///< Invalid parameter
    CALLBACK_WORKER_ERROR_NULL_POINTER,    ///< NULL pointer error
    CALLBACK_WORKER_ERROR_THREAD_STOPPED,  ///< Thread pool is stopped
    CALLBACK_WORKER_ERROR_MEMORY,          ///< Out of memory
    CALLBACK_WORKER_ERROR_UNKNOWN          ///< Unknown error
} CallbackWorkerResult;

/// Default callback function type definition (int, double, const char*)
typedef void (*DefaultCallbackFunc)(int arg1, double arg2, const char* arg3);

/// No-argument callback function type definition
typedef void (*NoArgCallbackFunc)(void);

/// Single integer argument callback function type definition
typedef void (*IntCallbackFunc)(int arg);

/// Two integer arguments callback function type definition (with return value)
typedef int (*IntReturnCallbackFunc)(int arg1, int arg2);

/// Single string argument callback function type definition
typedef void (*StringCallbackFunc)(const char* arg);

/**
 * @brief Create CallbackWorkerThread instance
 * @param thread_count Number of worker threads (1 or more)
 * @param worker Address of variable to store the created instance pointer
 * @return CallbackWorkerResult Status code
 */
CallbackWorkerResult callback_worker_create(size_t thread_count, CallbackWorkerThreadC** worker);

/**
 * @brief Destroy CallbackWorkerThread instance
 * @param worker Instance to destroy
 * @return CallbackWorkerResult Status code
 */
CallbackWorkerResult callback_worker_destroy(CallbackWorkerThreadC* worker);

/**
 * @brief Enqueue default callback
 * @param worker Worker instance
 * @param callback Callback function
 * @param arg1 First argument
 * @param arg2 Second argument
 * @param arg3 Third argument
 * @return CallbackWorkerResult Status code
 */
CallbackWorkerResult callback_worker_enqueue_default(CallbackWorkerThreadC* worker,
                                                      DefaultCallbackFunc callback,
                                                      int arg1,
                                                      double arg2,
                                                      const char* arg3);

/**
 * @brief Enqueue no-argument callback
 * @param worker Worker instance
 * @param callback Callback function
 * @return CallbackWorkerResult Status code
 */
CallbackWorkerResult callback_worker_enqueue_no_arg(CallbackWorkerThreadC* worker,
                                                     NoArgCallbackFunc callback);

/**
 * @brief Enqueue single integer argument callback
 * @param worker Worker instance
 * @param callback Callback function
 * @param arg Argument
 * @return CallbackWorkerResult Status code
 */
CallbackWorkerResult callback_worker_enqueue_int(CallbackWorkerThreadC* worker,
                                                  IntCallbackFunc callback,
                                                  int arg);

/**
 * @brief Enqueue two integer arguments callback (with return value, synchronous execution)
 * @param worker Worker instance
 * @param callback Callback function
 * @param arg1 First argument
 * @param arg2 Second argument
 * @param result Address of variable to store the result
 * @return CallbackWorkerResult Status code
 */
CallbackWorkerResult callback_worker_enqueue_int_return_sync(CallbackWorkerThreadC* worker,
                                                             IntReturnCallbackFunc callback,
                                                             int arg1,
                                                             int arg2,
                                                             int* result);

/**
 * @brief Enqueue single string argument callback
 * @param worker Worker instance
 * @param callback Callback function
 * @param arg String argument
 * @return CallbackWorkerResult Status code
 */
CallbackWorkerResult callback_worker_enqueue_string(CallbackWorkerThreadC* worker,
                                                     StringCallbackFunc callback,
                                                     const char* arg);

/**
 * @brief Get number of worker threads
 * @param worker Worker instance
 * @param count Address of variable to store thread count
 * @return CallbackWorkerResult Status code
 */
CallbackWorkerResult callback_worker_get_thread_count(CallbackWorkerThreadC* worker,
                                                       size_t* count);

/**
 * @brief Get number of pending tasks
 * @param worker Worker instance
 * @param size Address of variable to store queue size
 * @return CallbackWorkerResult Status code
 */
CallbackWorkerResult callback_worker_get_queue_size(CallbackWorkerThreadC* worker,
                                                     size_t* size);

/**
 * @brief Stop thread pool
 * @param worker Worker instance
 * @return CallbackWorkerResult Status code
 */
CallbackWorkerResult callback_worker_stop(CallbackWorkerThreadC* worker);

/**
 * @brief Wait for all tasks to complete
 * @param worker Worker instance
 * @return CallbackWorkerResult Status code
 */
CallbackWorkerResult callback_worker_wait_completion(CallbackWorkerThreadC* worker);

/**
 * @brief Convert error code to string
 * @param result Error code
 * @return Error message string
 */
const char* callback_worker_result_to_string(CallbackWorkerResult result);

#ifdef __cplusplus
}
#endif

#endif  // CALLBACK_WORKER_THREAD_C_H_ 