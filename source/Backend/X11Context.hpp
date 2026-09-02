#pragma once

#include "IObserver.hpp"
#include "MonitorDef.hpp"
#include "Window.hpp"
#include "Backend/IContext.hpp"
#include "Backend/X11Clipper.hpp"
#include "Backend/X11Connection.hpp"
#include "Backend/X11Hint.hpp"
#include "Backend/X11KeyboardManager.hpp"
#include "Backend/X11MonitorManager.hpp"
#include "Backend/X11MouseManager.hpp"
#include "Backend/X11SessionObserver.hpp"
#include "Input/Key.hpp"
#include "Input/MouseButton.hpp"
#include "Wx/Core/List.hpp"
#include "Wx/Core/HashTable.hpp"
#include "Wx/Core/String.hpp"
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

	virtual ISessionObserver* GetSessionObserver() override;

	virtual void BindMouseButton(ModKey modKey, MouseButton button) override;

	virtual void UnbindMouseButton(ModKey modKey, MouseButton button) override;

	virtual void BindMouseMovement() override;

	virtual void UnbindMouseMovement() override;

	virtual void BindKey(ModKey modKey, Key key) override;

	virtual void UnbindKey(ModKey modKey, Key key) override;

	virtual void BindModKey(ModKey modKey) override;

	virtual void UnbindModKey(ModKey modKey) override;

	virtual Point GetMousePos() override;

	virtual void SetMousePos(Point point) override;

	virtual Wx::List<MonitorDef> GetConnectedMonitorDefs() override;

	virtual void ClipWindow(Window* window, Rect rect) override;

	virtual void UnclipWindow(Window* window) override;

	virtual void HighlightWindow(Window* window) override;

	virtual void UnhighlightWindow(Window* window) override;

	virtual Margin GetDockExtend(Window* window) override;

	virtual void Run(IObserver* observer) override;

	virtual void Stop() override;

private:
	Window* QueryWindowByFrame(xcb_window_t handle);

	Window* QueryWindowByClient(xcb_window_t handle);

	int FromModKey(ModKey modKey);

	ModKey ToModKey(int modKey);

	int FromMouseButton(MouseButton button);

	MouseButton ToMouseButton(int button);

	void AttachKeyBindings();

	void DetachKeyBindings();

	void UpdateKeyBindings();

	void GrabWholeKeyboard();

	void UngrabWholeKeyboard();

	MonitorDef* QueryMonitorDefByPort(Wx::String const& port, Wx::List<MonitorDef>& defs);

	void UpdateMonitors(IObserver* observer);

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
