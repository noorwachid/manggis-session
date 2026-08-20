#pragma once

#include "backend/IWindowBackend.hpp"

class Window {
public:
	Window(IWindowBackend* backend): _backend{backend} {
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

private:
	IWindowBackend* _backend;
};
