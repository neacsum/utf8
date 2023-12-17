add_executable(gen_casetab gen_casetab.cpp)

add_custom_command(
  OUTPUT ${PROJECT_SOURCE_DIR}/src/uppertab.c ${PROJECT_SOURCE_DIR}/src/lowertab.c
  COMMAND $<TARGET_FILE:gen_casetab> ${PROJECT_SOURCE_DIR}/tools/gen_casetab/CaseFolding.txt ${PROJECT_SOURCE_DIR}/src
  MAIN_DEPENDENCY ${PROJECT_SOURCE_DIR}/tools/CaseFolding.txt
  VERBATIM
)
