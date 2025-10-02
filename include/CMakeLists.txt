# target_include_directories(${PROJECT_NAME} PUBLIC utf8)
target_include_directories(${PROJECT_NAME} PUBLIC .)
target_sources(${PROJECT_NAME} PUBLIC
  utf8/ini.h
  utf8/utf8.h
)
# Windows specific stuff
if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
  target_sources(${PROJECT_NAME} PUBLIC
    utf8/winutf8.h
  )
endif ()

target_sources(${PROJECT_NAME} PRIVATE
  uppertab.h
  lowertab.h
)
