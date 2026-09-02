#include "backend/X11SessionObserver.hpp"
#include "backend/X11WindowBackend.hpp"

X11SessionObserver::X11SessionObserver(X11Connection* conn, X11Hint* hint): _conn{conn}, _hint{hint} {
}

void X11SessionObserver::onWorkspaceConfigure(wx::List<Workspace*> const& workspaces) {
	wx::List<wx::String> list;
	for (int i = 0; i < workspaces.count(); i++) {
		list.append(workspaces[i]->getID());
	}
	_hint->setWorkspaces(list);
	_conn->flush();
}

void X11SessionObserver::onWorkspaceFocus(Workspace* workspace) {
	_hint->setActiveWorkspace(workspace->getID());
	_conn->flush();
}

void X11SessionObserver::onWindowFocus(Window* window) {
	xcb_set_input_focus(_conn->get(), XCB_INPUT_FOCUS_POINTER_ROOT, ((X11WindowBackend*) window->getBackend())->getClientHandle(), XCB_CURRENT_TIME);
	_conn->flush();
}

