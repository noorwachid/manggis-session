#pragma once

#include "Collider.hpp"
#include "Event.hpp"
#include "IObserver.hpp"
#include "MonitorInfo.hpp"
#include "backend/IWindowBackend.hpp"
#include "input/MouseButton.hpp"
#include "input/Key.hpp"
#include "system/Exec.hpp"
#include "backend/IContext.hpp"
#include "wx/core/Assert.hpp"
#include "wx/core/List.hpp"
#include <cstdio>

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

class Monitor;

class Workspace {
public:
	Workspace(wx::String const& id): _id{id} {
	}

	wx::String const& getID() {
		return _id;
	}

	void setMonitor(Monitor* monitor) {
		_monitor = monitor;
	}

	Monitor* getMonitor() {
		return _monitor;
	}

	Window* getActiveWindow() {
		return _activeWindow;
	}

	void focusWindow(Window* window) {
		if (_activeWindow == window) return;
		if (_activeWindow != nullptr) {
			_activeWindow->blur();
		}
		window->focus();
		_activeWindow = window;
	}

	void addWindow(Window* window) {
		// TODO: check if workspace already exists
		window->setMonitor(_monitor);
		window->setWorkspace(this);

		_windows.append(window);
		focusWindow(window);
	}

	void removeWindow(Window* window) {
		int index = _windows.find(window);
		if (index != -1) {
			bool activeWindowRemoved = _windows[index] == _activeWindow;
			_windows.remove(index);
			if (activeWindowRemoved) {
				_activeWindow = nullptr;
				if (!_windows.isEmpty()) {
					focusWindow(_windows.getLast());
				}
			}
		}
	}

	void focus() {
		if (_activeWindow) _activeWindow->focus();
	}

	void blur() {
		if (_activeWindow) _activeWindow->blur();
	}

	void show() {
		for (int i = 0; i < _windows.count(); i++) {
			_windows[i]->show();
		}
	}

	void hide() {
		for (int i = 0; i < _windows.count(); i++) {
			_windows[i]->hide();
		}
	}

	wx::List<Window*> const& getWindowList() const {
		return _windows;
	}

private:
	Monitor* _monitor;
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

	Rect getRect() const {
		return _info.rect;
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
		if (_activeWorkspace == workspace) return;
		if (_activeWorkspace != nullptr) {
			_activeWorkspace->blur();
		}
		workspace->focus();
		_activeWorkspace = workspace;
	}

	void addWorkspace(Workspace* workspace) {
		// TODO: check if monitor already exists
		workspace->setMonitor(this);

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

	wx::List<Workspace*> const& getWorkspaceList() const {
		return _workspaces;
	}

	void calculateWorkingRect() {
	}

	void focus() {
		_activeWorkspace->focus();
	}

	void blur() {
		_activeWorkspace->blur();
	}

	void addWindow(Window* window) {
		WindowType type = window->getType();
		if (type == WindowType::Normal) {

			Rect monitorRect = getRect();
			Rect windowRect = window->getRect();

			// TODO: add variant placement
			//
			// centered placement
			window->move(
				monitorRect.x + (monitorRect.width / 2) - (windowRect.width / 2),
				monitorRect.y + (monitorRect.height / 2) - (windowRect.height / 2)
			);
			_activeWorkspace->addWindow(window);

		} else if (type == WindowType::Dock) {
			_dockWindows.append(window);
		} else {
			_unmanagedWindows.append(window);
		}

		window->show();
	}

	void removeWindow(Window* window) {
		Workspace* ws = window->getWorkspace();
		if (ws != nullptr) {
			ws->removeWindow(window);
			return;
		}
		for (int i = 0; i < _dockWindows.count(); i++) {
			if (_dockWindows[i] == window) {
				_dockWindows.remove(i);
				return;
			}
		}
		for (int i = 0; i < _unmanagedWindows.count(); i++) {
			if (_unmanagedWindows[i] == window) {
				_unmanagedWindows.remove(i);
				return;
			}
		}
	}

private:
	MonitorInfo _info;
	Workspace* _activeWorkspace = nullptr;
	wx::List<Workspace*> _workspaces;
	wx::List<Window*> _unmanagedWindows;
	wx::List<Window*> _dockWindows;
	int _dockTop = 0;
	int _dockBottom = 0;
	int _dockRight = 0;
	int _dockLeft = 0;
	Rect _workingRect;
};


class Session: public IObserver {
public:
	void run() {
		_context = createContext();

		_context->bindMouseButton(ModKey::Alt, MouseButton::Left);
		_context->bindMouseButton(ModKey::Alt|ModKey::Shift, MouseButton::Left);

		_context->bindKey(ModKey::Alt, Key::Return);
		_context->bindKey(ModKey::Alt, Key::Backspace);
		_context->bindKey(ModKey::Alt, Key::Num1);
		_context->bindKey(ModKey::Alt, Key::Num2);
		_context->bindKey(ModKey::Alt, Key::Num3);
		_context->bindKey(ModKey::Alt, Key::Num4);
		_context->bindKey(ModKey::Alt, Key::A);

		_layoutMonitors();

		_context->listen(this);
	}

