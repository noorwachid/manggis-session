#pragma once

#include "Common.hpp"
#include "IWindow.hpp"
#include "input/Key.hpp"
#include "input/MouseButton.hpp"

struct MouseButtonEvent {
	Point pos;
	MouseButton button;
	ModKey modKey;
	IWindow* window;
};

struct MouseMovementEvent {
	Point pos;
};


