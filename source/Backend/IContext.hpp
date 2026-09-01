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
	virtual ISessionObserver* GetSessionObserver() = 0;

	virtual void BindMouseButton(ModKey modKey, MouseButton button) = 0;

	virtual void UnbindMouseButton(ModKey modKey, MouseButton button) = 0;

	virtual void BindMouseMovement() = 0;

	virtual void UnbindMouseMovement() = 0;

	virtual void BindKey(ModKey modKey, Key key) = 0;

	virtual void UnbindKey(ModKey modKey, Key key) = 0;

	virtual void BindModKey(ModKey modKey) = 0;

	virtual void UnbindModKey(ModKey modKey) = 0;

	virtual Point GetMousePos() = 0;

	virtual void SetMousePos(Point point) = 0;

	virtual Wx::List<MonitorDef> GetConnectedMonitorDefs() = 0;

	virtual void ClipWindow(Window* window, Rect rect) = 0;

	virtual void UnclipWindow(Window* window) = 0;

	virtual void HighlightWindow(Window* window) = 0;

	virtual void UnhighlightWindow(Window* window) = 0;

	virtual Margin GetDockExtend(Window* window) = 0;

	virtual void Run(IObserver* observer) = 0;

	virtual void Stop() = 0;

	virtual ~IContext() {
	}
};

IContext* createContext();
