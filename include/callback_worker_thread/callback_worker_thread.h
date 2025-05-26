#ifndef CALLBACK_WORKER_THREAD_CALLBACK_WORKER_THREAD_H_
#define CALLBACK_WORKER_THREAD_CALLBACK_WORKER_THREAD_H_

#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace callback_worker_thread {

/**
 * @brief Thread pool class for callback processing
 * 
 * Manages multiple worker threads and executes callback functions asynchronously.
 * Thread-safe, allowing callback tasks to be enqueued from other threads.
 */
class CallbackWorkerThread {
 public:
  /// Default callback function type (example with 3 arguments)
  using DefaultCallback = std::function<void(int, double, const std::string&)>;
  
  /// Generic callback function type
  template<typename... Args>
  using Callback = std::function<void(Args...)>;

  /**
   * @brief Constructor
   * @param thread_count Number of worker threads (default: 1)
   */
  explicit CallbackWorkerThread(size_t thread_count = 1);

  /**
   * @brief Destructor
   * 
   * Safely stops all worker threads.
   */
  ~CallbackWorkerThread();

  // Disable copy and move
  CallbackWorkerThread(const CallbackWorkerThread&) = delete;
  CallbackWorkerThread& operator=(const CallbackWorkerThread&) = delete;
  CallbackWorkerThread(CallbackWorkerThread&&) = delete;
  CallbackWorkerThread& operator=(CallbackWorkerThread&&) = delete;

  /**
   * @brief Enqueue default callback function
   * @param callback Callback function to execute
   * @param arg1 First argument
   * @param arg2 Second argument
   * @param arg3 Third argument
   * @return Future for retrieving execution result
   */
  std::future<void> EnqueueDefault(DefaultCallback callback,
                                   int arg1,
                                   double arg2,
                                   const std::string& arg3);

  /**
   * @brief Enqueue generic callback function
   * @tparam F Function type
   * @tparam Args Argument types
   * @param f Function to execute
   * @param args Function arguments
   * @return Future for retrieving execution result
   */
  template<typename F, typename... Args>
  auto Enqueue(F&& f, Args&&... args) 
      -> std::future<typename std::invoke_result<F, Args...>::type>;

  /**
   * @brief Get number of worker threads
   * @return Thread count
   */
  size_t GetThreadCount() const;

  /**
   * @brief Get number of pending tasks
   * @return Number of tasks in queue
   */
  size_t GetQueueSize() const;

  /**
   * @brief Stop thread pool
   * 
   * Stops accepting new tasks and waits for existing tasks to complete.
   */
  void Stop();

  /**
   * @brief Wait for all tasks to complete
   */
  void WaitForCompletion();

 private:
  /**
   * @brief Main worker thread processing
   */
  void WorkerThreadMain();

  std::vector<std::thread> workers_;
  std::queue<std::function<void()>> tasks_;
  mutable std::mutex queue_mutex_;
  std::condition_variable condition_;
  bool stop_;
};

// Template function implementation
template<typename F, typename... Args>
auto CallbackWorkerThread::Enqueue(F&& f, Args&&... args)
    -> std::future<typename std::invoke_result<F, Args...>::type> {
  using return_type = typename std::invoke_result<F, Args...>::type;

  auto task = std::make_shared<std::packaged_task<return_type()>>(
      std::bind(std::forward<F>(f), std::forward<Args>(args)...));

  std::future<return_type> res = task->get_future();
  
  {
    std::unique_lock<std::mutex> lock(queue_mutex_);
    
    // Throw exception if thread pool is stopped
    if (stop_) {
      throw std::runtime_error("Cannot enqueue task: thread pool is stopped");
    }

    tasks_.emplace([task]() { (*task)(); });
  }
  
  condition_.notify_one();
  return res;
}

}  // namespace callback_worker_thread

#endif  // CALLBACK_WORKER_THREAD_CALLBACK_WORKER_THREAD_H_ 