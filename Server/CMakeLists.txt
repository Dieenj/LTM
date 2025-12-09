cmake_minimum_required(VERSION 3.10)
project(FileServer)

set(CMAKE_CXX_STANDARD 17)

# 1. Khai báo thư mục Header (.h)
include_directories(Core/include)
include_directories(../Common) 

# 2. Tìm thư viện MySQL
find_package(PkgConfig REQUIRED)
pkg_check_modules(MYSQL REQUIRED mysqlclient)

# 3. Gom toàn bộ file Source (.cpp) trong các thư mục con
file(GLOB_RECURSE SERVER_SOURCES 
    "Core/src/*.cpp"
    "Core/src/db/*.cpp"
    "Core/src/handler/*.cpp"
    "Core/src/thread/*.cpp"
)

# 4. Tạo file thực thi
add_executable(FileServer ${SERVER_SOURCES})

# 5. Link thư viện pthread (đa luồng), MySQL và OpenSSL
target_link_libraries(FileServer pthread ${MYSQL_LIBRARIES} crypto)