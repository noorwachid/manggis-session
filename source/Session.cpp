#include "Session.hpp"
#include "Direction.hpp"
#include "backend/IWindowBackend.hpp"
#include "config/Config.hpp"
#include "input/Key.hpp"
#include <climits>

bool operator==(KeyIndex a, KeyIndex b) {
	return a.modKey == b.modKey && a.key == b.key;
}

wx::Uint32 getHash(KeyIndex index) {
	return wx::combineHash(wx::getHash((int)index.modKey), wx::getHash((int)index.key));
}

class Monitor;

#ifdef XEPHYR_ENABLE
extern int xephyrMonitorConfiguration;
#endif


void Session::run(Config const& config) {
	_config = config;

	_context = createContext();
	_observer = _context->getSessionObserver();

	_context->bindMouseButton(ModKey::Alt, MouseButton::Left);
	_context->bindMouseButton(ModKey::Alt|ModKey::Shift, MouseButton::Left);

	_context->bindKey(ModKey::Alt, Key::A);
	_context->bindKey(ModKey::Alt, Key::M);
	_context->bindKey(ModKey::Alt, Key::Tab);

	for (int i = 0; i < _config.bindings.count(); i++) {
		ConfigBinding const& binding = _config.bindings[i];

		KeyIndex index;
		index.modKey = binding.modKey;
		index.key = binding.key;

		_context->bindKey(binding.modKey, binding.key);
		_keyBindings.set(index, [this, binding]() {
			for (int k = 0; k < binding.commands.count(); k++) {
				_doCommand(binding.commands[k]);
			}
		});
	}

	_relayoutMonitors();

	for (int i = 0; i < _config.autostarts.count(); i++) {
		_doCommand(_config.autostarts[i]);
	}

	_context->run(this);
}

ConfigBinding const* Session::queryBinding(ModKey modKey, Key key) {
	for (int i = 0; i < _config.bindings.count(); i++) {
		ConfigBinding const& binding = _config.bindings[i];
		if (binding.modKey == modKey && binding.key == key) {
			return &binding;
		}
	}
	return nullptr;
}

void Session::onWindowCreate(WindowEvent const& e) {
	Point mousePos = _context->getMousePos();
	Monitor* monitor = _queryMonitorByPos(mousePos);
	if (monitor == nullptr) {
		WX_ASSERT(monitor == nullptr, "invalid monitor state\n");
		return;
	}

	if (_activeMonitor != monitor) {
		focusMonitor(monitor);
	}
	_activeMonitor->addWindow(e.window);
	_observer->onWindowFocus(e.window);

	LOG("ACTIEV MON %p\n", _activeMonitor);
	LOG("WINDOW CREATE %p, WS: %p, MON: %p\n", e.window, e.window->getWorkspace(), e.window->getMonitor());
	_clipWindow(e.window);
}

void Session::onWindowDestroy(WindowEvent const& e) {
	Monitor* monitor = e.window->getMonitor();
	monitor->removeWindow(e.window);
}

void Session::onWindowMoveRequest(WindowMovementEvent const& e) {
	_beginDrag(DragMode::Move, e.window);
}

void Session::onWindowResizeRequest(WindowResizingEvent const& e) {
	_beginDrag(DragMode::Resize, e.window, e.area);
}

void Session::onWindowMaximizeRequest(WindowActionEvent const& e) {
}

void Session::onWindowFullscreenRequest(WindowActionEvent const& e) {
}

void Session::onMousePress(MouseButtonEvent const& e) {
	if (e.window == nullptr) {
		Monitor* mon = _queryMonitorByPos(e.pos);
		if (mon != nullptr) {
			focusMonitor(mon);
		}
		return;
	}

	WindowType type = e.window->getType();
	if (type != WindowType::Normal) {
		return;
	}

	focusWindow(e.window);

	if (e.button == MouseButton::Left) {
		if (e.modKey == (ModKey::Alt)) {
			_beginDrag(DragMode::Move, e.window);
		} else if (e.modKey == (ModKey::Alt|ModKey::Shift)) {
			_beginDrag(DragMode::Resize, e.window);
		} else {
			// titlebar grabbing
			Rect rect = e.window->getRect();
			WindowProp prop = e.window->getProp();
			if (e.pos.y < rect.y + prop.titlebarHeight) {
				_beginDrag(DragMode::Move, e.window);
			}
		}
	}

}

