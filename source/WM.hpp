#pragma once

#include "IObserver.hpp"
#include "IContext.hpp"
#include "input/MouseButton.hpp"
#include "input/Key.hpp"

enum struct DragMode {
	None,
	Move,
	Resize,
};

struct DragState {
	DragMode mode = DragMode::None;
	IWindow* window;
	Point initialMousePos;
	Rect initialWindowRect;
};


class WM: public IObserver {
public:
	void run() {
		_context = createContext();

		_context->bindMouseButton(MouseButton::Left, ModKey::Alt);
		_context->bindMouseButton(MouseButton::Left, ModKey::Alt|ModKey::Shift);

		_context->listen(this);
	}

	virtual void onMousePress(MouseButtonEvent const& e) override {
		if (e.window == nullptr) {
			return;
		}

		_dragState.window = e.window;

		 // raise to top (if background wallpaper or desktop orginizer you dont have to move it forward)
		_dragState.window->raise();

		_dragState.initialWindowRect = _dragState.window->getRect();
		_dragState.initialMousePos = _context->getMousePos();

		if (e.button == MouseButton::Left) {
			_dragState.mode = DragMode::Move;
			if (e.modKey == (ModKey::Alt|ModKey::Shift)) {
				_dragState.mode = DragMode::Resize;
			}
		}

		_context->bindMouseMovement();
	}

	virtual void onMouseRelease(MouseButtonEvent const& e) override {
		if (_dragState.mode != DragMode::None) {
			_dragState.mode = DragMode::None;
		}
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
	DragState _dragState;
};
