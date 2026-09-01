#pragma once

#include "Common/Unit.hpp"
#include "MonitorDef.hpp"
#include "IObserver.hpp"
#include "ISessionObserver.hpp"
#include "Input/Key.hpp"
#include "Input/MouseButton.hpp"
#include "Wx/Core/List.hpp"

class IContext {
public:
	virtual ISessionObserver* getSessionObserver() = 0;

	virtual void bindMouseButton(ModKey modKey, MouseButton button) = 0;

	virtual void unbindMouseButton(ModKey modKey, MouseButton button) = 0;

	virtual void bindMouseMovement() = 0;

	virtual void unbindMouseMovement() = 0;

	virtual void bindKey(ModKey modKey, Key key) = 0;

	virtual void unbindKey(ModKey modKey, Key key) = 0;

	virtual void bindModKey(ModKey modKey) = 0;

	virtual void unbindModKey(ModKey modKey) = 0;

	virtual Point getMousePos() = 0;

	virtual void setMousePos(Point point) = 0;

	virtual Wx::List<MonitorDef> getConnectedMonitorDefs() = 0;

	virtual void clipWindow(Window* window, Rect rect) = 0;

	virtual void unclipWindow(Window* window) = 0;

	virtual void highlightWindow(Window* window) = 0;

	virtual void unhighlightWindow(Window* window) = 0;

	virtual Margin getDockExtend(Window* window) = 0;

	virtual void run(IObserver* observer) = 0;

	virtual void stop() = 0;

	virtual ~IContext() {
	}
};

IContext* createContext();
