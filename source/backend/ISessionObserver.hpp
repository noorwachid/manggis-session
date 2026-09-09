#pragma once

// NOTE: 
// this is used to abstract the session observer
// like to register the correct x11 hints foreach action without clutering the main logic

#include "Workspace.hpp"

class ISessionObserver {
public:
	virtual void onMonitorConfigure(wx::List<Monitor*> const& monitor) = 0;

	virtual void onWorkspaceConfigure(wx::List<Workspace*> const& workspaces) = 0;

	virtual void onWorkspaceFocus(Workspace* workspace) = 0;

	virtual void onWindowFocus(Window* window) = 0;
};
