cmake_minimum_required(VERSION 3.16)
project(GestionCollaborative)

set(CMAKE_CXX_STANDARD 17)
find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(GestionCollaborative
    main.cpp
    userwindow.cpp
    userwindow.h
)

target_link_libraries(GestionCollaborative PRIVATE Qt6::Widgets)
set_target_properties(GestionCollaborative PROPERTIES AUTOMOC ON)
