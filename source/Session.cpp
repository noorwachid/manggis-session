#include "Common/Logger.hpp"
#include "Common/Collider.hpp"
#include "Common/Direction.hpp"
#include "Session.hpp"
#include "Backend/IWindowBackend.hpp"
#include "Config/Config.hpp"
#include "Input/Key.hpp"
#include "OS/Exec.hpp"
#include <climits>
#include <cstdlib>

bool operator==(KeyIndex a, KeyIndex b) {
	return a.modKey == b.modKey && a.key == b.key;
}

Wx::Uint32 GetHash(KeyIndex index) {
	return Wx::CombineHash(Wx::GetHash((int)index.modKey), Wx::GetHash((int)index.key));
}

class Monitor;

#ifdef XEPHYR_ENABLE
extern int xephyrMonitorConfiguration;
#endif


void Session::Run(Config const& config) {
	_config = config;

	_context = CreateContext();
	_observer = _context->GetSessionObserver();

	_context->BindMouseButton(ModKey::Alt, MouseButton::Left);
	_context->BindMouseButton(ModKey::Alt|ModKey::Shift, MouseButton::Left);

	_context->BindKey(ModKey::Alt, Key::A);
	_context->BindKey(ModKey::Alt, Key::M);

	for (int i = 0; i < _config.bindings.Count(); i++) {
		ConfigBinding const& binding = _config.bindings[i];

		KeyIndex index;
		index.modKey = binding.modKey;
		index.key = binding.key;

		_context->BindKey(binding.modKey, binding.key);
		_keyBindings.Set(index, [this, binding]() {
			for (int k = 0; k < binding.commands.Count(); k++) {
				DoCommand(binding.commands[k]);
			}
		});
	}

	UpdateLayout();

	for (int i = 0; i < _config.autostarts.Count(); i++) {
		DoCommand(_config.autostarts[i]);
	}

	_context->Run(this);
}

ConfigBinding const* Session::QueryBinding(ModKey modKey, Key key) {
	for (int i = 0; i < _config.bindings.Count(); i++) {
		ConfigBinding const& binding = _config.bindings[i];
		if (binding.modKey == modKey && binding.key == key) {
			return &binding;
		}
	}
	return nullptr;
}

void Session::OnWindowCreate(WindowEvent const& e) {
	Point mousePos = _context->GetMousePos();
	Monitor* monitor = QueryMonitorByPos(mousePos);
	if (monitor == nullptr) {
		WX_ASSERT(monitor == nullptr, "invalid monitor state\n");
		return;
	}

	if (_activeMonitor != monitor) {
		FocusMonitor(monitor);
	}
	_activeMonitor->AddWindow(e.window);
	_observer->OnWindowFocus(e.window);

	LOG("ACTIEV MON %p\n", _activeMonitor);
	LOG("WINDOW CREATE %p, WS: %p, MON: %p\n", e.window, e.window->GetWorkspace(), e.window->GetMonitor());
	ClipWindow(e.window);
}

void Session::OnWindowDestroy(WindowEvent const& e) {
	Monitor* monitor = e.window->GetMonitor();
	monitor->RemoveWindow(e.window);
}

void Session::OnWindowMoveRequest(WindowMovementEvent const& e) {
	BeginDrag(DragMode::Move, e.window);
}

void Session::OnWindowResizeRequest(WindowResizingEvent const& e) {
	BeginDrag(DragMode::Resize, e.window, e.area);
}

void Session::OnWindowMaximizeRequest(WindowActionEvent const& e) {
}

void Session::OnWindowFullscreenRequest(WindowActionEvent const& e) {
}

void Session::OnMousePress(MouseButtonEvent const& e) {
	if (e.window == nullptr) {
		Monitor* mon = QueryMonitorByPos(e.pos);
		if (mon != nullptr) {
			FocusMonitor(mon);
		}
		return;
	}

	WindowType type = e.window->GetType();
	if (type != WindowType::Normal) {
		return;
	}

	FocusWindow(e.window, false);

	if (e.button == MouseButton::Left) {
		if (e.modKey == (ModKey::Alt)) {
			BeginDrag(DragMode::Move, e.window);
		} else if (e.modKey == (ModKey::Alt|ModKey::Shift)) {
			BeginDrag(DragMode::Resize, e.window);
		} else {
			// titlebar grabbing
			Rect rect = e.window->GetRect();
			WindowProp prop = e.window->GetProp();
			if (e.pos.y < rect.y + prop.titlebarHeight) {
				BeginDrag(DragMode::Move, e.window);
			}
		}
	}

}