	virtual void onWindowCreate(WindowEvent const& e) override {
		Point mousePos = _context->getMousePos();
		Monitor* monitor = _queryMonitorByPos(mousePos);
		if (monitor == nullptr) {
			WX_ASSERT(monitor == nullptr, "invalid monitor state\n");
			return;
		}

		if (_activeMonitor != monitor) {
			focusMonitor(monitor);
		}
		_activeMonitor->addWindow(e.window);
	}

	virtual void onWindowDestroy(WindowEvent const& e) override {
		Monitor* monitor = e.window->getMonitor();
		monitor->removeWindow(e.window);
	}

	virtual void onMousePress(MouseButtonEvent const& e) override {
		if (e.window == nullptr) {
			return;
		}

		focusWindow(e.window);

		if (e.button == MouseButton::Left) {
			if (e.modKey == (ModKey::Alt)) {
				_dragState.mode = DragMode::Move;
			} else if (e.modKey == (ModKey::Alt|ModKey::Shift)) {
				_dragState.mode = DragMode::Resize;
			} else {
				// titlebar grabbing
				Rect rect = e.window->getRect();
				WindowProp prop = e.window->getProp();
				if (e.pos.y < rect.y + prop.titlebarHeight) {
					_dragState.mode = DragMode::Move;
				}
			}

			if (_dragState.mode != DragMode::None) {
				_dragState.window = e.window;

				_dragState.initialWindowRect = _dragState.window->getRect();
				_dragState.initialMousePos = _context->getMousePos();

				_context->unclipWindow(_dragState.window);
			}
		}

		_context->bindMouseMovement();
	}

	virtual void onMouseRelease(MouseButtonEvent const& e) override {
		if (_dragState.mode != DragMode::None) {
			Window* window = _dragState.window;

			_dragState.mode = DragMode::None;
			_dragState.window = nullptr;
			_context->unbindMouseMovement();

			Rect windowRect = window->getRect();
			Monitor* bestMonitor = nullptr;
			int bestArea = 0;

			for (int i = 0; i < _monitors.count(); i++) {
				Monitor* monitor = _monitors[i];
				Rect monitorRect = monitor->getRect();
				Rect intersectRect = intersectRectRect(monitorRect, windowRect);
				int area = intersectRect.width * intersectRect.height;
				if (bestArea < area) {
					bestArea = area;
					bestMonitor = monitor;
				}
			}

			if (bestMonitor == nullptr) {
				printf("invalid drag\n");
				return;
			}

			if (_activeMonitor != bestMonitor) {
				focusMonitor(bestMonitor);
				sendWindowToWorkspace(window, _activeMonitor->getActiveWorkspace());
			}

			Rect workingRect = _activeMonitor->getWorkingRect();
			Rect intersectRect = intersectRectRect(workingRect, windowRect);

			if (intersectRect.width == 0 || intersectRect.height == 0) {
				printf("invalid intersection\n");
				return;
			}

			Rect clipRect;
			clipRect.x = intersectRect.x - windowRect.x;
			clipRect.y = intersectRect.y - windowRect.y;
			clipRect.width = intersectRect.width;
			clipRect.height = intersectRect.height;

			_context->clipWindow(window, clipRect);
		}
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
		if (e.modKey == ModKey::Alt && e.key == Key::Backspace) {
			closeWindow();
			return;
		}
		if (e.modKey == ModKey::Alt && e.key == Key::Num1) {
			focusWorkspaceID("1");
			return;
		}
		if (e.modKey == ModKey::Alt && e.key == Key::Num2) {
			focusWorkspaceID("2");
			return;
		}
		if (e.modKey == ModKey::Alt && e.key == Key::Num3) {
			focusWorkspaceID("3");
			return;
		}
		if (e.modKey == ModKey::Alt && e.key == Key::Num4) {
			focusWorkspaceID("4");
			return;
		}
		if (e.modKey == ModKey::Alt && e.key == Key::A) {
			_logLayout();
			return;
		}
	}

	virtual void onKeyRelease(KeyEvent const& e) override {
	}

	virtual void onMonitorConnect(MonitorEvent const& e) override {
		printf("monitor connected %s\n", e.port.cptr());
		_relayoutMonitors();
	}

