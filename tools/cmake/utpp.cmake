FetchContent_Declare(
  utpp-repo
  GIT_REPOSITORY https://github.com/neacsum/utpp.git
  GIT_SHALLOW    TRUE
  GIT_PROGRESS   TRUE
  SOURCE_SUBDIR  ""  # Prevent any build system from being processed
  CONFIGURE_COMMAND "echo"
)
FetchContent_MakeAvailable(utpp-repo)

# Header-only interface target that depends on the fetched repo
add_library(utpp INTERFACE)

target_include_directories(utpp INTERFACE ${utpp-repo_SOURCE_DIR}/include)
target_sources(utpp INTERFACE ${utpp-repo_SOURCE_DIR}/include/utpp/utpp.h)

# target_include_directories(utpp INTERFACE ${PROJECT_SOURCE_DIR}/include)
# target_sources(utpp INTERFACE  ${PROJECT_SOURCE_DIR}/include/utpp_shim.h)
