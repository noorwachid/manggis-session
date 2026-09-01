#pragma once

#include "Backend/IWindowBackend.hpp"
#include "Backend/X11Connection.hpp"
#include "Wx/Core/List.hpp"
#include "Wx/Core/String.hpp"
#include <xcb/xcb.h>
#include <xcb/xcb_ewmh.h>

class X11Hint {
public:
	X11Hint(X11Connection* conn);

	void Init();

	void Shutdown();

	WindowType GetWindowType(xcb_window_t id);

	Margin GetWindowDockExtend(xcb_window_t id);

	Wx::String GetWindowTitle(xcb_window_t id);

	bool IsWindowState(xcb_atom_t atom);

	bool IsActiveWindow(xcb_atom_t atom);

	Wx::String AtomToString(xcb_atom_t atom);

	WindowState AtomToWindowState(xcb_atom_t atom);

	xcb_atom_t WindowStateToAtom(WindowState state);

	void SetWindowState(xcb_window_t id, Wx::List<WindowState> const& states);

	bool IsWindowDecorated(xcb_window_t id);

	void SetActiveWindow(xcb_window_t id);

	void CloseWindow(xcb_window_t id);

	void SetWindowFrame(xcb_window_t id, int left, int top, int right, int bottom);

	void SetWorkspaces(Wx::List<Wx::String> const& workspaces);

	void SetActiveWorkspace(Wx::String const& workspace);

	bool IsMoveResizeProperty(xcb_atom_t atom);

private:
	X11Connection* _conn;

	xcb_ewmh_connection_t _ewmh;
	xcb_atom_t _wmPROTOCOLS;
	xcb_atom_t _wmDELETE_WINDOW;
	xcb_atom_t _wmMOTIF_HINTS;

	int _screenNumber = 0;

	xcb_window_t _wmWindowId;
	Wx::String _wmName = "ManggisWM";
	Wx::List<Wx::String> _workspaces;
};

