#pragma once

#include "IObserver.hpp"
#include "MonitorDef.hpp"
#include "Window.hpp"
#include "backend/IContext.hpp"
#include "backend/X11Clipper.hpp"
#include "backend/X11Connection.hpp"
#include "backend/X11Hint.hpp"
#include "backend/X11KeyboardManager.hpp"
#include "backend/X11MonitorManager.hpp"
#include "backend/X11MouseManager.hpp"
#include "backend/X11SessionObserver.hpp"
#include "input/Key.hpp"
#include "input/MouseButton.hpp"
#include "wx/core/List.hpp"
#include "wx/core/HashTable.hpp"
#include "wx/core/String.hpp"
#include <xcb/randr.h>
#include <xcb/xproto.h>

struct X11KeyBinding {
	ModKey modKey;
	Key key;
	xcb_keycode_t keyCode = 0;
	int modKeyCode = 1;
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

	virtual void bindModKey(ModKey modKey) override;

	virtual void unbindModKey(ModKey modKey) override;

	virtual Point getMousePos() override;

	virtual void setMousePos(Point point) override;

	virtual wx::List<MonitorDef> getConnectedMonitorDefs() override;

	virtual void clipWindow(Window* window, Rect rect) override;

	virtual void unclipWindow(Window* window) override;

	virtual void highlightWindow(Window* window) override;

	virtual void unhighlightWindow(Window* window) override;

	virtual Margin getDockExtend(Window* window) override;

	virtual void run(IObserver* observer) override;

	virtual void stop() override;

private:
	void _unmanagedWindowList(IObserver* observer);

	Window* _queryWindowByFrame(xcb_window_t handle);

	Window* _queryWindowByClient(xcb_window_t handle);

	int _fromModKey(ModKey modKey);

	ModKey _toModKey(int modKey);

	int _fromMouseButton(MouseButton button);

	MouseButton _toMouseButton(int button);

	void _attachKeyBindings();

	void _detachKeyBindings();

	void _updateKeyBindings();

	void _grabWholeKeyboard();

	void _ungrabWholeKeyboard();

	MonitorDef* _queryMonitorDefByPort(wx::String const& port, wx::List<MonitorDef>& defs);

	void _updateMonitors(IObserver* observer);

private:
	X11Connection* _conn;
	X11KeyboardManager* _keyboardManager;
	wx::HashTable<xcb_window_t, Window*> _clientWindowMap;
	wx::HashTable<xcb_window_t, Window*> _frameWindowMap;

	wx::List<X11KeyBinding> _keyBindings;
	wx::String _activeKeyboardLayout;
	wx::HashTable<xcb_keycode_t, Key> _codeToKey;

	X11MouseManager* _mouseManager;

	X11MonitorManager* _monitorManager;
	wx::List<MonitorDef> _monitorDefs;

	X11Clipper* _clipper;
	X11Hint* _hint;

	X11SessionObserver* _sessionObserver;

	bool _running = true;
	bool _grabbingWholeKeyboard = false;
	ModKey _grabbingModKeyOriginal = ModKey::None;
	ModKey _grabbingModKey = ModKey::None;
};
