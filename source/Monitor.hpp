#pragma once

#include "MonitorDef.hpp"
#include "backend/IContext.hpp"
#include "wx/core/List.hpp"
#include "wx/core/String.hpp"

class Window;

class Workspace;

class Monitor {
public:
	Monitor(IContext* context, MonitorDef def);

	void setDef(MonitorDef def);

	wx::String const& getPort() const;

	Rect getRect() const;

	Rect getWorkingRect() const;

	Workspace* getActiveWorkspace();

	Workspace* queryWorkspace(wx::String const& id);

	void focusWorkspace(Workspace* workspace);

	void addWorkspace(Workspace* workspace);

	void removeWorkspace(Workspace* workspace);

	void clearWorkspaceList();

	wx::List<Workspace*> const& getWorkspaceList() const;

	void calculateWorkingRect();

	void focus();

	void blur();

	void addWindow(Window* window);

	void removeWindow(Window* window);

	void showDocks();

	void hideDocks();

private:
	IContext* _context;
	MonitorDef _def;
	Workspace* _activeWorkspace = nullptr;
	wx::List<Workspace*> _workspaces;
	wx::List<Window*> _unmanagedWindows;
	wx::List<Window*> _dockWindows;
	Rect _workingRect;
};

