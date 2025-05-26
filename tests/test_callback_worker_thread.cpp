#include <gtest/gtest.h>
#include <chrono>
#include <atomic>
#include <thread>

#include "callback_worker_thread/callback_worker_thread.h"

namespace {

using namespace callback_worker_thread;

class CallbackWorkerThreadTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Common setup for tests
  }

  void TearDown() override {
    // Common cleanup for tests
  }
};

TEST_F(CallbackWorkerThreadTest, ConstructorWithDefaultThreadCount) {
  CallbackWorkerThread worker;
  EXPECT_EQ(1u, worker.GetThreadCount());
}

TEST_F(CallbackWorkerThreadTest, ConstructorWithSpecificThreadCount) {
  const size_t thread_count = 4;
  CallbackWorkerThread worker(thread_count);
  EXPECT_EQ(thread_count, worker.GetThreadCount());
}

TEST_F(CallbackWorkerThreadTest, ConstructorWithZeroThreadCount) {
  EXPECT_THROW(CallbackWorkerThread(0), std::invalid_argument);
}

TEST_F(CallbackWorkerThreadTest, EnqueueDefaultCallback) {
  CallbackWorkerThread worker;
  
  std::atomic<bool> callback_executed(false);
  int received_arg1 = 0;
  double received_arg2 = 0.0;
  std::string received_arg3;
  
  auto callback = [&](int arg1, double arg2, const std::string& arg3) {
    received_arg1 = arg1;
    received_arg2 = arg2;
    received_arg3 = arg3;
    callback_executed = true;
  };
  
  const int expected_arg1 = 42;
  const double expected_arg2 = 3.14;
  const std::string expected_arg3 = "test";
  
  auto future = worker.EnqueueDefault(callback, expected_arg1, expected_arg2, expected_arg3);
  future.wait();
  
  // Wait briefly for callback execution
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  
  EXPECT_TRUE(callback_executed);
  EXPECT_EQ(expected_arg1, received_arg1);
  EXPECT_DOUBLE_EQ(expected_arg2, received_arg2);
  EXPECT_EQ(expected_arg3, received_arg3);
}

TEST_F(CallbackWorkerThreadTest, EnqueueGenericCallback) {
  CallbackWorkerThread worker;
  
  std::atomic<int> result(0);
  
  auto task = [&result](int a, int b) {
    result = a + b;
    return a + b;
  };
  
  auto future = worker.Enqueue(task, 10, 20);
  int task_result = future.get();
  
  EXPECT_EQ(30, task_result);
  EXPECT_EQ(30, result.load());
}

TEST_F(CallbackWorkerThreadTest, MultipleTasksExecution) {
  CallbackWorkerThread worker(2);  // 2 worker threads
  
  const int task_count = 10;
  std::atomic<int> completed_tasks(0);
  std::vector<std::future<void>> futures;
  
  for (int i = 0; i < task_count; ++i) {
    auto future = worker.Enqueue([&completed_tasks, i]() {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      completed_tasks++;
    });
    futures.push_back(std::move(future));
  }
  
  // Wait for all tasks to complete
  for (auto& future : futures) {
    future.wait();
  }
  
  EXPECT_EQ(task_count, completed_tasks.load());
}

TEST_F(CallbackWorkerThreadTest, QueueSizeTracking) {
  CallbackWorkerThread worker(1);
  
  EXPECT_EQ(0u, worker.GetQueueSize());
  
  // Submit long-running tasks to fill the queue
  auto slow_task = []() {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  };
  
  const int queue_tasks = 5;
  std::vector<std::future<void>> futures;
  
  for (int i = 0; i < queue_tasks; ++i) {
    auto future = worker.Enqueue(slow_task);
    futures.push_back(std::move(future));
  }
  
  // Wait briefly before checking queue size
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  
  // First task is executing, so queue should have 4 tasks
  EXPECT_LE(worker.GetQueueSize(), static_cast<size_t>(queue_tasks));
  
  // Wait for all tasks to complete
  for (auto& future : futures) {
    future.wait();
  }
  
  EXPECT_EQ(0u, worker.GetQueueSize());
}

TEST_F(CallbackWorkerThreadTest, StopFunctionality) {
  CallbackWorkerThread worker(2);
  
  std::atomic<int> completed_before_stop(0);
  std::atomic<int> total_completed(0);
  
  // Submit some tasks
  std::vector<std::future<void>> futures;
  for (int i = 0; i < 5; ++i) {
    auto future = worker.Enqueue([&]() {
      completed_before_stop++;
      total_completed++;
      std::this_thread::sleep_for(std::chrono::milliseconds(20));
    });
    futures.push_back(std::move(future));
  }
  
  // Wait briefly before stopping
  std::this_thread::sleep_for(std::chrono::milliseconds(30));
  worker.Stop();
  
  // Attempting to enqueue after stop should throw
  EXPECT_THROW(worker.Enqueue([](){}), std::runtime_error);
  
  // Verify existing tasks complete
  for (auto& future : futures) {
    future.wait();
  }
  
  EXPECT_GT(total_completed.load(), 0);
}

TEST_F(CallbackWorkerThreadTest, ExceptionHandling) {
  CallbackWorkerThread worker;
  
  std::atomic<bool> normal_task_executed(false);
  
  // Submit a task that throws
  auto exception_task = []() {
    throw std::runtime_error("Test exception");
  };
  
  // Submit a normal task
  auto normal_task = [&normal_task_executed]() {
    normal_task_executed = true;
  };
  
  auto future1 = worker.Enqueue(exception_task);
  auto future2 = worker.Enqueue(normal_task);
  
  // Exception task will throw but program continues
  future1.wait();
  future2.wait();
  
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  
  // Next task should execute despite exception
  EXPECT_TRUE(normal_task_executed);
}

TEST_F(CallbackWorkerThreadTest, ThreadSafety) {
  CallbackWorkerThread worker(4);
  
  const int num_threads = 8;
  const int tasks_per_thread = 100;
  std::atomic<int> total_executed(0);
  
  std::vector<std::thread> producer_threads;
  
  for (int t = 0; t < num_threads; ++t) {
    producer_threads.emplace_back([&worker, &total_executed, tasks_per_thread]() {
      for (int i = 0; i < tasks_per_thread; ++i) {
        worker.Enqueue([&total_executed]() {
          total_executed++;
        });
      }
    });
  }
  
  // Wait for all producer threads to complete
  for (auto& thread : producer_threads) {
    thread.join();
  }
  
  // Wait briefly before checking results
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  
  EXPECT_EQ(num_threads * tasks_per_thread, total_executed.load());
}

}  // namespace 