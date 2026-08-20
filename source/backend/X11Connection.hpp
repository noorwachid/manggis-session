#pragma once

#include <xcb/xcb.h>

class X11Connection {
public:
	X11Connection() {
		_conn = xcb_connect(NULL, NULL);

		if (xcb_connection_has_error(_conn))
			return;

		_screen = xcb_setup_roots_iterator(xcb_get_setup(_conn)).data;
		_root = _screen->root;
	}

	~X11Connection() {
		xcb_disconnect(_conn);
	}

	xcb_connection_t* get() {
		return _conn;
	}

	xcb_screen_t* getScreen() {
		return _screen;
	}

	xcb_window_t getRoot() {
		return _root;
	}

	void flush() {
		xcb_flush(_conn);
	}

private:
	xcb_connection_t* _conn;
	xcb_screen_t* _screen;
	xcb_window_t _root;
};

