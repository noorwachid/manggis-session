#include "Monitor.hpp"
#include "Workspace.hpp"
#include "Window.hpp"

Monitor::Monitor(MonitorInfo info): _info{info} {
	_workingRect = info.rect;
}

void Monitor::setInfo(MonitorInfo info) {
	_info = info;
}

wx::String const& Monitor::getPort() const {
	return _info.port;
}

Rect Monitor::getRect() const {
	return _info.rect;
}

Rect Monitor::getWorkingRect() const {
	return _workingRect;
}

Workspace* Monitor::getActiveWorkspace() {
	return _activeWorkspace;
}

Workspace* Monitor::queryWorkspace(wx::String const& id) {
	for (int i = 0; i < _workspaces.count(); i++) {
		if (_workspaces[i]->getID() == id) {
			return _workspaces[i];
		}
	}
	return nullptr;
}

void Monitor::activateWorkspace(Workspace* workspace) {
	if (_activeWorkspace == workspace) return;
	if (_activeWorkspace != nullptr) {
		_activeWorkspace->blur();
	}
	workspace->focus();
	_activeWorkspace = workspace;
}

void Monitor::addWorkspace(Workspace* workspace) {
	// TODO: check if monitor already exists
	workspace->setMonitor(this);

	_workspaces.append(workspace);
	if (_activeWorkspace == nullptr) {
		_activeWorkspace = workspace;
	}
}

void Monitor::removeWorkspace(Workspace* workspace) {
	int index = _workspaces.find(workspace);
	if (index != -1) {
		bool activeWorkspaceRemoved = _workspaces[index] == _activeWorkspace;
		_workspaces.remove(index);
		if (activeWorkspaceRemoved) {
			_activeWorkspace = nullptr;
			if (!_workspaces.isEmpty()) {
				activateWorkspace(_workspaces.getLast());
			}
		}
	}
}

void Monitor::clearWorkspaceList() {
	_workspaces.clear();
}

wx::List<Workspace*> const& Monitor::getWorkspaceList() const {
	return _workspaces;
}

void Monitor::calculateWorkingRect() {
}

void Monitor::focus() {
	_activeWorkspace->focus();
}

void Monitor::blur() {
	_activeWorkspace->blur();
}

void Monitor::addWindow(Window* window) {
	WindowType type = window->getType();
	if (type == WindowType::Normal) {

		Rect monitorRect = getRect();
		Rect windowRect = window->getRect();

		// TODO: add variant placement
		//
		// centered placement
		window->move(
			monitorRect.x + (monitorRect.width / 2) - (windowRect.width / 2),
			monitorRect.y + (monitorRect.height / 2) - (windowRect.height / 2)
		);
		_activeWorkspace->addWindow(window);

	} else if (type == WindowType::Dock) {
		_dockWindows.append(window);
	} else {
		_unmanagedWindows.append(window);
	}

	window->show();
}

void Monitor::removeWindow(Window* window) {
	Workspace* ws = window->getWorkspace();
	if (ws != nullptr) {
		ws->removeWindow(window);
		return;
	}
	for (int i = 0; i < _dockWindows.count(); i++) {
		if (_dockWindows[i] == window) {
			_dockWindows.remove(i);
			return;
		}
	}
	for (int i = 0; i < _unmanagedWindows.count(); i++) {
		if (_unmanagedWindows[i] == window) {
			_unmanagedWindows.remove(i);
			return;
		}
	}
}

