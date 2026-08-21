#pragma once

#include "backend/IWindowBackend.hpp"

enum struct WindowFlags {
	None          = 0,
	Moveable      = 1 << 0,
	Resizeable    = 1 << 1,
	Framed        = 1 << 2,
	Reserved      = 1 << 3, // it consume the space they placed in (statusbar, dock)
};

class Window {
public:
	Window(IWindowBackend* backend, WindowFlags flags = WindowFlags::None): 
		_backend{backend}, _flags{flags} {
	}

	~Window() {
		delete _backend;
	}

	IWindowBackend* getBackend() {
		return _backend;
	}

	Rect getRect() {
		return _backend->getRect();
	}

	void raise() {
		_backend->raise();
	}

	void move(int x, int y) {
		_backend->move(x, y);
	}

	void resize(int width, int height) {
		_backend->resize(width, height);
	}

	void show() {
		_backend->show();
	}

	void hide() {
		_backend->hide();
	}

	void activate() {
		_backend->activate();
	}

	void deactivate() {
		_backend->deactivate();
	}

private:
	IWindowBackend* _backend;
	WindowFlags _flags = WindowFlags::None;
};
