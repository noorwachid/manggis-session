#pragma once

#include "Event.hpp"

class IObserver {
public:
	virtual void onWindowCreate(WindowEvent const& e) = 0;

	virtual void onWindowDestroy(WindowEvent const& e) = 0;

	virtual void onMousePress(MouseButtonEvent const& e) = 0;

	virtual void onMouseRelease(MouseButtonEvent const& e) = 0;

	virtual void onMouseMove(MouseMovementEvent const& e) = 0;

	virtual void onKeyPress(KeyEvent const& e) = 0;

	virtual void onKeyRelease(KeyEvent const& e) = 0;

	virtual void onMonitorConnect(MonitorEvent const& e) = 0;

	virtual void onMonitorDisconnect(MonitorEvent const& e) = 0;

	virtual void onMonitorPropertyChange(MonitorEvent const& e) = 0;
};

