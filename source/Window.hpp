#pragma once

#include "backend/IWindowBackend.hpp"

class Monitor;
class Workspace;

class Window {
public:
	Window(IWindowBackend* backend);

	~Window();

	IWindowBackend* getBackend();

	void setProp(WindowProp prop);

	WindowProp getProp();

	WindowType getType();

	wx::String getTitle();

	void setWorkspace(Workspace* workspace);

	Workspace* getWorkspace();

	void setMonitor(Monitor* monitor);

	Monitor* getMonitor();

	Rect getRect();

	WindowState getState();

	void setState(WindowState state);

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
