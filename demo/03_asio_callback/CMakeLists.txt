if(NOT BUILD_ASIO_DEMOS)
  message(STATUS "Skip Asio demos because BUILD_ASIO_DEMOS=OFF")
  return()
endif()

find_package(Boost 1.70 QUIET)
if(NOT Boost_FOUND)
  message(WARNING "Boost not found, skip demo/03_asio_callback targets. Install Boost or configure with -DBUILD_ASIO_DEMOS=OFF")
  return()
endif()

add_executable(asio_callback_server server.cpp)
add_executable(asio_callback_client client.cpp)

target_include_directories(asio_callback_server PRIVATE ${Boost_INCLUDE_DIRS})
target_include_directories(asio_callback_client PRIVATE ${Boost_INCLUDE_DIRS})
