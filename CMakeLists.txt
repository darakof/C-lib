cmake_minimum_required(VERSION 3.14)
# C++ used for tests due to testing libraries being built for it
# C++ also provides built in implementations of similar functionality allowing us to test performance differences
project(Clib VERSION 1.0 LANGUAGES C CXX)

set(clib_SOURCE_FILES
	src/arenamem.c
	src/file.c
	src/hashtable.c
	src/poolmem.c
	src/array.c
	src/heapmem.c
	src/string.c
)

# build Clib (static)
add_library(clib STATIC
	${clib_SOURCE_FILES}
)

target_include_directories(clib 
	PUBLIC 
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
)

#build Clib (shared)
add_library(clib_shared SHARED
	${clib_SOURCE_FILES}
)

target_include_directories(clib_shared 
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
)

# make the shared library have same output name as the static one
set_target_properties(clib_shared
	PROPERTIES
		OUTPUT_NAME clib
)

include(FetchContent)

# setup criterion for tests
FetchContent_Declare(
    Criterion
    GIT_REPOSITORY https://github.com/Snaipe/Criterion.git
    GIT_TAG v2.4.3
)

FetchContent_MakeAvailable(Criterion)

set(CRITERION_SOURCE_DIR
    "${criterion_SOURCE_DIR}"
)

set(CRITERION_BUILD_DIR
    "${criterion_BINARY_DIR}/build"
)

set(CRITERION_LIBRARY
    "${CRITERION_BUILD_DIR}/src/libcriterion.so"
)

# Build Criterion with Meson.
add_custom_command(
    OUTPUT "${CRITERION_LIBRARY}"

    COMMAND
        meson setup .
        "${CRITERION_BUILD_DIR}"

    COMMAND
        meson compile
        -C "${CRITERION_BUILD_DIR}"

    WORKING_DIRECTORY
        "${CRITERION_SOURCE_DIR}"

    COMMENT
        "Building Criterion"
)

add_custom_target(
    criterion_build
    DEPENDS "${CRITERION_LIBRARY}"
)

# CMake representation of the library built by Meson.
add_library(
    Criterion::Criterion
    SHARED
    IMPORTED
    GLOBAL
)

set_target_properties(
    Criterion::Criterion
    PROPERTIES
        IMPORTED_LOCATION
            "${CRITERION_LIBRARY}"

        INTERFACE_INCLUDE_DIRECTORIES
            "${CRITERION_SOURCE_DIR}/include"
)

add_dependencies(
    Criterion::Criterion
    criterion_build
)

# create all tests (adding a file automatically adds the test)
# test files dont require the tests directory
set(TEST_FILES
	test_hashtable.c
)

enable_testing()

foreach(TEST_SRC ${TEST_FILES})
	# get the name without extension
	get_filename_component(TEST_NAME ${TEST_SRC} NAME_WE)
	
	add_executable(${TEST_NAME} tests/${TEST_SRC})
	
	target_link_libraries(${TEST_NAME} PRIVATE clib Criterion::Criterion)

	# enable sanitizers and other debug options for the test
	target_compile_options(${TEST_NAME} PRIVATE
		-g3 -O0 -fsanitize=address,undefined -fno-omit-frame-pointer)
	target_link_options(${TEST_NAME} PRIVATE
		-fsanitize=address,undefined)

	add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endforeach()

# performance benchmarks with C++

# stop google benchmarks from doing internal tests
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
	googlebenchmark
	GIT_REPOSITORY https://github.com/google/benchmark.git
	GIT_TAG v1.9.5
)
FetchContent_MakeAvailable(googlebenchmark)

add_executable(benchmarks benchmarks/benchmark_map.cpp)
target_link_libraries(benchmarks PRIVATE clib benchmark::benchmark_main)

target_compile_options(benchmarks PRIVATE
	-O3 -g -fno-omit-frame-pointer -march=native)

# install clib to the system
# exposes names that follow the standard convention in cmake
add_library(clib::Static ALIAS clib)
add_library(clib::Shared ALIAS clib_shared)

# install library files
install(
	TARGETS clib clib_shared
	EXPORT clibTargets
	ARCHIVE DESTINATION lib
	LIBRARY DESTINATION lib
	RUNTIME DESTINATION bin
)

# install headers
install(
	DIRECTORY include/
	DESTINATION include/clib
)

# install the library for cmake so you can use `find_package(clib)`
install(
	EXPORT clibTargets
	FILE clibTargets.cmake
	NAMESPACE clib::
	DESTINATION lib/cmake/clib
)
