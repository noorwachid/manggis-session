#include "Common/Unit.hpp"
#include "Backend/X11Hint.hpp"
#include "Backend/X11Connection.hpp"
#include <cstdio>
#include <xcb/xcb_icccm.h>
#include <xcb/xcb_ewmh.h>

X11Hint::X11Hint(X11Connection* conn): _conn{conn} {
}

void X11Hint::Init() {
	xcb_intern_atom_cookie_t* cookies = xcb_ewmh_init_atoms(_conn->Get(), &_ewmh);
	if (!xcb_ewmh_init_atoms_replies(&_ewmh, cookies, nullptr)) {
		printf("ERROR: failed to initialize EWMH atoms\n");
		return;
	}

	// ICCCM atoms
	xcb_intern_atom_reply_t* protocols = xcb_intern_atom_reply(_conn->Get(), xcb_intern_atom(_conn->Get(), 0, 12, "WM_PROTOCOLS"), nullptr);
	xcb_intern_atom_reply_t* deletes = xcb_intern_atom_reply(_conn->Get(), xcb_intern_atom(_conn->Get(), 0, 16, "WM_DELETE_WINDOW"), nullptr);
	xcb_intern_atom_reply_t* motifHints = xcb_intern_atom_reply(_conn->Get(), xcb_intern_atom(_conn->Get(), 0, 15, "_MOTIF_WM_HINTS"), nullptr);

	_wmPROTOCOLS = protocols->atom;
	_wmDELETE_WINDOW = deletes->atom;
	_wmMOTIF_HINTS = motifHints->atom;

	free(protocols);
	free(deletes);
	free(motifHints);

	xcb_atom_t atoms[] = {
		_ewmh._NET_SUPPORTED,
		_ewmh._NET_SUPPORTING_WM_CHECK,

		_ewmh._NET_ACTIVE_WINDOW,
		_ewmh._NET_CLOSE_WINDOW,
		_ewmh._NET_MOVERESIZE_WINDOW,

		_ewmh._NET_CLIENT_LIST,
		_ewmh._NET_FRAME_EXTENTS,

		_ewmh._NET_WM_NAME,
		_ewmh._NET_WM_MOVERESIZE,

		_ewmh._NET_WM_WINDOW_TYPE,
		_ewmh._NET_WM_WINDOW_TYPE_DESKTOP,
		_ewmh._NET_WM_WINDOW_TYPE_DOCK,
		_ewmh._NET_WM_WINDOW_TYPE_TOOLBAR,
		_ewmh._NET_WM_WINDOW_TYPE_MENU,
		_ewmh._NET_WM_WINDOW_TYPE_UTILITY,
		_ewmh._NET_WM_WINDOW_TYPE_SPLASH,
		_ewmh._NET_WM_WINDOW_TYPE_DIALOG,
		_ewmh._NET_WM_WINDOW_TYPE_DROPDOWN_MENU,
		_ewmh._NET_WM_WINDOW_TYPE_POPUP_MENU,
		_ewmh._NET_WM_WINDOW_TYPE_TOOLTIP,
		_ewmh._NET_WM_WINDOW_TYPE_NOTIFICATION,
		_ewmh._NET_WM_WINDOW_TYPE_COMBO,
		_ewmh._NET_WM_WINDOW_TYPE_DND,
		_ewmh._NET_WM_WINDOW_TYPE_NORMAL,

		_ewmh._NET_WM_STRUT_PARTIAL,

		_ewmh._NET_WM_STATE,
		_ewmh._NET_WM_STATE_MODAL,
		_ewmh._NET_WM_STATE_STICKY,
		_ewmh._NET_WM_STATE_MAXIMIZED_VERT,
		_ewmh._NET_WM_STATE_MAXIMIZED_HORZ,
		_ewmh._NET_WM_STATE_SHADED,
		_ewmh._NET_WM_STATE_SKIP_TASKBAR,
		_ewmh._NET_WM_STATE_SKIP_PAGER,
		_ewmh._NET_WM_STATE_HIDDEN,
		_ewmh._NET_WM_STATE_FULLSCREEN,
		_ewmh._NET_WM_STATE_ABOVE,
		_ewmh._NET_WM_STATE_BELOW,
		_ewmh._NET_WM_STATE_DEMANDS_ATTENTION,

		_ewmh._NET_DESKTOP_NAMES,
		_ewmh._NET_CURRENT_DESKTOP,
		_ewmh._NET_NUMBER_OF_DESKTOPS,

		// non standart
		_wmPROTOCOLS,
		_wmDELETE_WINDOW,
		_wmMOTIF_HINTS,
	};

	int size = sizeof(atoms) / sizeof(xcb_atom_t);

	xcb_ewmh_set_supported(&_ewmh, _screenNumber, size, atoms);

	_wmWindowId = xcb_generate_id(_conn->Get());
	uint32_t mask = XCB_CW_OVERRIDE_REDIRECT | XCB_CW_EVENT_MASK;
	uint32_t vals[] = { 1, 0 };
	xcb_create_window(
		_conn->Get(),
		XCB_COPY_FROM_PARENT,
		_wmWindowId,
		_conn->GetScreen()->root,
		0, 0, 1, 1, 0,
		XCB_WINDOW_CLASS_INPUT_OUTPUT,
		_conn->GetScreen()->root_visual,
		mask, vals
	);

	xcb_ewmh_set_wm_name(&_ewmh, _wmWindowId, _wmName.Count(), _wmName.CPtr());
	xcb_ewmh_set_supporting_wm_check(&_ewmh, _conn->GetScreen()->root, _wmWindowId);
	xcb_ewmh_set_supporting_wm_check(&_ewmh, _wmWindowId, _wmWindowId);
}

