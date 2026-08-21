#pragma once

#include "Event.hpp"
#include "IObserver.hpp"
#include "IContext.hpp"
#include "input/MouseButton.hpp"
#include "input/Key.hpp"
#include "system/Exec.hpp"
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


private:
	Workspace* _activeWorkspace = nullptr;
	wx::List<Workspace*> _workspaces;
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

		_monitor = new Monitor();
		_monitor->addWorkspace(new Workspace("1"));
		_monitor->addWorkspace(new Workspace("2"));
		_monitor->addWorkspace(new Workspace("3"));
		_monitor->addWorkspace(new Workspace("4"));

		_context->listen(this);

	}

	virtual void onWindowCreate(WindowEvent const& e) override {
		_monitor->getActiveWorkspace()->addWindow(e.window);
	}

	virtual void onWindowDestroy(WindowEvent const& e) override {
		_monitor->getActiveWorkspace()->removeWindow(e.window);
	}

	virtual void onMousePress(MouseButtonEvent const& e) override {
		if (e.window == nullptr) {
			return;
		}

		_dragState.window = e.window;

		 // raise to top (if background wallpaper or desktop orginizer you dont have to move it forward)
		_dragState.window->raise();
		_monitor->getActiveWorkspace()->activateWindow(e.window);

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
	}

	virtual void onMonitorDisconnect(MonitorEvent const& e) override {
	}

	virtual void onMonitorPropertyChange(MonitorEvent const& e) override {
	}

	void activateWorkspace(wx::String const& id) {
		Workspace* workspace = _monitor->queryWorkspace(id);
		if (workspace == nullptr) {
			printf("no workspace with id %s\n", id.cptr());
			return;
		}
		printf("activate workspace %s\n", id.cptr());
		_monitor->activateWorkspace(workspace);
	}
private:
	IContext* _context;
	Monitor* _monitor;
	DragState _dragState;
};
