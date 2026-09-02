#include "Event.hpp"
#include "Monitor.hpp"
#include "Backend/IContext.hpp"
#include "Backend/X11WindowBackend.hpp"
#include "Backend/X11Context.hpp"
#include "Input/Key.hpp"

X11Context::X11Context() {
	_conn = new X11Connection();

	int mask = (
		XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT |
		XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
		XCB_EVENT_MASK_PROPERTY_CHANGE |
		XCB_EVENT_MASK_BUTTON_PRESS
	);
	xcb_change_window_attributes(_conn->Get(), _conn->GetRoot(), XCB_CW_EVENT_MASK, &mask);

	_keyboardManager = new X11KeyboardManager(_conn);
	_mouseManager = new X11MouseManager(_conn);
	_monitorManager = new X11MonitorManager(_conn);
	_clipper = new X11Clipper(_conn);
	_hint = new X11Hint(_conn);
	_sessionObserver = new X11SessionObserver(_conn, _hint);

	// NOTE: click to focus
	xcb_grab_button(
		_conn->Get(), 0, _conn->GetScreen()->root, 
		XCB_EVENT_MASK_BUTTON_PRESS,
		XCB_GRAB_MODE_SYNC, XCB_GRAB_MODE_ASYNC, 
		XCB_NONE, XCB_NONE, 1, XCB_NONE
	);
}

X11Context::~X11Context() {
	delete _sessionObserver;
	delete _hint;
	delete _clipper;
	delete _mouseManager;
	delete _keyboardManager;
	delete _conn;
}

ISessionObserver* X11Context::GetSessionObserver() {
	return _sessionObserver;
}

void X11Context::BindMouseButton(ModKey modKey, MouseButton button) {
	xcb_grab_button(
		_conn->Get(), 0, _conn->GetRoot(), 
		XCB_EVENT_MASK_BUTTON_PRESS | 
		XCB_EVENT_MASK_BUTTON_RELEASE, 
		XCB_GRAB_MODE_ASYNC, 
		XCB_GRAB_MODE_ASYNC, 
		_conn->GetRoot(), XCB_NONE, FromMouseButton(button), FromModKey(modKey)
	);
	_conn->Flush();
}

void X11Context::UnbindMouseButton(ModKey modKey, MouseButton button) {
	xcb_ungrab_button(_conn->Get(), FromMouseButton(button), _conn->GetRoot(), FromModKey(modKey));
	_conn->Flush();
}

void X11Context::BindMouseMovement() {
	xcb_grab_pointer(_conn->Get(), 0, _conn->GetRoot(), 
		XCB_EVENT_MASK_BUTTON_PRESS | 
		XCB_EVENT_MASK_BUTTON_RELEASE | 
		XCB_EVENT_MASK_BUTTON_MOTION,
		XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, _conn->GetRoot(), XCB_NONE, XCB_CURRENT_TIME
	);
	_conn->Flush();
}

void X11Context::UnbindMouseMovement() {
	xcb_ungrab_pointer(_conn->Get(), XCB_CURRENT_TIME);
	_conn->Flush();
}

void X11Context::BindKey(ModKey modKey, Key key) {
	X11KeyBinding binding;
	binding.modKey = modKey;
	binding.key = key;

	binding.keyCode = _keyboardManager->QueryKeyCode(binding.key);
	binding.modKeyCode = FromModKey(binding.modKey);
	if (binding.keyCode == 0) {
		LOG("ERROR: key %d is invalid\n", (int)binding.key);
		return;
	}

	LOG("BINDING %08X, %08X\n", binding.modKeyCode, binding.keyCode);

	_keyBindings.Append(binding);
	_codeToKey.Set(binding.keyCode, binding.key);

	xcb_grab_key(
		_conn->Get(), 0, _conn->GetScreen()->root, 
		binding.modKeyCode,
		binding.keyCode,
		XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC
	);
	_conn->Flush();
}

void X11Context::UnbindKey(ModKey modKey, Key key) {
	int modKeyCode = FromModKey(modKey);
	int keyCode = _keyboardManager->QueryKeyCode(key);
	if (keyCode == 0) return;
	xcb_ungrab_key(
		_conn->Get(),
		keyCode,
		_conn->GetScreen()->root,
		modKeyCode
	);
	_conn->Flush();
}

