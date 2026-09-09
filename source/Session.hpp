#pragma once

#include "Event.hpp"
#include "IObserver.hpp"
#include "Monitor.hpp"
#include "Workspace.hpp"
#include "Window.hpp"
#include "core/Direction.hpp"
#include "config/Config.hpp"
#include "input/Key.hpp"
#include "backend/IContext.hpp"
#include "backend/ISessionObserver.hpp"
#include "wx/core/List.hpp"
#include "wx/core/Func.hpp"

enum struct DragMode {
	None,
	Move,
	Resize,
};

struct DragState {
	DragMode mode = DragMode::None;
	Window* window;
	WindowResizingArea area;
	Point initialMousePos;
	Rect initialWindowRect;
};

struct KeyIndex {
	ModKey modKey;
	Key key;
};

struct MouseIndex {
	ModKey modKey;
};

using KeyCallback = wx::Func<void()>;

using MouseCallback = wx::Func<void(Window* window)>;

bool operator==(KeyIndex a, KeyIndex b);

bool operator==(MouseIndex a, MouseIndex b);

wx::Uint32 getHash(KeyIndex index);

wx::Uint32 getHash(MouseIndex index);

class Monitor;

#ifdef XEPHYR_ENABLE
extern int xephyrMonitorConfiguration;
#endif


class Session: public IObserver {
public:
	void run(Config const& config);

	KeyBinding const* queryBinding(ModKey modKey, Key key);

	virtual void onWindowCreate(WindowEvent const& e) override;

	virtual void onWindowDestroy(WindowEvent const& e) override;

	virtual void onWindowMoveRequest(WindowMovementEvent const& e) override;

	virtual void onWindowResizeRequest(WindowResizingEvent const& e) override;

	virtual void onWindowMaximizeRequest(WindowActionEvent const& e) override;

	virtual void onWindowFullscreenRequest(WindowActionEvent const& e) override;

	virtual void onMousePress(MouseButtonEvent const& e) override;

	virtual void onMouseRelease(MouseButtonEvent const& e) override;

	virtual void onMouseMove(MouseMovementEvent const& e) override;

	virtual void onKeyPress(KeyEvent const& e) override;

	virtual void onKeyRelease(KeyEvent const& e) override;

	virtual void onMonitorConnect(MonitorEvent const& e) override;

	virtual void onMonitorDisconnect(MonitorEvent const& e) override;

	virtual void onMonitorPropertyChange(MonitorEvent const& e) override;

	void focusMonitor(Monitor* monitor);

	void focusWorkspaceID(wx::String const& id);

	void focusWorkspace(Workspace* workspace, bool moveCursor);

	void sendWorkspaceToMonitor(Workspace* workspace, Monitor* monitor);

	void sendWindowToWorkspace(Window* window, Workspace* workspace);

	void sendWindowToWorkspaceID(wx::String const& id);

	void focusWindow(Window* window, bool moveCursor);

	void focusNearestWindow(Direction direction);

	Window* findNearestWindow(Window* window, Direction direction, bool onlyParallelWindow);

	Monitor* findNearestMonitor(Monitor* window, Direction direction, bool onlyParallelWindow);

	void cycleNextWindow();

	void cyclePreviousWindow();

	void closeWindow();

private:

	wx::List<Workspace*> _getWorkspaceList();

	void _beginDrag(DragMode mode, Window* window, WindowResizingArea area = WindowResizingArea::None);

	void _doCommand(Command const& command);

	void _doMouseCommand(CommandFunc func, Window* window);

	Monitor* _queryMonitorByPort(wx::String const& port);

	Monitor* _queryMonitorByPos(Point pos);

	void _updateLayout();

	void _logLayout();

	Monitor* _getOrCreateMonitor(MonitorDef const& def);

	Workspace* _getOrCreateWorkspace(wx::String const& id);

	void _clipWindow(Window* window);

private:
	IContext* _context;
	ISessionObserver* _observer;

	Monitor* _activeMonitor = nullptr;
	Monitor* _fallbackMonitor = nullptr; // incase of all monitor disconnected all workspace/windows are here
	wx::List<Monitor*> _monitors;
	DragState _dragState;

	Config _config;
	wx::List<Monitor*> _monitorDB;
	wx::List<Workspace*> _workspaceDB;

	wx::HashTable<KeyIndex, KeyCallback> _keyBindings;
	wx::HashTable<MouseIndex, MouseCallback> _mouseBindings;
};