void Session::onMouseRelease(MouseButtonEvent const& e) {
	if (_dragState.mode != DragMode::None) {
		Window* window = _dragState.window;

		_dragState.mode = DragMode::None;
		_dragState.window = nullptr;
		_context->unbindMouseMovement();

		Rect windowRect = window->getRect();
		Monitor* bestMonitor = nullptr;
		int bestArea = 0;

		for (int i = 0; i < _monitors.count(); i++) {
			Monitor* monitor = _monitors[i];
			Rect monitorRect = monitor->getRect();
			Rect intersectRect = intersectRectRect(monitorRect, windowRect);
			int area = intersectRect.width * intersectRect.height;
			if (bestArea < area) {
				bestArea = area;
				bestMonitor = monitor;
			}
		}

		if (bestMonitor == nullptr) {
			LOG("invalid drag\n");
			return;
		}

		if (_activeMonitor != bestMonitor) {
			focusWorkspace(bestMonitor->getActiveWorkspace());
			window->getWorkspace()->removeWindow(window);
			bestMonitor->getActiveWorkspace()->addWindow(window);
		}

		_clipWindow(window);
	}
}

void Session::onMouseMove(MouseMovementEvent const& e) {
	if (_dragState.mode == DragMode::Move) {
		int x = _dragState.initialWindowRect.x + (e.pos.x - _dragState.initialMousePos.x);
		int y = _dragState.initialWindowRect.y + (e.pos.y - _dragState.initialMousePos.y);
		_dragState.window->move(x, y);
	}
	if (_dragState.mode == DragMode::Resize) {
		if (_dragState.area == WindowResizingArea::NorthWest) {
			int x = _dragState.initialWindowRect.x + (e.pos.x - _dragState.initialMousePos.x);
			int y = _dragState.initialWindowRect.y + (e.pos.y - _dragState.initialMousePos.y);
			int width = _dragState.initialWindowRect.width - (e.pos.x - _dragState.initialMousePos.x);
			int height = _dragState.initialWindowRect.height - (e.pos.y - _dragState.initialMousePos.y);
			_dragState.window->move(x, y);
			_dragState.window->resize(width, height);
		}
		if (_dragState.area == WindowResizingArea::North) {
			int x = _dragState.initialWindowRect.x;
			int y = _dragState.initialWindowRect.y + (e.pos.y - _dragState.initialMousePos.y);
			int width = _dragState.initialWindowRect.width;
			int height = _dragState.initialWindowRect.height - (e.pos.y - _dragState.initialMousePos.y);
			_dragState.window->move(x, y);
			_dragState.window->resize(width, height);
		}
		if (_dragState.area == WindowResizingArea::West) {
			int x = _dragState.initialWindowRect.x + (e.pos.x - _dragState.initialMousePos.x);
			int y = _dragState.initialWindowRect.y;
			int width = _dragState.initialWindowRect.width - (e.pos.x - _dragState.initialMousePos.x);
			int height = _dragState.initialWindowRect.height;
			_dragState.window->move(x, y);
			_dragState.window->resize(width, height);
		}


		if (_dragState.area == WindowResizingArea::NorthEast) {
			int x = _dragState.initialWindowRect.x;
			int y = _dragState.initialWindowRect.y + (e.pos.y - _dragState.initialMousePos.y);
			int width = _dragState.initialWindowRect.width + (e.pos.x - _dragState.initialMousePos.x);
			int height = _dragState.initialWindowRect.height - (e.pos.y - _dragState.initialMousePos.y);
			_dragState.window->move(x, y);
			_dragState.window->resize(width, height);
		}
		if (_dragState.area == WindowResizingArea::SouthWest) {
			int x = _dragState.initialWindowRect.x + (e.pos.x - _dragState.initialMousePos.x);
			int y = _dragState.initialWindowRect.y;
			int width = _dragState.initialWindowRect.width - (e.pos.x - _dragState.initialMousePos.x);
			int height = _dragState.initialWindowRect.height + (e.pos.y - _dragState.initialMousePos.y);
			_dragState.window->move(x, y);
			_dragState.window->resize(width, height);
		}


		if (_dragState.area == WindowResizingArea::East) {
			int width = _dragState.initialWindowRect.width + (e.pos.x - _dragState.initialMousePos.x);
			int height = _dragState.initialWindowRect.height;
			_dragState.window->resize(width, height);
		}
		if (_dragState.area == WindowResizingArea::South) {
			int width = _dragState.initialWindowRect.width;
			int height = _dragState.initialWindowRect.height + (e.pos.y - _dragState.initialMousePos.y);
			_dragState.window->resize(width, height);
		}
		if (_dragState.area == WindowResizingArea::SouthEast) {
			int width = _dragState.initialWindowRect.width + (e.pos.x - _dragState.initialMousePos.x);
			int height = _dragState.initialWindowRect.height + (e.pos.y - _dragState.initialMousePos.y);
			_dragState.window->resize(width, height);
		}
	}
}

