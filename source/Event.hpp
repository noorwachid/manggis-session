#pragma once

#include "Wx/Core/String.hpp"
#include "Common/Unit.hpp"
#include "Input/Key.hpp"
#include "Input/MouseButton.hpp"
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
	Wx::String port;
};