void Session::OnMouseRelease(MouseButtonEvent const& e) {
	if (_dragState.mode != DragMode::None) {
		Window* window = _dragState.window;

		_dragState.mode = DragMode::None;
		_dragState.window = nullptr;
		_context->UnbindMouseMovement();

		Rect windowRect = window->GetRect();
		Monitor* bestMonitor = nullptr;
		int bestArea = 0;

		for (int i = 0; i < _monitors.Count(); i++) {
			Monitor* monitor = _monitors[i];
			Rect monitorRect = monitor->GetRect();
			Rect intersectRect = IntersectRectRect(monitorRect, windowRect);
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
			FocusWorkspace(bestMonitor->GetActiveWorkspace());
			window->GetWorkspace()->RemoveWindow(window);
			bestMonitor->GetActiveWorkspace()->AddWindow(window);
		}

		ClipWindow(window);
	}
}

void Session::OnMouseMove(MouseMovementEvent const& e) {
	if (_dragState.mode == DragMode::Move) {
		int x = _dragState.initialWindowRect.x + (e.pos.x - _dragState.initialMousePos.x);
		int y = _dragState.initialWindowRect.y + (e.pos.y - _dragState.initialMousePos.y);
		_dragState.window->Move(x, y);
	}
	if (_dragState.mode == DragMode::Resize) {
		if (_dragState.area == WindowResizingArea::NorthWest) {
			int x = _dragState.initialWindowRect.x + (e.pos.x - _dragState.initialMousePos.x);
			int y = _dragState.initialWindowRect.y + (e.pos.y - _dragState.initialMousePos.y);
			int width = _dragState.initialWindowRect.width - (e.pos.x - _dragState.initialMousePos.x);
			int height = _dragState.initialWindowRect.height - (e.pos.y - _dragState.initialMousePos.y);
			_dragState.window->Move(x, y);
			_dragState.window->Resize(width, height);
		}
		if (_dragState.area == WindowResizingArea::North) {
			int x = _dragState.initialWindowRect.x;
			int y = _dragState.initialWindowRect.y + (e.pos.y - _dragState.initialMousePos.y);
			int width = _dragState.initialWindowRect.width;
			int height = _dragState.initialWindowRect.height - (e.pos.y - _dragState.initialMousePos.y);
			_dragState.window->Move(x, y);
			_dragState.window->Resize(width, height);
		}
		if (_dragState.area == WindowResizingArea::West) {
			int x = _dragState.initialWindowRect.x + (e.pos.x - _dragState.initialMousePos.x);
			int y = _dragState.initialWindowRect.y;
			int width = _dragState.initialWindowRect.width - (e.pos.x - _dragState.initialMousePos.x);
			int height = _dragState.initialWindowRect.height;
			_dragState.window->Move(x, y);
			_dragState.window->Resize(width, height);
		}


		if (_dragState.area == WindowResizingArea::NorthEast) {
			int x = _dragState.initialWindowRect.x;
			int y = _dragState.initialWindowRect.y + (e.pos.y - _dragState.initialMousePos.y);
			int width = _dragState.initialWindowRect.width + (e.pos.x - _dragState.initialMousePos.x);
			int height = _dragState.initialWindowRect.height - (e.pos.y - _dragState.initialMousePos.y);
			_dragState.window->Move(x, y);
			_dragState.window->Resize(width, height);
		}
		if (_dragState.area == WindowResizingArea::SouthWest) {
			int x = _dragState.initialWindowRect.x + (e.pos.x - _dragState.initialMousePos.x);
			int y = _dragState.initialWindowRect.y;
			int width = _dragState.initialWindowRect.width - (e.pos.x - _dragState.initialMousePos.x);
			int height = _dragState.initialWindowRect.height + (e.pos.y - _dragState.initialMousePos.y);
			_dragState.window->Move(x, y);
			_dragState.window->Resize(width, height);
		}


		if (_dragState.area == WindowResizingArea::East) {
			int width = _dragState.initialWindowRect.width + (e.pos.x - _dragState.initialMousePos.x);
			int height = _dragState.initialWindowRect.height;
			_dragState.window->Resize(width, height);
		}
		if (_dragState.area == WindowResizingArea::South) {
			int width = _dragState.initialWindowRect.width;
			int height = _dragState.initialWindowRect.height + (e.pos.y - _dragState.initialMousePos.y);
			_dragState.window->Resize(width, height);
		}
		if (_dragState.area == WindowResizingArea::SouthEast) {
			int width = _dragState.initialWindowRect.width + (e.pos.x - _dragState.initialMousePos.x);
			int height = _dragState.initialWindowRect.height + (e.pos.y - _dragState.initialMousePos.y);
			_dragState.window->Resize(width, height);
		}
	}
}

