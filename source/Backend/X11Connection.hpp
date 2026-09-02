#pragma once

#include <xcb/xcb.h>

class X11Connection {
public:
	X11Connection();

	~X11Connection();

	xcb_connection_t* Get();

	xcb_screen_t* GetScreen();

	xcb_window_t GetRoot();

	void Flush();

private:
	xcb_connection_t* _conn;
	xcb_screen_t* _screen;
	xcb_window_t _root;
};

