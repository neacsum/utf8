add_executable(sample sample.cpp)

set_target_properties(sample PROPERTIES 
  CXX_STANDARD 17
  )

# All link directories are subfolders of ./lib
if (WIN32)
target_link_directories (sample PUBLIC ${CMAKE_SOURCE_DIR}/lib/${pfx}/$<CONFIG>)
else()
target_link_directories (sample PUBLIC ${CMAKE_SOURCE_DIR}/lib)
endif()

# Add dependent libraries
target_link_libraries (sample PRIVATE utf8)

