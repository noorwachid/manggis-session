#pragma once

#include "Backend/X11Connection.hpp"
#include "Input/MouseCursor.hpp"
#include <xcb/xcb_cursor.h>

class X11MouseManager {
public:
	X11MouseManager(X11Connection* conn);

	~X11MouseManager();

	void SetCursor(MouseCursor cursor);

private:
	void SetQueryCursor(char const* name);

private:
	X11Connection* _conn;
	xcb_cursor_context_t* _cursor;
};
