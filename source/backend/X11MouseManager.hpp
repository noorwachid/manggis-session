#pragma once

#include "backend/X11Connection.hpp"
#include "input/MouseCursor.hpp"
#include <xcb/xcb_cursor.h>
#include <cstdio>

class X11MouseManager {
public:
	X11MouseManager(X11Connection* conn): _conn{conn} {
	}

	void initialize() {
		if (xcb_cursor_context_new(_conn->get(), _conn->getScreen(), &_cursor) < 0) {
			printf("ERROR: failed to create cursor context\n");
			return;
		}
		setCursor(MouseCursor::Arrow);
	}

	void setCursor(MouseCursor cursor) {
		switch (cursor) {
			case MouseCursor::Arrow:
				_setCursor("left_ptr");
				break;
		}
	}

private:
	void _setCursor(char const* name) {
		xcb_cursor_t cursorId = xcb_cursor_load_cursor(_cursor, name);
		xcb_change_window_attributes(_conn->get(), _conn->getScreen()->root, XCB_CW_CURSOR, &cursorId);
		_conn->flush();
	}

private:
	X11Connection* _conn;
	xcb_cursor_context_t* _cursor;
};
