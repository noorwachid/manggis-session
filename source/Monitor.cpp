#include "Monitor.hpp"
#include "Workspace.hpp"
#include "Window.hpp"
#include "backend/IWindowBackend.hpp"
#include <cstdio>

Monitor::Monitor(IContext* context, MonitorDef def): _context{context}, _def{def} {
	_workingRect = def.rect;
}

void Monitor::setDef(MonitorDef def) {
	_def = def;
}

wx::String const& Monitor::getPort() const {
	return _def.port;
}

Rect Monitor::getRect() const {
	return _def.rect;
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

void Monitor::focusWorkspace(Workspace* workspace) {
	if (_activeWorkspace == workspace) return;
	if (_activeWorkspace != nullptr) {
		_activeWorkspace->blur();
		_activeWorkspace->hide();
	}
	workspace->focus();
	workspace->show();
	_activeWorkspace = workspace;
}

void Monitor::addWorkspace(Workspace* workspace) {
	// TODO: check if monitor already exists
	workspace->setMonitor(this);
	_workspaces.append(workspace);

	if (_activeWorkspace == nullptr) {
		focusWorkspace(workspace);
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
				focusWorkspace(_workspaces.getLast());
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
	Margin margin;

	Rect monitorRect = getRect();

	for (int i = 0; i < _dockWindows.count(); i++) {
		Window* window = _dockWindows[i];
		Margin windowExtend = _context->getDockExtend(window);
		if (margin.top < windowExtend.top) {
			margin.top = windowExtend.top;
		}
		if (margin.bottom < windowExtend.bottom) {
			margin.bottom = windowExtend.bottom;
		}
		if (margin.left < windowExtend.left) {
			margin.left = windowExtend.left;
		}
		if (margin.right < windowExtend.right) {
			margin.right = windowExtend.right;
		}
	}

	_workingRect.x = monitorRect.x + margin.left;
	_workingRect.y = monitorRect.y + margin.top;
	_workingRect.width = monitorRect.width - (margin.left + margin.right);
	_workingRect.height = monitorRect.height - (margin.top + margin.bottom);

	printf("monitor: (%d,%d,%d,%d) working: (%d,%d,%d,%d)\n",
		monitorRect.x, monitorRect.y, monitorRect.width, monitorRect.height,
		_workingRect.x, _workingRect.y, _workingRect.width, _workingRect.height
	);

}

void Monitor::focus() {
	_activeWorkspace->focus();
}

void Monitor::blur() {
	_activeWorkspace->blur();
}

void Monitor::addWindow(Window* window) {
	WindowType type = window->getType();

	if (type == WindowType::Normal || type == WindowType::Dialog) {

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
		_activeWorkspace->focusWindow(window);

	} else if (type == WindowType::Dock) {
		_dockWindows.append(window);
		calculateWorkingRect();
		window->setMonitor(this);
	} else {
		_unmanagedWindows.append(window);
		window->setMonitor(this);
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

void Monitor::showDocks() {
	for (int i = 0; i < _dockWindows.count(); i++) {
		_dockWindows[i]->show();
	}
	calculateWorkingRect();
}

void Monitor::hideDocks() {
	for (int i = 0; i < _dockWindows.count(); i++) {
		_dockWindows[i]->hide();
	}
	_workingRect = getRect();
}