void Session::onKeyPress(KeyEvent const& e) {
	KeyIndex index;
	index.modKey = e.modKey;
	index.key = e.key;
	KeyCallback* callback = _keyBindings.getOrNull(index);
	LOG("GOT %p\n", callback);
	if (callback != nullptr) {
		(*callback)();
		return;
	}

	if (e.modKey == ModKey::Alt && e.key == Key::A) {
		_logLayout();
		return;
	}
	if (e.modKey == ModKey::Alt && e.key == Key::M) {
		#ifdef XEPHYR_ENABLE
		xephyrMonitorConfiguration++;
		_relayoutMonitors();
		#endif
		return;
	}

	if (e.modKey == ModKey::Alt && e.key == Key::Tab) {
		_context->bindModKey(ModKey::Alt);
		cycleWindow();
		return;
	}

}

void Session::onKeyRelease(KeyEvent const& e) {
	if (e.modKey == ModKey::Alt && e.key == Key::None) {
		_context->unbindModKey(ModKey::Alt);
		_cyclingWindow = false;
		printf("END CYCLE\n");
	}
}

void Session::onMonitorConnect(MonitorEvent const& e) {
	LOG("monitor connected %s\n", e.port.cptr());
	_relayoutMonitors();
}

void Session::onMonitorDisconnect(MonitorEvent const& e) {
	LOG("monitor disconnected %s\n", e.port.cptr());
	_relayoutMonitors();
}

void Session::onMonitorPropertyChange(MonitorEvent const& e) {
	LOG("monitor property changed %s\n", e.port.cptr());
	_relayoutMonitors();
}

void Session::focusMonitor(Monitor* monitor) {
	if (_activeMonitor == monitor) return;
	if (_activeMonitor != nullptr) {
		_activeMonitor->blur();
	}
	_activeMonitor = monitor;
	_activeMonitor->focus();
	_observer->onWorkspaceFocus(_activeMonitor->getActiveWorkspace());
}

void Session::focusWorkspaceID(wx::String const& id) {
	Monitor* previousMonitor = _activeMonitor;
	for (int i = 0; i < _monitors.count(); i++) {
		Monitor* mon = _monitors[i];
		Workspace* ws = mon->queryWorkspace(id);
		if (ws == nullptr) {
			continue;
		}
		LOG("activate workspace %s\n", id.cptr());
		if (previousMonitor == mon) {
			mon->getActiveWorkspace()->hide();
		}
		focusMonitor(mon);
		mon->activateWorkspace(ws);
		_observer->onWorkspaceFocus(ws);
		mon->getActiveWorkspace()->show();
		return;
	}
	LOG("no workspace with id %s\n", id.cptr());
}

void Session::focusWorkspace(Workspace* workspace) {
	Monitor* previousMonitor = _activeMonitor;
	Monitor* mon = workspace->getMonitor();
	if (previousMonitor == mon) {
		if (previousMonitor->getActiveWorkspace() != workspace) {
			mon->getActiveWorkspace()->hide();
		}
	}
	focusMonitor(mon);
	mon->activateWorkspace(workspace);
	_observer->onWorkspaceFocus(workspace);
	LOG("activate workspace %s\n", workspace->getID().cptr());
}

void Session::sendWorkspaceToMonitor(Workspace* workspace, Monitor* monitor) {
	Monitor* oldMonitor = workspace->getMonitor();
	monitor->addWorkspace(workspace);
	if (oldMonitor != nullptr) {
		if (oldMonitor != monitor) {
			for (int i = 0; i < workspace->getWindowList().count(); i++) {
				Window* window = workspace->getWindowList()[i];
				sendWindowToWorkspace(window, workspace);
			}
		}
	}
}