void Session::OnKeyPress(KeyEvent const& e) {
	KeyIndex index;
	index.modKey = e.modKey;
	index.key = e.key;
	KeyCallback* callback = _keyBindings.TryGet(index);
	LOG("GOT %p\n", callback);
	if (callback != nullptr) {
		(*callback)();
		return;
	}

	if (e.modKey == ModKey::Alt && e.key == Key::A) {
		LogLayout();
		return;
	}
	if (e.modKey == ModKey::Alt && e.key == Key::M) {
		#ifdef XEPHYR_ENABLE
		xephyrMonitorConfiguration++;
		UpdateLayout();
		#endif
		return;
	}

}

void Session::OnKeyRelease(KeyEvent const& e) {
}

void Session::OnMonitorConnect(MonitorEvent const& e) {
	LOG("monitor connected %s\n", e.port.CPtr());
	UpdateLayout();
}

void Session::OnMonitorDisconnect(MonitorEvent const& e) {
	LOG("monitor disconnected %s\n", e.port.CPtr());
	UpdateLayout();
}

void Session::OnMonitorPropertyChange(MonitorEvent const& e) {
	LOG("monitor property changed %s\n", e.port.CPtr());
	UpdateLayout();
}

void Session::FocusMonitor(Monitor* monitor) {
	if (_activeMonitor == monitor) return;
	if (_activeMonitor != nullptr) {
		_activeMonitor->Blur();
	}
	_activeMonitor = monitor;
	_activeMonitor->Focus();
	_observer->OnWorkspaceFocus(_activeMonitor->GetActiveWorkspace());
}

void Session::FocusWorkspaceID(Wx::String const& id) {
	Monitor* previousMonitor = _activeMonitor;
	for (int i = 0; i < _monitors.Count(); i++) {
		Monitor* mon = _monitors[i];
		Workspace* ws = mon->QueryWorkspace(id);
		if (ws == nullptr) {
			continue;
		}
		FocusWorkspace(ws);
		return;
	}
	LOG("no workspace with id %s\n", id.CPtr());
}

void Session::FocusWorkspace(Workspace* workspace) {
	Monitor* mon = workspace->GetMonitor();
	FocusMonitor(mon);
	mon->FocusWorkspace(workspace);
	_observer->OnWorkspaceFocus(workspace);

	// if workspace is empty move the cursor in the middle of monitor
	if (workspace->GetWindowList().IsEmpty()) {
		// TODO: make it configurable 
		if (true) {
			Rect rect = mon->GetWorkingRect();
			Point point;
			point.x = rect.x + (rect.width / 2);
			point.y = rect.y + (rect.height / 2);
			_context->SetMousePos(point);
		}
	}
	LOG("activate workspace %s\n", workspace->GetID().CPtr());
}

void Session::SendWorkspaceToMonitor(Workspace* workspace, Monitor* monitor) {
	Monitor* oldMonitor = workspace->GetMonitor();
	monitor->AddWorkspace(workspace);
	if (oldMonitor != nullptr) {
		if (oldMonitor != monitor) {
			for (int i = 0; i < workspace->GetWindowList().Count(); i++) {
				Window* window = workspace->GetWindowList()[i];
				SendWindowToWorkspace(window, workspace);
			}
		}
	}
}

