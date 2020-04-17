##
## This file is part of the libserialport project.
##
## Copyright (C) 2020 Uwe Hermann <uwe@hermann-uwe.de>
##
## This program is free software: you can redistribute it and/or modify
## it under the terms of the GNU Lesser General Public License as
## published by the Free Software Foundation, either version 3 of the
## License, or (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU Lesser General Public License
## along with this program.  If not, see <http://www.gnu.org/licenses/>.
##

cmake_minimum_required(VERSION 3.12)

project(serialport VERSION 0.1.2 LANGUAGES C)

include(GNUInstallDirs)

find_package(Git QUIET)
if(Git_FOUND)
	execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} ERROR_QUIET
		OUTPUT_VARIABLE GITHASH OUTPUT_STRIP_TRAILING_WHITESPACE)
	execute_process(COMMAND
		${GIT_EXECUTABLE} name-rev --tags --name-only ${GITHASH}
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} ERROR_QUIET
		OUTPUT_VARIABLE GITTAG OUTPUT_STRIP_TRAILING_WHITESPACE)
	if((DEFINED GITTAG) AND ("${GITTAG}" STREQUAL "undefined"))
		string(APPEND PROJECT_VERSION "-git-${GITHASH}")
	endif()
endif()

option(BUILD_SHARED_LIBS "Shared (non-static) build" TRUE)
option(BUILD_EXAMPLES "Build examples" TRUE)
option(BUILD_DOCS "Build documentation" FALSE)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY VALUE RelWithDebInfo)
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS TRUE)

add_library(${PROJECT_NAME} "")
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}.c timing.c
	lib${PROJECT_NAME}_internal.h lib${PROJECT_NAME}.h
	${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}_export.h
	${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}_version.h)

set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "\
${CMAKE_CURRENT_SOURCE_DIR}/lib${PROJECT_NAME}.h;\
${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}_export.h;\
${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}_version.h")

# The last "compatibility version" and "current version" fields were 2.0.0
# on Mac OS X (in libserialport 0.1.1). The new ones need to be same or higher.
math(EXPR V "${PROJECT_VERSION_MAJOR} + 2")
set_target_properties(${PROJECT_NAME} PROPERTIES
	VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR}
	# Note: Requires cmake >= 3.17.
	MACHO_CURRENT_VERSION ${V}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH} MACHO_COMPATIBILITY_VERSION ${V})

include(CheckIncludeFile)
include(CheckSymbolExists)
include(CheckTypeSize)
include(CheckStructHasMember)

check_include_file("sys/file.h" HAVE_SYS_FILE_H)
check_symbol_exists("flock" "sys/file.h" HAVE_FLOCK)
check_symbol_exists("realpath" "stdlib.h" HAVE_REALPATH)
check_symbol_exists("clock_gettime" "time.h" HAVE_CLOCK_GETTIME)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	set(CMAKE_EXTRA_INCLUDE_FILES "linux/serial.h")
	check_type_size("struct serial_struct" STRUCT_SERIAL_STRUCT)
	set(CMAKE_EXTRA_INCLUDE_FILES "linux/termios.h")
	check_type_size("struct termios2" STRUCT_TERMIOS2)
	check_type_size("struct termiox" STRUCT_TERMIOX)
	unset(CMAKE_EXTRA_INCLUDE_FILES)
	check_symbol_exists("BOTHER" "linux/termios.h" HAVE_BOTHER)
	check_struct_has_member("struct termios" "c_ispeed"
		"linux/termios.h" STRUCT_TERMIOS_C_ISPEED)
	check_struct_has_member("struct termios" "c_ospeed"
		"linux/termios.h" STRUCT_TERMIOS_C_OSPEED)
	check_struct_has_member("struct termios2" "c_ispeed"
		"linux/termios.h" STRUCT_TERMIOS2_C_ISPEED)
	check_struct_has_member("struct termios2" "c_ospeed"
		"linux/termios.h" STRUCT_TERMIOS2_C_OSPEED)
	target_sources(${PROJECT_NAME} PRIVATE
		linux.c linux_termios.c linux_termios.h)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "FreeBSD")
	target_sources(${PROJECT_NAME} PRIVATE freebsd.c)
	target_link_libraries(${PROJECT_NAME} PUBLIC usb)
	set(SP_LIBS "-lusb")
elseif(WIN32)
	target_sources(${PROJECT_NAME} PRIVATE windows.c)
	target_link_libraries(${PROJECT_NAME} PUBLIC setupapi)
	set(SP_LIBS "-lsetupapi")
elseif(APPLE)
	target_sources(${PROJECT_NAME} PRIVATE macosx.c)
	find_library(IOKIT IOKit)
	find_library(CORE CoreFoundation)
	target_link_libraries(${PROJECT_NAME} PUBLIC ${IOKIT} ${CORE})