void Session::sendWindowToWorkspace(Window* window, Workspace* workspace) {
	Workspace* previousWorkspace = window->getWorkspace();
	Monitor* previousMonitor = window->getMonitor();
	Monitor* monitor = workspace->getMonitor();

	if (previousMonitor == monitor && previousWorkspace == workspace) {
		return;
	}

	if (previousMonitor == monitor) {
		window->hide();
	} else {
		if (workspace->getMonitor()->getActiveWorkspace() != workspace) {
			window->hide();
		}

		Rect windowRect = window->getRect();
		Rect previousRect = previousMonitor->getRect();
		Rect nextRect = monitor->getRect();
		window->move(
			nextRect.x + (windowRect.x - previousRect.x),
			nextRect.y + (windowRect.y - previousRect.y)
		);

		_clipWindow(window);
	}

	window->getWorkspace()->removeWindow(window);
	workspace->addWindow(window);
}

void Session::sendWindowToWorkspaceID(wx::String const& id) {
	Window* window = _activeMonitor->getActiveWorkspace()->getActiveWindow();
	if (window == nullptr) {
		return;
	}
	for (int i = 0; i < _monitors.count(); i++) {
		Monitor* mon = _monitors[i];
		Workspace* ws = mon->queryWorkspace(id);
		if (ws == nullptr) {
			continue;
		}
		sendWindowToWorkspace(window, ws);
		return;
	}
	LOG("no workspace with id %s\n", id.cptr());
}

void Session::focusWindow(Window* window) {
	// raise to top (if background wallpaper or desktop orginizer you dont have to move it forward)
	window->raise();
	
	Workspace* ws = window->getWorkspace();
	focusWorkspace(ws);
	ws->focusWindow(window);
	_observer->onWindowFocus(window);
}

void Session::focusNearestWindow(Direction direction) {
	Workspace* workspace = _activeMonitor->getActiveWorkspace();
	Window* sourceWindow = workspace->getActiveWindow();
	if (sourceWindow == nullptr) return;

	Rect sourceRect = sourceWindow->getRect();

	int shortestDistance = INT_MAX;
	Window* shortestWindow = nullptr;

	if (direction == Direction::East) {
		for (int i = 0; i < workspace->getWindowList().count(); i++) {
			Window* targetWindow = workspace->getWindowList()[i];
			if (targetWindow == sourceWindow) continue;

			Rect targetRect = targetWindow->getRect();
			int distance = (targetRect.x) - (sourceRect.x + sourceRect.width);
			if (distance > 0 && shortestDistance > distance) {
				shortestDistance = distance;
				shortestWindow = targetWindow;
			}
		}
	} else if (direction == Direction::West) {
		for (int i = 0; i < workspace->getWindowList().count(); i++) {
			Window* targetWindow = workspace->getWindowList()[i];
			if (targetWindow == sourceWindow) continue;

			Rect targetRect = targetWindow->getRect();
			int distance = (sourceRect.x) - (targetRect.x + targetRect.width) ;
			if (distance > 0 && shortestDistance > distance) {
				shortestDistance = distance;
				shortestWindow = targetWindow;
			}
		}
	}

	if (shortestWindow) {
		focusWindow(shortestWindow);
	}
}

void Session::cycleWindow() {
	Workspace* workspace = _activeMonitor->getActiveWorkspace();
	// TODO:
	// on the first key-press create index where the cycle began
	// pick the modifier like alt or super
	// listen to that key-release to end and accept new active window
	if (!_cyclingWindow) {
		LOG("BEGIN CYCLE\n");
		_cyclingWindow = true;
	}

	LOG("NEXT WINDOW\n");
}

void Session::closeWindow() {
	Workspace* ws = _activeMonitor->getActiveWorkspace();
	Window* window = ws->getActiveWindow();
	if (window) {
		window->close();
	}
}

void Session::_beginDrag(DragMode mode, Window* window, WindowResizingArea area) {
	_dragState.mode = mode;
	_dragState.window = window;


	Rect windowRect = _dragState.window->getRect();
	Point mousePos = _context->getMousePos();

	if (area == WindowResizingArea::None) {
		int xSlice = (float(mousePos.x - windowRect.x) / float(windowRect.width)) * 3;
		int ySlice = (float(mousePos.y - windowRect.y) / float(windowRect.height)) * 3;

		if (xSlice == 0) {
			if (ySlice == 0) {
				area = WindowResizingArea::NorthWest;
			} else if (ySlice == 1) {
				area = WindowResizingArea::West;
			} else if (ySlice == 2) {
				area = WindowResizingArea::SouthWest;
			}
		} else if (xSlice == 1) {
			if (ySlice == 0) {
				area = WindowResizingArea::North;
			} else if (ySlice == 2) {
				area = WindowResizingArea::South;
			}
		} else if (xSlice == 2) {
			if (ySlice == 0) {
				area = WindowResizingArea::NorthEast;
			} else if (ySlice == 1) {
				area = WindowResizingArea::East;
			} else if (ySlice == 2) {
				area = WindowResizingArea::SouthEast;
			}
		}
	}

	_dragState.initialWindowRect = windowRect;
	_dragState.initialMousePos = mousePos;

	_dragState.area = area;

	_context->unclipWindow(_dragState.window);
	_context->bindMouseMovement();
}

