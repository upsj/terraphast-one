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
		lib/bipartitions.hpp
		lib/bitmatrix.cpp
		lib/bits.hpp
		lib/bitvector.hpp
		lib/clamped_uint.cpp
		lib/constraints.cpp
		lib/multitree.cpp
		lib/multitree.hpp
		lib/multitree_impl.hpp
		lib/multitree_iterator.cpp
		lib/multitree_iterator.hpp
		lib/nodes.cpp
		lib/parser.cpp
		lib/ranked_bitvector.hpp
		lib/rooting.cpp
		lib/simple.cpp
		lib/small_bipartition.hpp
		lib/stack_allocator.hpp
		lib/subtree_extraction.cpp
		lib/supertree_enumerator.hpp
		lib/supertree_helpers.cpp
		lib/supertree_helpers.hpp
		lib/supertree_variants.hpp
		lib/supertree_variants_debug.hpp
		lib/supertree_variants_multitree.hpp
		lib/trees.cpp
		lib/trees_impl.hpp
		lib/union_find.cpp
		lib/union_find.hpp
		lib/union_find_debug.hpp
		lib/utils.hpp
		lib/validation.cpp
		lib/validation.hpp
		include/terraces/advanced.hpp
		include/terraces/bigint.hpp
		include/terraces/bitmatrix.hpp
		include/terraces/clamped_uint.hpp
		include/terraces/constraints.hpp
		include/terraces/definitions.hpp
		include/terraces/errors.hpp
		include/terraces/io_utils.hpp
		include/terraces/parser.hpp
		include/terraces/rooting.hpp
		include/terraces/simple.hpp
		include/terraces/subtree_extraction.hpp
		include/terraces/trees.hpp
)
target_include_directories(terraces
		PUBLIC include
		PRIVATE lib
)
find_package(Threads REQUIRED)
find_package(OpenMP REQUIRED)
target_link_libraries(terraces Threads::Threads OpenMP::OpenMP_CXX)

find_package(GMP)
if(GMP_FOUND)
	message(STATUS "GMP libraries found")
	target_link_libraries(terraces gmpxx gmp)
	target_compile_definitions(terraces PUBLIC USE_GMP)
else()
	message(STATUS "GMP libraries not found, falling back to clamped integers")
endif()

add_library(terraces_c c_lib/terraces.cpp c_include/terraces/terraces.h)
target_include_directories(terraces_c PUBLIC c_include)
target_link_libraries(terraces_c terraces)

#####################################################################
# internal compiler flags
#####################################################################
if(DEV_ENVIRONMENT AND CMAKE_BUILD_TYPE STREQUAL "Debug")
	target_compile_definitions(terraces PUBLIC _GLIBCXX_DEBUG) # PUBLIC to maintain ABI compatibility
	target_compile_definitions(terraces_c PRIVATE _GLIBCXX_DEBUG) # PRIVATE since no stdlib objects are used
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

add_library(Catch INTERFACE)
target_include_directories(Catch INTERFACE ${CMAKE_SOURCE_DIR}/Catch/single_include)

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
		test/advanced.cpp
)
target_link_libraries(unittests terraces terraces_c Catch)
add_test(NAME unittests COMMAND unittests)
enable_testing()

set_target_properties(terraces terraces_c app isomorphic reroot subtree tree_gen site_gen unittests PROPERTIES CXX_STANDARD 14)

#####################################################################
# set platform-specific options, include platform-specific files
#####################################################################
if(WIN32)
	target_include_directories(terraces PUBLIC lib/cl)
	target_include_directories(unittests PRIVATE lib/cl)
	target_compile_options(terraces PRIVATE -Oi -W4)
	target_compile_options(terraces_c PRIVATE -W4)

	# Most of our files only compile with disabled language extensions for VC++
	# Unfortunately, Catch uses some windows-specific features, so we have to
	# enable these extensions for the Catch main method (more specific: not disable them)
	file(GLOB ALL_SOURCES lib/*.cpp c_lib/*.cpp test/*.cpp tools/*.cpp app/*.cpp)
	list(REMOVE_ITEM ALL_SOURCES "test/main.cpp")
	set_source_files_properties(${ALL_SOURCES} PROPERTIES COMPILE_FLAGS "-Za")
else()
	target_include_directories(terraces PUBLIC lib/gcc_clang)
	target_include_directories(unittests PRIVATE lib/gcc_clang)
	target_compile_options(terraces PRIVATE -march=native -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Werror)
	target_compile_options(terraces_c PRIVATE -Werror)
endif()
