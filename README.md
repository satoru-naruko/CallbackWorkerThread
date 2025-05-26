# CallbackWorkerThread

A thread pool library for callback execution using C++17.

## Features

- **Thread Pool**: Manages multiple worker threads (default: 1)
- **Thread Safety**: Safely enqueue callbacks from multiple threads
- **Flexibility**: Supports any function type (no restrictions on argument or return types)
- **C Language Interface**: Usable from C code
- **STL Only**: No external dependencies, uses only STL libraries
- **Cross-Platform**: Windows and Linux support

## 🚀 Quick Start

### Build and Run in 5 Minutes

#### For Linux/WSL

```bash
# 1. Install required tools
sudo apt update && sudo apt install -y build-essential cmake git

# 2. Clone the project (or download)
git clone <repository-url>
cd CallbackWorkerThread

# 3. Build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# 4. Run
./Release/simple_example    # C++ example
./Release/c_example        # C example
```

#### For Windows

```cmd
rem 1. Open Visual Studio Developer Command Prompt

rem 2. Navigate to project directory
cd C:\path\to\CallbackWorkerThread

rem 3. Build
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release

rem 4. Run
Release\simple_example.exe  rem C++ example
Release\c_example.exe       rem C example
```

#### Troubleshooting

```bash
# Minimal test (GoogleTest not required)
./Release/test_callback_worker_thread_c

# If errors occur, refer to the troubleshooting section below
```

## Requirements

### Mandatory Requirements
- **C++17 or later** compiler
- **CMake 3.20 or later**
- **Make** (Linux/WSL) or **Visual Studio** (Windows)

### Supported Platforms
- **Windows 10 or later** (Visual Studio 2019/2022)
- **WSL Ubuntu 22.04 or later**
- **Ubuntu 22.04 or later** (Native Linux)

### Optional Requirements
- **GoogleTest** (Optional, for C++ tests)

## Build Instructions

### 🐧 Building on WSL Ubuntu 22.04 / Linux

#### 1. Install Dependencies

```bash
# Update system packages
sudo apt update

# Install required tools
sudo apt install -y build-essential cmake git

# Install GoogleTest (for running tests)
sudo apt install -y libgtest-dev

# Verify C++17 compiler
g++ --version  # GCC 7.0 or later required
```

#### 2. Build Project

```bash
# Navigate to project directory
cd /path/to/CallbackWorkerThread

# Create build directory
mkdir build
cd build

# Configure CMake (Release build)
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
make -j$(nproc)

# Or specify parallel build count
make -j4
```

#### 3. Verify Build

```bash
# Check build artifacts
ls -la
ls -la Release/  # Check executables

# Run C++ example
./Release/simple_example

# Run C example
./Release/c_example
```

### 🪟 Building on Windows

#### 1. Prerequisites

- **Visual Studio 2019** or **Visual Studio 2022** installed
- **CMake** in PATH (can be added via Visual Studio Installer)
- **Git** installed

#### 2. Build Project

```cmd
rem Navigate to project directory
cd C:\path\to\CallbackWorkerThread

rem Create build directory
mkdir build
cd build

rem Configure CMake (Visual Studio 2022)
cmake .. -G "Visual Studio 17 2022" -A x64

rem Or for Visual Studio 2019
cmake .. -G "Visual Studio 16 2019" -A x64

rem Build (Release configuration)
cmake --build . --config Release

rem Or Debug configuration
cmake --build . --config Debug
```

#### 3. Verify Build

```cmd
rem Check build artifacts
dir Release\

rem Run C++ example
Release\simple_example.exe

rem Run C example
Release\c_example.exe
```

### ⚙️ Build Options

The following options are available during CMake configuration:

```bash
# Disable building examples
cmake .. -DBUILD_EXAMPLES=OFF

# Disable building tests
cmake .. -DBUILD_TESTS=OFF

# Debug build
cmake .. -DCMAKE_BUILD_TYPE=Debug

# Combine all options
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_EXAMPLES=ON -DBUILD_TESTS=ON
```

## Usage

### Basic Usage Example

```cpp
#include "callback_worker_thread/callback_worker_thread.h"

using namespace callback_worker_thread;

int main() {
    // Create thread pool with one worker thread
    CallbackWorkerThread worker;
    
    // Default callback (int, double, string)
    auto callback = [](int id, double value, const std::string& msg) {
        std::cout << "ID: " << id << ", Value: " << value << ", Message: " << msg << std::endl;
    };
    
    auto future = worker.EnqueueDefault(callback, 1, 3.14, "Hello");
    future.wait();  // Wait for completion
    
    return 0;
}
```

### Generic Callbacks

```cpp
// Function with arbitrary arguments
auto future = worker.Enqueue([](int a, float b) -> int {
    return a + static_cast<int>(b);
}, 10, 2.5f);

int result = future.get();  // Get return value
```

### Multiple Worker Threads

```cpp
// Use 4 worker threads
CallbackWorkerThread worker(4);

// Execute multiple tasks in parallel
std::vector<std::future<void>> futures;
for (int i = 0; i < 10; ++i) {
    auto future = worker.Enqueue([i]() {
        std::cout << "Task " << i << " executing\n";
    });
    futures.push_back(std::move(future));
}

// Wait for all tasks to complete
for (auto& future : futures) {
    future.wait();
}
```

### C Language Interface

