#pragma once

#include "backend/X11Connection.hpp"
#include <string>
#include <vector>
#include <xcb/xcb.h>
#include <xcb/xcb_ewmh.h>

enum struct WindowType {
	Desktop,
	Dock,
	Toolbar,
	Menu,
	Utility,
	Splash,
	Dialog,
	DropdownMenu,
	PopupMenu,
	Tooltip,
	Notification,
	Combo,
	DragAndDrop,
	Normal,
};

enum struct WindowState {
	Unknown,
	Modal,
	Sticky,
	MaximizedVertical,
	MaximizedHorizontal,
	Shaded,
	SkipTaskbar,
	SkipPager,
	Hidden,
	Fullscreen,
	Above,
	Below,
	DemandsAttention,
};

struct WindowStrut {
	int left;
	int top;
	int right;
	int bottom;
};

class X11Hint {
public:
	X11Hint(X11Connection* conn);

	void initialize();

	void terminate();

	WindowType getWindowType(xcb_window_t id);

	WindowStrut getWindowStrut(xcb_window_t id);

	bool isWindowState(xcb_atom_t atom);

	bool isActiveWindow(xcb_atom_t atom);

	std::string atomToString(xcb_atom_t atom);

	WindowState atomToWindowState(xcb_atom_t atom);

	xcb_atom_t windowStateToAtom(WindowState state);

	void setWindowState(xcb_window_t id, std::vector<WindowState> const& states);

	bool isWindowDecorated(xcb_window_t id);

	void setActiveWindow(xcb_window_t id);

	void closeWindow(xcb_window_t id);

	void setWindowFrame(xcb_window_t id, int left, int top, int right, int bottom);

	void setWorkspaces(std::vector<std::string> const& workspaces);

	void setActiveWorkspace(std::string const& workspace);

	bool isMoveResizeProperty(xcb_atom_t atom);

private:
	X11Connection* _conn;

	xcb_ewmh_connection_t _ewmh;
	xcb_atom_t _wmPROTOCOLS;
	xcb_atom_t _wmDELETE_WINDOW;
	xcb_atom_t _wmMOTIF_HINTS;

	int _screenNumber = 0;

	xcb_window_t _wmWindowId;
	std::string _wmName = "ManggisWM";
	std::vector<std::string> _workspaces;
};

