target_sources(${PROJECT_NAME} PRIVATE
  casecvt.cpp
  ini.cpp
  utf8.cpp
)

# Set up dependency for casecvt.cpp on generated headers
add_dependencies(${PROJECT_NAME} run_gen_casetab)
set_source_files_properties(casecvt.cpp PROPERTIES
  OBJECT_DEPENDS "${PROJECT_SOURCE_DIR}/include/uppertab.h;${PROJECT_SOURCE_DIR}/include/lowertab.h"
)

# Windows specific stuff
if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
  target_sources(${PROJECT_NAME} PRIVATE
    buffer.cpp
    win.cpp
  )
endif ()
