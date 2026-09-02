#pragma once

#include "Event.hpp"
#include "IObserver.hpp"
#include "Monitor.hpp"
#include "Workspace.hpp"
#include "Window.hpp"
#include "Common/Direction.hpp"
#include "Config/Config.hpp"
#include "Input/Key.hpp"
#include "Backend/IContext.hpp"
#include "Backend/ISessionObserver.hpp"
#include "Wx/Core/List.hpp"
#include "Wx/Core/Func.hpp"

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

using KeyCallback = Wx::Func<void()>;

bool operator==(KeyIndex a, KeyIndex b);

Wx::Uint32 getHash(KeyIndex index);

class Monitor;

#ifdef XEPHYR_ENABLE
extern int xephyrMonitorConfiguration;
#endif


class Session: public IObserver {
public:
	void Run(Config const& config);

	ConfigBinding const* QueryBinding(ModKey modKey, Key key);

	virtual void OnWindowCreate(WindowEvent const& e) override;

	virtual void OnWindowDestroy(WindowEvent const& e) override;

	virtual void OnWindowMoveRequest(WindowMovementEvent const& e) override;

	virtual void OnWindowResizeRequest(WindowResizingEvent const& e) override;

	virtual void OnWindowMaximizeRequest(WindowActionEvent const& e) override;

	virtual void OnWindowFullscreenRequest(WindowActionEvent const& e) override;

	virtual void OnMousePress(MouseButtonEvent const& e) override;

	virtual void OnMouseRelease(MouseButtonEvent const& e) override;

	virtual void OnMouseMove(MouseMovementEvent const& e) override;

	virtual void OnKeyPress(KeyEvent const& e) override;

	virtual void OnKeyRelease(KeyEvent const& e) override;

	virtual void OnMonitorConnect(MonitorEvent const& e) override;

	virtual void OnMonitorDisconnect(MonitorEvent const& e) override;

	virtual void OnMonitorPropertyChange(MonitorEvent const& e) override;

	void FocusMonitor(Monitor* monitor);

	void FocusWorkspaceID(Wx::String const& id);

	void FocusWorkspace(Workspace* workspace);

	void SendWorkspaceToMonitor(Workspace* workspace, Monitor* monitor);

	void SendWindowToWorkspace(Window* window, Workspace* workspace);

	void SendWindowToWorkspaceID(Wx::String const& id);

	void FocusWindow(Window* window, bool moveCursor);

	void FocusNearestWindow(Direction direction);

	Window* FindNearestWindow(Window* window, Direction direction, bool onlyParallelWindow);

	Monitor* FindNearestMonitor(Monitor* window, Direction direction, bool onlyParallelWindow);

	void CycleNextWindow();

	void CyclePreviousWindow();

	void CloseWindow();

private:

	void BeginDrag(DragMode mode, Window* window, WindowResizingArea area = WindowResizingArea::None);

	void DoCommand(ConfigCommand const& command);

	Monitor* QueryMonitorByPort(Wx::String const& port);

	Monitor* QueryMonitorByPos(Point pos);

	void UpdateLayout();

	void LogLayout();

	Monitor* GetOrCreateMonitor(MonitorDef const& def);

	Workspace* GetOrCreateWorkspace(Wx::String const& id);

	void ClipWindow(Window* window);

private:
	IContext* _context;
	ISessionObserver* _observer;

	Monitor* _activeMonitor = nullptr;
	Monitor* _fallbackMonitor = nullptr; // incase of all monitor disconnected all workspace/windows are here
	Wx::List<Monitor*> _monitors;
	DragState _dragState;

	Config _config;
	Wx::List<Monitor*> _monitorDB;
	Wx::List<Workspace*> _workspaceDB;

	Wx::HashTable<KeyIndex, KeyCallback> _keyBindings;
};