void X11Context::BindModKey(ModKey modKey) {
	GrabWholeKeyboard();
	_grabbingWholeKeyboard = true;
	_grabbingModKey = modKey;
	_grabbingModKeyOriginal = modKey;
}

void X11Context::UnbindModKey(ModKey modKey) {
	UngrabWholeKeyboard();
	_grabbingWholeKeyboard = false;
}

Point X11Context::GetMousePos() {
	xcb_query_pointer_reply_t* pointer = xcb_query_pointer_reply(_conn->Get(), xcb_query_pointer(_conn->Get(), _conn->GetRoot()), 0);
	Point pos;
	pos.x = pointer->root_x;
	pos.y = pointer->root_y;
	free(pointer);
	return pos;
}

void X11Context::SetMousePos(Point point) {
	xcb_warp_pointer(_conn->Get(), XCB_NONE, _conn->GetRoot(), 0, 0, 0, 0, point.x, point.y);
	_conn->Flush();
}

Wx::List<MonitorDef> X11Context::GetConnectedMonitorDefs() {
	return _monitorManager->GetConnectedMonitorDefs();
}

void X11Context::ClipWindow(Window* window, Rect rect) {
	_clipper->Clip((X11WindowBackend*)window->GetBackend(), rect);
}

void X11Context::UnclipWindow(Window* window) {
	_clipper->Unclip((X11WindowBackend*)window->GetBackend());
}

void X11Context::HighlightWindow(Window* window) {
}

void X11Context::UnhighlightWindow(Window* window) {
}

Margin X11Context::GetDockExtend(Window* window) {
	return _hint->GetWindowDockExtend(((X11WindowBackend*)window->GetBackend())->GetClientHandle());
}