void Session::_doCommand(ConfigCommand const& command) {
	if (command.func == ConfigCommandFunc::Exec) {
		wx::List<wx::String> argv;
		for (int i = 0; i < command.arguments.asList().count(); i++) {
			wx::WkValue const& arg = command.arguments.asList()[i];
			argv.append(arg.asString());
		}
		exec(argv);
		return;
	}
	if (command.func == ConfigCommandFunc::FocusWindow) {
		focusNearestWindow((Direction) command.arguments.asInt64());
		return;
	}
	if (command.func == ConfigCommandFunc::CloseWindow) {
		closeWindow();
		return;
	}
	if (command.func == ConfigCommandFunc::TileWindow) {
		Window* window = _activeMonitor->getActiveWorkspace()->getActiveWindow();
		if (window == nullptr) return;
		Rect avail = _activeMonitor->getWorkingRect();
		wx::List<wx::WkValue> const& args = command.arguments.asList();
		wx::Float64 x = args[0].asFloat64();
		wx::Float64 y = args[1].asFloat64();
		wx::Float64 width = args[2].asFloat64();
		wx::Float64 height = args[3].asFloat64();
		Rect target;
		target.x = avail.x + (avail.width * x);
		target.y = avail.y + (avail.height * y);
		target.width = (avail.width * width);
		target.height = (avail.height * height);
		LOG("[session] avail: (%d, %d, %d, %d)\n", avail.x, avail.y, avail.width, avail.height);
		LOG("[session] multiplier: (%g, %g, %g, %g)\n", x, y, width, height);
		LOG("[session] computed: (%d, %d, %d, %d)\n", target.x, target.y, target.width, target.height);
		window->move(target.x, target.y);
		window->resize(target.width, target.height);
		_clipWindow(window);
		return;
	}
	if (command.func == ConfigCommandFunc::FocusWorkspace) {
		focusWorkspaceID(command.arguments.asString());
		return;
	}
	if (command.func == ConfigCommandFunc::SendWindowToWorkspace) {
		sendWindowToWorkspaceID(command.arguments.asString());
		return;
	}
	if (command.func == ConfigCommandFunc::Quit) {
		_context->stop();
		return;
	}
}

Monitor* Session::_queryMonitorByPort(wx::String const& port) {
	for (int i = 0; i < _monitors.count(); i++) {
		if (_monitors[i]->getPort() == port) {
			return _monitors[i];
		}
	}
	return nullptr;
}

Monitor* Session::_queryMonitorByPos(Point pos) {
	for (int i = 0; i < _monitors.count(); i++) {
		if (isPointIntersectRect(pos, _monitors[i]->getRect())) {
			return _monitors[i];
		}
	}
	return nullptr;
}

