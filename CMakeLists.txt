cmake_minimum_required(VERSION 3.10)
project(TransportCatalogue LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SOURCES "transport-catalogue/*.cpp")

add_executable(transport_catalogue_ ${SOURCES})

target_include_directories(transport_catalogue_ PRIVATE "transport-catalogue")

