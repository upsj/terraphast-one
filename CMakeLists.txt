project(terraphast C CXX)

cmake_minimum_required(VERSION 3.5)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

Option(USE_GMP "USE_GMP" OFF)
Option(USE_THREADS "USE_THREADS" OFF)
Option(BUILD_TESTS "BUILD_TESTS" OFF)
Option(BUILD_C_WRAPPER "BUILD_C_WRAPPER" OFF)
Option(DEV_ENVIRONMENT "DEV_ENVIRONMENT" OFF)

#####################################################################
# create sane environment
#####################################################################
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_EXTENSIONS OFF)

#####################################################################
# set platform-specific options
#####################################################################
if(WIN32)
	include_directories("platform/windows")
	set(BASIC_WARNINGS "-W4")
	set(OPTIMIZATION_FLAGS "-Oi")
else()
	include_directories("platform/linux")
	set(BASIC_WARNINGS "-Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion")
	set(OPTIMIZATION_FLAGS "-march=native")
endif()

#####################################################################
# find required libraries
#####################################################################
set(LIBRARIES "")
if (USE_GMP)
	find_package(GMP REQUIRED)
	include_directories(GMP_INCLUDE_DIR)
	set(LIBRARIES ${LIBRARIES} ${GMP_LIBRARIES} ${GMPXX_LIBRARIES})
endif()
if (USE_THREADS)
	find_package(Threads REQUIRED)
	set(LIBRARIES ${LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
endif()

# Use debug version of the standard library
if(DEV_ENVIRONMENT AND CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GLIBCXX_DEBUG -Werror")
endif()

#####################################################################
# general flags 
#####################################################################
include_directories("include" "c_include")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${BASIC_WARNINGS} ${OPTIMIZATION_FLAGS}")

#####################################################################
# build library
#####################################################################
file(GLOB lib_headers "include/terraces/*.hpp")
file(GLOB lib_sources "lib/*.cpp")
file(GLOB lib_int_headers "lib/*.hpp")
add_library(terraces ${lib_sources} ${lib_headers} ${lib_int_headers})
target_link_libraries(terraces ${LIBRARIES})

#####################################################################
# build C library - this requires GMP!
#####################################################################
if(BUILD_C_WRAPPER)
	if(!USE_GMP)
		message(FATAL_ERROR "The C interface requires GMP to function")
	endif()
	file(GLOB c_lib_headers "c_include/terraces/*.h")
	file(GLOB c_lib_sources "c_lib/*.cpp")
	file(GLOB c_lib_int_headers "c_lib/*.hpp")
	add_library(c_terraces ${c_lib_sources} ${c_lib_headers} ${c_lib_int_headers})
	target_link_libraries(c_terraces terraces)
	target_link_libraries(terraces ${LIBRARIES})
endif()

#####################################################################
# build tests
#####################################################################
if(BUILD_TESTS)
	include_directories("Catch/include")
	file(GLOB test_headers "test/*.hpp")
	file(GLOB test_sources "test/*.cpp")
	add_executable(unittests ${test_sources} ${test_headers})
	target_link_libraries(unittests terraces ${LIBRARIES})
	add_test(NAME unittests COMMAND unittests)
	enable_testing()
endif()

file(GLOB app_sources "app/*.cpp")
file(GLOB tool_sources "tools/*.cpp")

#####################################################################
# build example app
#####################################################################
add_executable(app "app/app.cpp")
target_link_libraries(app terraces ${LIBRARIES})

#####################################################################
# build helper tools
#####################################################################
add_executable(isomorphic "tools/isomorphic.cpp")
add_executable(reroot "tools/reroot.cpp")
add_executable(subtree "tools/subtree.cpp")
add_executable(tree_gen "tools/tree_gen.cpp")
add_executable(site_gen "tools/site_gen.cpp")
target_link_libraries(isomorphic terraces)
target_link_libraries(reroot terraces)
target_link_libraries(subtree terraces)
target_link_libraries(tree_gen terraces)
target_link_libraries(site_gen terraces)

# Most of our files only compile with disabled language extensions for VC++
# Unfortunately, Catch uses some windows-specific features, so we have to
# enable these extensions for the Catch main method (more specific: not disable them)
if(BUILD_TESTS AND WIN32)
	file(GLOB test_main_source "test/main.cpp")
	list(REMOVE_ITEM test_sources ${test_main_source})
	set_source_files_properties(${lib_sources} ${test_sources} ${app_sources} ${tool_sources} PROPERTIES COMPILE_FLAGS "-Za")
endif()
