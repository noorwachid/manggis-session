#pragma once

#include "Event.hpp"
#include "IObserver.hpp"
#include "MonitorDef.hpp"
#include "Window.hpp"
#include "Common/Logger.hpp"
#include "Backend/IContext.hpp"
#include "Backend/X11Clipper.hpp"
#include "Backend/X11Connection.hpp"
#include "Backend/X11Hint.hpp"
#include "Backend/X11KeyboardManager.hpp"
#include "Backend/X11MonitorManager.hpp"
#include "Backend/X11MouseManager.hpp"
#include "Backend/X11SessionObserver.hpp"
#include "Backend/X11WindowBackend.hpp"
#include "Input/Key.hpp"
#include "Input/MouseButton.hpp"
#include "Wx/Core/List.hpp"
#include "Wx/Core/HashTable.hpp"
#include "Wx/Core/String.hpp"
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

	virtual void bindModKey(ModKey modKey) override;

	virtual void unbindModKey(ModKey modKey) override;

	virtual Point getMousePos() override;

	virtual void setMousePos(Point point) override;

	virtual Wx::List<MonitorDef> getConnectedMonitorDefs() override;

	virtual void clipWindow(Window* window, Rect rect) override;

	virtual void unclipWindow(Window* window) override;

	virtual void highlightWindow(Window* window) override;

	virtual void unhighlightWindow(Window* window) override;

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

	void _grabWholeKeyboard();

	void _ungrabWholeKeyboard();

	MonitorDef* _getMonitorDefByPort(Wx::String const& port, Wx::List<MonitorDef>& defs);

	void _updateMonitors(IObserver* observer);

private:
	X11Connection* _conn;
	X11KeyboardManager* _keyboardManager;
	Wx::HashTable<xcb_window_t, Window*> _clientWindowMap;
	Wx::HashTable<xcb_window_t, Window*> _frameWindowMap;

	Wx::List<X11KeyBinding> _keyBindings;
	Wx::String _activeKeyboardLayout;
	Wx::HashTable<xcb_keycode_t, Key> _codeToKey;

	X11MouseManager* _mouseManager;

	X11MonitorManager* _monitorManager;
	Wx::List<MonitorDef> _monitorDefs;

	X11Clipper* _clipper;
	X11Hint* _hint;

	X11SessionObserver* _sessionObserver;

	bool _running = true;
	bool _grabbingWholeKeyboard = false;
	ModKey _grabbingModKeyOriginal = ModKey::None;
	ModKey _grabbingModKey = ModKey::None;
};
