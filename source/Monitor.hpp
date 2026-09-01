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

	void setDef(MonitorDef def);

	Wx::String const& getPort() const;

	Rect getRect() const;

	Rect getWorkingRect() const;

	Workspace* getActiveWorkspace();

	Workspace* queryWorkspace(Wx::String const& id);

	void activateWorkspace(Workspace* workspace);

	void addWorkspace(Workspace* workspace);

	void removeWorkspace(Workspace* workspace);

	void clearWorkspaceList();

	Wx::List<Workspace*> const& getWorkspaceList() const;

	void calculateWorkingRect();

	void focus();

	void blur();

	void addWindow(Window* window);

	void removeWindow(Window* window);

private:
	IContext* _context;
	MonitorDef _def;
	Workspace* _activeWorkspace = nullptr;
	Wx::List<Workspace*> _workspaces;
	Wx::List<Window*> _unmanagedWindows;
	Wx::List<Window*> _dockWindows;
	Rect _workingRect;
};

