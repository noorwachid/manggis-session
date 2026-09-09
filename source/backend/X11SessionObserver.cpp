#include "backend/X11SessionObserver.hpp"
#include "backend/X11WindowBackend.hpp"
#include "Monitor.hpp"

X11SessionObserver::X11SessionObserver(X11Connection* conn, X11Hint* hint): _conn{conn}, _hint{hint} {
}

void X11SessionObserver::onMonitorConfigure(wx::List<Monitor*> const& monitors) {
	_monitors = monitors;
	wx::List<Rect> workingRects;
	for (int i = 0; i < monitors.count(); i++) {
		for (int j = 0; j < monitors[i]->getWorkspaceList().count(); j++) {
			workingRects.append(monitors[i]->getWorkingRect());
		}
	}
	_hint->setWorkspaceWorkingRect(workingRects);
	_conn->flush();
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
	wx::List<xcb_window_t> handles;
	for (int i = 0; i < _monitors.count(); i++) {
		Monitor* monitor = _monitors[i];
		for (int j = 0; j < monitor->getWorkspaceList().count(); j++) {
			Workspace* workspace = monitor->getWorkspaceList()[j];
			for (int k = 0; k < workspace->getWindowList().count(); k++) {
				Window* window = workspace->getWindowList()[k];
				X11WindowBackend* backend = (X11WindowBackend*) window->getBackend();
				handles.append(backend->getClientHandle());
			}
		}
	}
	_hint->setWindows(handles);
	_conn->flush();
}

