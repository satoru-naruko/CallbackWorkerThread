#include <iostream>
#include <thread>
#include <chrono>

#include "callback_worker_thread/callback_worker_thread.h"

using namespace callback_worker_thread;

int main() {
  std::cout << "CallbackWorkerThread Library Usage Example\n";
  std::cout << "=====================================\n\n";

  // 1. Default constructor (single worker thread)
  std::cout << "1. Default callback execution with single worker thread:\n";
  {
    CallbackWorkerThread worker;
    
    auto callback = [](int id, double value, const std::string& message) {
      std::cout << "  Callback executed - ID: " << id 
                << ", Value: " << value 
                << ", Message: " << message << std::endl;
    };
    
    auto future = worker.EnqueueDefault(callback, 1, 3.14, "Hello World");
    future.wait();
  }
  
  std::cout << "\n";

  // 2. Generic callback with multiple worker threads
  std::cout << "2. Generic callback execution with multiple worker threads:\n";
  {
    CallbackWorkerThread worker(3);  // 3 worker threads
    
    std::vector<std::future<int>> futures;
    
    for (int i = 0; i < 10; ++i) {
      auto future = worker.Enqueue([](int task_id) -> int {
        std::cout << "  Task " << task_id << " executing... (Thread ID: " 
                  << std::this_thread::get_id() << ")" << std::endl;
        
        // Simulate processing
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        return task_id * 2;
      }, i);
      
      futures.push_back(std::move(future));
    }
    
    // Get results of all tasks
    std::cout << "  Results:\n";
    for (size_t i = 0; i < futures.size(); ++i) {
      int result = futures[i].get();
      std::cout << "    Task " << i << " result: " << result << std::endl;
    }
  }
  
  std::cout << "\n";

  // 3. Callbacks with different arguments
  std::cout << "3. Callbacks with different argument types:\n";
  {
    CallbackWorkerThread worker(2);
    
    // No arguments
    auto future1 = worker.Enqueue([]() {
      std::cout << "  No-argument callback executed\n";
    });
    
    // Single argument
    auto future2 = worker.Enqueue([](const std::string& msg) {
      std::cout << "  Single argument callback: " << msg << std::endl;
    }, "Test message");
    
    // Multiple arguments (different types)
    auto future3 = worker.Enqueue([](int a, float b, bool c, const std::string& d) {
      std::cout << "  Multiple argument callback: " << a << ", " << b 
                << ", " << (c ? "true" : "false") << ", " << d << std::endl;
    }, 42, 2.71f, true, "Final argument");
    
    future1.wait();
    future2.wait();
    future3.wait();
  }
  
  std::cout << "\n";

  // 4. Callback with return value
  std::cout << "4. Callback with return value:\n";
  {
    CallbackWorkerThread worker;
    
    auto future = worker.Enqueue([](int x, int y) -> int {
      int result = x + y;
      std::cout << "  Calculation executed: " << x << " + " << y << " = " << result << std::endl;
      return result;
    }, 15, 27);
    
    int result = future.get();
    std::cout << "  Received result: " << result << std::endl;
  }
  
  std::cout << "\n";

  // 5. Thread pool information
  std::cout << "5. Thread pool information:\n";
  {
    CallbackWorkerThread worker(4);
    
    std::cout << "  Worker thread count: " << worker.GetThreadCount() << std::endl;
    std::cout << "  Initial queue size: " << worker.GetQueueSize() << std::endl;
    
    // Submit long-running tasks
    std::vector<std::future<void>> futures;
    for (int i = 0; i < 10; ++i) {
      auto future = worker.Enqueue([i]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
      });
      futures.push_back(std::move(future));
    }
    
    std::cout << "  Queue size after task submission: " << worker.GetQueueSize() << std::endl;
    
    // Wait for all tasks to complete
    for (auto& future : futures) {
      future.wait();
    }
    
    std::cout << "  Queue size after all tasks completed: " << worker.GetQueueSize() << std::endl;
  }

  std::cout << "\n";
  std::cout << "All examples executed successfully.\n";
  
  return 0;
} 