void X11Context::Run(IObserver* observer) {
	UpdateMonitors(observer);

	while (_running) {
		xcb_generic_event_t* ev = xcb_wait_for_event(_conn->Get());
		int type = ev->response_type & ~0x80;
		switch (type) {
			case XCB_CREATE_NOTIFY: {
				LOG("[x11] create notify\n");
				break;
			}

			case XCB_MAP_REQUEST: {
				xcb_map_request_event_t* e = (xcb_map_request_event_t*) ev;
				LOG("[x11] map request %08X (%s)\n", e->window, _hint->GetWindowTitle(e->window).CPtr());

				Window* window = QueryWindowByClient(e->window);
				if (window == nullptr) {
					xcb_get_window_attributes_cookie_t cookie = xcb_get_window_attributes(_conn->Get(), e->window);
					xcb_get_window_attributes_reply_t* attr = xcb_get_window_attributes_reply(_conn->Get(), cookie, nullptr);

					if (attr == nullptr) {
						xcb_map_window(_conn->Get(), e->window);
						_conn->Flush();
						break;
					}
					bool unmanaged = attr->override_redirect;
					free(attr);
					if (unmanaged) {
						LOG("[x11] unmanaged map request\n");
						xcb_map_window(_conn->Get(), e->window);
						_conn->Flush();
						break;
					}

					X11WindowBackend* backend = new X11WindowBackend(_conn, _hint, e->window);
					window = new Window(backend);
					WindowEvent event;
					event.window = window;

					_clientWindowMap.Set(e->window, window);
					_frameWindowMap.Set(backend->GetFrameHandle(), window);

					observer->OnWindowCreate(event);
				} else {
					window->Show();
				}
				window->Raise();
				window->Focus();
				break;
			}

			case XCB_MAP_NOTIFY: {
				xcb_map_notify_event_t* e = (xcb_map_notify_event_t*) ev;
				LOG("[x11] map notify %08X\n", e->window);
				break;
			}

			case XCB_CONFIGURE_REQUEST: {
				xcb_configure_request_event_t* e = (xcb_configure_request_event_t*) ev;
				LOG("[x11] configure request %08X\n", e->window);
				LOG("      x: %d, y: %d, w: %d, h: %d\n", e->x, e->y, e->width, e->height);

				Window* window = QueryWindowByClient(e->window);
				if (window) {
					X11WindowBackend* backend = (X11WindowBackend*) window->GetBackend();
					backend->NotifyClient();
					break;
				}
				uint16_t mask = 0;
				uint32_t values[7];
				uint32_t i = 0;

				if (e->value_mask & XCB_CONFIG_WINDOW_X) {
					mask |= XCB_CONFIG_WINDOW_X;
					values[i++] = e->x;
				}
				if (e->value_mask & XCB_CONFIG_WINDOW_Y) {
					mask |= XCB_CONFIG_WINDOW_Y;
					values[i++] = e->y;
				}
				if (e->value_mask & XCB_CONFIG_WINDOW_WIDTH) {
					mask |= XCB_CONFIG_WINDOW_WIDTH;
					values[i++] = e->width;
				}
				if (e->value_mask & XCB_CONFIG_WINDOW_HEIGHT) {
					mask |= XCB_CONFIG_WINDOW_HEIGHT;
					values[i++] = e->height; 
				}
				if (e->value_mask & XCB_CONFIG_WINDOW_BORDER_WIDTH) {
					mask |= XCB_CONFIG_WINDOW_BORDER_WIDTH;
					values[i++] = e->border_width;
				}
				if (e->value_mask & XCB_CONFIG_WINDOW_SIBLING) {
					mask |= XCB_CONFIG_WINDOW_SIBLING;
					values[i++] = e->sibling;
				}
				if (e->value_mask & XCB_CONFIG_WINDOW_STACK_MODE) {
					mask |= XCB_CONFIG_WINDOW_STACK_MODE;
					values[i++] = e->stack_mode;
				}

				xcb_configure_window(_conn->Get(), e->window, mask, values);
				_conn->Flush();
				break;
			}

			case XCB_CONFIGURE_NOTIFY: {
				xcb_configure_notify_event_t* e = (xcb_configure_notify_event_t*) ev;
				LOG("[x11] configure notify %08X\n", e->window);
				break;
			}

			case XCB_UNMAP_NOTIFY: {
				xcb_unmap_notify_event_t* e = (xcb_unmap_notify_event_t*) ev;
				LOG("[x11] unmap notify %08X\n", e->window);
				Window* window = QueryWindowByClient(e->window);
				if (window == nullptr) {
					break;
				}
				window->Hide();
				break;
			}

			case XCB_DESTROY_NOTIFY: {
				xcb_destroy_notify_event_t* e = (xcb_destroy_notify_event_t*) ev;
				LOG("[x11] destroy notify %08X\n", e->window);
				Window* window = QueryWindowByClient(e->window);
				if (window == nullptr) {
					break;
				}
				WindowEvent event;
				event.window = window;
				observer->OnWindowDestroy(event);

				_clientWindowMap.Remove(e->window);
				_frameWindowMap.Remove(((X11WindowBackend*)window->GetBackend())->GetFrameHandle());

				delete window;
				break;
			}

			case XCB_EXPOSE: {
				xcb_expose_event_t* e = (xcb_expose_event_t*) ev;
				LOG("[x11] expose %08X\n", e->window);
				Window* window = QueryWindowByFrame(e->window);
				if (window == nullptr) {
					break;
				}
				((X11WindowBackend*)window->GetBackend())->Draw();
				break;
			}

			case XCB_BUTTON_PRESS: {
				xcb_button_press_event_t* e = (xcb_button_press_event_t *) ev;
				LOG("[x11] button press %d, %d\n", e->root_x, e->root_y);

				MouseButtonEvent event;
				event.pos.x = e->root_x;
				event.pos.y = e->root_y;
				event.button = ToMouseButton(e->detail);
				event.modKey = ToModKey(e->state);
				event.window = QueryWindowByFrame(e->child);

				if (e->detail == 1 && e->state == 0) {
					xcb_allow_events(_conn->Get(), XCB_ALLOW_REPLAY_POINTER, e->time);
					_conn->Flush();
				}

				observer->OnMousePress(event);
				break;
			}

			case XCB_BUTTON_RELEASE: {
				xcb_button_release_event_t* e = (xcb_button_release_event_t *) ev;
				LOG("[x11] button release %d, %d\n", e->root_x, e->root_y);

				MouseButtonEvent event;
				event.pos.x = e->root_x;
				event.pos.y = e->root_y;
				event.button = ToMouseButton(e->detail);
				event.modKey = ToModKey(e->state);
				event.window = QueryWindowByFrame(e->child);
				observer->OnMouseRelease(event);
				break;
			}

			case XCB_MOTION_NOTIFY: {
				xcb_motion_notify_event_t* e = (xcb_motion_notify_event_t*) ev;
				LOG("[x11] motion notify %d, %d\n", e->root_x, e->root_y);

				MouseMovementEvent event;
				event.pos.x = e->root_x;
				event.pos.y = e->root_y;
				observer->OnMouseMove(event);
				break;
			}

			case XCB_KEY_PRESS: {
				xcb_key_press_event_t* e = (xcb_key_press_event_t*) ev;
				Key* key = _codeToKey.TryGet(e->detail);
				if (key == nullptr) {
					break;
				}
				LOG("[x11] key press %d\n", e->detail);
				KeyEvent event;
				event.modKey = ToModKey(e->state);
				event.key = *key;

				observer->OnKeyPress(event);
				break;
			}

			case XCB_KEY_RELEASE: {
				xcb_key_release_event_t* e = (xcb_key_release_event_t*) ev;
				if (_grabbingWholeKeyboard) {
					if (WX_FLAGS_CONTAIN(_grabbingModKey, ModKey::Shift)) {
						if (_keyboardManager->QueryKeyCode(Key::ShiftL) == e->detail ||
							_keyboardManager->QueryKeyCode(Key::ShiftR) == e->detail) {
							_grabbingModKey &= ~ModKey::Shift;
						}
					}
					if (WX_FLAGS_CONTAIN(_grabbingModKey, ModKey::Ctrl)) {
						if (_keyboardManager->QueryKeyCode(Key::CtrlL) == e->detail ||
							_keyboardManager->QueryKeyCode(Key::CtrlR) == e->detail) {
							_grabbingModKey &= ~ModKey::Ctrl;
						}
					}
					if (WX_FLAGS_CONTAIN(_grabbingModKey, ModKey::Alt)) {
						if (_keyboardManager->QueryKeyCode(Key::AltL) == e->detail ||
							_keyboardManager->QueryKeyCode(Key::AltR) == e->detail) {
							_grabbingModKey &= ~ModKey::Alt;
						}
					}
					if (WX_FLAGS_CONTAIN(_grabbingModKey, ModKey::Super)) {
						if (_keyboardManager->QueryKeyCode(Key::SuperL) == e->detail ||
							_keyboardManager->QueryKeyCode(Key::SuperR) == e->detail) {
							_grabbingModKey &= ~ModKey::Super;
						}
					}
					if (_grabbingModKey != ModKey::None) {
						break;
					}

					LOG("[x11] holding-mod key release %d\n", e->detail);
					KeyEvent event;
					event.modKey = _grabbingModKeyOriginal;
					event.key = Key::None;
					observer->OnKeyRelease(event);
					_grabbingModKey = _grabbingModKeyOriginal;
					break;
				}
				LOG("[x11] key release %d\n", e->detail);
				Key* key = _codeToKey.TryGet(e->detail);
				if (key == nullptr) {
					break;
				}
				KeyEvent event;
				event.modKey = ToModKey(e->state);
				event.key = *key;
				observer->OnKeyRelease(event);
				break;
			}

			case XCB_CLIENT_MESSAGE: {
				xcb_client_message_event_t* e = (xcb_client_message_event_t*) ev;
				LOG("[x11] client message %d\n", e->window);

				Window* window = QueryWindowByClient(e->window);
				if (window == nullptr) {
					LOG("[x11] unmanaged window id message request %d (%s)\n", e->window, _hint->AtomToString(e->type).CPtr());
					break;
				}

				if (_hint->IsActiveWindow(e->type)) {
					// SetActiveWindow(window);
					// Context::Sync();
					break;
				}

				if (_hint->IsWindowState(e->type)) {
					// uint32_t action = e->data.data32[0];
					// xcb_atom_t state = e->data.data32[1];
					//
					// WindowState hintState = _hint->atomToWindowState(state);
					// LOG("client message state %s with action %d", _hint->atomToString(e->data.data32[1]).cptr(), action);
					//
					// if (hintState == WindowState::Unknown) {
					// 	break;
					// }
					//
					// if (hintState == WindowState::Fullscreen) {
					// 	// SetActiveWindow(window);
					//
					// 	Workspace* workspace = window->getWorkspace();
					// 	if (action == 2) {
					// 		if (workspace->CanRestoreActiveWindow()) {
					// 			action = 0;
					// 		} else {
					// 			action = 1;
					// 		}
					// 	}
					// 	if (action == 1) {
					// 		workspace->FullscreenActiveWindow();
					// 	}
					// 	if (action == 0) {
					// 		workspace->RestoreActiveWindow();
					// 	}
					// }
					//
					// if (hintState == Hint::WindowState::MaximizedHorizontal || hintState == Hint::WindowState::MaximizedVertical) {
					// 	SetActiveWindow(window);
					//
					// 	Workspace* workspace = window->GetWorkspace();
					// 	if (action == 2) {
					// 		if (workspace->CanRestoreActiveWindow()) {
					// 			action = 0;
					// 		} else {
					// 			action = 1;
					// 		}
					// 	}
					// 	if (action == 1) {
					// 		workspace->MaximizeActiveWindow(
					// 			Unit::CreatePercentage(0.0),
					// 			Unit::CreatePercentage(0.0),
					// 			Unit::CreatePercentage(1.0),
					// 			Unit::CreatePercentage(1.0)
					// 		);
					// 	}
					// 	if (action == 0) {
					// 		workspace->RestoreActiveWindow();
					// 	}
					// }
					//
					// Context::Sync();
					break;
				}

				if (_hint->IsMoveResizeProperty(e->type)) {
					Point cursor;
					cursor.x = e->data.data32[0];
					cursor.y = e->data.data32[1];
					int direction = e->data.data32[2];
					uint32_t button = e->data.data32[3];

					if (direction == 0) {
						WindowResizingEvent event;
						event.window = window;
						event.area = WindowResizingArea::NorthWest;
						observer->OnWindowResizeRequest(event);
						break;
					}
					if (direction == 1) {
						WindowResizingEvent event;
						event.window = window;
						event.area = WindowResizingArea::North;
						observer->OnWindowResizeRequest(event);
						break;
					}
					if (direction == 2) {
						WindowResizingEvent event;
						event.window = window;
						event.area = WindowResizingArea::NorthEast;
						observer->OnWindowResizeRequest(event);
						break;
					}
					if (direction == 3) {
						WindowResizingEvent event;
						event.window = window;
						event.area = WindowResizingArea::East;
						observer->OnWindowResizeRequest(event);
						break;
					}
					if (direction == 4) {
						WindowResizingEvent event;
						event.window = window;
						event.area = WindowResizingArea::SouthEast;
						observer->OnWindowResizeRequest(event);
						break;
					}
					if (direction == 5) {
						WindowResizingEvent event;
						event.window = window;
						event.area = WindowResizingArea::South;
						observer->OnWindowResizeRequest(event);
						break;
					}
					if (direction == 6) {
						WindowResizingEvent event;
						event.window = window;
						event.area = WindowResizingArea::SouthWest;
						observer->OnWindowResizeRequest(event);
						break;
					}
					if (direction == 7) {
						WindowResizingEvent event;
						event.window = window;
						event.area = WindowResizingArea::West;
						observer->OnWindowResizeRequest(event);
						break;
					}
					if (direction == 8) {
						WindowMovementEvent event;
						event.window = window;
						event.offset = cursor;
						observer->OnWindowMoveRequest(event);
						break;
					}
				}

				LOG("uncaptured client message %d, %s\n", e->type, _hint->AtomToString(e->type).CPtr());
				break;
			}


			default: {
				if (type == _keyboardManager->GetEventOffset()) {
					switch (ev->pad0) {
						case XCB_XKB_NEW_KEYBOARD_NOTIFY: {
							xcb_xkb_new_keyboard_notify_event_t* event = (xcb_xkb_new_keyboard_notify_event_t*) ev;
							if (event->changed & XCB_XKB_NKN_DETAIL_KEYCODES) {
								UpdateKeyBindings();
							}
							break;
						}
						case XCB_XKB_MAP_NOTIFY:
							UpdateKeyBindings();
							break;
					}
					break;
				}
				if (type == _monitorManager->GetEventOffset() + XCB_RANDR_SCREEN_CHANGE_NOTIFY) {
					UpdateMonitors(observer);
					break;
				}

				if (type == _monitorManager->GetEventOffset() + XCB_RANDR_NOTIFY) {
					xcb_randr_notify_event_t* event = (xcb_randr_notify_event_t *) ev;
					if (event->subCode == XCB_RANDR_NOTIFY_OUTPUT_CHANGE) {
						xcb_randr_output_change_t oc = event->u.oc;
						if (oc.connection == XCB_RANDR_CONNECTION_CONNECTED) {
							UpdateMonitors(observer);
						} else if (oc.connection == XCB_RANDR_CONNECTION_DISCONNECTED) {
							UpdateMonitors(observer);
						}
					}
					if (event->subCode == XCB_RANDR_NOTIFY_CRTC_CHANGE) {
						xcb_randr_crtc_change_t cc = event->u.cc;
						UpdateMonitors(observer);
					}
					break;
				}
				break;
			}
		}
		free(ev);
	}
}

