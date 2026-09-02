#include "core/Logger.hpp"
#include "backend/X11MouseManager.hpp"

X11MouseManager::X11MouseManager(X11Connection* conn): _conn{conn} {
	if (xcb_cursor_context_new(_conn->get(), _conn->getScreen(), &_cursor) < 0) {
		LOG("ERROR: failed to create cursor context\n");
		return;
	}
	setCursor(MouseCursor::Arrow);
}

X11MouseManager::~X11MouseManager() {
	xcb_cursor_context_free(_cursor);
}

void X11MouseManager::setCursor(MouseCursor cursor) {
	switch (cursor) {
		case MouseCursor::Arrow:
			_queryCursor("left_ptr");
			break;
	}
}

void X11MouseManager::_queryCursor(char const* name) {
	xcb_cursor_t cursorId = xcb_cursor_load_cursor(_cursor, name);
	xcb_change_window_attributes(_conn->get(), _conn->getScreen()->root, XCB_CW_CURSOR, &cursorId);
	_conn->flush();
}
