cmake_minimum_required(VERSION 2.8)

set(PROJECT_NAME "TileImporter")

project(${PROJECT_NAME})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../resource/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/../../resource/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/../../resource/bin)

find_package(Gorgon REQUIRED)

add_compile_options(-std=c++14)

add_executable(TileImporter
    Main.cpp
)

linkgorgon(${PROJECT_NAME})

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../../bin/Icon.png ${CMAKE_CURRENT_SOURCE_DIR}/../../resource/bin/Icon.png COPYONLY)
