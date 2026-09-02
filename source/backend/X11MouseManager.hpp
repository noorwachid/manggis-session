#pragma once

#include "backend/X11Connection.hpp"
#include "input/MouseCursor.hpp"
#include <xcb/xcb_cursor.h>

class X11MouseManager {
public:
	X11MouseManager(X11Connection* conn);

	~X11MouseManager();

	void setCursor(MouseCursor cursor);

private:
	void _queryCursor(char const* name);

private:
	X11Connection* _conn;
	xcb_cursor_context_t* _cursor;
};
