#pragma once

#include "Collider.hpp"
#include "Direction.hpp"
#include "Event.hpp"
#include "IObserver.hpp"
#include "MonitorInfo.hpp"
#include "Monitor.hpp"
#include "Workspace.hpp"
#include "Window.hpp"
#include "config/Config.hpp"
#include "input/MouseButton.hpp"
#include "input/Key.hpp"
#include "system/Exec.hpp"
#include "backend/IContext.hpp"
#include "backend/ISessionObserver.hpp"
#include "wx/core/Assert.hpp"
#include "wx/core/Hash.hpp"
#include "wx/core/List.hpp"
#include "wx/core/Func.hpp"
#include <cstdio>

enum struct DragMode {
	None,
	Move,
	Resize,
};

struct DragState {
	DragMode mode = DragMode::None;
	Window* window;
	Point initialMousePos;
	Rect initialWindowRect;
};

struct KeyIndex {
	ModKey modKey;
	Key key;
};

using KeyCallback = wx::Func<void()>;

bool operator==(KeyIndex a, KeyIndex b) {
	return a.modKey == b.modKey && a.key == b.key;
}

wx::Uint32 getHash(KeyIndex index) {
	return wx::combineHash(wx::getHash((int)index.modKey), wx::getHash((int)index.key));
}

class Monitor;

#ifdef USE_XEPHYR
extern int xephyrMonitorConfiguration;
#endif


class Session: public IObserver {
public:
	void run(Config const& config) {
		_config = config;

		_context = createContext();
		_observer = _context->getSessionObserver();

		_context->bindMouseButton(ModKey::Alt, MouseButton::Left);
		_context->bindMouseButton(ModKey::Alt|ModKey::Shift, MouseButton::Left);

		_context->bindKey(ModKey::Alt, Key::A);
		_context->bindKey(ModKey::Alt, Key::M);

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

		_context->listen(this);
	}

	ConfigBinding const* queryBinding(ModKey modKey, Key key) {
		for (int i = 0; i < _config.bindings.count(); i++) {
			ConfigBinding const& binding = _config.bindings[i];
			if (binding.modKey == modKey && binding.key == key) {
				return &binding;
			}
		}
		return nullptr;
	}

	virtual void onWindowCreate(WindowEvent const& e) override {
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
	}

	virtual void onWindowDestroy(WindowEvent const& e) override {
		Monitor* monitor = e.window->getMonitor();
		monitor->removeWindow(e.window);
	}

	virtual void onMousePress(MouseButtonEvent const& e) override {
		if (e.window == nullptr) {
			return;
		}

		WindowType type = e.window->getType();
		if (type != WindowType::Normal) {
			return;
		}

		focusWindow(e.window);

		if (e.button == MouseButton::Left) {
			if (e.modKey == (ModKey::Alt)) {
				_dragState.mode = DragMode::Move;
			} else if (e.modKey == (ModKey::Alt|ModKey::Shift)) {
				_dragState.mode = DragMode::Resize;
			} else {
				// titlebar grabbing
				Rect rect = e.window->getRect();
				WindowProp prop = e.window->getProp();
				if (e.pos.y < rect.y + prop.titlebarHeight) {
					_dragState.mode = DragMode::Move;
				}
			}

			if (_dragState.mode != DragMode::None) {
				_dragState.window = e.window;

				_dragState.initialWindowRect = _dragState.window->getRect();
				_dragState.initialMousePos = _context->getMousePos();

				_context->unclipWindow(_dragState.window);
			}
		}

		_context->bindMouseMovement();
	}

	virtual void onMouseRelease(MouseButtonEvent const& e) override {
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
				printf("invalid drag\n");
				return;
			}

			if (_activeMonitor != bestMonitor) {
				focusMonitor(bestMonitor);
				window->getWorkspace()->removeWindow(window);
				bestMonitor->getActiveWorkspace()->addWindow(window);
			}