void X11Context::Stop() {
	_running = false;
}

Window* X11Context::QueryWindowByFrame(xcb_window_t handle) {
	if (handle == XCB_NONE) return nullptr;
	Window** window = _frameWindowMap.TryGet(handle);
	if (window != nullptr) return *window;
	return nullptr;
}

Window* X11Context::QueryWindowByClient(xcb_window_t handle) {
	if (handle == XCB_NONE) return nullptr;
	Window** window = _clientWindowMap.TryGet(handle);
	if (window != nullptr) return *window;
	return nullptr;
}

int X11Context::FromModKey(ModKey modKey) {
	Wx::Uint32 flags = 0;

	if (WX_FLAGS_CONTAIN(modKey, ModKey::Shift)) {
		flags |= XCB_MOD_MASK_SHIFT;
	}
	if (WX_FLAGS_CONTAIN(modKey, ModKey::Ctrl)) {
		flags |= XCB_MOD_MASK_CONTROL;
	}
	if (WX_FLAGS_CONTAIN(modKey, ModKey::Alt)) {
		flags |= XCB_MOD_MASK_1;
	}
	if (WX_FLAGS_CONTAIN(modKey, ModKey::Super)) {
		flags |= XCB_MOD_MASK_4;
	}
	return flags;
}

ModKey X11Context::ToModKey(int modKey) {
	ModKey flags = ModKey::None;
	if (WX_FLAGS_CONTAIN(modKey, XCB_MOD_MASK_SHIFT)) {
		flags |= ModKey::Shift;
	}
	if (WX_FLAGS_CONTAIN(modKey, XCB_MOD_MASK_CONTROL)) {
		flags |= ModKey::Ctrl;
	}
	if (WX_FLAGS_CONTAIN(modKey, XCB_MOD_MASK_1)) {
		flags |= ModKey::Alt;
	}
	if (WX_FLAGS_CONTAIN(modKey, XCB_MOD_MASK_4)) {
		flags |= ModKey::Super;
	}
	return flags;
}

