add_executable(build_gen_casetab gen_casetab.cpp)
set_property(TARGET build_gen_casetab PROPERTY CXX_STANDARD 20)

# Custom command to generate the header files
add_custom_command(
  OUTPUT ${PROJECT_SOURCE_DIR}/include/uppertab.h ${PROJECT_SOURCE_DIR}/include/lowertab.h
  COMMAND $<TARGET_FILE:build_gen_casetab> ${PROJECT_SOURCE_DIR}/data/UnicodeData.txt ${PROJECT_SOURCE_DIR}/include
  MAIN_DEPENDENCY ${PROJECT_SOURCE_DIR}/data/UnicodeData.txt
  DEPENDS build_gen_casetab
  VERBATIM
  COMMENT "Generating Unicode case conversion tables"
)

# Custom target that depends on the generated files
add_custom_target(run_gen_casetab DEPENDS
  ${PROJECT_SOURCE_DIR}/include/uppertab.h
  ${PROJECT_SOURCE_DIR}/include/lowertab.h
)
