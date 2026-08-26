#pragma once

#include "Common.hpp"
#include "IObserver.hpp"
#include "ISessionObserver.hpp"
#include "MonitorInfo.hpp"
#include "input/Key.hpp"
#include "input/MouseButton.hpp"
#include "wx/core/List.hpp"

class IContext {
public:
	virtual ISessionObserver* getSessionObserver() = 0;

	virtual void bindMouseButton(ModKey modKey, MouseButton button) = 0;

	virtual void unbindMouseButton(ModKey modKey, MouseButton button) = 0;

	virtual void bindMouseMovement() = 0;

	virtual void unbindMouseMovement() = 0;

	virtual void bindKey(ModKey modKey, Key key) = 0;

	virtual void unbindKey(ModKey modKey, Key key) = 0;

	virtual Point getMousePos() = 0;

	// virtual void setMousePos(Point point) = 0;

	virtual wx::List<MonitorInfo> getConnectedMonitorInfos() = 0;

	virtual void clipWindow(Window* window, Rect rect) = 0;

	virtual void unclipWindow(Window* window) = 0;

	virtual Margin getDockExtend(Window* window) = 0;

	virtual void listen(IObserver* observer) = 0;

	virtual ~IContext() {
	}
};

IContext* createContext();
