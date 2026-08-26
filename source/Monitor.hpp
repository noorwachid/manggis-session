#pragma once

#include "MonitorInfo.hpp"
#include "backend/IContext.hpp"
#include "wx/core/List.hpp"
#include "wx/core/String.hpp"

class Window;

class Workspace;

class Monitor {
public:
	Monitor(IContext* context, MonitorInfo info);

	void setInfo(MonitorInfo info);

	wx::String const& getPort() const;

	Rect getRect() const;

	Rect getWorkingRect() const;

	Workspace* getActiveWorkspace();

	Workspace* queryWorkspace(wx::String const& id);

	void activateWorkspace(Workspace* workspace);

	void addWorkspace(Workspace* workspace);

	void removeWorkspace(Workspace* workspace);

	void clearWorkspaceList();

	wx::List<Workspace*> const& getWorkspaceList() const;

	void calculateWorkingRect();

	void focus();

	void blur();

	void addWindow(Window* window);

	void removeWindow(Window* window);

private:
	IContext* _context;
	MonitorInfo _info;
	Workspace* _activeWorkspace = nullptr;
	wx::List<Workspace*> _workspaces;
	wx::List<Window*> _unmanagedWindows;
	wx::List<Window*> _dockWindows;
	Rect _workingRect;
};

