#pragma once

#include "backend/ISessionObserver.hpp"
#include "backend/X11Connection.hpp"
#include "backend/X11Hint.hpp"

class X11SessionObserver: public ISessionObserver {
public:
	X11SessionObserver(X11Connection* conn, X11Hint* hint): _conn{conn}, _hint{hint} {
	}

	virtual void onWorkspaceConfigure(wx::List<Workspace*> const& workspaces) override {
		wx::List<wx::String> list;
		for (int i = 0; i < workspaces.count(); i++) {
			list.append(workspaces[i]->getID());
		}
		_hint->setWorkspaces(list);
		_conn->flush();
	}

	virtual void onWorkspaceActivate(Workspace* workspace) override {
		_hint->setActiveWorkspace(workspace->getID());
		_conn->flush();
	}

public:
	X11Connection* _conn;
	X11Hint* _hint;
};
