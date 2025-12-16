cmake_minimum_required(VERSION 2.8)

project(chatServer)



set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED on)


add_executable(${PROJECT_NAME}
    src/main.cpp
    src/chatServer.cpp)

target_include_directories(${PROJECT_NAME}
    PRIVATE
    ${PROJECT_SOURCE_DIR}/head)

target_link_libraries(chatServer pthread)