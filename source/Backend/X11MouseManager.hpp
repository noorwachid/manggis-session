#pragma once

#include "Common/Logger.hpp"
#include "Backend/X11Connection.hpp"
#include "Input/MouseCursor.hpp"
#include <xcb/xcb_cursor.h>

class X11MouseManager {
public:
	X11MouseManager(X11Connection* conn): _conn{conn} {
	}

	void Init() {
		if (xcb_cursor_context_new(_conn->Get(), _conn->GetScreen(), &_cursor) < 0) {
			LOG("ERROR: failed to create cursor context\n");
			return;
		}
		SetCursor(MouseCursor::Arrow);
	}

	void SetCursor(MouseCursor cursor) {
		switch (cursor) {
			case MouseCursor::Arrow:
				SetQueryCursor("left_ptr");
				break;
		}
	}

private:
	void SetQueryCursor(char const* name) {
		xcb_cursor_t cursorId = xcb_cursor_load_cursor(_cursor, name);
		xcb_change_window_attributes(_conn->Get(), _conn->GetScreen()->root, XCB_CW_CURSOR, &cursorId);
		_conn->Flush();
	}

private:
	X11Connection* _conn;
	xcb_cursor_context_t* _cursor;
};
