#pragma once

#include "backend/X11Connection.hpp"
#include "input/Key.hpp"
#include "wx/core/String.hpp"

#ifdef __clang__
	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Wkeyword-macro"
#endif

#define explicit explicit_

#ifdef __clang__
	#pragma clang diagnostic pop
#endif

#include <xcb/xkb.h>

#undef explicit

#include <xkbcommon/xkbcommon.h>
#include <xkbcommon/xkbcommon-x11.h>

class X11KeyboardManager {
public:
	X11KeyboardManager(X11Connection* conn);

	xcb_keycode_t queryKeyCode(Key key);

	int getEventOffset();

	wx::String getLayout();

	void updateKeymap();

private:
	X11Connection* _conn;
	xkb_context* _keyboard;
	int _keyboardDeviceId;
	xkb_keymap* _keymap;
	uint8_t _keyboardEventOffset;
};
