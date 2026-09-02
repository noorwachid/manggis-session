#pragma once

#include "core/Unit.hpp"
#include "input/Key.hpp"
#include "input/MouseButton.hpp"
#include "wx/core/String.hpp"
#include "Window.hpp"

struct WindowEvent {
	Window* window;
};

struct WindowMovementEvent {
	Window* window;
	Point offset;
};

enum struct WindowAction {
	Disable,
	Enable,
	Toggle,
};

struct WindowActionEvent {
	Window* window;
	WindowAction action;
};

enum struct WindowResizingArea {
	None,
	North,
	South,
	West,
	East,
	NorthWest,
	NorthEast,
	SouthWest,
	SouthEast,
};

struct WindowResizingEvent {
	Window* window;
	WindowResizingArea area;
};

struct MouseButtonEvent {
	Point pos;
	MouseButton button;
	ModKey modKey;
	Window* window;
};

struct MouseMovementEvent {
	Point pos;
};

struct KeyEvent {
	ModKey modKey;
	Key key;
};

struct MonitorEvent {
	wx::String port;
};


