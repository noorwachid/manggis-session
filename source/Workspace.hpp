#pragma once

#include "wx/core/String.hpp"
#include "wx/core/List.hpp"

class Window;

class Monitor;

class Workspace {
public:
	Workspace(wx::String const& id);

	wx::String const& getID();

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

	wx::List<Window*> const& getWindowList() const;

	bool isFullscreen() const {
		return _fullscreen;
	}

	void setFullscreen(bool fullscreen) {
		_fullscreen = fullscreen;
	}

private:
	Monitor* _monitor;
	Window* _activeWindow = nullptr;
	wx::String _id;
	wx::List<Window*> _windows;

	bool _fullscreen = false;
};