			_clipWindow(window);
		}
	}

	virtual void onMouseMove(MouseMovementEvent const& e) override {
		if (_dragState.mode == DragMode::Move) {
			int x = _dragState.initialWindowRect.x + (e.pos.x - _dragState.initialMousePos.x);
			int y = _dragState.initialWindowRect.y + (e.pos.y - _dragState.initialMousePos.y);
			_dragState.window->move(x, y);
		}
		if (_dragState.mode == DragMode::Resize) {
			int width = _dragState.initialWindowRect.width + (e.pos.x - _dragState.initialMousePos.x);
			int height = _dragState.initialWindowRect.height + (e.pos.y - _dragState.initialMousePos.y);
			_dragState.window->resize(width, height);
		}
	}

	virtual void onKeyPress(KeyEvent const& e) override {
		KeyIndex index;
		index.modKey = e.modKey;
		index.key = e.key;
		KeyCallback* callback = _keyBindings.getOrNull(index);
		printf("GOT %p\n", callback);
		if (callback != nullptr) {
			(*callback)();
			return;
		}

		if (e.modKey == ModKey::Alt && e.key == Key::A) {
			_logLayout();
			return;
		}
		if (e.modKey == ModKey::Alt && e.key == Key::M) {
			#ifdef USE_XEPHYR
			if (xephyrMonitorConfiguration == 0) {
				xephyrMonitorConfiguration = 1;
				_relayoutMonitors();
			} else if (xephyrMonitorConfiguration == 1) {
				xephyrMonitorConfiguration = 2;
				_relayoutMonitors();
			} else if (xephyrMonitorConfiguration == 2) {
				xephyrMonitorConfiguration = 0;
				_relayoutMonitors();
			}
			#endif
			return;
		}
	}

	virtual void onKeyRelease(KeyEvent const& e) override {
	}

	virtual void onMonitorConnect(MonitorEvent const& e) override {
		printf("monitor connected %s\n", e.port.cptr());
		_relayoutMonitors();
	}

	virtual void onMonitorDisconnect(MonitorEvent const& e) override {
		printf("monitor disconnected %s\n", e.port.cptr());
		_relayoutMonitors();
	}

	virtual void onMonitorPropertyChange(MonitorEvent const& e) override {
		printf("monitor property changed %s\n", e.port.cptr());
		_relayoutMonitors();
	}

	void focusMonitor(Monitor* monitor) {
		if (_activeMonitor == monitor) return;
		if (_activeMonitor != nullptr) {
			_activeMonitor->blur();
		}
		_activeMonitor = monitor;
		_activeMonitor->focus();
	}

	void focusWorkspaceID(wx::String const& id) {
		Monitor* previousMonitor = _activeMonitor;
		for (int i = 0; i < _monitors.count(); i++) {
			Monitor* mon = _monitors[i];
			Workspace* ws = mon->queryWorkspace(id);
			if (ws == nullptr) {
				continue;
			}
			printf("activate workspace %s\n", id.cptr());
			if (previousMonitor == mon) {
				mon->getActiveWorkspace()->hide();
			}
			focusMonitor(mon);
			mon->activateWorkspace(ws);
			_observer->onWorkspaceActivate(ws);
			mon->getActiveWorkspace()->show();
			return;
		}
		printf("no workspace with id %s\n", id.cptr());
	}

	void focusWorkspace(Workspace* workspace) {
		Monitor* previousMonitor = _activeMonitor;
		Monitor* mon = workspace->getMonitor();
		if (previousMonitor == mon) {
			if (previousMonitor->getActiveWorkspace() != workspace) {
				mon->getActiveWorkspace()->hide();
			}
		}
		focusMonitor(mon);
		mon->activateWorkspace(workspace);
		_observer->onWorkspaceActivate(workspace);
		printf("activate workspace %s\n", workspace->getID().cptr());
	}

	void sendWorkspaceToMonitor(Workspace* workspace, Monitor* monitor) {
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

	void sendWindowToWorkspace(Window* window, Workspace* workspace) {
		Workspace* previousWorkspace = window->getWorkspace();
		Monitor* previousMonitor = window->getMonitor();
		Monitor* monitor = workspace->getMonitor();

		if (previousMonitor == monitor && previousWorkspace == workspace) {
			return;
		}

		if (previousMonitor == monitor) {
			window->hide();
		} else {
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

	void sendWindowToWorkspaceID(wx::String const& id) {
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
		printf("no workspace with id %s\n", id.cptr());
	}

	void focusWindow(Window* window) {
		// raise to top (if background wallpaper or desktop orginizer you dont have to move it forward)
		window->raise();
		
		Workspace* ws = window->getWorkspace();
		focusWorkspace(ws);
		ws->focusWindow(window);
	}

	void focusNearestWindow(Direction direction) {
	}

	void cycleWindow() {
		Workspace* workspace = _activeMonitor->getActiveWorkspace();
		// TODO:
		// on the first key-press create index where the cycle began
		// pick the modifier like alt or super
		// listen to that key-release to end and accept new active window
	}

	void closeWindow() {
		Workspace* ws = _activeMonitor->getActiveWorkspace();
		Window* window = ws->getActiveWindow();
		if (window) {
			window->close();
		}
	}

private:

	void _doCommand(ConfigCommand const& command) {
		if (command.func == ConfigCommandFunc::Exec) {
			wx::List<wx::String> argv;
			for (int i = 0; i < command.arguments.asList().count(); i++) {
				wx::WkValue const& arg = command.arguments.asList()[i];
				argv.append(arg.asString());
			}
			exec(argv);
			return;
		}
		if (command.func == ConfigCommandFunc::CloseWindow) {
			closeWindow();
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
	}

	Monitor* _queryMonitorByPort(wx::String const& port) {
		for (int i = 0; i < _monitors.count(); i++) {
			if (_monitors[i]->getPort() == port) {
				return _monitors[i];
			}
		}
		return nullptr;
	}

	Monitor* _queryMonitorByPos(Point pos) {
		for (int i = 0; i < _monitors.count(); i++) {
			if (isPointIntersectRect(pos, _monitors[i]->getRect())) {
				return _monitors[i];
			}
		}
		return nullptr;
	}

	void _relayoutMonitors() {
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

	void _logLayout() {
		printf("layout:\n");
		for (int i = 0; i < _monitors.count(); i++) {
			Monitor* mon = _monitors[i];
			printf("  mon %s:", mon->getPort().cptr());
			if (mon == _activeMonitor) {
				printf(" # active");
			}
			printf("\n");
			for (int j = 0; j < mon->getWorkspaceList().count(); j++) {
				Workspace* ws = mon->getWorkspaceList()[j];
				printf("    ws %s: (mon: %s)", ws->getID().cptr(), ws->getMonitor()->getPort().cptr());
				if (ws == mon->getActiveWorkspace()) {
					printf(" # active");
				}
				printf("\n");

				for (int k = 0; k < ws->getWindowList().count(); k++) {
					Window* win = ws->getWindowList()[k];
					printf("      win %p: (ws: %s, mon: %s)", win, win->getWorkspace()->getID().cptr(), win->getMonitor()->getPort().cptr());
					if (win == ws->getActiveWindow()) {
						printf(" # active");
					}
					printf("\n");
				}
			}
		}
	}

	Monitor* _getOrCreateMonitor(MonitorInfo const& info) {
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

	Workspace* _getOrCreateWorkspace(wx::String const& id) {
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

	void _clipWindow(Window* window) {
		Monitor* monitor = window->getMonitor();
		Rect windowRect = window->getRect();
		Rect workingRect = monitor->getWorkingRect();
		Rect intersectRect = intersectRectRect(workingRect, windowRect);

		if (intersectRect.width == 0 || intersectRect.height == 0) {
			printf("invalid intersection\n");
			return;
		}

		Rect clipRect;
		clipRect.x = intersectRect.x - windowRect.x;
		clipRect.y = intersectRect.y - windowRect.y;
		clipRect.width = intersectRect.width;
		clipRect.height = intersectRect.height;

		_context->clipWindow(window, clipRect);
	}
private:
	IContext* _context;
	ISessionObserver* _observer;

	Monitor* _activeMonitor = nullptr;
	Monitor* _fallbackMonitor = nullptr; // incase of all monitor disconnected all workspace/windows are here
	wx::List<Monitor*> _monitors;
	DragState _dragState;

	Config _config;
	wx::List<Monitor*> _monitorDB;
	wx::List<Workspace*> _workspaceDB;

	wx::HashTable<KeyIndex, KeyCallback> _keyBindings;

};