void X11Hint::Shutdown() {
	xcb_destroy_window(_conn->Get(), _wmWindowId);
	xcb_ewmh_connection_wipe(&_ewmh);
}

WindowType X11Hint::GetWindowType(xcb_window_t id) {
	xcb_ewmh_get_atoms_reply_t types;
	xcb_get_property_cookie_t cookie = xcb_ewmh_get_wm_window_type_unchecked(&_ewmh, id);

	if (xcb_ewmh_get_wm_window_type_reply(&_ewmh, cookie, &types, nullptr)) {
		WindowType proxy;
		for (unsigned int j = 0; j < types.atoms_len; ++j) {
			xcb_atom_t atom = types.atoms[j];
			if (false) {
			} else if (atom == _ewmh._NET_WM_WINDOW_TYPE_DESKTOP) {
				proxy = WindowType::Desktop;
				break;
			} else if (atom == _ewmh._NET_WM_WINDOW_TYPE_DOCK) {
				proxy = WindowType::Dock;
				break;
			} else if (atom == _ewmh._NET_WM_WINDOW_TYPE_TOOLBAR) {
				proxy = WindowType::Toolbar;
				break;
			} else if (atom == _ewmh._NET_WM_WINDOW_TYPE_MENU) {
				proxy = WindowType::Menu;
				break;
			} else if (atom == _ewmh._NET_WM_WINDOW_TYPE_UTILITY) {
				proxy = WindowType::Utility;
				break;
			} else if (atom == _ewmh._NET_WM_WINDOW_TYPE_SPLASH) {
				proxy = WindowType::Splash;
				break;
			} else if (atom == _ewmh._NET_WM_WINDOW_TYPE_DIALOG) {
				proxy = WindowType::Dialog;
				break;
			} else if (atom == _ewmh._NET_WM_WINDOW_TYPE_DROPDOWN_MENU) {
				proxy = WindowType::DropdownMenu;
				break;
			} else if (atom == _ewmh._NET_WM_WINDOW_TYPE_POPUP_MENU) {
				proxy = WindowType::PopupMenu;
				break;
			} else if (atom == _ewmh._NET_WM_WINDOW_TYPE_TOOLTIP) {
				proxy = WindowType::Tooltip;
				break;
			} else if (atom == _ewmh._NET_WM_WINDOW_TYPE_NOTIFICATION) {
				proxy = WindowType::Notification;
				break;
			} else if (atom == _ewmh._NET_WM_WINDOW_TYPE_COMBO) {
				proxy = WindowType::Combo;
				break;
			} else if (atom == _ewmh._NET_WM_WINDOW_TYPE_DND) {
				proxy = WindowType::DragAndDrop;
				break;
			} else if (atom == _ewmh._NET_WM_WINDOW_TYPE_NORMAL) {
				proxy = WindowType::Normal;
				break;
			}
		}
		xcb_ewmh_get_atoms_reply_wipe(&types);
		return proxy;
	}

	return WindowType::Normal;
}

Margin X11Hint::GetWindowDockExtend(xcb_window_t id) {
	xcb_get_property_cookie_t cookie = xcb_ewmh_get_wm_strut_partial_unchecked(&_ewmh, id);
	xcb_ewmh_wm_strut_partial_t reply;

	if (xcb_ewmh_get_wm_strut_partial_reply(&_ewmh, cookie, &reply, nullptr)) {
		Margin margin {};
		margin.left   = reply.left;
		margin.right  = reply.right;
		margin.top    = reply.top;
		margin.bottom = reply.bottom;
		return margin;
	}
	return {};
}
Wx::String X11Hint::GetWindowTitle(xcb_window_t id) {
	Wx::String content;
	xcb_ewmh_get_utf8_strings_reply_t title;

	if (xcb_ewmh_get_wm_name_reply(&_ewmh, xcb_ewmh_get_wm_name(&_ewmh, id), &title, NULL)) {
		content = title.strings;
		xcb_ewmh_get_utf8_strings_reply_wipe(&title);
	}

	return content;
}

