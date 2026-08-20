#pragma once

#include "Common.hpp"
#include "IWindow.hpp"
#include "input/Key.hpp"
#include "input/MouseButton.hpp"
#include "wx/core/String.hpp"

struct MouseButtonEvent {
	Point pos;
	MouseButton button;
	ModKey modKey;
	IWindow* window;
};

struct MouseMovementEvent {
	Point pos;
};

struct KeyEvent {
	Key key;
};

struct MonitorEvent {
	wx::String port;
};


