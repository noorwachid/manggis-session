#pragma once

#include "Backend/IWindowBackend.hpp"

class Monitor;
class Workspace;

class Window {
public:
	Window(IWindowBackend* backend);

	~Window();

	IWindowBackend* getBackend();

	WindowProp getProp();

	WindowType getType();

	Wx::String getTitle();

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
	Monitor* _monitor = nullptr;
	Workspace* _workspace = nullptr;
};
