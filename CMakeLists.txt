cmake_minimum_required(VERSION 3.28)
project(utf8)

if (WIN32)
set(pfx $<IF:$<STREQUAL:${CMAKE_GENERATOR_PLATFORM},win32>,x86,x64>)
else()
set(pfx "")
endif()

add_compile_definitions(_UNICODE UNICODE)

# https://wg21.link/p2513
if(MSVC)
  add_compile_options(/Zc:char8_t- /utf-8)
else()
  add_compile_options(-fno-char8_t)
endif()

set (CMAKE_CXX_STANDARD 17)


## configure CMake module search paths that depend on the project
## proj src dir is location of this file
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/tools/cmake)
include(ConfigFetchContent)
include(CompileCommands)
# include dependencies
include(utpp)

add_subdirectory(tools/gen_casetab)

add_library(${PROJECT_NAME})
set_target_properties(${PROJECT_NAME} PROPERTIES
	ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib/${pfx}/$<CONFIG>
)
add_subdirectory(src)
add_subdirectory(include)

set(BUILD_TESTS FALSE CACHE BOOL "Build tests")
if (BUILD_TESTS)
  add_subdirectory(tests)
endif ()

set(BUILD_EXAMPLES FALSE CACHE BOOL "Build examples")
if (BUILD_EXAMPLES)
  add_subdirectory(examples)
endif ()
