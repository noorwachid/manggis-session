#pragma once

#include "Backend/ISessionObserver.hpp"
#include "Backend/X11Connection.hpp"
#include "Backend/X11Hint.hpp"
#include "Window.hpp"
#include "Workspace.hpp"

class X11SessionObserver: public ISessionObserver {
public:
	X11SessionObserver(X11Connection* conn, X11Hint* hint);

	virtual void OnWorkspaceConfigure(Wx::List<Workspace*> const& workspaces) override;

	virtual void OnWorkspaceFocus(Workspace* workspace) override;

	virtual void OnWindowFocus(Window* window) override;

public:
	X11Connection* _conn;
	X11Hint* _hint;
};
