cmake_minimum_required(VERSION 3.21)
project(HospitalQt VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Sql)

add_executable(HospitalQt
    src/main.cpp
    src/database.cpp
    src/patientmodel.cpp
    src/dialogs.cpp
    src/loginpage.cpp
    src/workspace.cpp
    src/mainwindow.cpp
    src/capture.cpp
    src/selftest.cpp
)

target_include_directories(HospitalQt PRIVATE src)
target_link_libraries(HospitalQt PRIVATE Qt6::Widgets Qt6::Sql)
