# File: Client/CMakeLists.txt
cmake_minimum_required(VERSION 3.16)
project(FileClient LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Bật tự động xử lý Qt MOC, UIC, RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network)

# Thư mục chứa file .h
include_directories(include)
include_directories(../Common) 

# Thêm tất cả file nguồn .cpp
file(GLOB_RECURSE CLIENT_SOURCES 
    "src/*.cpp" 
    "src/modules/*.cpp"
)

# Thêm tất cả file header .h
file(GLOB_RECURSE CLIENT_HEADERS 
    "include/*.h"
)

add_executable(FileClient ${CLIENT_SOURCES} ${CLIENT_HEADERS})

target_link_libraries(FileClient PRIVATE Qt6::Widgets Qt6::Network)