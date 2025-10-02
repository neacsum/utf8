set(FETCHCONTENT_QUIET FALSE CACHE BOOL "Suppress output from FetchContent" FORCE)

# Cache FetchContent downloads outside build directory
set(FETCHCONTENT_BASE_DIR "${CMAKE_BINARY_DIR}/../.dep-cache" CACHE PATH "FetchContent cache directory")

# Include FetchContent for external dependencies
include(FetchContent)
