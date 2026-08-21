#pragma once

#include "Event.hpp"
#include "IObserver.hpp"
#include "IContext.hpp"
#include "MonitorInfo.hpp"
#include "input/MouseButton.hpp"
#include "input/Key.hpp"
#include "system/Exec.hpp"
#include "wx/core/List.hpp"
#include <cstdio>

#define USE_XEPHYR

enum struct DragMode {
	None,
	Move,
	Resize,
};

struct DragState {
	DragMode mode = DragMode::None;
	Window* window;
	Point initialMousePos;
	Rect initialWindowRect;
};

class Workspace {
public:
	Workspace(wx::String const& id): _id{id} {
	}

	wx::String const& getID() {
		return _id;
	}

	Window* getActiveWindow() {
		return _activeWindow;
	}

	void activateWindow(Window* window) {
		if (_activeWindow != nullptr) {
			_activeWindow->deactivate();
		}
		window->activate();
		_activeWindow = window;
	}

	void addWindow(Window* window) {
		_windows.append(window);
		activateWindow(window);
	}

	void removeWindow(Window* window) {
		int index = _windows.find(window);
		if (index != -1) {
			bool activeWindowRemoved = _windows[index] == _activeWindow;
			_windows.remove(index);
			if (activeWindowRemoved) {
				_activeWindow = nullptr;
				if (!_windows.isEmpty()) {
					activateWindow(_windows.getLast());
				}
			}
		}
	}

	void activate() {
		for (int i = 0; i < _windows.count(); i++) {
			_windows[i]->show();
		}
	}

	void deactivate() {
		for (int i = 0; i < _windows.count(); i++) {
			_windows[i]->hide();
		}
	}

private:
	Window* _activeWindow = nullptr;
	wx::String _id;
	wx::List<Window*> _windows;
};

class Monitor {
public:
	Monitor(MonitorInfo info): _info{info} {
		_workingRect = info.rect;
	}

	wx::String const& getPort() const {
		return _info.port;
	}

	Rect getWorkingRect() const {
		return _workingRect;
	}

	Workspace* getActiveWorkspace() {
		return _activeWorkspace;
	}

	Workspace* queryWorkspace(wx::String const& id) {
		for (int i = 0; i < _workspaces.count(); i++) {
			if (_workspaces[i]->getID() == id) {
				return _workspaces[i];
			}
		}
		return nullptr;
	}

	void activateWorkspace(Workspace* workspace) {
		if (_activeWorkspace != nullptr) {
			_activeWorkspace->deactivate();
		}
		workspace->activate();
		_activeWorkspace = workspace;
	}

	void addWorkspace(Workspace* workspace) {
		_workspaces.append(workspace);
		if (_activeWorkspace == nullptr) {
			_activeWorkspace = workspace;
		}
	}

	void removeWorkspace(Workspace* workspace) {
		int index = _workspaces.find(workspace);
		if (index != -1) {
			bool activeWorkspaceRemoved = _workspaces[index] == _activeWorkspace;
			_workspaces.remove(index);
			if (activeWorkspaceRemoved) {
				_activeWorkspace = nullptr;
				if (!_workspaces.isEmpty()) {
					activateWorkspace(_workspaces.getLast());
				}
			}
		}
	}

	void calculateWorkingRect() {
	}

private:
	MonitorInfo _info;
	Workspace* _activeWorkspace = nullptr;
	wx::List<Workspace*> _workspaces;
	wx::List<Window*> _reservedWindows;
	int _reservedTop = 0;
	int _reservedBottom = 0;
	int _reservedRight = 0;
	int _reservedLeft = 0;
	Rect _workingRect;
};


class Session: public IObserver {
public:
	void run() {
		_context = createContext();

		_context->bindMouseButton(ModKey::Alt, MouseButton::Left);
		_context->bindMouseButton(ModKey::Alt|ModKey::Shift, MouseButton::Left);

		_context->bindKey(ModKey::Alt, Key::Return);
		_context->bindKey(ModKey::Alt, Key::Num1);
		_context->bindKey(ModKey::Alt, Key::Num2);
		_context->bindKey(ModKey::Alt, Key::Num3);
		_context->bindKey(ModKey::Alt, Key::Num4);

		wx::List<MonitorInfo> monitorInfos = getConnectedMonitorInfos();

		MonitorInfo fallbackInfo;
		fallbackInfo.port = "<fallback>";
		_fallbackMonitor = new Monitor(fallbackInfo);

		if (monitorInfos.isEmpty()) {
			_activeMonitor = _fallbackMonitor;
		} else {
			for (int i = 0; i < monitorInfos.count(); i++) {
				MonitorInfo const& info = monitorInfos[i];
				printf("monitor %s (%d, %d, %d, %d)\n", 
					info.port.cptr(), 
					info.rect.x, 
					info.rect.y, 
					info.rect.width, 
					info.rect.height
				);
				Monitor* monitor = new Monitor(info);
				if (_activeMonitor == nullptr) {
					_activeMonitor = monitor;
				}
				_monitors.append(monitor);
			}
		}

		Monitor* mon1 = queryMonitor("VHDMI-1");
		if (mon1) {
			mon1->addWorkspace(new Workspace("1"));
			mon1->addWorkspace(new Workspace("2"));
		}

		Monitor* mon2 = queryMonitor("VHDMI-2");
		if (mon2) {
			mon2->addWorkspace(new Workspace("3"));
			mon2->addWorkspace(new Workspace("4"));
		}

		_context->listen(this);
	}

