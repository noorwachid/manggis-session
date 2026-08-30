#pragma once

#include "Collider.hpp"
#include "Direction.hpp"
#include "Event.hpp"
#include "IObserver.hpp"
#include "MonitorInfo.hpp"
#include "Monitor.hpp"
#include "Workspace.hpp"
#include "Window.hpp"
#include "Logger.hpp"
#include "config/Config.hpp"
#include "input/MouseButton.hpp"
#include "input/Key.hpp"
#include "system/Exec.hpp"
#include "backend/IContext.hpp"
#include "backend/ISessionObserver.hpp"
#include "wx/core/Assert.hpp"
#include "wx/core/Hash.hpp"
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

using KeyCallback = wx::Func<void()>;

bool operator==(KeyIndex a, KeyIndex b);

wx::Uint32 getHash(KeyIndex index);

class Monitor;

#ifdef XEPHYR_ENABLE
extern int xephyrMonitorConfiguration;
#endif


class Session: public IObserver {
public:
	void run(Config const& config);

	ConfigBinding const* queryBinding(ModKey modKey, Key key);

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

	void focusWorkspace(Workspace* workspace);

	void sendWorkspaceToMonitor(Workspace* workspace, Monitor* monitor);

	void sendWindowToWorkspace(Window* window, Workspace* workspace);

	void sendWindowToWorkspaceID(wx::String const& id);

	void focusWindow(Window* window);

	void focusNearestWindow(Direction direction);

	void cycleWindow();

	void closeWindow();

private:
	void _beginDrag(DragMode mode, Window* window, WindowResizingArea area = WindowResizingArea::None);

	void _doCommand(ConfigCommand const& command);

	Monitor* _queryMonitorByPort(wx::String const& port);

	Monitor* _queryMonitorByPos(Point pos);

	void _relayoutMonitors();

	void _logLayout();

	Monitor* _getOrCreateMonitor(MonitorInfo const& info);

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

	bool _cyclingWindow = false;
};
