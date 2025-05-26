cmake_minimum_required(VERSION 3.20)

project(CallbackWorkerThread 
        VERSION 1.0.0
        DESCRIPTION "A thread pool library for callback execution"
        LANGUAGES C CXX)

# C++17 を要求
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# コンパイル時に詳細出力
set(CMAKE_VERBOSE_MAKEFILE ON)

# ビルドタイプがDebugの場合、デバッグシンボルを有効化
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
endif()

# プラットフォーム固有の設定
if(WIN32)
    # Windows用設定
    add_compile_definitions(WIN32_LEAN_AND_MEAN)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
elseif(UNIX)
    # Linux用設定
    find_package(Threads REQUIRED)
endif()

# インクルードディレクトリ
include_directories(include)

# ライブラリのソースファイル
set(LIBRARY_SOURCES
    src/callback_worker_thread.cpp
    src/callback_worker_thread_c.cpp
)

# ライブラリの作成
add_library(callback_worker_thread STATIC ${LIBRARY_SOURCES})

# ライブラリのプロパティ設定
target_include_directories(callback_worker_thread
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# プラットフォーム固有のライブラリリンク
if(WIN32)
    # Windowsでは追加の設定は不要（STLのスレッドライブラリが自動的にリンクされる）
elseif(UNIX)
    target_link_libraries(callback_worker_thread PUBLIC Threads::Threads)
endif()

# コンパイラ固有の警告レベル設定
if(MSVC)
    target_compile_options(callback_worker_thread PRIVATE /W4)
else()
    target_compile_options(callback_worker_thread PRIVATE -Wall -Wextra -Wpedantic)
endif()

# 使用例をビルドするかどうかのオプション
option(BUILD_EXAMPLES "Build example programs" ON)

if(BUILD_EXAMPLES)
    add_executable(simple_example examples/simple_example.cpp)
    target_link_libraries(simple_example callback_worker_thread)
    
    # C言語の使用例を追加
    add_executable(c_example examples/c_example.c)
    target_link_libraries(c_example callback_worker_thread)
    
    # Cコンパイラでも適切にコンパイルされるように設定
    set_target_properties(c_example PROPERTIES
        LINKER_LANGUAGE CXX
    )
endif()

# テストをビルドするかどうかのオプション
option(BUILD_TESTS "Build tests" ON)

if(BUILD_TESTS)
    # GoogleTestの検索
    find_package(GTest QUIET)
    
    if(GTest_FOUND)
        enable_testing()
        
        add_executable(test_callback_worker_thread tests/test_callback_worker_thread.cpp)
        target_link_libraries(test_callback_worker_thread 
            callback_worker_thread 
            GTest::gtest 
            GTest::gtest_main
        )
        
        include(GoogleTest)
        gtest_discover_tests(test_callback_worker_thread)
        
        message(STATUS "GoogleTest found. Tests will be built.")
    else()
        message(WARNING "GoogleTest not found. Tests will not be built. To enable tests, install GoogleTest.")
    endif()
    
    # C言語のテストを追加（GoogleTestに依存しない）
    add_executable(test_callback_worker_thread_c tests/test_callback_worker_thread_c.c)
    target_link_libraries(test_callback_worker_thread_c callback_worker_thread)
    
    # Cコンパイラでも適切にコンパイルされるように設定
    set_target_properties(test_callback_worker_thread_c PROPERTIES
        LINKER_LANGUAGE CXX
    )
    
    # CTestにC言語テストを追加
    add_test(NAME test_c_interface COMMAND test_callback_worker_thread_c)
endif()

# インストール設定
include(GNUInstallDirs)

install(TARGETS callback_worker_thread
    EXPORT CallbackWorkerThreadTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# CMake設定ファイルのインストール
install(EXPORT CallbackWorkerThreadTargets
    FILE CallbackWorkerThreadTargets.cmake
    NAMESPACE CallbackWorkerThread::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/CallbackWorkerThread
)

# パッケージ設定ファイルの作成
include(CMakePackageConfigHelpers)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/CallbackWorkerThreadConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/CallbackWorkerThreadConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/CallbackWorkerThread
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/CallbackWorkerThreadConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/CallbackWorkerThreadConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/CallbackWorkerThreadConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/CallbackWorkerThread
)

# CPack設定（パッケージング用）
set(CPACK_PACKAGE_NAME "CallbackWorkerThread")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION ${PROJECT_DESCRIPTION})
set(CPACK_PACKAGE_CONTACT "your-email@example.com")

if(WIN32)
    set(CPACK_GENERATOR "NSIS;ZIP")
else()
    set(CPACK_GENERATOR "TGZ;DEB")
endif()

include(CPack) 