void Session::SendWindowToWorkspace(Window* window, Workspace* workspace) {
	Workspace* previousWorkspace = window->GetWorkspace();
	Monitor* previousMonitor = window->GetMonitor();
	Monitor* monitor = workspace->GetMonitor();

	if (previousMonitor == monitor && previousWorkspace == workspace) {
		return;
	}

	if (previousMonitor == monitor) {
		window->Hide();
	} else {
		if (workspace->GetMonitor()->GetActiveWorkspace() != workspace) {
			window->Hide();
		}

		Rect windowRect = window->GetRect();
		Rect previousRect = previousMonitor->GetRect();
		Rect nextRect = monitor->GetRect();
		window->Move(
			nextRect.x + (windowRect.x - previousRect.x),
			nextRect.y + (windowRect.y - previousRect.y)
		);

		ClipWindow(window);
	}

	window->GetWorkspace()->RemoveWindow(window);
	workspace->AddWindow(window);
}

void Session::SendWindowToWorkspaceID(Wx::String const& id) {
	Window* window = _activeMonitor->GetActiveWorkspace()->GetActiveWindow();
	if (window == nullptr) {
		return;
	}
	for (int i = 0; i < _monitors.Count(); i++) {
		Monitor* mon = _monitors[i];
		Workspace* ws = mon->QueryWorkspace(id);
		if (ws == nullptr) {
			continue;
		}
		SendWindowToWorkspace(window, ws);
		return;
	}
	LOG("no workspace with id %s\n", id.CPtr());
}

void Session::FocusWindow(Window* window, bool moveCursor) {
	// raise to top (if background wallpaper or desktop orginizer you dont have to move it forward)
	window->Raise();
	
	Workspace* ws = window->GetWorkspace();
	FocusWorkspace(ws);
	ws->FocusWindow(window);
	_observer->OnWindowFocus(window);

	// TODO: make it configurable 
	if (moveCursor) {
		Rect rect = window->GetRect();
		Point point;
		point.x = rect.x + (rect.width / 2);
		point.y = rect.y + (rect.height / 2);
		_context->SetMousePos(point);
	}
}

static int CalculateCrossSurface(int a1, int a2, int b1, int b2) {
	const int start = Helper::Max(a1, b1);
    const int end   = Helper::Min(a2, b2);
    return Helper::Max(0, end - start);
}

void Session::FocusNearestWindow(Direction direction) {
	LOG("FOCUS NEAREST WINDOW %d\n", (int) direction);
	Window* sourceWindow = _activeMonitor->GetActiveWorkspace()->GetActiveWindow();
	if (sourceWindow == nullptr) return;

	Window* nearestWindow = FindNearestWindow(sourceWindow, direction, true);
	if (nearestWindow != nullptr) {
		FocusWindow(nearestWindow, true);
		return;
	}
	Window* nearestNonParalellWindow = FindNearestWindow(sourceWindow, direction, false);
	if (nearestNonParalellWindow != nullptr) {
		FocusWindow(nearestNonParalellWindow, true);
		return;
	}
	// TODO: find other monitor
}

