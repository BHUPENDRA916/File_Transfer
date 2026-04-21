cmake_minimum_required(VERSION 3.16)
project(file_transfer_system LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(transfer
    src/main.cpp
    src/Logger.cpp
    src/StateStore.cpp
    src/Checksum.cpp
    src/FileCopier.cpp
    src/FileScanner.cpp
    src/WorkerPool.cpp
)

target_include_directories(transfer PRIVATE include)