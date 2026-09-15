#include "Event.hpp"
#include "WindowStyle.hpp"
#include "core/Logger.hpp"
#include "core/Collider.hpp"
#include "core/Direction.hpp"
#include "backend/IWindowBackend.hpp"
#include "config/Config.hpp"
#include "input/Key.hpp"
#include "os/Exec.hpp"
#include "Session.hpp"
#include <climits>
#include <cstdlib>

bool operator==(KeyIndex a, KeyIndex b) {
	return a.modKey == b.modKey && a.key == b.key;
}

bool operator==(MouseIndex a, MouseIndex b) {
	return a.modKey == b.modKey;
}

wx::Uint32 getHash(KeyIndex index) {
	return wx::combineHash(wx::getHash((int)index.modKey), wx::getHash((int)index.key));
}

wx::Uint32 getHash(MouseIndex index) {
	return wx::getHash((int) index.modKey);
}

class Monitor;

#ifdef XEPHYR_ENABLE
extern int xephyrMonitorConfiguration;
#endif


void Session::run(Config const& config) {
	_config = config;

	_context = createContext();
	_observer = _context->getSessionObserver();

	for (int i = 0; i < _config.mouseBindings.count(); i++) {
		MouseBinding const& binding = _config.mouseBindings[i];
		_context->bindMouseButton(binding.modKey, MouseButton::Left);
		MouseIndex index;
		index.modKey = binding.modKey;
		_mouseBindings.set(index, [this, binding](Window* window) {
			_doMouseCommand(binding.func, window);
		});
	}

	_context->bindKey(ModKey::Alt, Key::A);
	_context->bindKey(ModKey::Alt, Key::M);

	for (int i = 0; i < _config.keyBindings.count(); i++) {
		KeyBinding const& binding = _config.keyBindings[i];

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

	_updateLayout();

	for (int i = 0; i < _config.autostarts.count(); i++) {
		_doCommand(_config.autostarts[i]);
	}

	_context->run(this);

	// NOTE: on why not using wx::Ref
	//       since the lifetime is predictable and contain circular references
	//       i pick raw pointer over reference-counted solution

	// clean-up
	for (int i = 0; i < _workspaceDB.count(); i++) {
		for (int k = 0; k < _workspaceDB[i]->getWindowList().count(); k++) {
			delete _workspaceDB[i]->getWindowList()[k];
		}
		delete _workspaceDB[i];
	}
	for (int i = 0; i < _monitorDB.count(); i++) {
		delete _monitorDB[i];
	}
}

KeyBinding const* Session::queryBinding(ModKey modKey, Key key) {
	for (int i = 0; i < _config.keyBindings.count(); i++) {
		KeyBinding const& binding = _config.keyBindings[i];
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

	WindowType type = e.window->getType();
	if (type == WindowType::Normal || type == WindowType::Dialog) {
		_observer->onWindowFocus(e.window);
	}
	if (type == WindowType::Dock) {
		_observer->onMonitorConfigure(_monitors);
	}

	LOG("ACTIEV MON %p\n", _activeMonitor);
	LOG("WINDOW CREATE %p, WS: %p, MON: %p\n", e.window, e.window->getWorkspace(), e.window->getMonitor());
	_clipWindow(e.window);
}

void Session::onWindowDestroy(WindowEvent const& e) {
	Monitor* monitor = e.window->getMonitor();
	monitor->removeWindow(e.window);

	Window* window = monitor->getActiveWorkspace()->getActiveWindow();
	if (window) {
		_observer->onWindowFocus(window);
	}
}

void Session::onWindowMoveRequest(WindowMovementEvent const& e) {
	_beginDrag(DragMode::Move, e.window);
}

void Session::onWindowResizeRequest(WindowResizingEvent const& e) {
	_beginDrag(DragMode::Resize, e.window, e.area);
}

void Session::onWindowMaximizeRequest(WindowActionEvent const& e) {
	Workspace* workspace = e.window->getWorkspace();
	if (workspace == nullptr) {
		return;
	}
	WindowAction action = e.action;

	if (action == WindowAction::Enable) {
		Rect target = _activeMonitor->getWorkingRect();

		if (!_windowRestoreRects.has((uintptr_t)e.window)) {
			_windowRestoreRects.set((uintptr_t)e.window, e.window->getRect());
		}

		e.window->move(target.x, target.y);
		e.window->resize(target.width, target.height);
		e.window->setState(e.window->getState() | WindowState::Maximized);
		_clipWindow(e.window);
	}
	if (action == WindowAction::Disable) {
		Rect* target = _windowRestoreRects.getOrNull((uintptr_t)e.window);
		if (target) {
			e.window->move(target->x, target->y);
			e.window->resize(target->width, target->height);
			_windowRestoreRects.remove((uintptr_t)e.window);
		}
		e.window->setState(e.window->getState() & ~WindowState::Maximized);
		_clipWindow(e.window);
	}
}

void Session::onWindowFullscreenRequest(WindowActionEvent const& e) {
	Workspace* workspace = e.window->getWorkspace();
	if (workspace == nullptr) {
		return;
	}
	WindowAction action = e.action;

	if (action == WindowAction::Enable) {
		if (workspace->isFullscreen()) return;
		workspace->setFullscreen(true);

		Rect target = _activeMonitor->getRect();

		if (!_windowRestoreRects.has((uintptr_t)e.window)) {
			_windowRestoreRects.set((uintptr_t)e.window, e.window->getRect());
		}

		_activeMonitor->hideDocks();

		WindowProp prop;
		prop.borderWidth = 0;
		prop.titlebarHeight = 0;
		prop.borderColor = 0x000000FF;
		prop.titlebarColor = 0x000000FF;

		e.window->setProp(prop);
		e.window->move(target.x, target.y);
		e.window->resize(target.width, target.height);
		e.window->setState(e.window->getState() | WindowState::Fullscreen);
		_clipWindow(e.window);
	}
	if (action == WindowAction::Disable) {
		workspace->setFullscreen(false);
		_activeMonitor->showDocks();

		WindowProp prop;
		prop.borderWidth = WindowStyle::borderWidth;
		prop.titlebarHeight = WindowStyle::titlebarHeight;
		prop.borderColor = WindowStyle::Active::borderColor;
		prop.titlebarColor = WindowStyle::Active::titlebarColor;
		e.window->setProp(prop);

		Rect* target = _windowRestoreRects.getOrNull((uintptr_t)e.window);
		if (target) {
			e.window->move(target->x, target->y);
			e.window->resize(target->width, target->height);
			_windowRestoreRects.remove((uintptr_t)e.window);
		}
		e.window->setState(e.window->getState() & ~WindowState::Fullscreen);
		_clipWindow(e.window);
	}
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

	focusWindow(e.window, false);

	if (e.button == MouseButton::Left) {
		MouseIndex index;
		index.modKey = e.modKey;
		MouseCallback* callback = _mouseBindings.getOrNull(index);

		if (callback != nullptr) {
			(*callback)((Window*)e.window);
		} else {
			// titlebar grabbing
			Rect rect = e.window->getRect();
			WindowProp prop = e.window->getProp();
			if (e.pos.y < rect.y + prop.titlebarHeight) {
				int x = e.pos.x - rect.x;
				int y = e.pos.y - rect.y;
				int size = WindowStyle::buttonSize;
				int gap = WindowStyle::buttonGap;
				int pad = WindowStyle::buttonPadding;
				int x1 = pad;
				int x2 = pad + size + gap;
				int x3 = pad + size + gap + size + gap;

				if (x >= x1 && y >= pad && x <= x1 + size && y <= pad + size) {
					printf("CLOSE WINDOW\n");
					focusWindow(e.window, false);
					closeWindow();
				} else if (x >= x2 && y >= pad && x <= x2 + size && y <= pad + size) {
					printf("MINIMIZE WINDOW\n");
				} else if (x >= x3 && y >= pad && x <= x3 + size && y <= pad + size) {
					printf("MAXIMIZE WINDOW\n");
					focusWindow(e.window, false);
					WindowActionEvent se;
					se.action = WindowAction::Enable;
					se.window = e.window;
					onWindowMaximizeRequest(se);
				} else {
					_beginDrag(DragMode::Move, e.window);
				}
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
			window->getWorkspace()->removeWindow(window);
			bestMonitor->getActiveWorkspace()->addWindow(window);
			focusWorkspace(bestMonitor->getActiveWorkspace(), false);
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
		_updateLayout();
		#endif
		return;
	}

}

void Session::onKeyRelease(KeyEvent const& e) {
}

void Session::onMonitorConnect(MonitorEvent const& e) {
	LOG("monitor connected %s\n", e.port.cptr());
	_updateLayout();
}

void Session::onMonitorDisconnect(MonitorEvent const& e) {
	LOG("monitor disconnected %s\n", e.port.cptr());
	_updateLayout();
}

void Session::onMonitorPropertyChange(MonitorEvent const& e) {
	LOG("monitor property changed %s\n", e.port.cptr());
	_updateLayout();
}

void Session::focusMonitor(Monitor* monitor) {
	if (_activeMonitor == monitor) return;
	if (_activeMonitor != nullptr) {
		_activeMonitor->blur();
	}
	_activeMonitor = monitor;
	_activeMonitor->focus();
	_observer->onWorkspaceFocus(_activeMonitor->getActiveWorkspace());
	Window* window = _activeMonitor->getActiveWorkspace()->getActiveWindow();
	if (window) {
		_observer->onWindowFocus(window);
	}
}

void Session::focusWorkspaceID(wx::String const& id) {
	Monitor* previousMonitor = _activeMonitor;
	for (int i = 0; i < _monitors.count(); i++) {
		Monitor* mon = _monitors[i];
		Workspace* ws = mon->queryWorkspace(id);
		if (ws == nullptr) {
			continue;
		}
		focusWorkspace(ws, true);
		return;
	}
	LOG("no workspace with id %s\n", id.cptr());
}

void Session::focusWorkspace(Workspace* workspace, bool moveCursor) {
	Monitor* mon = workspace->getMonitor();
	focusMonitor(mon);
	mon->focusWorkspace(workspace);
	_observer->onWorkspaceFocus(workspace);

	// if workspace is empty move the cursor in the middle of monitor
	if (workspace->getWindowList().isEmpty()) {
		// TODO: make it configurable 
		if (moveCursor) {
			Rect rect = mon->getWorkingRect();
			Point point;
			point.x = rect.x + (rect.width / 2);
			point.y = rect.y + (rect.height / 2);
			_context->setMousePos(point);
		}
	} else {
		if (moveCursor) {
			Rect rect = workspace->getActiveWindow()->getRect();
			Point point;
			point.x = rect.x + (rect.width / 2);
			point.y = rect.y + (rect.height / 2);
			_context->setMousePos(point);
		}
	}
	Window* window = _activeMonitor->getActiveWorkspace()->getActiveWindow();
	if (window) {
		_observer->onWindowFocus(window);
	}
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

	previousWorkspace->removeWindow(window);

	workspace->addWindow(window);
	workspace->focusWindow(window);
	workspace->blur();

	previousWorkspace->focus();
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

void Session::focusWindow(Window* window, bool moveCursor) {
	Workspace* ws = window->getWorkspace();
	focusWorkspace(ws, false);
	ws->focusWindow(window);
	_observer->onWindowFocus(window);

	// TODO: make it configurable 
	if (moveCursor) {
		Rect rect = window->getRect();
		Point point;
		point.x = rect.x + (rect.width / 2);
		point.y = rect.y + (rect.height / 2);
		_context->setMousePos(point);
	}
}

static int calculateCrossSurface(int a1, int a2, int b1, int b2) {
	const int start = Helper::max(a1, b1);
    const int end   = Helper::min(a2, b2);
    return Helper::max(0, end - start);
}

void Session::focusNearestWindow(Direction direction) {
	LOG("FOCUS NEAREST WINDOW %d\n", (int) direction);
	Window* sourceWindow = _activeMonitor->getActiveWorkspace()->getActiveWindow();
	if (sourceWindow == nullptr) return;

	Window* nearestWindow = findNearestWindow(sourceWindow, sourceWindow->getWorkspace()->getWindowList(), direction, true);
	if (nearestWindow != nullptr) {
		focusWindow(nearestWindow, true);
		return;
	}
	Window* nearestNonParalellWindow = findNearestWindow(sourceWindow, sourceWindow->getWorkspace()->getWindowList(), direction, false);
	if (nearestNonParalellWindow != nullptr) {
		focusWindow(nearestNonParalellWindow, true);
		return;
	}

	Monitor* nearestMonitor = findNearestMonitor(_activeMonitor, direction, true);
	if (nearestMonitor != nullptr) {
		Window* nearestWindow = findNearestWindow(sourceWindow, nearestMonitor->getActiveWorkspace()->getWindowList(), direction, true);
		if (nearestWindow != nullptr) {
			focusWindow(nearestWindow, true);
			return;
		}
		Window* nearestNonParalellWindow = findNearestWindow(sourceWindow, nearestMonitor->getActiveWorkspace()->getWindowList(), direction, false);
		if (nearestNonParalellWindow != nullptr) {
			focusWindow(nearestNonParalellWindow, true);
			return;
		}
	}

	Monitor* nearestNonParalellMonitor = findNearestMonitor(_activeMonitor, direction, true);
	if (nearestNonParalellMonitor != nullptr) {
		Window* nearestWindow = findNearestWindow(sourceWindow, nearestNonParalellMonitor->getActiveWorkspace()->getWindowList(), direction, true);
		if (nearestWindow != nullptr) {
			focusWindow(nearestWindow, true);
			return;
		}
		Window* nearestNonParalellWindow = findNearestWindow(sourceWindow, nearestNonParalellMonitor->getActiveWorkspace()->getWindowList(), direction, false);
		if (nearestNonParalellWindow != nullptr) {
			focusWindow(nearestNonParalellWindow, true);
			return;
		}
	}
}

Window* Session::findNearestWindow(Window* sourceWindow, wx::List<Window*> const& targetWindows, Direction direction, bool onlyParallelWindow) {
	if (sourceWindow == nullptr) return nullptr;

	Rect source = sourceWindow->getRect();

	int shortestDistance = INT_MAX;
	Window* shortestWindow = nullptr;

	int largestSurface = 0;
	int errorLimit = 20;

	if (direction == Direction::East) {
		for (int i = 0; i < targetWindows.count(); i++) {
			Window* targetWindow = targetWindows[i];
			if (targetWindow == sourceWindow) continue;
			if (!targetWindow->isShown()) continue;

			Rect target = targetWindow->getRect();
			int distance = 0;
			int surface = calculateCrossSurface(source.y, source.y + source.height, target.y, target.y + target.height);
			if (onlyParallelWindow && surface == 0) continue;

			// partial overlap but target is visible
			// +---+
			// | +----+
			// | |    |
			// | +----+
			// +---+
			bool partialOverlap = isInsideRange(source.x + source.width, target.x, target.x + target.width);

			// non overlap
			// +---+
			// |   | +----+
			// |   | |    |
			// |   | +----+
			// +---+
			bool nonOverlap = target.x >= source.x + source.width;

			if (partialOverlap || nonOverlap) {
				distance = (target.x) - (source.x);
			}

			if (distance > 0) {
				if (abs(shortestDistance - distance) <= errorLimit) {
					if (largestSurface <= surface) {
						largestSurface = surface;
						shortestDistance = distance;
						shortestWindow = targetWindow;
					}
				} else if (shortestDistance > distance) {
					largestSurface = surface;
					shortestDistance = distance;
					shortestWindow = targetWindow;
				}
			}
		}
	} else if (direction == Direction::West) {
		for (int i = 0; i < targetWindows.count(); i++) {
			Window* targetWindow = targetWindows[i];
			if (targetWindow == sourceWindow) continue;

			Rect target = targetWindow->getRect();
			int distance = 0;
			int surface = calculateCrossSurface(source.y, source.y + source.height, target.y, target.y + target.height);
			if (onlyParallelWindow && surface == 0) continue;

			// partial overlap but target is visible
			//    +---+
			// +----+ |
			// |    | |
			// +----+ |
			//    +---+
			bool partialOverLap = isInsideRange(source.x, target.x, target.x + target.width);

			// non overlap
			//        +---+
			// +----+ |   | 
			// |    | |   | 
			// +----+ |   | 
			//        +---+
			bool nonOverlap = target.x + target.width <= source.x;

			if (partialOverLap || nonOverlap) {
				distance = (source.x + source.width) - (target.x + target.width);
			}

			if (distance > 0) {
				if (abs(shortestDistance - distance) <= errorLimit) {
					if (largestSurface <= surface) {
						largestSurface = surface;
						shortestDistance = distance;
						shortestWindow = targetWindow;
					}
				} else if (shortestDistance > distance) {
					largestSurface = surface;
					shortestDistance = distance;
					shortestWindow = targetWindow;
				}
			}
		}
	} else if (direction == Direction::South) {
		for (int i = 0; i < targetWindows.count(); i++) {
			Window* targetWindow = targetWindows[i];
			if (targetWindow == sourceWindow) continue;

			Rect target = targetWindow->getRect();
			int distance = 0;
			int surface = calculateCrossSurface(source.x, source.x + source.width, target.x, target.x + target.width);
			if (onlyParallelWindow && surface == 0) continue;

			// partial overlap but target is visible
			// +--------+
			// | +----+ |
			// +-|    |-+
			//   +----+
			bool partialOverLap = isInsideRange(source.y + source.height, target.y, target.y + target.height);

			// non overlap
			// +---------+
			// |         |
			// +---------+
			//   +----+
			//   |    |
			//   +----+
			bool nonOverlap = target.y >= source.y + source.height;

			if (partialOverLap || nonOverlap) {
				distance = (target.y) - (source.y);
			}

			if (distance > 0) {
				if (abs(shortestDistance - distance) <= errorLimit) {
					if (largestSurface <= surface) {
						largestSurface = surface;
						shortestDistance = distance;
						shortestWindow = targetWindow;
					}
				} else if (shortestDistance > distance) {
					largestSurface = surface;
					shortestDistance = distance;
					shortestWindow = targetWindow;
				}
			}
		}
	} else if (direction == Direction::North) {
		for (int i = 0; i < targetWindows.count(); i++) {
			Window* targetWindow = targetWindows[i];
			if (targetWindow == sourceWindow) continue;

			Rect target = targetWindow->getRect();
			int distance = 0;
			int surface = calculateCrossSurface(source.x, source.x + source.width, target.x, target.x + target.width);
			if (onlyParallelWindow && surface == 0) continue;

			// partial overlap but target is visible
			//   +----+  
			// +-|    |-+
			// | +----+ |
			// +--------+
			bool partialOverLap = isInsideRange(source.y, target.y, target.y + target.height);

			// non overlap
			//   +----+
			//   |    |
			//   +----+
			// +---------+
			// |         |
			// +---------+
			bool nonOverlap = target.y + target.height <= source.y;

			if (partialOverLap || nonOverlap) {
				distance = (source.y + source.height) - (target.y + target.height);
			}

			if (distance > 0) {
				if (abs(shortestDistance - distance) <= errorLimit) {
					if (largestSurface <= surface) {
						largestSurface = surface;
						shortestDistance = distance;
						shortestWindow = targetWindow;
					}
				} else if (shortestDistance > distance) {
					largestSurface = surface;
					shortestDistance = distance;
					shortestWindow = targetWindow;
				}
			}
		}
	}

	return shortestWindow;
}

Monitor* Session::findNearestMonitor(Monitor* sourceMonitor, Direction direction, bool onlyParallel) {
	if (sourceMonitor == nullptr) return nullptr;

	Rect source = sourceMonitor->getRect();

	int shortestDistance = INT_MAX;
	Monitor* shortestMonitor = nullptr;

	int largestSurface = 0;
	int errorLimit = 20;

	if (direction == Direction::East) {
		for (int i = 0; i < _monitors.count(); i++) {
			Monitor* targetMonitor = _monitors[i];
			if (targetMonitor == sourceMonitor) continue;
			Rect target = targetMonitor->getRect();
			int distance = 0;
			int surface = calculateCrossSurface(source.y, source.y + source.height, target.y, target.y + target.height);
			if (onlyParallel && surface == 0) continue;

			// partial overlap but target is visible
			// +---+
			// | +----+
			// | |    |
			// | +----+
			// +---+
			bool partialOverlap = isInsideRange(source.x + source.width, target.x, target.x + target.width);

			// non overlap
			// +---+
			// |   | +----+
			// |   | |    |
			// |   | +----+
			// +---+
			bool nonOverlap = target.x >= source.x + source.width;

			if (partialOverlap || nonOverlap) {
				distance = (target.x) - (source.x);
			}

			if (distance > 0) {
				if (abs(shortestDistance - distance) <= errorLimit) {
					if (largestSurface <= surface) {
						largestSurface = surface;
						shortestDistance = distance;
						shortestMonitor = targetMonitor;
					}
				} else if (shortestDistance > distance) {
					largestSurface = surface;
					shortestDistance = distance;
					shortestMonitor = targetMonitor;
				}
			}
		}
	} else if (direction == Direction::West) {
		for (int i = 0; i < _monitors.count(); i++) {
			Monitor* targetMonitor = _monitors[i];
			if (targetMonitor == sourceMonitor) continue;

			Rect target = targetMonitor->getRect();
			int distance = 0;
			int surface = calculateCrossSurface(source.y, source.y + source.height, target.y, target.y + target.height);
			if (onlyParallel && surface == 0) continue;

			// partial overlap but target is visible
			//    +---+
			// +----+ |
			// |    | |
			// +----+ |
			//    +---+
			bool partialOverLap = isInsideRange(source.x, target.x, target.x + target.width);

			// non overlap
			//        +---+
			// +----+ |   | 
			// |    | |   | 
			// +----+ |   | 
			//        +---+
			bool nonOverlap = target.x + target.width <= source.x;

			if (partialOverLap || nonOverlap) {
				distance = (source.x + source.width) - (target.x + target.width);
			}

			if (distance > 0) {
				if (abs(shortestDistance - distance) <= errorLimit) {
					if (largestSurface <= surface) {
						largestSurface = surface;
						shortestDistance = distance;
						shortestMonitor = targetMonitor;
					}
				} else if (shortestDistance > distance) {
					largestSurface = surface;
					shortestDistance = distance;
					shortestMonitor = targetMonitor;
				}
			}
		}
	} else if (direction == Direction::South) {
		for (int i = 0; i < _monitors.count(); i++) {
			Monitor* targetMonitor = _monitors[i];
			if (targetMonitor == sourceMonitor) continue;

			Rect target = targetMonitor->getRect();
			int distance = 0;
			int surface = calculateCrossSurface(source.x, source.x + source.width, target.x, target.x + target.width);
			if (onlyParallel && surface == 0) continue;

			// partial overlap but target is visible
			// +--------+
			// | +----+ |
			// +-|    |-+
			//   +----+
			bool partialOverLap = isInsideRange(source.y + source.height, target.y, target.y + target.height);

			// non overlap
			// +---------+
			// |         |
			// +---------+
			//   +----+
			//   |    |
			//   +----+
			bool nonOverlap = target.y >= source.y + source.height;

			if (partialOverLap || nonOverlap) {
				distance = (target.y) - (source.y);
			}

			if (distance > 0) {
				if (abs(shortestDistance - distance) <= errorLimit) {
					if (largestSurface <= surface) {
						largestSurface = surface;
						shortestDistance = distance;
						shortestMonitor = targetMonitor;
					}
				} else if (shortestDistance > distance) {
					largestSurface = surface;
					shortestDistance = distance;
					shortestMonitor = targetMonitor;
				}
			}
		}
	} else if (direction == Direction::North) {
		for (int i = 0; i < _monitors.count(); i++) {
			Monitor* targetMonitor = _monitors[i];
			if (targetMonitor == sourceMonitor) continue;

			Rect target = targetMonitor->getRect();
			int distance = 0;
			int surface = calculateCrossSurface(source.x, source.x + source.width, target.x, target.x + target.width);
			if (onlyParallel && surface == 0) continue;

			// partial overlap but target is visible
			//   +----+  
			// +-|    |-+
			// | +----+ |
			// +--------+
			bool partialOverLap = isInsideRange(source.y, target.y, target.y + target.height);

			// non overlap
			//   +----+
			//   |    |
			//   +----+
			// +---------+
			// |         |
			// +---------+
			bool nonOverlap = target.y + target.height <= source.y;

			if (partialOverLap || nonOverlap) {
				distance = (source.y + source.height) - (target.y + target.height);
			}

			if (distance > 0) {
				if (abs(shortestDistance - distance) <= errorLimit) {
					if (largestSurface <= surface) {
						largestSurface = surface;
						shortestDistance = distance;
						shortestMonitor = targetMonitor;
					}
				} else if (shortestDistance > distance) {
					largestSurface = surface;
					shortestDistance = distance;
					shortestMonitor = targetMonitor;
				}
			}
		}
	}

	return shortestMonitor;
}
void Session::cycleNextWindow() {
	Workspace* workspace = _activeMonitor->getActiveWorkspace();
	if (workspace->getWindowList().count() < 2) return;

	wx::List<Window*> windows = workspace->getWindowList();
	Window* activeWindow = workspace->getActiveWindow();
	int idx = -1;
	for (int i = 0; i < windows.count(); i++) {
		Window* window = windows[i];
		if (window == activeWindow) {
			idx = i;
			break;
		}
	}
	if (idx == -1) return;
	Window* nextWindow = windows[(idx + 1) % windows.count()];
	focusWindow(nextWindow, true);
}

void Session::cyclePreviousWindow() {
	Workspace* workspace = _activeMonitor->getActiveWorkspace();
	if (workspace->getWindowList().count() < 2) return;

	wx::List<Window*> windows = workspace->getWindowList();
	Window* activeWindow = workspace->getActiveWindow();
	int idx = -1;
	for (int i = 0; i < windows.count(); i++) {
		Window* window = windows[i];
		if (window == activeWindow) {
			idx = i;
			break;
		}
	}
	if (idx == -1) return;
	Window* nextWindow = windows[(idx + windows.count() - 1) % windows.count()];
	focusWindow(nextWindow, true);
}

void Session::closeWindow() {
	Workspace* workspace = _activeMonitor->getActiveWorkspace();
	Window* window = workspace->getActiveWindow();
	if (window) {
		if (workspace->isFullscreen() && WX_FLAGS_CONTAIN(window->getState(), WindowState::Fullscreen)) {
			restoreWindow();
		}
		window->close();
	}
}

void Session::fullscreenWindow() {
	Workspace* workspace = _activeMonitor->getActiveWorkspace();
	Window* window = workspace->getActiveWindow();
	if (window == nullptr) return;
}

void Session::maximizeWindow() {
	Workspace* workspace = _activeMonitor->getActiveWorkspace();
	Window* window = workspace->getActiveWindow();
	if (window == nullptr) return;

}

void Session::restoreWindow() {
}

wx::List<Workspace*> Session::_getWorkspaceList() {
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

	return workspaces;
}

void Session::_beginDrag(DragMode mode, Window* window, WindowResizingArea area) {
	_dragState.mode = mode;
	_dragState.window = window;

	Rect* target = _windowRestoreRects.getOrNull((uintptr_t)window);
	if (target) {
		window->resize(target->width, target->height);
		_windowRestoreRects.remove((uintptr_t)window);
	}

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

void Session::_doCommand(Command const& command) {
	if (command.func == CommandFunc::Exec) {
		wx::List<wx::String> argv;
		for (int i = 0; i < command.arguments.asList().count(); i++) {
			wx::WkValue const& arg = command.arguments.asList()[i];
			argv.append(arg.asString());
		}
		exec(argv);
		return;
	}
	if (command.func == CommandFunc::CloseWindow) {
		closeWindow();
		return;
	}
	if (command.func == CommandFunc::FocusWindow) {
		focusNearestWindow((Direction) command.arguments.asInt64());
		return;
	}
	if (command.func == CommandFunc::CycleNextWindow) {
		cycleNextWindow();
		return;
	}
	if (command.func == CommandFunc::CyclePreviousWindow) {
		cyclePreviousWindow();
		return;
	}
	if (command.func == CommandFunc::TileWindow) {
		Window* window = _activeMonitor->getActiveWorkspace()->getActiveWindow();
		if (window == nullptr) return;

		if (!_windowRestoreRects.has((uintptr_t)window)) {
			_windowRestoreRects.set((uintptr_t)window, window->getRect());
		}

		Rect avail = _activeMonitor->getWorkingRect();
		wx::HashTable<wx::String, wx::WkValue> const& args = command.arguments.asHashTable();
		wx::Float64 x = args.get("x").asFloat64();
		wx::Float64 y = args.get("y").asFloat64();
		wx::Float64 width = args.get("width").asFloat64();
		wx::Float64 height = args.get("height").asFloat64();
		Rect target;
		target.x = avail.x + (avail.width * x);
		target.y = avail.y + (avail.height * y);
		target.width = (avail.width * width);
		target.height = (avail.height * height);
		window->move(target.x, target.y);
		window->resize(target.width, target.height);
		_clipWindow(window);
		return;
	}
	if (command.func == CommandFunc::FocusWorkspace) {
		focusWorkspaceID(command.arguments.asString());
		return;
	}
	if (command.func == CommandFunc::SendWindowToWorkspace) {
		sendWindowToWorkspaceID(command.arguments.asString());
		return;
	}
	if (command.func == CommandFunc::Quit) {
		_context->stop();
		return;
	}
}

void Session::_doMouseCommand(CommandFunc func, Window* window) {
	if (func == CommandFunc::MoveWindow) {
		_beginDrag(DragMode::Move, window);
		return;
	}
	if (func == CommandFunc::ResizeWindow) {
		_beginDrag(DragMode::Resize, window);
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

void Session::_updateLayout() {
	wx::List<MonitorDef> monitorDefs = _context->getConnectedMonitorDefs();

	MonitorDef fallbackDef;
	fallbackDef.port = "<fallback>";
	_fallbackMonitor = _getOrCreateMonitor(fallbackDef);
	_fallbackMonitor->clearWorkspaceList();

	// TODO: reuse the monitor instance
	_monitors.clear();

	if (monitorDefs.isEmpty()) {
		_activeMonitor = _fallbackMonitor;
		_monitors.append(_activeMonitor);
	} else {
		Monitor* activeMonitor = nullptr;
		for (int i = 0; i < monitorDefs.count(); i++) {
			MonitorDef const& def = monitorDefs[i];
			Monitor* monitor = _getOrCreateMonitor(def);
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
			monitor->addWorkspace(_getOrCreateWorkspace(monitor->getPort()));
		}
	}

	for (int i = 0; i < _monitors.count(); i++) {
		Monitor* monitor = _monitors[i];
		for (int j = 0; j < monitor->getWorkspaceList().count(); j++) {
			Workspace* workspace = monitor->getWorkspaceList()[j];
			for (int k = 0; k < workspace->getWindowList().count(); k++) {
				Window* window = workspace->getWindowList()[k];
				// reclip window
				_clipWindow(window);
			}
		}

		monitor->getActiveWorkspace()->focus();
		monitor->getActiveWorkspace()->show();
	}

	_observer->onMonitorConfigure(_monitors);
	_observer->onWorkspaceConfigure(_getWorkspaceList());
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
				LOG("      win %p: (ws: %s, mon: %s) %s", win, win->getWorkspace()->getID().cptr(), win->getMonitor()->getPort().cptr(), win->getTitle().cptr());
				if (win == ws->getActiveWindow()) {
					LOG(" # active");
				}
				LOG("\n");
			}
		}
	}
}

Monitor* Session::_getOrCreateMonitor(MonitorDef const& def) {
	for (int i = 0; i < _monitorDB.count(); i++) {
		Monitor* monitor = _monitorDB[i];
		if (monitor->getPort() == def.port) {
			monitor->setDef(def);
			return monitor;
		}
	}
	Monitor* monitor = new Monitor(_context, def);
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
	Rect workingRect = monitor->getWorkingRect();
	Rect intersectRect = intersectRectRect(workingRect, windowRect);

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
