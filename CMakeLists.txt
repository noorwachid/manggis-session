cmake_minimum_required(VERSION 3.15)
project(manggis-session)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SOURCES
	source/EntryPoint.cpp
	source/backend/X11Context.cpp
	source/system/Exec.cpp
)

add_definitions(-DUSE_XEPHYR)
add_executable(manggis-session ${SOURCES})

add_subdirectory(vendor/wxcore)
add_subdirectory(vendor/wxserializer)

target_include_directories(manggis-session
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/source
)

target_link_libraries(manggis-session
	wxcore
	wxserializer
	xcb

	xcb-keysyms
	xcb-xkb
	xkbcommon
	xkbcommon-x11

	xcb-randr
)



