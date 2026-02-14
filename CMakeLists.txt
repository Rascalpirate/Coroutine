cmake_minimum_required(VERSION 3.16)
project(CoroutineEvolutionDemo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_ASIO_DEMOS "Build Boost.Asio callback demos" ON)

add_subdirectory(demo/01_blocking)
add_subdirectory(demo/02_epoll)
add_subdirectory(demo/04_future_promise)
add_subdirectory(demo/05_cps)
add_subdirectory(demo/06_coroutine)
add_subdirectory(demo/03_asio_callback)

message(STATUS "Project configured. BUILD_ASIO_DEMOS=${BUILD_ASIO_DEMOS}")