```c
#include "callback_worker_thread/callback_worker_thread_c.h"

// Define callback function
void my_callback(int id, double value, const char* message) {
    printf("ID: %d, Value: %.2f, Message: %s\n", id, value, message);
}

int main() {
    CallbackWorkerThreadC* worker = NULL;
    CallbackWorkerResult result;
    
    // Create worker thread pool
    result = callback_worker_create(2, &worker);
    if (result != CALLBACK_WORKER_SUCCESS) {
        printf("Error: %s\n", callback_worker_result_to_string(result));
        return -1;
    }
    
    // Execute callback
    result = callback_worker_enqueue_default(worker, my_callback, 
                                             1, 3.14, "Hello from C!");
    
    // Cleanup
    callback_worker_destroy(worker);
    return 0;
}
```

## API Reference

### CallbackWorkerThread Class

#### Constructor

```cpp
explicit CallbackWorkerThread(size_t thread_count = 1);
```

- `thread_count`: Number of worker threads (default: 1)

#### Methods

- `EnqueueDefault()`: Enqueue default callback (int, double, string)
- `Enqueue()`: Enqueue generic callback
- `GetThreadCount()`: Get worker thread count
- `GetQueueSize()`: Get pending task count
- `Stop()`: Stop thread pool
- `WaitForCompletion()`: Wait for all tasks to complete

### C Language Interface

#### Main Functions

- `callback_worker_create()`: Create worker instance
- `callback_worker_destroy()`: Destroy worker instance
- `callback_worker_enqueue_default()`: Enqueue default callback
- `callback_worker_enqueue_no_arg()`: Enqueue no-argument callback
- `callback_worker_enqueue_int()`: Enqueue integer argument callback
- `callback_worker_enqueue_string()`: Enqueue string argument callback
- `callback_worker_enqueue_int_return_sync()`: Enqueue callback with return value (synchronous)
- `callback_worker_get_thread_count()`: Get thread count
- `callback_worker_get_queue_size()`: Get queue size
- `callback_worker_result_to_string()`: Convert error code to string

## Testing

### 🧪 Running Tests

This project includes two types of tests:
- **C++ Tests**: Using GoogleTest (optional)
- **C Language Tests**: Custom implementation (always available)

### 🐧 Running Tests on Linux/WSL

#### 1. C++ Tests with GoogleTest

```bash
# Install GoogleTest (if not installed)
sudo apt install -y libgtest-dev

# Build with tests
cd build
cmake .. -DBUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# Run all tests using CTest
ctest --output-on-failure

# Or run with verbose output
ctest --verbose

# Run specific test
ctest -R test_callback_worker_thread

# Run GoogleTest binary directly
./Release/test_callback_worker_thread --gtest_output=xml:test_results.xml
```

#### 2. C Language Tests (Always Available)

```bash
# Build C language tests (GoogleTest not required)
cd build
cmake .. -DBUILD_TESTS=ON
make -j$(nproc)

# Run C language tests
./Release/test_callback_worker_thread_c

# Run via CTest
ctest -R test_c_interface --verbose
```

#### 3. Run All Tests

```bash
# Run all tests at once
cd build
ctest --output-on-failure

# Show detailed test output
ctest --verbose

# Run tests in parallel
ctest -j4
```

### 🪟 Running Tests on Windows

#### 1. C++ Tests with GoogleTest

```cmd
rem Install GoogleTest (using vcpkg)
vcpkg install gtest:x64-windows

rem Build with tests
cd build
cmake .. -DBUILD_TESTS=ON -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake
cmake --build . --config Release

rem Run all tests using CTest
ctest --output-on-failure -C Release

rem Run GoogleTest binary directly
Release\test_callback_worker_thread.exe
```

#### 2. C Language Tests (Always Available)

```cmd
rem Run C language tests
Release\test_callback_worker_thread_c.exe

rem Run via CTest
ctest -R test_c_interface --verbose -C Release
```

### 📊 Test Contents

#### C++ Tests (`test_callback_worker_thread`)
- Constructor tests
- Default callback execution tests
- Generic callback execution tests
- Multiple task parallel execution tests
- Queue size tracking tests
- Stop functionality tests
- Exception handling tests
- Thread safety tests

#### C Language Tests (`test_callback_worker_thread_c`)
- Instance creation/destruction tests
- Default callback execution tests
- Various callback execution tests
- Return value callback tests
- Queue size retrieval tests
- Error handling tests
- Error string conversion tests

### 🔧 Test Troubleshooting

#### GoogleTest Not Found (Linux/WSL)

```bash
# For Ubuntu/Debian
sudo apt update
sudo apt install -y libgtest-dev

# Build from source
cd /usr/src/gtest
sudo cmake .
sudo make
sudo cp lib/*.a /usr/lib/
```

#### GoogleTest Not Found in Visual Studio (Windows)

```cmd
rem Install using vcpkg
vcpkg install gtest:x64-windows

rem Specify toolchain file during CMake configuration
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake
```

#### Test Execution Permission Issues (Linux/WSL)

```bash
# Grant execution permissions
chmod +x Release/test_callback_worker_thread_c
chmod +x Release/test_callback_worker_thread

# Or
find . -name "test_*" -type f -exec chmod +x {} \;
```

### 📈 Continuous Integration (CI)

Example of automatic testing with GitHub Actions:

```yaml
name: Build and Test
on: [push, pull_request]

jobs:
  test:
    runs-on: ubuntu-latest
    steps:
    - uses: actions/checkout@v3
    - name: Install dependencies
      run: |
        sudo apt update
        sudo apt install -y build-essential cmake libgtest-dev
    - name: Build
      run: |
        mkdir build && cd build
        cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON
        make -j$(nproc)
    - name: Run tests
      run: |
        cd build
        ctest --output-on-failure
```

