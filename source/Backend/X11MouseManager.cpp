#include "Backend/X11MouseManager.hpp"
#include "Common/Logger.hpp"

X11MouseManager::X11MouseManager(X11Connection* conn): _conn{conn} {
	if (xcb_cursor_context_new(_conn->Get(), _conn->GetScreen(), &_cursor) < 0) {
		LOG("ERROR: failed to create cursor context\n");
		return;
	}
	SetCursor(MouseCursor::Arrow);
}

X11MouseManager::~X11MouseManager() {
	xcb_cursor_context_free(_cursor);
}

void X11MouseManager::SetCursor(MouseCursor cursor) {
	switch (cursor) {
		case MouseCursor::Arrow:
			SetQueryCursor("left_ptr");
			break;
	}
}

void X11MouseManager::SetQueryCursor(char const* name) {
	xcb_cursor_t cursorId = xcb_cursor_load_cursor(_cursor, name);
	xcb_change_window_attributes(_conn->Get(), _conn->GetScreen()->root, XCB_CW_CURSOR, &cursorId);
	_conn->Flush();
}
