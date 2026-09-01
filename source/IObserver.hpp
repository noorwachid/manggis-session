#pragma once

#include "Event.hpp"

class IObserver {
public:
	virtual void OnWindowCreate(WindowEvent const& e) = 0;

	virtual void OnWindowDestroy(WindowEvent const& e) = 0;

	virtual void OnWindowMoveRequest(WindowMovementEvent const& e) = 0;

	virtual void OnWindowResizeRequest(WindowResizingEvent const& e) = 0;

	virtual void OnWindowMaximizeRequest(WindowActionEvent const& e) = 0;

	virtual void OnWindowFullscreenRequest(WindowActionEvent const& e) = 0;

	virtual void OnMousePress(MouseButtonEvent const& e) = 0;

	virtual void OnMouseRelease(MouseButtonEvent const& e) = 0;

	virtual void OnMouseMove(MouseMovementEvent const& e) = 0;

	virtual void OnKeyPress(KeyEvent const& e) = 0;

	virtual void OnKeyRelease(KeyEvent const& e) = 0;

	virtual void OnMonitorConnect(MonitorEvent const& e) = 0;

	virtual void OnMonitorDisconnect(MonitorEvent const& e) = 0;

	virtual void OnMonitorPropertyChange(MonitorEvent const& e) = 0;

};

