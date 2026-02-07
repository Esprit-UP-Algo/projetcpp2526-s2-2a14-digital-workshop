cmake_minimum_required(VERSION 3.16)

project(interfaceG VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Trouver Qt6 ou Qt5
find_package(Qt6 COMPONENTS Core Gui Widgets PrintSupport QUIET)
if (NOT Qt6_FOUND)
    find_package(Qt5 COMPONENTS Core Gui Widgets PrintSupport REQUIRED)
endif()

# Fichiers sources
set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
)

# Créer l'exécutable
add_executable(interfaceG ${PROJECT_SOURCES})

# Lier les bibliothèques Qt
if (Qt6_FOUND)
    target_link_libraries(interfaceG PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::PrintSupport
    )
else()
    target_link_libraries(interfaceG PRIVATE
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::PrintSupport
    )
endif()
