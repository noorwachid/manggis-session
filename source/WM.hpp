#pragma once

#include "Event.hpp"
#include "IObserver.hpp"
#include "IContext.hpp"
#include "input/MouseButton.hpp"
#include "input/Key.hpp"
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
	void activateWindow(Window* window) {
		for (int i = 0; i < _windows.count(); i++) {
			_windows[i]->deactivate();
		}
		window->activate();
	}

	void addWindow(Window* window) {
		for (int i = 0; i < _windows.count(); i++) {
			_windows[i]->deactivate();
		}
		_windows.append(window);
		window->activate();
	}

	void removeWindow(Window* window) {
		int index = _windows.find(window);
		if (index != -1) {
			_windows.remove(index);
		}
	}

private:
	wx::List<Window*> _windows;
};


class WM: public IObserver {
public:
	void run() {
		_context = createContext();

		_context->bindMouseButton(ModKey::Alt, MouseButton::Left);
		_context->bindMouseButton(ModKey::Alt|ModKey::Shift, MouseButton::Left);

		_workspace = new Workspace();

		_context->listen(this);

	}

	virtual void onWindowCreate(WindowEvent const& e) override {
		printf("window.created %p\n", e.window);

		_workspace->addWindow(e.window);
	}

	virtual void onWindowDestroy(WindowEvent const& e) override {
		printf("window.destroyed %p\n", e.window);

		_workspace->removeWindow(e.window);
	}

	virtual void onMousePress(MouseButtonEvent const& e) override {
		if (e.window == nullptr) {
			return;
		}

		_dragState.window = e.window;

		 // raise to top (if background wallpaper or desktop orginizer you dont have to move it forward)
		_dragState.window->raise();
		_workspace->activateWindow(e.window);

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
	}

	virtual void onKeyRelease(KeyEvent const& e) override {
	}

	virtual void onMonitorConnect(MonitorEvent const& e) override {
	}

	virtual void onMonitorDisconnect(MonitorEvent const& e) override {
	}

private:
	IContext* _context;
	Workspace* _workspace;
	DragState _dragState;
};
