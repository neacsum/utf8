add_executable(gen_casetab gen_casetab.cpp)

add_custom_command(
  TARGET gen_casetab
  COMMAND $<TARGET_FILE:gen_casetab> ${PROJECT_SOURCE_DIR}/tools/gen_casetab/CaseFolding.txt ${PROJECT_SOURCE_DIR}/src
  DEPENDS ${PROJECT_SOURCE_DIR}/tools/CaseFolding.txt
)