void Session::_relayoutMonitors() {
	wx::List<MonitorInfo> monitorInfos = _context->getConnectedMonitorInfos();

	MonitorInfo fallbackInfo;
	fallbackInfo.port = "<fallback>";
	_fallbackMonitor = _getOrCreateMonitor(fallbackInfo);
	_fallbackMonitor->clearWorkspaceList();

	// TODO: reuse the monitor instance
	_monitors.clear();

	if (monitorInfos.isEmpty()) {
		_activeMonitor = _fallbackMonitor;
		_monitors.append(_activeMonitor);
	} else {
		Monitor* activeMonitor = nullptr;
		for (int i = 0; i < monitorInfos.count(); i++) {
			MonitorInfo const& info = monitorInfos[i];
			Monitor* monitor = _getOrCreateMonitor(info);
			monitor->clearWorkspaceList();
			if (_activeMonitor == monitor) {
				activeMonitor = monitor;
			}
			_monitors.append(monitor);
		}
		if (activeMonitor == nullptr) {
			_activeMonitor = _monitors.getFirst();
		}
	}

	for (int i = 0; i < _config.workspaces.count(); i++) {
		ConfigWorkspace const& wsConfig = _config.workspaces[i];
		Workspace* ws = _getOrCreateWorkspace(wsConfig.id);
		if (wsConfig.ports.isEmpty()) {
			sendWorkspaceToMonitor(ws, _activeMonitor);
		} else {
			Monitor* monitor = nullptr;
			for (int j = 0; j < wsConfig.ports.count(); j++) {
				monitor = _queryMonitorByPort(wsConfig.ports[j]);
				if (monitor) {
					sendWorkspaceToMonitor(ws, monitor);
					break;
				}
			}
			if (monitor == nullptr) {
				sendWorkspaceToMonitor(ws, _activeMonitor);
			}
		}
	}

	for (int i = 0; i < _monitors.count(); i++) {
		Monitor* monitor = _monitors[i];
		if (monitor->getActiveWorkspace() == nullptr) {
			monitor->addWorkspace(new Workspace(monitor->getPort()));
		}
	}

	// collect living workspace list
	wx::List<Workspace*> workspaces;

	for (int i = 0; i < _monitors.count(); i++) {
		Monitor* monitor = _monitors[i];
		for (int j = 0; j < monitor->getWorkspaceList().count(); j++) {
			Workspace* workspace = monitor->getWorkspaceList()[j];
			workspaces.append(workspace);
			for (int k = 0; k < workspace->getWindowList().count(); k++) {
				Window* window = workspace->getWindowList()[k];
				// reclip window
				_clipWindow(window);
			}
		}
	}

	_observer->onWorkspaceConfigure(workspaces);
	_logLayout();
}

void Session::_logLayout() {
	LOG("layout:\n");
	for (int i = 0; i < _monitors.count(); i++) {
		Monitor* mon = _monitors[i];
		LOG("  mon %s:", mon->getPort().cptr());
		if (mon == _activeMonitor) {
			LOG(" # active");
		}
		LOG("\n");
		for (int j = 0; j < mon->getWorkspaceList().count(); j++) {
			Workspace* ws = mon->getWorkspaceList()[j];
			LOG("    ws %s: (mon: %s)", ws->getID().cptr(), ws->getMonitor()->getPort().cptr());
			if (ws == mon->getActiveWorkspace()) {
				LOG(" # active");
			}
			LOG("\n");

			for (int k = 0; k < ws->getWindowList().count(); k++) {
				Window* win = ws->getWindowList()[k];
				LOG("      win %p: (ws: %s, mon: %s)", win, win->getWorkspace()->getID().cptr(), win->getMonitor()->getPort().cptr());
				if (win == ws->getActiveWindow()) {
					LOG(" # active");
				}
				LOG("\n");
			}
		}
	}
}

Monitor* Session::_getOrCreateMonitor(MonitorInfo const& info) {
	for (int i = 0; i < _monitorDB.count(); i++) {
		Monitor* monitor = _monitorDB[i];
		if (monitor->getPort() == info.port) {
			monitor->setInfo(info);
			return monitor;
		}
	}
	Monitor* monitor = new Monitor(_context, info);
	_monitorDB.append(monitor);
	return monitor;
}

Workspace* Session::_getOrCreateWorkspace(wx::String const& id) {
	for (int i = 0; i < _workspaceDB.count(); i++) {
		Workspace* workspace = _workspaceDB[i];
		if (workspace->getID() == id) {
			return workspace;
		}
	}
	Workspace* workspace = new Workspace(id);
	_workspaceDB.append(workspace);
	return workspace;
}

void Session::_clipWindow(Window* window) {
	Monitor* monitor = window->getMonitor();
	Rect windowRect = window->getRect();
	WindowProp windowProp = window->getProp();
	Rect windowBorderRect = windowRect;
	windowBorderRect.x -= windowProp.borderWidth;
	windowBorderRect.y -= windowProp.borderWidth;
	windowBorderRect.width += 2 * windowProp.borderWidth;
	windowBorderRect.height += 2 * windowProp.borderWidth;
	Rect workingRect = monitor->getWorkingRect();
	Rect intersectRect = intersectRectRect(workingRect, windowBorderRect);

	if (intersectRect.width == 0 || intersectRect.height == 0) {
		LOG("invalid intersection\n");
		return;
	}

	Rect clipRect;
	clipRect.x = intersectRect.x - windowRect.x;
	clipRect.y = intersectRect.y - windowRect.y;
	clipRect.width = intersectRect.width;
	clipRect.height = intersectRect.height;

	_context->clipWindow(window, clipRect);
}
