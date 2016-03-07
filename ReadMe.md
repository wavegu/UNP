#Read Me

## How to use "unp.h" in C++
* compile and config package "unpv13e" (you can find out how to do this anywhere)
* main.cpp
	
		extern "C"{
			#include "unp.h"
		}

* compiling
	* g++ -o main main.cpp /usr/lib/libunp.a
	* gcc -o main main.c -lunp (if you wanna use C style)
	* cmakelist.txt (if you use cmake in Clion to build the project)
		
			cmake_minimum_required(VERSION 3.1)
			project(udp_server)
			
			link_libraries(libunp.a)
			
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
			set(SOURCE_FILES
			    main.cpp)
			add_executable(udp_server ${SOURCE_FILES})