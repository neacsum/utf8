add_library(${PROJECT_NAME})

set_target_properties(${PROJECT_NAME} PROPERTIES
	ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib/${pfx}/$<CONFIG>
  CXX_STANDARD 17
)

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)

add_custom_command(
  OUTPUT ${PROJECT_SOURCE_DIR}/include/uppertab.h ${PROJECT_SOURCE_DIR}/include/lowertab.h
  COMMAND $<TARGET_FILE:gen_casetab> ${PROJECT_SOURCE_DIR}/data/UnicodeData.txt ${PROJECT_SOURCE_DIR}/include
  MAIN_DEPENDENCY ${PROJECT_SOURCE_DIR}/data/UnicodeData.txt
  DEPENDS gen_casetab
  VERBATIM
)
target_sources(${PROJECT_NAME}
	PRIVATE ${PROJECT_SOURCE_DIR}/include/uppertab.h ${PROJECT_SOURCE_DIR}/include/lowertab.h
)

target_sources(${PROJECT_NAME} PUBLIC 
  casecvt.cpp 
  ini.cpp
  utf8.cpp 
)

# Windows specific stuff
if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
target_sources(${PROJECT_NAME} PUBLIC 
  buffer.cpp
  win.cpp
)
endif ()
