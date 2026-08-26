#pragma once

#include "backend/IWindowBackend.hpp"
#include "backend/X11Connection.hpp"
#include "wx/core/List.hpp"
#include "wx/core/String.hpp"
#include <xcb/xcb.h>
#include <xcb/xcb_ewmh.h>

class X11Hint {
public:
	X11Hint(X11Connection* conn);

	void initialize();

	void terminate();

	WindowType getWindowType(xcb_window_t id);

	Margin getWindowDockExtend(xcb_window_t id);

	bool isWindowState(xcb_atom_t atom);

	bool isActiveWindow(xcb_atom_t atom);

	wx::String atomToString(xcb_atom_t atom);

	WindowState atomToWindowState(xcb_atom_t atom);

	xcb_atom_t windowStateToAtom(WindowState state);

	void setWindowState(xcb_window_t id, wx::List<WindowState> const& states);

	bool isWindowDecorated(xcb_window_t id);

	void setActiveWindow(xcb_window_t id);

	void closeWindow(xcb_window_t id);

	void setWindowFrame(xcb_window_t id, int left, int top, int right, int bottom);

	void setWorkspaces(wx::List<wx::String> const& workspaces);

	void setActiveWorkspace(wx::String const& workspace);

	bool isMoveResizeProperty(xcb_atom_t atom);

private:
	X11Connection* _conn;

	xcb_ewmh_connection_t _ewmh;
	xcb_atom_t _wmPROTOCOLS;
	xcb_atom_t _wmDELETE_WINDOW;
	xcb_atom_t _wmMOTIF_HINTS;

	int _screenNumber = 0;

	xcb_window_t _wmWindowId;
	wx::String _wmName = "ManggisWM";
	wx::List<wx::String> _workspaces;
};

