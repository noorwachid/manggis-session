#pragma once

#include "Event.hpp"

class IObserver {
public:
	virtual void onMousePress(MouseButtonEvent const& e) = 0;

	virtual void onMouseRelease(MouseButtonEvent const& e) = 0;

	virtual void onMouseMove(MouseMovementEvent const& e) = 0;
};

