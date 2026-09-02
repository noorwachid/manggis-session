#pragma once

#include <xcb/xcb.h>

class X11Connection {
public:
	X11Connection();

	~X11Connection();

	xcb_connection_t* get();

	xcb_screen_t* getScreen();

	xcb_window_t getRoot();

	void flush();

private:
	xcb_connection_t* _conn;
	xcb_screen_t* _screen;
	xcb_window_t _root;
};

