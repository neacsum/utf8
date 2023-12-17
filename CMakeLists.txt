cmake_minimum_required(VERSION 3.13)
project(utf8)

add_library(${PROJECT_NAME})

if (WIN32)
set(pfx $<IF:$<STREQUAL:${CMAKE_GENERATOR_PLATFORM},win32>,x86,x64>)
else()
set(pfx "")
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
	ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib/${pfx}/$<CONFIG>
)

add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
add_definitions(-DUNICODE -D_UNICODE)

target_include_directories(${PROJECT_NAME} PUBLIC include)
target_include_directories(${PROJECT_NAME} PRIVATE src)

if (NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/src/uppertab.c OR 
    NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/src/lowertab.c)
  add_subdirectory(tools/gen_casetab)
endif()

add_subdirectory(src)
add_subdirectory(tests)

if (BUILD_EXAMPLES)
  add_subdirectory(examples)
endif ()

