#include "callback_worker_thread/callback_worker_thread.h"

#include <stdexcept>

namespace callback_worker_thread {

CallbackWorkerThread::CallbackWorkerThread(size_t thread_count) 
    : stop_(false) {
  if (thread_count == 0) {
    throw std::invalid_argument("Thread count must be greater than 0");
  }

  // Launch worker threads
  workers_.reserve(thread_count);
  for (size_t i = 0; i < thread_count; ++i) {
    workers_.emplace_back(&CallbackWorkerThread::WorkerThreadMain, this);
  }
}

CallbackWorkerThread::~CallbackWorkerThread() {
  Stop();
  
  // Wait for all worker threads to finish
  for (auto& worker : workers_) {
    if (worker.joinable()) {
      worker.join();
    }
  }
}

std::future<void> CallbackWorkerThread::EnqueueDefault(
    DefaultCallback callback,
    int arg1,
    double arg2,
    const std::string& arg3) {
  return Enqueue([callback, arg1, arg2, arg3]() {
    callback(arg1, arg2, arg3);
  });
}

size_t CallbackWorkerThread::GetThreadCount() const {
  return workers_.size();
}

size_t CallbackWorkerThread::GetQueueSize() const {
  std::unique_lock<std::mutex> lock(queue_mutex_);
  return tasks_.size();
}

void CallbackWorkerThread::Stop() {
  {
    std::unique_lock<std::mutex> lock(queue_mutex_);
    stop_ = true;
  }
  condition_.notify_all();
}

void CallbackWorkerThread::WaitForCompletion() {
  // Wait until all tasks are completed
  std::unique_lock<std::mutex> lock(queue_mutex_);
  condition_.wait(lock, [this] { 
    return tasks_.empty() && stop_; 
  });
}

void CallbackWorkerThread::WorkerThreadMain() {
  while (true) {
    std::function<void()> task;
    
    {
      std::unique_lock<std::mutex> lock(queue_mutex_);
      
      // Wait for a task or stop flag
      condition_.wait(lock, [this] { 
        return stop_ || !tasks_.empty(); 
      });
      
      // Exit if stop flag is set and no tasks remain
      if (stop_ && tasks_.empty()) {
        return;
      }
      
      // Get task
      task = std::move(tasks_.front());
      tasks_.pop();
    }
    
    // Execute task
    try {
      task();
    } catch (const std::exception&) {
      // In a real application, implement proper error handling (e.g., logging)
      // Here, we simply ignore exceptions
    } catch (...) {
      // Ignore unknown exceptions as well
    }
  }
}

}  // namespace callback_worker_thread 