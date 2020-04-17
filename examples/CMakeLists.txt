# A simple CMakeLists.txt to build the examples in this directory.
# 
# This example file is released to the public domain.

cmake_minimum_required(VERSION 3.12)

project(serialport-examples LANGUAGES C)

find_package(serialport 0.1.2)

function(add_example ex)
	add_executable(${ex} ${ex}.c)
	target_link_libraries(${ex} PRIVATE serialport::serialport)
endfunction()

add_example("await_events")
add_example("handle_errors")
add_example("list_ports")
add_example("port_config")
add_example("port_info")
add_example("send_receive")
