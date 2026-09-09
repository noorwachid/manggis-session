#pragma once

#include "backend/ISessionObserver.hpp"
#include "backend/X11Connection.hpp"
#include "backend/X11Hint.hpp"
#include "Window.hpp"
#include "Workspace.hpp"

class X11SessionObserver: public ISessionObserver {
public:
	X11SessionObserver(X11Connection* conn, X11Hint* hint);

	virtual void onMonitorConfigure(wx::List<Monitor*> const& monitors) override;

	virtual void onWorkspaceConfigure(wx::List<Workspace*> const& workspaces) override;

	virtual void onWorkspaceFocus(Workspace* workspace) override;

	virtual void onWindowFocus(Window* window) override;

public:
	X11Connection* _conn;
	X11Hint* _hint;
	wx::List<Monitor*> _monitors;
};
