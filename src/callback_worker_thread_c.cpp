#include "callback_worker_thread/callback_worker_thread_c.h"
#include "callback_worker_thread/callback_worker_thread.h"

#include <cstring>
#include <new>
#include <stdexcept>

using namespace callback_worker_thread;

// Structure to manage C++ object as an opaque pointer
struct CallbackWorkerThreadC {
  CallbackWorkerThread* worker;
  
  explicit CallbackWorkerThreadC(size_t thread_count) 
      : worker(new(std::nothrow) CallbackWorkerThread(thread_count)) {}
  
  ~CallbackWorkerThreadC() {
    delete worker;
  }
};

extern "C" {

CallbackWorkerResult callback_worker_create(size_t thread_count, CallbackWorkerThreadC** worker) {
  if (worker == nullptr) {
    return CALLBACK_WORKER_ERROR_NULL_POINTER;
  }
  
  if (thread_count == 0) {
    return CALLBACK_WORKER_ERROR_INVALID_PARAM;
  }
  
  try {
    auto* wrapper = new(std::nothrow) CallbackWorkerThreadC(thread_count);
    if (wrapper == nullptr || wrapper->worker == nullptr) {
      delete wrapper;
      return CALLBACK_WORKER_ERROR_MEMORY;
    }
    
    *worker = wrapper;
    return CALLBACK_WORKER_SUCCESS;
  } catch (const std::invalid_argument&) {
    return CALLBACK_WORKER_ERROR_INVALID_PARAM;
  } catch (const std::exception&) {
    return CALLBACK_WORKER_ERROR_UNKNOWN;
  } catch (...) {
    return CALLBACK_WORKER_ERROR_UNKNOWN;
  }
}

CallbackWorkerResult callback_worker_destroy(CallbackWorkerThreadC* worker) {
  if (worker == nullptr) {
    return CALLBACK_WORKER_ERROR_NULL_POINTER;
  }
  
  try {
    delete worker;
    return CALLBACK_WORKER_SUCCESS;
  } catch (const std::exception&) {
    return CALLBACK_WORKER_ERROR_UNKNOWN;
  } catch (...) {
    return CALLBACK_WORKER_ERROR_UNKNOWN;
  }
}

CallbackWorkerResult callback_worker_enqueue_default(CallbackWorkerThreadC* worker,
                                                      DefaultCallbackFunc callback,
                                                      int arg1,
                                                      double arg2,
                                                      const char* arg3) {
  if (worker == nullptr || callback == nullptr) {
    return CALLBACK_WORKER_ERROR_NULL_POINTER;
  }
  
  if (arg3 == nullptr) {
    return CALLBACK_WORKER_ERROR_NULL_POINTER;
  }
  
  try {
    // Copy string for capture
    std::string arg3_copy(arg3);
    
    auto future = worker->worker->Enqueue([callback, arg1, arg2, arg3_copy]() {
      callback(arg1, arg2, arg3_copy.c_str());
    });
    
    // Discard future as we don't provide wait functionality in C interface
    future.wait();
    
    return CALLBACK_WORKER_SUCCESS;
  } catch (const std::runtime_error&) {
    return CALLBACK_WORKER_ERROR_THREAD_STOPPED;
  } catch (const std::bad_alloc&) {
    return CALLBACK_WORKER_ERROR_MEMORY;
  } catch (const std::exception&) {
    return CALLBACK_WORKER_ERROR_UNKNOWN;
  } catch (...) {
    return CALLBACK_WORKER_ERROR_UNKNOWN;
  }
}

CallbackWorkerResult callback_worker_enqueue_no_arg(CallbackWorkerThreadC* worker,
                                                     NoArgCallbackFunc callback) {
  if (worker == nullptr || callback == nullptr) {
    return CALLBACK_WORKER_ERROR_NULL_POINTER;
  }
  
  try {
    auto future = worker->worker->Enqueue([callback]() {
      callback();
    });
    
    future.wait();
    return CALLBACK_WORKER_SUCCESS;
  } catch (const std::runtime_error&) {
    return CALLBACK_WORKER_ERROR_THREAD_STOPPED;
  } catch (const std::bad_alloc&) {
    return CALLBACK_WORKER_ERROR_MEMORY;
  } catch (const std::exception&) {
    return CALLBACK_WORKER_ERROR_UNKNOWN;
  } catch (...) {
    return CALLBACK_WORKER_ERROR_UNKNOWN;
  }
}

CallbackWorkerResult callback_worker_enqueue_int(CallbackWorkerThreadC* worker,
                                                  IntCallbackFunc callback,
                                                  int arg) {
  if (worker == nullptr || callback == nullptr) {
    return CALLBACK_WORKER_ERROR_NULL_POINTER;
  }
  
  try {
    auto future = worker->worker->Enqueue([callback, arg]() {
      callback(arg);
    });
    
    future.wait();
    return CALLBACK_WORKER_SUCCESS;
  } catch (const std::runtime_error&) {
    return CALLBACK_WORKER_ERROR_THREAD_STOPPED;
  } catch (const std::bad_alloc&) {
    return CALLBACK_WORKER_ERROR_MEMORY;
  } catch (const std::exception&) {
    return CALLBACK_WORKER_ERROR_UNKNOWN;
  } catch (...) {
    return CALLBACK_WORKER_ERROR_UNKNOWN;
  }
}

CallbackWorkerResult callback_worker_enqueue_int_return_sync(CallbackWorkerThreadC* worker,
                                                             IntReturnCallbackFunc callback,
                                                             int arg1,
                                                             int arg2,
                                                             int* result) {
  if (worker == nullptr || callback == nullptr || result == nullptr) {
    return CALLBACK_WORKER_ERROR_NULL_POINTER;
  }
  
  try {
    auto future = worker->worker->Enqueue([callback, arg1, arg2]() -> int {
      return callback(arg1, arg2);
    });
    
    *result = future.get();
    return CALLBACK_WORKER_SUCCESS;
  } catch (const std::runtime_error&) {
    return CALLBACK_WORKER_ERROR_THREAD_STOPPED;
  } catch (const std::bad_alloc&) {
    return CALLBACK_WORKER_ERROR_MEMORY;
  } catch (const std::exception&) {
    return CALLBACK_WORKER_ERROR_UNKNOWN;
  } catch (...) {
    return CALLBACK_WORKER_ERROR_UNKNOWN;
  }
}

CallbackWorkerResult callback_worker_enqueue_string(CallbackWorkerThreadC* worker,
                                                     StringCallbackFunc callback,
                                                     const char* arg) {
  if (worker == nullptr || callback == nullptr || arg == nullptr) {
    return CALLBACK_WORKER_ERROR_NULL_POINTER;
  }
  
  try {
    // Copy string for capture
    std::string arg_copy(arg);
    
    auto future = worker->worker->Enqueue([callback, arg_copy]() {
      callback(arg_copy.c_str());
    });
    
    future.wait();
    return CALLBACK_WORKER_SUCCESS;
  } catch (const std::runtime_error&) {
    return CALLBACK_WORKER_ERROR_THREAD_STOPPED;
  } catch (const std::bad_alloc&) {
    return CALLBACK_WORKER_ERROR_MEMORY;
  } catch (const std::exception&) {
    return CALLBACK_WORKER_ERROR_UNKNOWN;
  } catch (...) {
    return CALLBACK_WORKER_ERROR_UNKNOWN;
  }
}

CallbackWorkerResult callback_worker_get_thread_count(CallbackWorkerThreadC* worker,
                                                       size_t* count) {
  if (worker == nullptr || count == nullptr) {
    return CALLBACK_WORKER_ERROR_NULL_POINTER;
  }
  
  try {
    *count = worker->worker->GetThreadCount();
    return CALLBACK_WORKER_SUCCESS;
  } catch (const std::exception&) {
    return CALLBACK_WORKER_ERROR_UNKNOWN;
  } catch (...) {
    return CALLBACK_WORKER_ERROR_UNKNOWN;
  }
}

CallbackWorkerResult callback_worker_get_queue_size(CallbackWorkerThreadC* worker,
                                                     size_t* size) {
  if (worker == nullptr || size == nullptr) {
    return CALLBACK_WORKER_ERROR_NULL_POINTER;
  }
  
  try {
    *size = worker->worker->GetQueueSize();
    return CALLBACK_WORKER_SUCCESS;
  } catch (const std::exception&) {
    return CALLBACK_WORKER_ERROR_UNKNOWN;
  } catch (...) {
    return CALLBACK_WORKER_ERROR_UNKNOWN;
  }
}

CallbackWorkerResult callback_worker_stop(CallbackWorkerThreadC* worker) {
  if (worker == nullptr) {
    return CALLBACK_WORKER_ERROR_NULL_POINTER;
  }
  
  try {
    worker->worker->Stop();
    return CALLBACK_WORKER_SUCCESS;
  } catch (const std::exception&) {
    return CALLBACK_WORKER_ERROR_UNKNOWN;
  } catch (...) {
    return CALLBACK_WORKER_ERROR_UNKNOWN;
  }
}

CallbackWorkerResult callback_worker_wait_completion(CallbackWorkerThreadC* worker) {
  if (worker == nullptr) {
    return CALLBACK_WORKER_ERROR_NULL_POINTER;
  }
  
  try {
    worker->worker->WaitForCompletion();
    return CALLBACK_WORKER_SUCCESS;
  } catch (const std::exception&) {
    return CALLBACK_WORKER_ERROR_UNKNOWN;
  } catch (...) {
    return CALLBACK_WORKER_ERROR_UNKNOWN;
  }
}

const char* callback_worker_result_to_string(CallbackWorkerResult result) {
  switch (result) {
    case CALLBACK_WORKER_SUCCESS:
      return "Success";
    case CALLBACK_WORKER_ERROR_INVALID_PARAM:
      return "Invalid parameter";
    case CALLBACK_WORKER_ERROR_NULL_POINTER:
      return "Null pointer error";
    case CALLBACK_WORKER_ERROR_THREAD_STOPPED:
      return "Thread pool is stopped";
    case CALLBACK_WORKER_ERROR_MEMORY:
      return "Memory allocation error";
    case CALLBACK_WORKER_ERROR_UNKNOWN:
      return "Unknown error";
    default:
      return "Undefined error";
  }
}

}  // extern "C" 