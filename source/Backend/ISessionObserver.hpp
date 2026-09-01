#pragma once

// NOTE: 
// this is used to abstract the session observer
// like to register the correct x11 hints foreach action without clutering the main logic

#include "Workspace.hpp"

class ISessionObserver {
public:
	virtual void OnWorkspaceConfigure(Wx::List<Workspace*> const& workspaces) = 0;

	virtual void OnWorkspaceFocus(Workspace* workspace) = 0;

	virtual void OnWindowFocus(Window* window) = 0;
};
