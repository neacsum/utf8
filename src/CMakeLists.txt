target_sources(${PROJECT_NAME} 
	PRIVATE uppertab.c lowertab.c
)
target_sources(${PROJECT_NAME} 
	PUBLIC buffer.cpp casecvt.cpp ini.cpp utf8.cpp win.cpp
)

