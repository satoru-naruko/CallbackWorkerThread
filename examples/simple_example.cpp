#include <iostream>
#include <thread>
#include <chrono>

#include "callback_worker_thread/callback_worker_thread.h"

using namespace callback_worker_thread;

int main() {
  std::cout << "CallbackWorkerThread ライブラリの使用例\n";
  std::cout << "=====================================\n\n";

  // 1. デフォルトコンストラクタ（1つのワーカースレッド）
  std::cout << "1. 単一ワーカースレッドでのデフォルトコールバック実行:\n";
  {
    CallbackWorkerThread worker;
    
    auto callback = [](int id, double value, const std::string& message) {
      std::cout << "  コールバック実行 - ID: " << id 
                << ", 値: " << value 
                << ", メッセージ: " << message << std::endl;
    };
    
    auto future = worker.EnqueueDefault(callback, 1, 3.14, "Hello World");
    future.wait();
  }
  
  std::cout << "\n";

  // 2. 複数ワーカースレッドでの汎用コールバック
  std::cout << "2. 複数ワーカースレッドでの汎用コールバック実行:\n";
  {
    CallbackWorkerThread worker(3);  // 3つのワーカースレッド
    
    std::vector<std::future<int>> futures;
    
    for (int i = 0; i < 10; ++i) {
      auto future = worker.Enqueue([](int task_id) -> int {
        std::cout << "  タスク " << task_id << " 実行中... (スレッドID: " 
                  << std::this_thread::get_id() << ")" << std::endl;
        
        // 処理をシミュレート
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        return task_id * 2;
      }, i);
      
      futures.push_back(std::move(future));
    }
    
    // 全てのタスクの結果を取得
    std::cout << "  結果:\n";
    for (size_t i = 0; i < futures.size(); ++i) {
      int result = futures[i].get();
      std::cout << "    タスク " << i << " の結果: " << result << std::endl;
    }
  }
  
  std::cout << "\n";

  // 3. 異なる引数を持つコールバック
  std::cout << "3. 異なる引数を持つコールバック関数:\n";
  {
    CallbackWorkerThread worker(2);
    
    // 引数なし
    auto future1 = worker.Enqueue([]() {
      std::cout << "  引数なしのコールバック実行\n";
    });
    
    // 単一引数
    auto future2 = worker.Enqueue([](const std::string& msg) {
      std::cout << "  単一引数のコールバック: " << msg << std::endl;
    }, "テストメッセージ");
    
    // 複数引数（異なる型）
    auto future3 = worker.Enqueue([](int a, float b, bool c, const std::string& d) {
      std::cout << "  複数引数のコールバック: " << a << ", " << b 
                << ", " << (c ? "true" : "false") << ", " << d << std::endl;
    }, 42, 2.71f, true, "最終引数");
    
    future1.wait();
    future2.wait();
    future3.wait();
  }
  
  std::cout << "\n";

  // 4. 戻り値を持つコールバック
  std::cout << "4. 戻り値を持つコールバック:\n";
  {
    CallbackWorkerThread worker;
    
    auto future = worker.Enqueue([](int x, int y) -> int {
      int result = x + y;
      std::cout << "  計算実行: " << x << " + " << y << " = " << result << std::endl;
      return result;
    }, 15, 27);
    
    int result = future.get();
    std::cout << "  受信した結果: " << result << std::endl;
  }
  
  std::cout << "\n";

  // 5. スレッドプールの情報取得
  std::cout << "5. スレッドプール情報:\n";
  {
    CallbackWorkerThread worker(4);
    
    std::cout << "  ワーカースレッド数: " << worker.GetThreadCount() << std::endl;
    std::cout << "  初期キューサイズ: " << worker.GetQueueSize() << std::endl;
    
    // 長時間実行されるタスクを投入
    std::vector<std::future<void>> futures;
    for (int i = 0; i < 10; ++i) {
      auto future = worker.Enqueue([i]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
      });
      futures.push_back(std::move(future));
    }
    
    std::cout << "  タスク投入後のキューサイズ: " << worker.GetQueueSize() << std::endl;
    
    // 全タスクの完了を待機
    for (auto& future : futures) {
      future.wait();
    }
    
    std::cout << "  全タスク完了後のキューサイズ: " << worker.GetQueueSize() << std::endl;
  }

  std::cout << "\n";
  std::cout << "全ての例が正常に実行されました。\n";
  
  return 0;
} 