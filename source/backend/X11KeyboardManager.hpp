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
	X11KeyboardManager(X11Connection* conn): _conn{conn} {
	}

	void initialize() {
		// Initialize libxkbcommon-x11
		if (!xkb_x11_setup_xkb_extension(
				_conn->get(), XKB_X11_MIN_MAJOR_XKB_VERSION,
				XKB_X11_MIN_MINOR_XKB_VERSION,
				XKB_X11_SETUP_XKB_EXTENSION_NO_FLAGS,
				NULL, NULL, &_keyboardEventOffset, NULL)) {
			printf("ERROR: XKB extension not available\n");
		}

		_keyboard = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
		if (!_keyboard) {
			printf("ERROR: failed to create xkb context\n");
		}

		_keyboardDeviceId = xkb_x11_get_core_keyboard_device_id(_conn->get());
		if (_keyboardDeviceId == -1) {
			printf("ERROR: failed to get core keyboard device\n");
		}

		_keymap = xkb_x11_keymap_new_from_device(_keyboard, _conn->get(), _keyboardDeviceId, XKB_KEYMAP_COMPILE_NO_FLAGS);

		const char *layout = xkb_keymap_layout_get_name(_keymap, 0);
		printf("initial keyboard layout is %s\n", layout);

		enum {
			required_events =
				(XCB_XKB_EVENT_TYPE_NEW_KEYBOARD_NOTIFY |
				 XCB_XKB_EVENT_TYPE_MAP_NOTIFY),

			required_nkn_details = XCB_XKB_NKN_DETAIL_KEYCODES,

			required_map_parts =
				(XCB_XKB_MAP_PART_KEY_TYPES |
				 XCB_XKB_MAP_PART_KEY_SYMS |
				 XCB_XKB_MAP_PART_MODIFIER_MAP |
				 XCB_XKB_MAP_PART_EXPLICIT_COMPONENTS |
				 XCB_XKB_MAP_PART_KEY_ACTIONS |
				 XCB_XKB_MAP_PART_VIRTUAL_MODS |
				 XCB_XKB_MAP_PART_VIRTUAL_MOD_MAP),

			required_state_details = XCB_NONE,
		};

		static const xcb_xkb_select_events_details_t details = {
			.affectNewKeyboard = required_nkn_details,
			.newKeyboardDetails = required_nkn_details,
			.affectState = required_state_details,
			.stateDetails = required_state_details,
		};

		xcb_void_cookie_t cookie = xcb_xkb_select_events_aux_checked(
			_conn->get(),
			_keyboardDeviceId,
			required_events,    /* affectWhich */
			0,                  /* clear */
			0,                  /* selectAll */
			required_map_parts, /* affectMap */
			required_map_parts, /* map */
			&details);          /* details */

		xcb_generic_error_t* error = xcb_request_check(_conn->get(), cookie);
		if (error) {
			printf("ERROR: listening keyboard notification\n");
			return;
		}
	}

	xcb_keycode_t queryKeyCode(Key key) {
		xkb_keysym_t targetKeysym = (xkb_keysym_t) key;

		for (xkb_keycode_t keycode = xkb_keymap_min_keycode(_keymap); keycode <= xkb_keymap_max_keycode(_keymap); keycode++) {
			int num_levels = xkb_keymap_num_levels_for_key(_keymap, keycode, 0);
			for (int level = 0; level < num_levels; level++) {
				const xkb_keysym_t *syms;
				int count = xkb_keymap_key_get_syms_by_level(_keymap, keycode, 0, level, &syms);
				for (int i = 0; i < count; i++) {
					if (syms[i] == targetKeysym) {
						return keycode;
					}
				}
			}
		}

		return 0;
	}


	int getEventOffset() {
		return _keyboardEventOffset;
	}

	wx::String getLayout() {
		const char *layout = xkb_keymap_layout_get_name(_keymap, 0);
		return layout;
	}

	void updateKeymap() {
		_keymap = xkb_x11_keymap_new_from_device(_keyboard, _conn->get(), _keyboardDeviceId, XKB_KEYMAP_COMPILE_NO_FLAGS);
	}


private:
	X11Connection* _conn;
	xkb_context* _keyboard;
	int _keyboardDeviceId;
	xkb_keymap* _keymap;
	uint8_t _keyboardEventOffset;
};
