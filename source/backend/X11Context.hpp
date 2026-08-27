#pragma once

#include "Event.hpp"
#include "IObserver.hpp"
#include "MonitorInfo.hpp"
#include "Window.hpp"
#include "Logger.hpp"
#include "backend/IContext.hpp"
#include "backend/X11Clipper.hpp"
#include "backend/X11Connection.hpp"
#include "backend/X11Hint.hpp"
#include "backend/X11KeyboardManager.hpp"
#include "backend/X11MonitorManager.hpp"
#include "backend/X11MouseManager.hpp"
#include "backend/X11SessionObserver.hpp"
#include "backend/X11WindowBackend.hpp"
#include "input/Key.hpp"
#include "input/MouseButton.hpp"
#include "wx/core/List.hpp"
#include "wx/core/HashTable.hpp"
#include "wx/core/String.hpp"
#include <cstdlib>
#include <xcb/randr.h>
#include <xcb/xproto.h>

struct X11KeyBinding {
	ModKey modKey;
	Key key;
	xcb_keycode_t keyCode = 0;
	int modKeyCode = 0;
};

class X11Context: public IContext {
public:
	X11Context();

	virtual ~X11Context() override;

	virtual ISessionObserver* getSessionObserver() override;

	virtual void bindMouseButton(ModKey modKey, MouseButton button) override;

	virtual void unbindMouseButton(ModKey modKey, MouseButton button) override;

	virtual void bindMouseMovement() override;

	virtual void unbindMouseMovement() override;

	virtual void bindKey(ModKey modKey, Key key) override;

	virtual void unbindKey(ModKey modKey, Key key) override;

	virtual Point getMousePos() override;

	virtual wx::List<MonitorInfo> getConnectedMonitorInfos() override;

	virtual void clipWindow(Window* window, Rect rect) override;

	virtual void unclipWindow(Window* window) override;

	virtual Margin getDockExtend(Window* window) override;

	virtual void run(IObserver* observer) override;

	virtual void stop() override;

private:
	Window* _getWindowByFrame(xcb_window_t handle);

	Window* _getWindowByClient(xcb_window_t handle);

	int _fromModKey(ModKey modKey);

	ModKey _toModKey(int modKey);

	int _fromMouseButton(MouseButton button);

	MouseButton _toMouseButton(int button);

	void _attachKeyBindings();

	void _detachKeyBindings();

	void _updateKeyBindings();

	MonitorInfo* _getMonitorInfoByPort(wx::String const& port, wx::List<MonitorInfo>& infos);

	void _updateMonitors(IObserver* observer);

private:
	X11Connection* _conn;
	X11KeyboardManager* _keyboardManager;
	wx::HashTable<xcb_window_t, Window*> _clientWindowMap;
	wx::HashTable<xcb_window_t, Window*> _frameWindowMap;

	wx::List<X11KeyBinding> _keyBindings;
	wx::String _activeKeyboardLayout;
	wx::HashTable<xcb_keycode_t, Key> _keyCodeMap;

	X11MouseManager* _mouseManager;

	X11MonitorManager* _monitorManager;
	wx::List<MonitorInfo> _monitorInfos;

	X11Clipper* _clipper;
	X11Hint* _hint;

	X11SessionObserver* _sessionObserver;

	bool _running = true;
};
