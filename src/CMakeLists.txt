add_library(${PROJECT_NAME})

set_target_properties(${PROJECT_NAME} PROPERTIES
	ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib/${pfx}/$<CONFIG>
)

target_include_directories(${PROJECT_NAME} PUBLIC ../include)
target_include_directories(${PROJECT_NAME} PRIVATE .)

add_custom_command(
  OUTPUT ${PROJECT_SOURCE_DIR}/src/uppertab.c ${PROJECT_SOURCE_DIR}/src/lowertab.c
  COMMAND $<TARGET_FILE:gen_casetab> ${PROJECT_SOURCE_DIR}/tools/gen_casetab/CaseFolding.txt ${PROJECT_SOURCE_DIR}/src
  MAIN_DEPENDENCY ${PROJECT_SOURCE_DIR}/tools/gen_casetab/CaseFolding.txt
  DEPENDS gen_casetab
  VERBATIM
)
target_sources(${PROJECT_NAME}
	PRIVATE uppertab.c lowertab.c
)

target_sources(${PROJECT_NAME}
	PUBLIC buffer.cpp casecvt.cpp ini.cpp utf8.cpp win.cpp
)
