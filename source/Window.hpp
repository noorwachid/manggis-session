#pragma once

#include "backend/IWindowBackend.hpp"

enum struct WindowFlags {
	None          = 0,
	Moveable      = 1 << 0,
	Resizeable    = 1 << 1,
	Framed        = 1 << 2,
	Reserved      = 1 << 3, // it consume the space they placed in (statusbar, dock)
};

class Monitor;
class Workspace;

class Window {
public:
	Window(IWindowBackend* backend, WindowFlags flags);

	~Window();

	IWindowBackend* getBackend();

	WindowProp getProp();

	WindowType getType();

	void setWorkspace(Workspace* workspace);

	Workspace* getWorkspace();

	void setMonitor(Monitor* monitor);

	Monitor* getMonitor();

	Rect getRect();

	void raise();

	void move(int x, int y);

	void resize(int width, int height);

	void show();

	void hide();

	void focus();

	void blur();

	void close();

private:
	IWindowBackend* _backend;
	WindowFlags _flags = WindowFlags::None;
	Monitor* _monitor = nullptr;
	Workspace* _workspace = nullptr;
};
