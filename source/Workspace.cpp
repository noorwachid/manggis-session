#include "Workspace.hpp"
#include "Window.hpp"

Workspace::Workspace(Wx::String const& id): _id{id} {
}

Wx::String const& Workspace::getID() {
	return _id;
}

void Workspace::setMonitor(Monitor* monitor) {
	_monitor = monitor;
}

Monitor* Workspace::getMonitor() {
	return _monitor;
}

Window* Workspace::getActiveWindow() {
	return _activeWindow;
}

void Workspace::focusWindow(Window* window) {
	if (_activeWindow == window) return;
	if (_activeWindow != nullptr) {
		_activeWindow->blur();
	}
	window->raise();
	window->focus();
	_activeWindow = window;
}

void Workspace::addWindow(Window* window) {
	// TODO: check if workspace already exists
	window->setMonitor(_monitor);
	window->setWorkspace(this);

	_windows.Append(window);
	focusWindow(window);
}

void Workspace::removeWindow(Window* window) {
	int index = _windows.Find(window);
	if (index != -1) {
		bool activeWindowRemoved = _windows[index] == _activeWindow;
		_windows.Remove(index);
		if (activeWindowRemoved) {
			_activeWindow = nullptr;
			if (!_windows.IsEmpty()) {
				focusWindow(_windows.GetLast());
			}
		}
	}
}

void Workspace::focus() {
	if (_activeWindow) _activeWindow->focus();
}

void Workspace::blur() {
	if (_activeWindow) _activeWindow->blur();
}

void Workspace::show() {
	for (int i = 0; i < _windows.Count(); i++) {
		_windows[i]->show();
	}
}

void Workspace::hide() {
	for (int i = 0; i < _windows.Count(); i++) {
		_windows[i]->hide();
	}
}

Wx::List<Window*> const& Workspace::getWindowList() const {
	return _windows;
}

