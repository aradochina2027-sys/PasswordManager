cmake_minimum_required(VERSION 3.5)

project(PasswordManager VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Додаємо модуль Concurrent сюди
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Sql Network Concurrent)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Sql Network Concurrent)

set(PROJECT_SOURCES
    main.cpp
    passwordmanager.cpp
    passwordmanager.h
    databasemanager.h
    passwordmanager.ui
)

add_executable(PasswordManager ${PROJECT_SOURCES})

# І сюди теж додаємо Qt${QT_VERSION_MAJOR}::Concurrent
target_link_libraries(PasswordManager PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Sql
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Concurrent
)
