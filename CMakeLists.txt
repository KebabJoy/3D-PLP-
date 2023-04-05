cmake_minimum_required(VERSION 3.25)
project(MainVKR)

set(CMAKE_CXX_STANDARD 17)
add_executable(MainVKR ./main.cpp)

find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(MainVKR PRIVATE nlohmann_json::nlohmann_json)

