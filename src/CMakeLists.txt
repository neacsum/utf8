add_library(${PROJECT_NAME})

set_target_properties(${PROJECT_NAME} PROPERTIES
	ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib/${pfx}/$<CONFIG>
)

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)

add_custom_command(
  OUTPUT ${PROJECT_SOURCE_DIR}/include/uppertab.h ${PROJECT_SOURCE_DIR}/include/lowertab.h
  COMMAND $<TARGET_FILE:gen_casetab> ${PROJECT_SOURCE_DIR}/tools/gen_casetab/CaseFolding.txt ${PROJECT_SOURCE_DIR}/include
  MAIN_DEPENDENCY ${PROJECT_SOURCE_DIR}/tools/gen_casetab/CaseFolding.txt
  DEPENDS gen_casetab
  VERBATIM
)
target_sources(${PROJECT_NAME}
	PRIVATE ${PROJECT_SOURCE_DIR}/include/uppertab.h ${PROJECT_SOURCE_DIR}/include/lowertab.h
)

target_sources(${PROJECT_NAME}
	PUBLIC buffer.cpp casecvt.cpp ini.cpp utf8.cpp win.cpp
)
