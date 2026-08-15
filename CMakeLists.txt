cmake_minimum_required(VERSION 3.14)
# C++ used for tests due to testing libraries being built for it
# C++ also provides built in implementations of similar functionality allowing us to test performance differences
project(Clib VERSION 1.0 LANGUAGES C CXX)

# build Clib
add_library(clib STATIC 
	src/arenamem.c 
	src/file.c 
	src/hashtable.c
	src/poolmem.c
	src/array.c
	src/heapmem.c
	src/string.c
)

target_include_directories(clib PUBLIC include)

# setup criterion for tests
find_package(PkgConfig REQUIRED)
pkg_check_modules(CRITERION REQUIRED criterion)

add_executable(unit_tests tests/test_suite.c)
target_include_directories(unit_tests PRIVATE ${CRITERION_INCLUDE_DIRS})
target_link_libraries(unit_tests PRIVATE clib ${CRITERION_LIBRARIES})

# enable sanitizers and other debug options for unit tests
target_compile_options(unit_tests PRIVATE
	-g3 -O0 -fsanitize=address,undefined -fno-omit-frame-pointer)
target_link_options(unit_tests PRIVATE
	-fsanitize=address,undefined)

# performance benchmarks with C++

# needed for google benchmark
include(FetchContent)

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
target_link_libraries(benchmarks PRIVATE clib benchmark::benchmark benchmark::benchmark_main)

target_compile_options(benchmarks PRIVATE
	-O3 -g -fno-omit-frame-pointer -march=native)
