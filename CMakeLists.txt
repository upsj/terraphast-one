project(terraphast C CXX)

cmake_minimum_required(VERSION 3.5)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

Option(DEV_ENVIRONMENT "DEV_ENVIRONMENT" OFF)

#####################################################################
# build library
#####################################################################
add_library(terraces
		lib/advanced.cpp
        lib/bigint.cpp
		lib/bipartitions.cpp
		lib/bitmatrix.cpp
        lib/clamped_uint.cpp
		lib/constraints.cpp
		lib/multitree.cpp
		lib/multitree_iterator.cpp
		lib/nodes.cpp
		lib/parser.cpp
		lib/rooting.cpp
		lib/simple.cpp
		lib/subtree_extraction.cpp
		lib/supertree_helpers.cpp
		lib/trees.cpp
		lib/union_find.cpp
		lib/validation.cpp
)
target_include_directories(terraces
		PUBLIC include
		PRIVATE lib
)
find_package(Threads REQUIRED)
target_link_libraries(terraces Threads::Threads)

find_package(GMP)
if(GMP_FOUND)
    target_link_libraries(terraces gmpxx gmp)
    target_compile_definitions(terraces PUBLIC USE_GMP)
endif()

add_library(terraces_c c_lib/terraces.cpp)
target_include_directories(terraces_c PUBLIC c_include)
target_link_libraries(terraces_c terraces)

#####################################################################
# internal compiler flags
#####################################################################
target_compile_options(terraces PRIVATE -Werror)
target_compile_options(terraces_c PRIVATE -Werror)
if(DEV_ENVIRONMENT AND CMAKE_BUILD_TYPE STREQUAL "Debug")
	target_compile_definitions(terraces PUBLIC _GLIBCXX_DEBUG) # PUBLIC to maintain ABI compatibility
	target_compile_definitions(terraces_c PRIVATE _GLIBCXX_DEBUG) # PRIVATE since no stdlib objects are used
endif()

#####################################################################
# set platform-specific options, include platform-specific files
#####################################################################
if(WIN32)
	target_sources(terraces PRIVATE lib/intrinsics_cl.cpp)
	target_compile_options(terraces PRIVATE -Oi -W4)
else()
	target_sources(terraces PRIVATE lib/intrinsics_gcc_clang.cpp)
	target_compile_options(terraces PRIVATE -march=native -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

#####################################################################
# build tools
#####################################################################
add_executable(app "app/app.cpp")
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
target_link_libraries(app terraces)

#####################################################################
# build tests
#####################################################################

add_executable(unittests
        test/main.cpp
        test/bipartitions.cpp
        test/bitmatrix.cpp
        test/bits.cpp
        test/bitvector.cpp
        test/clamped_uint.cpp
        test/constraints.cpp
        test/fast_set.cpp
        test/integration.cpp
        test/multitree_iterator.cpp
        test/parser.cpp
        test/rooting.cpp
        test/small_bipartition.cpp
        test/stack_allocator.cpp
        test/subtree_extraction.cpp
        test/supertree.cpp
        test/trees.cpp
        test/union_find.cpp
        test/util.cpp
        test/validation.cpp
        test/c_api.cpp
        test/c_api_gmp.cpp
)
target_link_libraries(unittests terraces terraces_c)
add_test(NAME unittests COMMAND unittests)
enable_testing()

set_target_properties(terraces terraces_c app isomorphic reroot subtree tree_gen site_gen unittests PROPERTIES CXX_STANDARD 14)

# Most of our files only compile with disabled language extensions for VC++
# Unfortunately, Catch uses some windows-specific features, so we have to
# enable these extensions for the Catch main method (more specific: not disable them)
if(WIN32)
	file(GLOB test_main_source "test/main.cpp")
	list(REMOVE_ITEM test_sources ${test_main_source})
	set_source_files_properties(${lib_sources} ${test_sources} ${app_sources} ${tool_sources} PROPERTIES COMPILE_FLAGS "-Za")
endif()
