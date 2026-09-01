#include "Workspace.hpp"
#include "Window.hpp"

Workspace::Workspace(Wx::String const& id): _id{id} {
}

Wx::String const& Workspace::GetID() {
	return _id;
}

void Workspace::SetMonitor(Monitor* monitor) {
	_monitor = monitor;
}

Monitor* Workspace::GetMonitor() {
	return _monitor;
}

Window* Workspace::GetActiveWindow() {
	return _activeWindow;
}

void Workspace::FocusWindow(Window* window) {
	if (_activeWindow == window) return;
	if (_activeWindow != nullptr) {
		_activeWindow->Blur();
	}
	window->Raise();
	window->Focus();
	_activeWindow = window;
}

void Workspace::AddWindow(Window* window) {
	// TODO: check if workspace already exists
	window->SetMonitor(_monitor);
	window->SetWorkspace(this);

	_windows.Append(window);
	FocusWindow(window);
}

void Workspace::RemoveWindow(Window* window) {
	int index = _windows.Find(window);
	if (index != -1) {
		bool activeWindowRemoved = _windows[index] == _activeWindow;
		_windows.Remove(index);
		if (activeWindowRemoved) {
			_activeWindow = nullptr;
			if (!_windows.IsEmpty()) {
				FocusWindow(_windows.GetLast());
			}
		}
	}
}

void Workspace::Focus() {
	if (_activeWindow) _activeWindow->Focus();
}

void Workspace::Blur() {
	if (_activeWindow) _activeWindow->Blur();
}

void Workspace::Show() {
	for (int i = 0; i < _windows.Count(); i++) {
		_windows[i]->Show();
	}
}

void Workspace::Hide() {
	for (int i = 0; i < _windows.Count(); i++) {
		_windows[i]->Hide();
	}
}

Wx::List<Window*> const& Workspace::GetWindowList() const {
	return _windows;
}

