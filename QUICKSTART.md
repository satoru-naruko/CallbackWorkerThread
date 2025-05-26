# クイックスタートガイド

## 5分で始めるCallbackWorkerThread

このガイドでは、CallbackWorkerThreadライブラリを使用して簡単なアプリケーションを作成する手順を説明します。

## 1. プロジェクトの取得とビルド

```bash
# プロジェクトディレクトリに移動
cd CallbackWorkerThread

# ビルドディレクトリを作成
mkdir build
cd build

# CMakeでプロジェクトを設定
cmake ..

# ビルド実行
make

# 使用例の実行
./simple_example
```

## 2. 最小限のサンプルコード

以下のコードを `my_app.cpp` として保存してください：

```cpp
#include <iostream>
#include "callback_worker_thread/callback_worker_thread.h"

using namespace callback_worker_thread;

int main() {
    // スレッドプールを作成（デフォルト: 1スレッド）
    CallbackWorkerThread worker;
    
    // シンプルなコールバック
    auto future = worker.Enqueue([]() {
        std::cout << "Hello from worker thread!" << std::endl;
    });
    
    // 完了を待機
    future.wait();
    
    std::cout << "完了しました。" << std::endl;
    return 0;
}
```

## 3. ビルドして実行

```bash
# コンパイル
g++ -std=c++17 -I../include -pthread my_app.cpp ../build/libcallback_worker_thread.a -o my_app

# 実行
./my_app
```

## 4. よく使用されるパターン

### パターン1: 引数付きコールバック

```cpp
auto future = worker.Enqueue([](int id, const std::string& message) {
    std::cout << "Task " << id << ": " << message << std::endl;
}, 42, "処理中です");

future.wait();
```

### パターン2: 戻り値のあるコールバック

```cpp
auto future = worker.Enqueue([](int a, int b) -> int {
    return a + b;
}, 10, 20);

int result = future.get();  // 30
std::cout << "結果: " << result << std::endl;
```

### パターン3: 複数ワーカースレッド

```cpp
CallbackWorkerThread worker(4);  // 4つのスレッド

// 複数のタスクを投入
std::vector<std::future<void>> futures;
for (int i = 0; i < 10; ++i) {
    auto future = worker.Enqueue([i]() {
        std::cout << "Task " << i << " 実行中" << std::endl;
    });
    futures.push_back(std::move(future));
}

// 全て完了まで待機
for (auto& f : futures) {
    f.wait();
}
```

## 5. エラーハンドリング

```cpp
try {
    CallbackWorkerThread worker;
    
    auto future = worker.Enqueue([]() {
        throw std::runtime_error("何かエラーが発生");
    });
    
    future.wait();  // 例外は内部でキャッチされる
    
} catch (const std::exception& e) {
    std::cout << "エラー: " << e.what() << std::endl;
}
```

## 6. 実践的な使用例

### ファイル処理の並列化

```cpp
#include <filesystem>
#include <fstream>

CallbackWorkerThread worker(4);
std::vector<std::future<void>> futures;

// 複数ファイルを並列処理
for (const auto& entry : std::filesystem::directory_iterator("./data")) {
    if (entry.is_regular_file()) {
        auto future = worker.Enqueue([filepath = entry.path()]() {
            std::ifstream file(filepath);
            // ファイル処理ロジック
            std::cout << "処理完了: " << filepath << std::endl;
        });
        futures.push_back(std::move(future));
    }
}

// 全ファイル処理の完了を待機
for (auto& f : futures) {
    f.wait();
}
```

## 7. パフォーマンス考慮事項

- **適切なスレッド数**: CPU数と同程度、または少し多めが効果的
- **タスクの粒度**: 小さすぎるタスクはオーバーヘッドが大きい
- **メモリ使用**: 大量のタスクを一度に投入するとメモリを消費

```cpp
// 良い例: 適度な粒度のタスク
worker.Enqueue([&data]() {
    processLargeDataChunk(data);
});

// 悪い例: 過度に細かいタスク
for (int i = 0; i < 1000000; ++i) {
    worker.Enqueue([i]() { trivialOperation(i); });  // オーバーヘッドが大きい
}
```

## 8. デバッグのヒント

```cpp
// スレッドプールの状態確認
std::cout << "ワーカー数: " << worker.GetThreadCount() << std::endl;
std::cout << "待機タスク数: " << worker.GetQueueSize() << std::endl;

// 現在のスレッドID表示
worker.Enqueue([]() {
    std::cout << "実行スレッドID: " << std::this_thread::get_id() << std::endl;
});
```

## 次のステップ

- `examples/simple_example.cpp` で更なる使用例を確認
- `tests/` ディレクトリでテストコードを参照
- `include/callback_worker_thread/callback_worker_thread.h` でAPIドキュメントを確認

何か問題が発生した場合は、プロジェクトのREADME.mdを参照するか、Issueを作成してください。 