	virtual void onMonitorDisconnect(MonitorEvent const& e) override {
		printf("monitor disconnected %s\n", e.port.cptr());
		_relayoutMonitors();
	}

	virtual void onMonitorPropertyChange(MonitorEvent const& e) override {
		printf("monitor property changed %s\n", e.port.cptr());
		_relayoutMonitors();
	}

	void focusMonitor(Monitor* monitor) {
		if (_activeMonitor == monitor) return;
		if (_activeMonitor != nullptr) {
			_activeMonitor->blur();
		}
		_activeMonitor = monitor;
		_activeMonitor->focus();
	}

	void focusWorkspaceID(wx::String const& id) {
		Monitor* previousMonitor = _activeMonitor;
		for (int i = 0; i < _monitors.count(); i++) {
			Monitor* mon = _monitors[i];
			Workspace* ws = mon->queryWorkspace(id);
			if (ws == nullptr) {
				continue;
			}
			printf("activate workspace %s\n", id.cptr());
			if (previousMonitor == mon) {
				mon->getActiveWorkspace()->hide();
			}
			focusMonitor(mon);
			mon->activateWorkspace(ws);
			mon->getActiveWorkspace()->show();
			return;
		}
		printf("no workspace with id %s\n", id.cptr());
	}

	void focusWorkspace(Workspace* workspace) {
		Monitor* previousMonitor = _activeMonitor;
		Monitor* mon = workspace->getMonitor();
		if (previousMonitor == mon) {
			if (previousMonitor->getActiveWorkspace() != workspace) {
				mon->getActiveWorkspace()->hide();
			}
		}
		focusMonitor(mon);
		mon->activateWorkspace(workspace);
		printf("activate workspace %s\n", workspace->getID().cptr());
	}

	void sendWindowToWorkspace(Window* window, Workspace* workspace) {
		window->getWorkspace()->removeWindow(window);
		workspace->addWindow(window);
	}

	void focusWindow(Window* window) {
		// raise to top (if background wallpaper or desktop orginizer you dont have to move it forward)
		window->raise();
		
		Workspace* ws = window->getWorkspace();
		focusWorkspace(ws);
		ws->focusWindow(window);
	}

	void closeWindow() {
		Workspace* ws = _activeMonitor->getActiveWorkspace();
		Window* window = ws->getActiveWindow();
		if (window) {
			window->close();
		}
	}

private:
	Monitor* _queryMonitorByPort(wx::String const& port) {
		for (int i = 0; i < _monitors.count(); i++) {
			if (_monitors[i]->getPort() == port) {
				return _monitors[i];
			}
		}
		return nullptr;
	}

	Monitor* _queryMonitorByPos(Point pos) {
		for (int i = 0; i < _monitors.count(); i++) {
			if (isPointIntersectRect(pos, _monitors[i]->getRect())) {
				return _monitors[i];
			}
		}
		return nullptr;
	}

	void _layoutMonitors() {
		wx::List<MonitorInfo> monitorInfos = _context->getConnectedMonitorInfos();

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

		Monitor* mon1 = _queryMonitorByPort("VHDMI-1");
		if (mon1) {
			mon1->addWorkspace(new Workspace("1"));
			mon1->addWorkspace(new Workspace("2"));
		}

		Monitor* mon2 = _queryMonitorByPort("VHDMI-2");
		if (mon2) {
			mon2->addWorkspace(new Workspace("3"));
			mon2->addWorkspace(new Workspace("4"));
		}
	}

	void _logLayout() {
		printf("\033[2J\033[H");
		for (int i = 0; i < _monitors.count(); i++) {
			Monitor* mon = _monitors[i];
			printf("mon %s:", mon->getPort().cptr());
			if (mon == _activeMonitor) {
				printf(" # active");
			}
			printf("\n");
			for (int j = 0; j < mon->getWorkspaceList().count(); j++) {
				Workspace* ws = mon->getWorkspaceList()[j];
				printf("  ws %s:", ws->getID().cptr());
				if (ws == mon->getActiveWorkspace()) {
					printf(" # active");
				}
				printf("\n");

				for (int k = 0; k < ws->getWindowList().count(); k++) {
					Window* win = ws->getWindowList()[k];
					printf("    win %p:", win);
					if (win == ws->getActiveWindow()) {
						printf(" # active");
					}
					printf("\n");
				}
			}
		}
	}

	void _relayoutMonitors() {
	}

private:
	IContext* _context;
	Monitor* _activeMonitor = nullptr;
	Monitor* _fallbackMonitor = nullptr; // incase of all monitor disconnected all workspace/windows are here
	wx::List<Monitor*> _monitors;
	DragState _dragState;
};
