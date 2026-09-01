#pragma once

#include "MonitorDef.hpp"
#include "Backend/IContext.hpp"
#include "Wx/Core/List.hpp"
#include "Wx/Core/String.hpp"

class Window;

class Workspace;

class Monitor {
public:
	Monitor(IContext* context, MonitorDef def);

	void SetDef(MonitorDef def);

	Wx::String const& GetPort() const;

	Rect GetRect() const;

	Rect GetWorkingRect() const;

	Workspace* GetActiveWorkspace();

	Workspace* QueryWorkspace(Wx::String const& id);

	void FocusWorkspace(Workspace* workspace);

	void AddWorkspace(Workspace* workspace);

	void RemoveWorkspace(Workspace* workspace);

	void ClearWorkspaceList();

	Wx::List<Workspace*> const& GetWorkspaceList() const;

	void CalculateWorkingRect();

	void Focus();

	void Blur();

	void AddWindow(Window* window);

	void RemoveWindow(Window* window);

private:
	IContext* _context;
	MonitorDef _def;
	Workspace* _activeWorkspace = nullptr;
	Wx::List<Workspace*> _workspaces;
	Wx::List<Window*> _unmanagedWindows;
	Wx::List<Window*> _dockWindows;
	Rect _workingRect;
};

