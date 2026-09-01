#pragma once

#include "Wx/Core/String.hpp"
#include "Wx/Core/List.hpp"

class Window;

class Monitor;

class Workspace {
public:
	Workspace(Wx::String const& id);

	Wx::String const& GetID();

	void SetMonitor(Monitor* monitor);

	Monitor* GetMonitor();

	Window* GetActiveWindow();

	void FocusWindow(Window* window);

	void AddWindow(Window* window);

	void RemoveWindow(Window* window);

	void Focus();

	void Blur();

	void Show();

	void Hide(); 

	Wx::List<Window*> const& GetWindowList() const;

private:
	Monitor* _monitor;
	Window* _activeWindow = nullptr;
	Wx::String _id;
	Wx::List<Window*> _windows;
};