else()
	target_compile_definitions(${PROJECT_NAME} PRIVATE
		-DNO_ENUMERATION=1 -DNO_PORT_METADATA=1)
endif()

target_compile_features(${PROJECT_NAME} PRIVATE c_std_99)

target_compile_options(${PROJECT_NAME} PRIVATE
	$<IF:$<C_COMPILER_ID:MSVC>,/W4 /wd4706,-Wall -Wextra>)

target_include_directories(${PROJECT_NAME} PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(VISIBILITY_INLINES_HIDDEN TRUE)
include(GenerateExportHeader)
generate_export_header(${PROJECT_NAME} BASE_NAME lib${PROJECT_NAME}
	PREFIX_NAME "SP_" EXPORT_MACRO_NAME "API" NO_EXPORT_MACRO_NAME "PRIV"
	DEPRECATED_MACRO_NAME "DEPR" NO_DEPRECATED_MACRO_NAME "NO_DEPR")

include(CMakePackageConfigHelpers)
set(INST_INC_DIR ${CMAKE_INSTALL_INCLUDEDIR})
set(INST_LIB_DIR ${CMAKE_INSTALL_LIBDIR})
configure_package_config_file(${PROJECT_NAME}Config.cmake.in
	${PROJECT_NAME}Config.cmake PATH_VARS INST_INC_DIR INST_LIB_DIR
	INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})
write_basic_package_version_file(
	${PROJECT_NAME}ConfigVersion.cmake COMPATIBILITY SameMajorVersion)

install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}Targets
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT ${PROJECT_NAME}Targets
	FILE ${PROJECT_NAME}Targets.cmake NAMESPACE ${PROJECT_NAME}::
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
	${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}.pc
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

message(STATUS "CMake: ${CMAKE_COMMAND} ${CMAKE_VERSION}")
message(STATUS "Building lib${PROJECT_NAME} ${PROJECT_VERSION} for ${CMAKE_SYSTEM} ${CMAKE_SYSTEM_PROCESSOR}")

include(CTest)
if(BUILD_TESTING)
	enable_testing()
	add_executable(unittests timing.c test_timing.c)
	target_compile_options(unittests PRIVATE
		$<IF:$<C_COMPILER_ID:MSVC>,/W4 /wd4706,-Wall -Wextra>)
	target_link_libraries(unittests PRIVATE ${PROJECT_NAME})
	add_test(NAME unittests COMMAND unittests)
endif()

function(add_example ex)
	add_executable(example_${ex} examples/${ex}.c)
	set_target_properties(example_${ex} PROPERTIES FOLDER examples)
	target_compile_options(example_${ex} PRIVATE
		$<IF:$<C_COMPILER_ID:MSVC>,/W4 /wd4706,-Wall -Wextra>)
	target_link_libraries(example_${ex} PRIVATE ${PROJECT_NAME})
endfunction()

if(BUILD_EXAMPLES)
	add_example("await_events")
	add_example("handle_errors")
	add_example("list_ports")
	add_example("port_config")
	add_example("port_info")
	add_example("send_receive")
endif()

find_package(Doxygen COMPONENTS doxygen)
if(Doxygen_FOUND AND BUILD_DOCS)
	set(S ${CMAKE_CURRENT_BINARY_DIR}/doxy/html-api)
	set(DF ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
	set(D ${CMAKE_INSTALL_DATADIR}/doc/lib${PROJECT_NAME})
	add_custom_command(OUTPUT ${S}/index.html
		DEPENDS ${DF} ${PROJECT_NAME} COMMAND Doxygen::doxygen ${DF}
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
	add_custom_target(doc ALL DEPENDS ${PROJECT_NAME} ${S}/index.html)
	install(DIRECTORY ${S} DESTINATION ${D} PATTERN "html-api/*" EXCLUDE)
	install(DIRECTORY ${S} DESTINATION ${D} MESSAGE_NEVER)
endif()

set(CPACK_SOURCE_PACKAGE_FILE_NAME "lib${PROJECT_NAME}-${PROJECT_VERSION}")
set(CPACK_SOURCE_IGNORE_FILES ${CMAKE_CURRENT_BINARY_DIR} ".git*")
set(CPACK_SOURCE_GENERATOR "TGZ")
include(CPack)

configure_file(config.h.in config.h @ONLY)
configure_file(lib${PROJECT_NAME}_version.h.in lib${PROJECT_NAME}_version.h @ONLY)
configure_file(lib${PROJECT_NAME}.pc.in lib${PROJECT_NAME}.pc @ONLY)
configure_file(Doxyfile.in Doxyfile @ONLY)
