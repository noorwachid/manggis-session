#pragma once

#include "Backend/ISessionObserver.hpp"
#include "Backend/X11Connection.hpp"
#include "Backend/X11Hint.hpp"
#include "Backend/X11WindowBackend.hpp"
#include "Window.hpp"
#include "Workspace.hpp"

class X11SessionObserver: public ISessionObserver {
public:
	X11SessionObserver(X11Connection* conn, X11Hint* hint): _conn{conn}, _hint{hint} {
	}

	virtual void OnWorkspaceConfigure(Wx::List<Workspace*> const& workspaces) override {
		Wx::List<Wx::String> list;
		for (int i = 0; i < workspaces.Count(); i++) {
			list.Append(workspaces[i]->GetID());
		}
		_hint->SetWorkspaces(list);
		_conn->Flush();
	}

	virtual void OnWorkspaceFocus(Workspace* workspace) override {
		_hint->SetActiveWorkspace(workspace->GetID());
		_conn->Flush();
	}

	virtual void OnWindowFocus(Window* window) override {
		xcb_set_input_focus(_conn->Get(), XCB_INPUT_FOCUS_POINTER_ROOT, ((X11WindowBackend*) window->GetBackend())->getClientHandle(), XCB_CURRENT_TIME);
		_conn->Flush();
	}

public:
	X11Connection* _conn;
	X11Hint* _hint;
};