int X11Context::FromMouseButton(MouseButton button) {
	return (int)button;
}

MouseButton X11Context::ToMouseButton(int button) {
	return (MouseButton)button;
}

void X11Context::AttachKeyBindings() {
	_codeToKey.Clear();

	for (int i = 0; i < _keyBindings.Count(); i++) {
		X11KeyBinding& binding = _keyBindings[i];
		binding.keyCode = _keyboardManager->QueryKeyCode(binding.key);
		binding.modKeyCode = FromModKey(binding.modKey);
		if (binding.keyCode == 0) {
			LOG("ERROR: key %d is invalid\n", (int)binding.key);
			continue;
		}

		_codeToKey.Set(binding.keyCode, binding.key);

		xcb_grab_key(
			_conn->Get(), 0, _conn->GetScreen()->root, 
			binding.modKeyCode,
			binding.keyCode,
			XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC
		);
	}
	_conn->Flush();
}

void X11Context::DetachKeyBindings() {
	for (int i = 0; i < _keyBindings.Count(); i++) {
		X11KeyBinding& binding = _keyBindings[i];
		if (binding.keyCode == 0) continue;

		xcb_ungrab_key(
			_conn->Get(),
			binding.keyCode,
			_conn->GetScreen()->root,
			binding.modKeyCode
		);
	}
	_conn->Flush();
}

