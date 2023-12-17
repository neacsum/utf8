cmake_minimum_required(VERSION 3.13)
project(utf8)

if (WIN32)
set(pfx $<IF:$<STREQUAL:${CMAKE_GENERATOR_PLATFORM},win32>,x86,x64>)
else()
set(pfx "")
endif()

add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
add_definitions(-DUNICODE -D_UNICODE)

add_subdirectory(tools/gen_casetab)
add_subdirectory(src)
add_subdirectory(tests)

if (BUILD_EXAMPLES)
  add_subdirectory(examples)
endif ()