void X11Hint::SetWindowState(xcb_window_t id, Wx::List<WindowState> const& states) {
	int count = 0;
	xcb_atom_t atoms[12];
	for (int i = 0; i < states.Count(); i++) {
		WindowState state = states[i];
		xcb_atom_t atom = WindowStateToAtom(state);
		if (atom == XCB_NONE) {
			continue;
		}
		atoms[count++] = atom;
	}
	xcb_ewmh_set_wm_state(&_ewmh, id, count, atoms);
}

bool X11Hint::IsWindowState(xcb_atom_t atom) {
	return atom == _ewmh._NET_WM_STATE;
}

bool X11Hint::IsActiveWindow(xcb_atom_t atom) {
	return atom == _ewmh._NET_ACTIVE_WINDOW;
}

WindowState X11Hint::AtomToWindowState(xcb_atom_t atom) {
	if (_ewmh._NET_WM_STATE_MODAL == atom) return WindowState::Modal;
	if (_ewmh._NET_WM_STATE_STICKY == atom) return WindowState::Sticky;
	if (_ewmh._NET_WM_STATE_MAXIMIZED_VERT == atom) return WindowState::MaximizedVertical;
	if (_ewmh._NET_WM_STATE_MAXIMIZED_HORZ == atom) return WindowState::MaximizedHorizontal;
	if (_ewmh._NET_WM_STATE_SHADED == atom) return WindowState::Shaded;
	if (_ewmh._NET_WM_STATE_SKIP_TASKBAR == atom) return WindowState::SkipTaskbar;
	if (_ewmh._NET_WM_STATE_SKIP_PAGER == atom) return WindowState::SkipPager;
	if (_ewmh._NET_WM_STATE_HIDDEN == atom) return WindowState::Hidden;
	if (_ewmh._NET_WM_STATE_FULLSCREEN == atom) return WindowState::Fullscreen;
	if (_ewmh._NET_WM_STATE_ABOVE == atom) return WindowState::Above;
	if (_ewmh._NET_WM_STATE_BELOW == atom) return WindowState::Below;
	if (_ewmh._NET_WM_STATE_DEMANDS_ATTENTION == atom) return WindowState::DemandsAttention;

	return WindowState::Unknown;
}

xcb_atom_t X11Hint::WindowStateToAtom(WindowState state) {
	if (WindowState::Modal == state) return _ewmh._NET_WM_STATE_MODAL;
	if (WindowState::Sticky == state) return _ewmh._NET_WM_STATE_STICKY;
	if (WindowState::MaximizedVertical == state) return _ewmh._NET_WM_STATE_MAXIMIZED_VERT;
	if (WindowState::MaximizedHorizontal == state) return _ewmh._NET_WM_STATE_MAXIMIZED_HORZ;
	if (WindowState::Shaded == state) return _ewmh._NET_WM_STATE_SHADED;
	if (WindowState::SkipTaskbar == state) return _ewmh._NET_WM_STATE_SKIP_TASKBAR;
	if (WindowState::SkipPager == state) return _ewmh._NET_WM_STATE_SKIP_PAGER;
	if (WindowState::Hidden == state) return _ewmh._NET_WM_STATE_HIDDEN;
	if (WindowState::Fullscreen == state) return _ewmh._NET_WM_STATE_FULLSCREEN;
	if (WindowState::Above == state) return _ewmh._NET_WM_STATE_ABOVE;
	if (WindowState::Below == state) return _ewmh._NET_WM_STATE_BELOW;
	if (WindowState::DemandsAttention == state) return _ewmh._NET_WM_STATE_DEMANDS_ATTENTION;

	return XCB_NONE;
}

#define _MOTIF_WM_HINTS_ELEMENTS 5
#define MWM_HINTS_DECORATIONS (1L << 1)

