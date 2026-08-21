#pragma once

#include "Common.hpp"
#include "Window.hpp"
#include "input/Key.hpp"
#include "input/MouseButton.hpp"
#include "wx/core/String.hpp"

struct WindowEvent {
	Window* window;
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


