#pragma once

#include "backend/ISessionObserver.hpp"
#include "backend/X11Connection.hpp"
#include "backend/X11Hint.hpp"
#include "backend/X11WindowBackend.hpp"
#include "Window.hpp"
#include "Workspace.hpp"

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

	virtual void onWorkspaceFocus(Workspace* workspace) override {
		_hint->setActiveWorkspace(workspace->getID());
		_conn->flush();
	}

	virtual void onWindowFocus(Window* window) override {
		xcb_set_input_focus(_conn->get(), XCB_INPUT_FOCUS_POINTER_ROOT, ((X11WindowBackend*) window->getBackend())->getClientHandle(), XCB_CURRENT_TIME);
		_conn->flush();
	}

public:
	X11Connection* _conn;
	X11Hint* _hint;
};
