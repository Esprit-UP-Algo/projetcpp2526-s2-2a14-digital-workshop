cmake_minimum_required(VERSION 3.16)
project(GestionCollaborative)

set(CMAKE_CXX_STANDARD 17)

find_package(Qt6 REQUIRED COMPONENTS Widgets Sql)

add_executable(GestionCollaborative
    main.cpp
    utulisateur/loginwindow.h
    utulisateur/loginwindow.cpp
    utulisateur/userwindow.cpp
    utulisateur/userwindow.h
    connection/connection.h
    connection/connection.cpp
    matriel/matriele.h
    matriel/matriele.cpp
    matriel/qrcodegen.hpp
    matriel/qrcodegen.cpp
)

target_link_libraries(GestionCollaborative PRIVATE Qt6::Widgets Qt6::Sql)
set_target_properties(GestionCollaborative PROPERTIES AUTOMOC ON AUTOUIC ON)

target_include_directories(GestionCollaborative PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/utulisateur
    ${CMAKE_CURRENT_SOURCE_DIR}/matriel
    ${CMAKE_CURRENT_SOURCE_DIR}/connection
)
