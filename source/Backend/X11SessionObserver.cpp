#include "Backend/X11SessionObserver.hpp"
#include "Backend/X11WindowBackend.hpp"

X11SessionObserver::X11SessionObserver(X11Connection* conn, X11Hint* hint): _conn{conn}, _hint{hint} {
}

void X11SessionObserver::OnWorkspaceConfigure(Wx::List<Workspace*> const& workspaces) {
	Wx::List<Wx::String> list;
	for (int i = 0; i < workspaces.Count(); i++) {
		list.Append(workspaces[i]->GetID());
	}
	_hint->SetWorkspaces(list);
	_conn->Flush();
}

void X11SessionObserver::OnWorkspaceFocus(Workspace* workspace) {
	_hint->SetActiveWorkspace(workspace->GetID());
	_conn->Flush();
}

void X11SessionObserver::OnWindowFocus(Window* window) {
	xcb_set_input_focus(_conn->Get(), XCB_INPUT_FOCUS_POINTER_ROOT, ((X11WindowBackend*) window->GetBackend())->GetClientHandle(), XCB_CURRENT_TIME);
	_conn->Flush();
}