void X11Context::UpdateKeyBindings() {
	_keyboardManager->UpdateKeymap();
	Wx::String layout = _keyboardManager->GetLayout();

	if (layout == _activeKeyboardLayout) {
		return;
	}

	DetachKeyBindings();
	AttachKeyBindings();

	LOG("changing keyboard layout to %s\n", layout.CPtr());
	_activeKeyboardLayout = layout;
}

void X11Context::GrabWholeKeyboard() {
	/*
     * Now take an active keyboard grab.
     *
     * From this point until ungrab, keyboard events are directed
     * to our WM.
     */
    xcb_grab_keyboard_cookie_t cookie = xcb_grab_keyboard(
		_conn->Get(),
		0,                      // owner_events
		_conn->GetRoot(),
		XCB_CURRENT_TIME,
		XCB_GRAB_MODE_ASYNC,
		XCB_GRAB_MODE_ASYNC
	);

    xcb_grab_keyboard_reply_t *reply = xcb_grab_keyboard_reply(_conn->Get(), cookie, NULL);

    if (!reply) {
        return;
    }

    if (reply->status != XCB_GRAB_STATUS_SUCCESS) {
    }
    free(reply);
}

void X11Context::UngrabWholeKeyboard() {
	xcb_ungrab_keyboard(_conn->Get(), XCB_CURRENT_TIME);
	_conn->Flush();
}

