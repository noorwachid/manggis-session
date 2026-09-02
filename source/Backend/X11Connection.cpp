#include "Backend/X11Connection.hpp"

X11Connection::X11Connection() {
	_conn = xcb_connect(NULL, NULL);

	if (xcb_connection_has_error(_conn))
		return;

	_screen = xcb_setup_roots_iterator(xcb_get_setup(_conn)).data;
	_root = _screen->root;
}

X11Connection::~X11Connection() {
	xcb_disconnect(_conn);
}

xcb_connection_t* X11Connection::Get() {
	return _conn;
}

xcb_screen_t* X11Connection::GetScreen() {
	return _screen;
}

xcb_window_t X11Connection::GetRoot() {
	return _root;
}

void X11Connection::Flush() {
	xcb_flush(_conn);
}

