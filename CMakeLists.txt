cmake_minimum_required(VERSION 3.15)
project(manggis-session)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SOURCES
	source/EntryPoint.cpp
	source/Monitor.cpp
	source/Workspace.cpp
	source/Window.cpp
	source/Session.cpp
	source/Logger.cpp
	source/backend/X11Context.cpp
	source/backend/X11MonitorManager.cpp
	source/backend/X11Hint.cpp
	source/system/Exec.cpp
	source/config/ConfigParser.cpp
)

option(XEPHYR_ENABLE "Enable Xephyr sanboxing" OFF)
option(XEPHYR_EMULATE_TWO_MONITOR "Emulate Xephyr two monitor" OFF)

if(XEPHYR_ENABLE)
	add_definitions(-DXEPHYR_ENABLE)
	add_definitions(-DXEPHYR_WIDTH=1200)
	add_definitions(-DXEPHYR_HEIGHT=600)
endif()

if (XEPHYR_EMULATE_TWO_MONITOR)
	add_definitions(-DXEPHYR_EMULATE_TWO_MONITOR)
endif()

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

	xcb-cursor

	xcb-randr
	xcb-shape
	xcb-ewmh
	xcb-icccm
)