	wx::List<MonitorInfo> getConnectedMonitorInfos() {
		#ifdef USE_XEPHYR
		wx::List<MonitorInfo> infos;

		MonitorInfo left;
		left.port = "VHDMI-1";
		left.rect.x = 0;
		left.rect.y = 0;
		left.rect.width = 500;
		left.rect.height = 600;
		infos.append(left);

		MonitorInfo right;
		right.port = "VHDMI-2";
		right.rect.x = 500;
		right.rect.y = 0;
		right.rect.width = 500;
		right.rect.height = 600;
		infos.append(right);

		return infos;
		#endif

		return _context->getConnectedMonitorInfos();
	}

	virtual void onWindowCreate(WindowEvent const& e) override {
		_activeMonitor->getActiveWorkspace()->addWindow(e.window);
	}

	virtual void onWindowDestroy(WindowEvent const& e) override {
		_activeMonitor->getActiveWorkspace()->removeWindow(e.window);
	}

	virtual void onMousePress(MouseButtonEvent const& e) override {
		if (e.window == nullptr) {
			return;
		}

		_dragState.window = e.window;

		 // raise to top (if background wallpaper or desktop orginizer you dont have to move it forward)
		_dragState.window->raise();
		_activeMonitor->getActiveWorkspace()->activateWindow(e.window);

		_dragState.initialWindowRect = _dragState.window->getRect();
		_dragState.initialMousePos = _context->getMousePos();

		if (e.button == MouseButton::Left) {
			if (e.modKey == (ModKey::Alt)) {
				_dragState.mode = DragMode::Move;
			}
			if (e.modKey == (ModKey::Alt|ModKey::Shift)) {
				_dragState.mode = DragMode::Resize;
			}
		}

		_context->bindMouseMovement();
	}

	virtual void onMouseRelease(MouseButtonEvent const& e) override {
		_dragState.mode = DragMode::None;
		_dragState.window = nullptr;
		_context->unbindMouseMovement();
	}

	virtual void onMouseMove(MouseMovementEvent const& e) override {
		if (_dragState.mode == DragMode::Move) {
			int x = _dragState.initialWindowRect.x + (e.pos.x - _dragState.initialMousePos.x);
			int y = _dragState.initialWindowRect.y + (e.pos.y - _dragState.initialMousePos.y);
			_dragState.window->move(x, y);
		}
		if (_dragState.mode == DragMode::Resize) {
			int width = _dragState.initialWindowRect.width + (e.pos.x - _dragState.initialMousePos.x);
			int height = _dragState.initialWindowRect.height + (e.pos.y - _dragState.initialMousePos.y);
			_dragState.window->resize(width, height);
		}
	}

	virtual void onKeyPress(KeyEvent const& e) override {
		if (e.modKey == ModKey::Alt && e.key == Key::Return) {
			wx::List<wx::String> args;
			args.append("xterm");
			exec(args);
			return;
		}
		if (e.modKey == ModKey::Alt && e.key == Key::Num1) {
			activateWorkspace("1");
			return;
		}
		if (e.modKey == ModKey::Alt && e.key == Key::Num2) {
			activateWorkspace("2");
			return;
		}
		if (e.modKey == ModKey::Alt && e.key == Key::Num3) {
			activateWorkspace("3");
			return;
		}
		if (e.modKey == ModKey::Alt && e.key == Key::Num4) {
			activateWorkspace("4");
			return;
		}
	}

	virtual void onKeyRelease(KeyEvent const& e) override {
	}

	virtual void onMonitorConnect(MonitorEvent const& e) override {
		printf("monitor connected %s\n", e.port.cptr());
	}

	virtual void onMonitorDisconnect(MonitorEvent const& e) override {
		printf("monitor disconnected %s\n", e.port.cptr());
	}

	virtual void onMonitorPropertyChange(MonitorEvent const& e) override {
		printf("monitor property changed %s\n", e.port.cptr());
	}

	void activateWorkspace(wx::String const& id) {
		Workspace* workspace = _activeMonitor->queryWorkspace(id);
		if (workspace == nullptr) {
			printf("no workspace with id %s\n", id.cptr());
			return;
		}
		printf("activate workspace %s\n", id.cptr());
		_activeMonitor->activateWorkspace(workspace);
	}

	Monitor* queryMonitor(wx::String const& port) {
		for (int i = 0; i < _monitors.count(); i++) {
			if (_monitors[i]->getPort() == port) {
				return _monitors[i];
			}
		}
		return nullptr;
	}
private:
	IContext* _context;
	Monitor* _activeMonitor = nullptr;
	Monitor* _fallbackMonitor = nullptr; // incase of all monitor disconnected all workspace/windows are here
	wx::List<Monitor*> _monitors;
	DragState _dragState;
};
