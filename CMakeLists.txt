cmake_minimum_required(VERSION 3.10)
project(BakuSubway)

# Устанавливаем стандарт C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Добавляем библиотеку BakuSubway
add_library(BakuSubwayLib STATIC BakuSubway.cpp)
target_include_directories(BakuSubwayLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Добавляем исполняемый файл
add_executable(BakuSubway main.cpp)

# Линкуем библиотеку с исполняемым файлом
target_link_libraries(BakuSubway PRIVATE BakuSubwayLib)

# Добавляем поддержку потоков
find_package(Threads REQUIRED)
target_link_libraries(BakuSubway PRIVATE Threads::Threads)
