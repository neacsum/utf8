add_executable(sample sample.cpp)

# All link directories are subfolders of ./lib
target_link_directories (sample PUBLIC ${CMAKE_SOURCE_DIR}/lib/${pfx}/$<CONFIG>)

# Add dependent libraries
add_dependencies(sample utf8)
target_link_libraries (sample PRIVATE utf8.lib)