bool X11Hint::IsWindowDecorated(xcb_window_t id) {
	WindowType type = GetWindowType(id);

	switch (type) {
		case WindowType::Desktop:
		case WindowType::Dock:
		case WindowType::Toolbar:
		case WindowType::Menu:
		case WindowType::Utility:
		case WindowType::Splash:
		case WindowType::DropdownMenu:
		case WindowType::PopupMenu:
		case WindowType::Tooltip:
		case WindowType::Notification:
		case WindowType::Combo:
		case WindowType::DragAndDrop:
			return false;
		default: break;
	}

	typedef struct {
		uint32_t flags;
		uint32_t functions;
		uint32_t decorations;
		int32_t input_mode;
		uint32_t status;
	} MotifWmHints;

	xcb_get_property_cookie_t propCookie = xcb_get_property(_conn->Get(), 0, id, _wmMOTIF_HINTS, _wmMOTIF_HINTS, 0, _MOTIF_WM_HINTS_ELEMENTS);
	xcb_get_property_reply_t *prop = xcb_get_property_reply(_conn->Get(), propCookie, NULL);

	if (prop && prop->type != XCB_NONE && prop->value_len >= _MOTIF_WM_HINTS_ELEMENTS) {
		MotifWmHints *hints = (MotifWmHints *) xcb_get_property_value(prop);
		if ((hints->flags & MWM_HINTS_DECORATIONS) && hints->decorations == 0) {
			return false;
		}
	}
	return true;
}

void X11Hint::SetActiveWindow(xcb_window_t id) {
	xcb_ewmh_set_active_window(&_ewmh, _screenNumber, id);
}

void X11Hint::CloseWindow(xcb_window_t id) {
	xcb_get_property_cookie_t protocolCookie = xcb_icccm_get_wm_protocols_unchecked(_conn->Get(), id, _wmPROTOCOLS);
	xcb_icccm_get_wm_protocols_reply_t protocols;
	if (xcb_icccm_get_wm_protocols_reply(_conn->Get(), protocolCookie, &protocols, nullptr)) {
		bool has_delete = false;
		for (unsigned i = 0; i < protocols.atoms_len; ++i) {
			if (protocols.atoms[i] == _wmDELETE_WINDOW) {
				has_delete = true;
				break;
			}
		}
		if (has_delete) {
			xcb_client_message_event_t ev = {};
			ev.response_type = XCB_CLIENT_MESSAGE;
			ev.window = id;
			ev.type = _wmPROTOCOLS;
			ev.format = 32;
			ev.data.data32[0] = _wmDELETE_WINDOW;
			ev.data.data32[1] = XCB_CURRENT_TIME;

			xcb_send_event(_conn->Get(), false, id, XCB_EVENT_MASK_NO_EVENT, (const char*)&ev);
			xcb_flush(_conn->Get());
			printf("sent WM_DELETE_WINDOW to %d\n", id);
		} else {
			xcb_kill_client(_conn->Get(), id);  // Force kill
			printf("force kill %d\n", id);
		}
		xcb_icccm_get_wm_protocols_reply_wipe(&protocols);
	} else {
		xcb_kill_client(_conn->Get(), id);
		printf("force kill %d\n", id);
	}
}

void X11Hint::SetWindowFrame(xcb_window_t id, int left, int top, int right, int bottom) {
	xcb_ewmh_set_frame_extents(&_ewmh, id, left, right, top, bottom);
}

void X11Hint::SetWorkspaces(Wx::List<Wx::String> const& workspaces) {
	_workspaces = workspaces;
	xcb_ewmh_set_number_of_desktops(&_ewmh, _screenNumber, _workspaces.Count());
	Wx::String combinedNames;
	for (int i = 0; i < workspaces.Count(); i++) {
		Wx::Byte nullByte = '\0';
		Wx::String const& workspace = _workspaces[i];
		combinedNames.Append(workspace);
		combinedNames.Append(1, &nullByte);
	}
	xcb_ewmh_set_desktop_names(&_ewmh, _screenNumber, combinedNames.Count(), combinedNames.CPtr());
}

void X11Hint::SetActiveWorkspace(Wx::String const& workspace) {
	for (int i = 0; i < _workspaces.Count(); i++) {
		if (_workspaces[i] == workspace) {
			xcb_ewmh_set_current_desktop(&_ewmh, _screenNumber, i);
			return;
		}
	}
}

bool X11Hint::IsMoveResizeProperty(xcb_atom_t atom) {
	return atom == _ewmh._NET_WM_MOVERESIZE;
}

Wx::String X11Hint::AtomToString(xcb_atom_t atom) {
	xcb_get_atom_name_cookie_t cookie = xcb_get_atom_name(_conn->Get(), atom);
	xcb_get_atom_name_reply_t* reply = xcb_get_atom_name_reply(_conn->Get(), cookie, NULL);

	Wx::String content;

	if (reply) {
		int nameSize = xcb_get_atom_name_name_length(reply);
		content = Wx::String(nameSize, (Wx::Byte*)xcb_get_atom_name_name(reply));
		free(reply);
	} else {
		printf("Failed to get atom name for atom %u\n", atom);
	}

	return content;
}