Window* Session::FindNearestWindow(Window* window, Direction direction, bool onlyParallelWindow) {
	Workspace* workspace = window->GetWorkspace();
	Window* sourceWindow = window;
	if (sourceWindow == nullptr) return nullptr;

	Rect source = sourceWindow->GetRect();

	int shortestDistance = INT_MAX;
	Window* shortestWindow = nullptr;

	int largestSurface = 0;
	int errorLimit = 20;

	if (direction == Direction::East) {
		for (int i = 0; i < workspace->GetWindowList().Count(); i++) {
			Window* targetWindow = workspace->GetWindowList()[i];
			if (targetWindow == sourceWindow) continue;

			Rect target = targetWindow->GetRect();
			int distance = 0;
			int surface = CalculateCrossSurface(source.y, source.y + source.height, target.y, target.y + target.height);
			if (onlyParallelWindow && surface == 0) continue;

			// partial overlap but target is visible
			// +---+
			// | +----+
			// | |    |
			// | +----+
			// +---+
			bool partialOverlap = IsInsideRange(source.x + source.width, target.x, target.x + target.width);

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
		for (int i = 0; i < workspace->GetWindowList().Count(); i++) {
			Window* targetWindow = workspace->GetWindowList()[i];
			if (targetWindow == sourceWindow) continue;

			Rect target = targetWindow->GetRect();
			int distance = 0;
			int surface = CalculateCrossSurface(source.y, source.y + source.height, target.y, target.y + target.height);
			if (onlyParallelWindow && surface == 0) continue;

			// partial overlap but target is visible
			//    +---+
			// +----+ |
			// |    | |
			// +----+ |
			//    +---+
			bool partialOverLap = IsInsideRange(source.x, target.x, target.x + target.width);

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
		for (int i = 0; i < workspace->GetWindowList().Count(); i++) {
			Window* targetWindow = workspace->GetWindowList()[i];
			if (targetWindow == sourceWindow) continue;

			Rect target = targetWindow->GetRect();
			int distance = 0;
			int surface = CalculateCrossSurface(source.x, source.x + source.width, target.x, target.x + target.width);
			if (onlyParallelWindow && surface == 0) continue;

			// partial overlap but target is visible
			// +--------+
			// | +----+ |
			// +-|    |-+
			//   +----+
			bool partialOverLap = IsInsideRange(source.y + source.height, target.y, target.y + target.height);

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
		for (int i = 0; i < workspace->GetWindowList().Count(); i++) {
			Window* targetWindow = workspace->GetWindowList()[i];
			if (targetWindow == sourceWindow) continue;

			Rect target = targetWindow->GetRect();
			int distance = 0;
			int surface = CalculateCrossSurface(source.x, source.x + source.width, target.x, target.x + target.width);
			if (onlyParallelWindow && surface == 0) continue;

			// partial overlap but target is visible
			//   +----+  
			// +-|    |-+
			// | +----+ |
			// +--------+
			bool partialOverLap = IsInsideRange(source.y, target.y, target.y + target.height);

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

void Session::CycleNextWindow() {
	Workspace* workspace = _activeMonitor->GetActiveWorkspace();
	if (workspace->GetWindowList().Count() < 2) return;

	Wx::List<Window*> windows = workspace->GetWindowList();
	Window* activeWindow = workspace->GetActiveWindow();
	int idx = -1;
	for (int i = 0; i < windows.Count(); i++) {
		Window* window = windows[i];
		if (window == activeWindow) {
			idx = i;
			break;
		}
	}
	if (idx == -1) return;
	Window* nextWindow = windows[(idx + 1) % windows.Count()];
	FocusWindow(nextWindow, true);
}

void Session::CyclePreviousWindow() {
	Workspace* workspace = _activeMonitor->GetActiveWorkspace();
	if (workspace->GetWindowList().Count() < 2) return;

	Wx::List<Window*> windows = workspace->GetWindowList();
	Window* activeWindow = workspace->GetActiveWindow();
	int idx = -1;
	for (int i = 0; i < windows.Count(); i++) {
		Window* window = windows[i];
		if (window == activeWindow) {
			idx = i;
			break;
		}
	}
	if (idx == -1) return;
	Window* nextWindow = windows[(idx + windows.Count() - 1) % windows.Count()];
	FocusWindow(nextWindow, true);
}

void Session::CloseWindow() {
	Workspace* ws = _activeMonitor->GetActiveWorkspace();
	Window* window = ws->GetActiveWindow();
	if (window) {
		window->Close();
	}
}

void Session::BeginDrag(DragMode mode, Window* window, WindowResizingArea area) {
	_dragState.mode = mode;
	_dragState.window = window;


	Rect windowRect = _dragState.window->GetRect();
	Point mousePos = _context->GetMousePos();

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

	_context->UnclipWindow(_dragState.window);
	_context->BindMouseMovement();
}

void Session::DoCommand(ConfigCommand const& command) {
	if (command.func == ConfigCommandFunc::Exec) {
		Wx::List<Wx::String> argv;
		for (int i = 0; i < command.arguments.AsList().Count(); i++) {
			Wx::WkValue const& arg = command.arguments.AsList()[i];
			argv.Append(arg.AsString());
		}
		Exec(argv);
		return;
	}
	if (command.func == ConfigCommandFunc::CloseWindow) {
		CloseWindow();
		return;
	}
	if (command.func == ConfigCommandFunc::FocusWindow) {
		FocusNearestWindow((Direction) command.arguments.AsInt64());
		return;
	}
	if (command.func == ConfigCommandFunc::CycleNextWindow) {
		CycleNextWindow();
		return;
	}
	if (command.func == ConfigCommandFunc::CyclePreviousWindow) {
		CyclePreviousWindow();
		return;
	}
	if (command.func == ConfigCommandFunc::TileWindow) {
		Window* window = _activeMonitor->GetActiveWorkspace()->GetActiveWindow();
		if (window == nullptr) return;
		Rect avail = _activeMonitor->GetWorkingRect();
		Wx::HashTable<Wx::String, Wx::WkValue> const& args = command.arguments.AsHashTable();
		Wx::Float64 x = args.Get("x").AsFloat64();
		Wx::Float64 y = args.Get("y").AsFloat64();
		Wx::Float64 width = args.Get("width").AsFloat64();
		Wx::Float64 height = args.Get("height").AsFloat64();
		Rect target;
		target.x = avail.x + (avail.width * x);
		target.y = avail.y + (avail.height * y);
		target.width = (avail.width * width);
		target.height = (avail.height * height);
		LOG("[session] avail: (%d, %d, %d, %d)\n", avail.x, avail.y, avail.width, avail.height);
		LOG("[session] multiplier: (%g, %g, %g, %g)\n", x, y, width, height);
		LOG("[session] computed: (%d, %d, %d, %d)\n", target.x, target.y, target.width, target.height);
		window->Move(target.x, target.y);
		window->Resize(target.width, target.height);
		ClipWindow(window);
		return;
	}
	if (command.func == ConfigCommandFunc::FocusWorkspace) {
		FocusWorkspaceID(command.arguments.AsString());
		return;
	}
	if (command.func == ConfigCommandFunc::SendWindowToWorkspace) {
		SendWindowToWorkspaceID(command.arguments.AsString());
		return;
	}
	if (command.func == ConfigCommandFunc::Quit) {
		_context->Stop();
		return;
	}
}

Monitor* Session::QueryMonitorByPort(Wx::String const& port) {
	for (int i = 0; i < _monitors.Count(); i++) {
		if (_monitors[i]->GetPort() == port) {
			return _monitors[i];
		}
	}
	return nullptr;
}

Monitor* Session::QueryMonitorByPos(Point pos) {
	for (int i = 0; i < _monitors.Count(); i++) {
		if (IsPointIntersectRect(pos, _monitors[i]->GetRect())) {
			return _monitors[i];
		}
	}
	return nullptr;
}

void Session::UpdateLayout() {
	Wx::List<MonitorDef> monitorDefs = _context->GetConnectedMonitorDefs();

	MonitorDef fallbackDef;
	fallbackDef.port = "<fallback>";
	_fallbackMonitor = GetOrCreateMonitor(fallbackDef);
	_fallbackMonitor->ClearWorkspaceList();

	// TODO: reuse the monitor instance
	_monitors.Clear();

	if (monitorDefs.IsEmpty()) {
		_activeMonitor = _fallbackMonitor;
		_monitors.Append(_activeMonitor);
	} else {
		Monitor* activeMonitor = nullptr;
		for (int i = 0; i < monitorDefs.Count(); i++) {
			MonitorDef const& def = monitorDefs[i];
			Monitor* monitor = GetOrCreateMonitor(def);
			monitor->ClearWorkspaceList();
			if (_activeMonitor == monitor) {
				activeMonitor = monitor;
			}
			_monitors.Append(monitor);
		}
		if (activeMonitor == nullptr) {
			_activeMonitor = _monitors.GetFirst();
		}
	}

	for (int i = 0; i < _config.workspaces.Count(); i++) {
		ConfigWorkspace const& wsConfig = _config.workspaces[i];
		Workspace* ws = GetOrCreateWorkspace(wsConfig.id);
		if (wsConfig.ports.IsEmpty()) {
			SendWorkspaceToMonitor(ws, _activeMonitor);
		} else {
			Monitor* monitor = nullptr;
			for (int j = 0; j < wsConfig.ports.Count(); j++) {
				monitor = QueryMonitorByPort(wsConfig.ports[j]);
				if (monitor) {
					SendWorkspaceToMonitor(ws, monitor);
					break;
				}
			}
			if (monitor == nullptr) {
				SendWorkspaceToMonitor(ws, _activeMonitor);
			}
		}
	}

	for (int i = 0; i < _monitors.Count(); i++) {
		Monitor* monitor = _monitors[i];
		if (monitor->GetActiveWorkspace() == nullptr) {
			monitor->AddWorkspace(new Workspace(monitor->GetPort()));
		}
	}

	// collect living workspace list
	Wx::List<Workspace*> workspaces;

	for (int i = 0; i < _monitors.Count(); i++) {
		Monitor* monitor = _monitors[i];
		for (int j = 0; j < monitor->GetWorkspaceList().Count(); j++) {
			Workspace* workspace = monitor->GetWorkspaceList()[j];
			workspaces.Append(workspace);
			for (int k = 0; k < workspace->GetWindowList().Count(); k++) {
				Window* window = workspace->GetWindowList()[k];
				// reclip window
				ClipWindow(window);
			}
		}
	}

	_observer->OnWorkspaceConfigure(workspaces);
	LogLayout();
}

void Session::LogLayout() {
	LOG("layout:\n");
	for (int i = 0; i < _monitors.Count(); i++) {
		Monitor* mon = _monitors[i];
		LOG("  mon %s:", mon->GetPort().CPtr());
		if (mon == _activeMonitor) {
			LOG(" # active");
		}
		LOG("\n");
		for (int j = 0; j < mon->GetWorkspaceList().Count(); j++) {
			Workspace* ws = mon->GetWorkspaceList()[j];
			LOG("    ws %s: (mon: %s)", ws->GetID().CPtr(), ws->GetMonitor()->GetPort().CPtr());
			if (ws == mon->GetActiveWorkspace()) {
				LOG(" # active");
			}
			LOG("\n");

			for (int k = 0; k < ws->GetWindowList().Count(); k++) {
				Window* win = ws->GetWindowList()[k];
				LOG("      win %p: (ws: %s, mon: %s)", win, win->GetWorkspace()->GetID().CPtr(), win->GetMonitor()->GetPort().CPtr());
				if (win == ws->GetActiveWindow()) {
					LOG(" # active");
				}
				LOG("\n");
			}
		}
	}
}

Monitor* Session::GetOrCreateMonitor(MonitorDef const& def) {
	for (int i = 0; i < _monitorDB.Count(); i++) {
		Monitor* monitor = _monitorDB[i];
		if (monitor->GetPort() == def.port) {
			monitor->SetDef(def);
			return monitor;
		}
	}
	Monitor* monitor = new Monitor(_context, def);
	_monitorDB.Append(monitor);
	return monitor;
}

Workspace* Session::GetOrCreateWorkspace(Wx::String const& id) {
	for (int i = 0; i < _workspaceDB.Count(); i++) {
		Workspace* workspace = _workspaceDB[i];
		if (workspace->GetID() == id) {
			return workspace;
		}
	}
	Workspace* workspace = new Workspace(id);
	_workspaceDB.Append(workspace);
	return workspace;
}

void Session::ClipWindow(Window* window) {
	Monitor* monitor = window->GetMonitor();
	Rect windowRect = window->GetRect();
	WindowProp windowProp = window->GetProp();
	Rect windowBorderRect = windowRect;
	windowBorderRect.x -= windowProp.borderWidth;
	windowBorderRect.y -= windowProp.borderWidth;
	windowBorderRect.width += 2 * windowProp.borderWidth;
	windowBorderRect.height += 2 * windowProp.borderWidth;
	Rect workingRect = monitor->GetWorkingRect();
	Rect intersectRect = IntersectRectRect(workingRect, windowBorderRect);

	if (intersectRect.width == 0 || intersectRect.height == 0) {
		LOG("invalid intersection\n");
		return;
	}

	Rect clipRect;
	clipRect.x = intersectRect.x - windowRect.x;
	clipRect.y = intersectRect.y - windowRect.y;
	clipRect.width = intersectRect.width;
	clipRect.height = intersectRect.height;

	_context->ClipWindow(window, clipRect);
}
