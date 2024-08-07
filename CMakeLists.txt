cmake_minimum_required(VERSION 3.22)

project(bookypedia-1 CXX)
set(CMAKE_CXX_STANDARD 20)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

add_executable(bookpedia
	src/main.cpp
	src/user_commands.h
	src/console_handler.h
	src/libpqxx_boost_uuid.h
)

target_link_libraries(bookpedia PRIVATE CONAN_PKG::boost PRIVATE CONAN_PKG::libpqxx)
