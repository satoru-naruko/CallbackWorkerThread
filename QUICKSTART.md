# Quick Start Guide

## Getting Started with CallbackWorkerThread in 5 Minutes

This guide explains how to create a simple application using the CallbackWorkerThread library.

## 1. Get and Build the Project

```bash
# Navigate to project directory
cd CallbackWorkerThread

# Create build directory
mkdir build
cd build

# Configure project with CMake
cmake ..

# Build
make

# Run example
./simple_example
```

## 2. Minimal Sample Code

Save the following code as `my_app.cpp`:

```cpp
#include <iostream>
#include "callback_worker_thread/callback_worker_thread.h"

using namespace callback_worker_thread;

int main() {
    // Create thread pool (default: 1 thread)
    CallbackWorkerThread worker;
    
    // Simple callback
    auto future = worker.Enqueue([]() {
        std::cout << "Hello from worker thread!" << std::endl;
    });
    
    // Wait for completion
    future.wait();
    
    std::cout << "Completed." << std::endl;
    return 0;
}
```

## 3. Build and Run

```bash
# Compile
g++ -std=c++17 -I../include -pthread my_app.cpp ../build/libcallback_worker_thread.a -o my_app

# Run
./my_app
```

## 4. Common Usage Patterns

### Pattern 1: Callback with Arguments

```cpp
auto future = worker.Enqueue([](int id, const std::string& message) {
    std::cout << "Task " << id << ": " << message << std::endl;
}, 42, "Processing");

future.wait();
```

### Pattern 2: Callback with Return Value

```cpp
auto future = worker.Enqueue([](int a, int b) -> int {
    return a + b;
}, 10, 20);

int result = future.get();  // 30
std::cout << "Result: " << result << std::endl;
```

### Pattern 3: Multiple Worker Threads

```cpp
CallbackWorkerThread worker(4);  // 4 threads

// Submit multiple tasks
std::vector<std::future<void>> futures;
for (int i = 0; i < 10; ++i) {
    auto future = worker.Enqueue([i]() {
        std::cout << "Task " << i << " executing" << std::endl;
    });
    futures.push_back(std::move(future));
}

// Wait for all to complete
for (auto& f : futures) {
    f.wait();
}
```

## 5. Error Handling

```cpp
try {
    CallbackWorkerThread worker;
    
    auto future = worker.Enqueue([]() {
        throw std::runtime_error("Some error occurred");
    });
    
    future.wait();  // Exception is caught internally
    
} catch (const std::exception& e) {
    std::cout << "Error: " << e.what() << std::endl;
}
```

## 6. Practical Usage Example

### Parallel File Processing

```cpp
#include <filesystem>
#include <fstream>

CallbackWorkerThread worker(4);
std::vector<std::future<void>> futures;

// Process multiple files in parallel
for (const auto& entry : std::filesystem::directory_iterator("./data")) {
    if (entry.is_regular_file()) {
        auto future = worker.Enqueue([filepath = entry.path()]() {
            std::ifstream file(filepath);
            // File processing logic
            std::cout << "Processing completed: " << filepath << std::endl;
        });
        futures.push_back(std::move(future));
    }
}

// Wait for all file processing to complete
for (auto& f : futures) {
    f.wait();
}
```

## 7. Performance Considerations

- **Appropriate Thread Count**: Effective around CPU count or slightly more
- **Task Granularity**: Too small tasks have high overhead
- **Memory Usage**: Submitting many tasks at once consumes memory

```cpp
// Good example: Appropriate task granularity
worker.Enqueue([&data]() {
    processLargeDataChunk(data);
});

// Bad example: Too fine-grained tasks
for (int i = 0; i < 1000000; ++i) {
    worker.Enqueue([i]() { trivialOperation(i); });  // High overhead
}
```

## 8. Debugging Tips

```cpp
// Check thread pool status
std::cout << "Worker count: " << worker.GetThreadCount() << std::endl;
std::cout << "Pending tasks: " << worker.GetQueueSize() << std::endl;

// Display current thread ID
worker.Enqueue([]() {
    std::cout << "Executing thread ID: " << std::this_thread::get_id() << std::endl;
});
```

## Next Steps

- Check more examples in `examples/simple_example.cpp`
- Reference test code in `tests/` directory
- Review API documentation in `include/callback_worker_thread/callback_worker_thread.h`

If you encounter any issues, please refer to the README.md or create an Issue. 