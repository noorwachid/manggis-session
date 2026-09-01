#include "Monitor.hpp"
#include "Workspace.hpp"
#include "Window.hpp"
#include <cstdio>

Monitor::Monitor(IContext* context, MonitorDef def): _context{context}, _def{def} {
	_workingRect = def.rect;
}

void Monitor::SetDef(MonitorDef def) {
	_def = def;
}

Wx::String const& Monitor::GetPort() const {
	return _def.port;
}

Rect Monitor::GetRect() const {
	return _def.rect;
}

Rect Monitor::GetWorkingRect() const {
	return _workingRect;
}

Workspace* Monitor::GetActiveWorkspace() {
	return _activeWorkspace;
}

Workspace* Monitor::QueryWorkspace(Wx::String const& id) {
	for (int i = 0; i < _workspaces.Count(); i++) {
		if (_workspaces[i]->GetID() == id) {
			return _workspaces[i];
		}
	}
	return nullptr;
}

void Monitor::FocusWorkspace(Workspace* workspace) {
	if (_activeWorkspace == workspace) return;
	if (_activeWorkspace != nullptr) {
		_activeWorkspace->Blur();
	}
	workspace->Focus();
	_activeWorkspace = workspace;
}

void Monitor::AddWorkspace(Workspace* workspace) {
	// TODO: check if monitor already exists
	workspace->SetMonitor(this);

	_workspaces.Append(workspace);
	if (_activeWorkspace == nullptr) {
		_activeWorkspace = workspace;
	}
}

void Monitor::RemoveWorkspace(Workspace* workspace) {
	int index = _workspaces.Find(workspace);
	if (index != -1) {
		bool activeWorkspaceRemoved = _workspaces[index] == _activeWorkspace;
		_workspaces.Remove(index);
		if (activeWorkspaceRemoved) {
			_activeWorkspace = nullptr;
			if (!_workspaces.IsEmpty()) {
				FocusWorkspace(_workspaces.GetLast());
			}
		}
	}
}

void Monitor::ClearWorkspaceList() {
	_workspaces.Clear();
}

Wx::List<Workspace*> const& Monitor::GetWorkspaceList() const {
	return _workspaces;
}

void Monitor::CalculateWorkingRect() {
	Margin margin;

	Rect monitorRect = GetRect();

	for (int i = 0; i < _dockWindows.Count(); i++) {
		Window* window = _dockWindows[i];
		Margin windowExtend = _context->GetDockExtend(window);
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

void Monitor::Focus() {
	_activeWorkspace->Focus();
}

void Monitor::Blur() {
	_activeWorkspace->Blur();
}

void Monitor::AddWindow(Window* window) {
	WindowType type = window->GetType();
	printf("WIN TYPE %d\n", (int) type);
	if (type == WindowType::Normal) {

		Rect monitorRect = GetRect();
		Rect windowRect = window->GetRect();

		// TODO: add variant placement
		//
		// centered placement
		window->Move(
			monitorRect.x + (monitorRect.width / 2) - (windowRect.width / 2),
			monitorRect.y + (monitorRect.height / 2) - (windowRect.height / 2)
		);
		_activeWorkspace->AddWindow(window);

	} else if (type == WindowType::Dock) {
		_dockWindows.Append(window);
		CalculateWorkingRect();
		window->SetMonitor(this);
	} else {
		_unmanagedWindows.Append(window);
		window->SetMonitor(this);
	}

	window->Show();
}

void Monitor::RemoveWindow(Window* window) {
	Workspace* ws = window->GetWorkspace();
	if (ws != nullptr) {
		ws->RemoveWindow(window);
		return;
	}
	for (int i = 0; i < _dockWindows.Count(); i++) {
		if (_dockWindows[i] == window) {
			_dockWindows.Remove(i);
			return;
		}
	}
	for (int i = 0; i < _unmanagedWindows.Count(); i++) {
		if (_unmanagedWindows[i] == window) {
			_unmanagedWindows.Remove(i);
			return;
		}
	}
}

