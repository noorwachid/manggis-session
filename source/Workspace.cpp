#include "Workspace.hpp"
#include "Window.hpp"

Workspace::Workspace(wx::String const& id): _id{id} {
}

wx::String const& Workspace::getID() {
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
	window->focus();
	_activeWindow = window;
}

void Workspace::addWindow(Window* window) {
	// TODO: check if workspace already exists
	window->setMonitor(_monitor);
	window->setWorkspace(this);

	_windows.append(window);
	focusWindow(window);
}

void Workspace::removeWindow(Window* window) {
	int index = _windows.find(window);
	if (index != -1) {
		bool activeWindowRemoved = _windows[index] == _activeWindow;
		_windows.remove(index);
		if (activeWindowRemoved) {
			_activeWindow = nullptr;
			if (!_windows.isEmpty()) {
				focusWindow(_windows.getLast());
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
	for (int i = 0; i < _windows.count(); i++) {
		_windows[i]->show();
	}
}

void Workspace::hide() {
	for (int i = 0; i < _windows.count(); i++) {
		_windows[i]->hide();
	}
}

wx::List<Window*> const& Workspace::getWindowList() const {
	return _windows;
}

