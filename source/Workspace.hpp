#pragma once

#include "Wx/Core/String.hpp"
#include "Wx/Core/List.hpp"

class Window;

class Monitor;

class Workspace {
public:
	Workspace(Wx::String const& id);

	Wx::String const& getID();

	void setMonitor(Monitor* monitor);

	Monitor* getMonitor();

	Window* getActiveWindow();

	void focusWindow(Window* window);

	void addWindow(Window* window);

	void removeWindow(Window* window);

	void focus();

	void blur();

	void show();

	void hide(); 

	Wx::List<Window*> const& getWindowList() const;

private:
	Monitor* _monitor;
	Window* _activeWindow = nullptr;
	Wx::String _id;
	Wx::List<Window*> _windows;
};

