add_executable(tests
  tests_ini.cpp tests_win.cpp tests_utf8.cpp
  tests.rc
)
target_include_directories(tests PRIVATE .)
set_property(TARGET tests PROPERTY CXX_STANDARD 20)
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT tests)
set_target_properties(tests PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests"
  # RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/tests"
  # RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/tests"
)

# Install the tests executable (helps with CMake extension discovery)
install(TARGETS tests
  RUNTIME DESTINATION tests
)

target_link_libraries (tests PRIVATE utf8 utpp)

# All link directories are subfolders of ./lib
target_link_directories (tests PUBLIC ${PROJECT_SOURCE_DIR}/lib/${pfx}/$<CONFIG>)