MonitorDef* X11Context::QueryMonitorDefByPort(Wx::String const& port, Wx::List<MonitorDef>& defs) {
	for (int i = 0; i < defs.Count(); i++) {
		if (defs[i].port == port) return &defs[i];
	}
	return nullptr;
}

void X11Context::UpdateMonitors(IObserver* observer) {
	Wx::List<MonitorDef> currentMonitorDefs = _monitorManager->GetConnectedMonitorDefs();

	Wx::List<Wx::String> connectedPorts;
	Wx::List<Wx::String> disconnectedPorts;
	Wx::List<Wx::String> propertyChangedPorts;
	Wx::List<Wx::String> stayPorts;

	for (int i = 0; i < currentMonitorDefs.Count(); i++) {
		MonitorDef const& current = currentMonitorDefs[i];
		MonitorDef* old = QueryMonitorDefByPort(current.port, _monitorDefs);
		if (old == nullptr) {
			connectedPorts.Append(current.port);
		} else {
			if (old->rect.x != current.rect.x ||
				old->rect.y != current.rect.y ||
				old->rect.width != current.rect.width ||
				old->rect.height != current.rect.height) {
				propertyChangedPorts.Append(current.port);
			}
			stayPorts.Append(current.port);
		}
	}
	for (int i = 0; i < _monitorDefs.Count(); i++) {
		MonitorDef const& old = _monitorDefs[i];
		MonitorDef* current = QueryMonitorDefByPort(old.port, currentMonitorDefs);
		if (current == nullptr) {
			disconnectedPorts.Append(old.port);
		}
	}

	for (int i = 0; i < connectedPorts.Count(); i++) {
		MonitorEvent event;
		event.port = connectedPorts[i];
		observer->OnMonitorConnect(event);
	}

	for (int i = 0; i < disconnectedPorts.Count(); i++) {
		MonitorEvent event;
		event.port = disconnectedPorts[i];
		observer->OnMonitorDisconnect(event);
	}

	for (int i = 0; i < propertyChangedPorts.Count(); i++) {
		MonitorEvent event;
		event.port = propertyChangedPorts[i];
		observer->OnMonitorPropertyChange(event);
	}

	_monitorDefs = static_cast<Wx::List<MonitorDef>&&>(currentMonitorDefs);
}

IContext* CreateContext() {
	return new X11Context();
}

