#include "Event.hpp"
#include "IObserver.hpp"
#include "Monitor.hpp"
#include "backend/IContext.hpp"
#include "backend/X11WindowBackend.hpp"
#include "backend/X11Context.hpp"
#include "input/Key.hpp"
#include "core/Logger.hpp"
#include <iostream>
#include <unistd.h>

X11Context::X11Context() {
	_conn = new X11Connection();

	int mask = (
		XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT |
		XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
		XCB_EVENT_MASK_PROPERTY_CHANGE |
		XCB_EVENT_MASK_BUTTON_PRESS
	);
	xcb_change_window_attributes(_conn->get(), _conn->getRoot(), XCB_CW_EVENT_MASK, &mask);

	_keyboardManager = new X11KeyboardManager(_conn);
	_mouseManager = new X11MouseManager(_conn);
	_monitorManager = new X11MonitorManager(_conn);
	_clipper = new X11Clipper(_conn);
	_hint = new X11Hint(_conn);
	_sessionObserver = new X11SessionObserver(_conn, _hint);

	// NOTE: click to focus
	xcb_grab_button(
		_conn->get(), 0, _conn->getScreen()->root, 
		XCB_EVENT_MASK_BUTTON_PRESS,
		XCB_GRAB_MODE_SYNC, XCB_GRAB_MODE_ASYNC, 
		XCB_NONE, XCB_NONE, 1, XCB_NONE
	);
}

X11Context::~X11Context() {
	delete _sessionObserver;
	delete _hint;
	delete _clipper;
	delete _monitorManager;
	delete _mouseManager;
	delete _keyboardManager;
	delete _conn;
}

ISessionObserver* X11Context::getSessionObserver() {
	return _sessionObserver;
}

void X11Context::bindMouseButton(ModKey modKey, MouseButton button) {
	xcb_grab_button(
		_conn->get(), 0, _conn->getRoot(), 
		XCB_EVENT_MASK_BUTTON_PRESS | 
		XCB_EVENT_MASK_BUTTON_RELEASE, 
		XCB_GRAB_MODE_ASYNC, 
		XCB_GRAB_MODE_ASYNC, 
		_conn->getRoot(), XCB_NONE, _fromMouseButton(button), _fromModKey(modKey)
	);
	_conn->flush();
}

void X11Context::unbindMouseButton(ModKey modKey, MouseButton button) {
	xcb_ungrab_button(_conn->get(), _fromMouseButton(button), _conn->getRoot(), _fromModKey(modKey));
	_conn->flush();
}

void X11Context::bindMouseMovement() {
	xcb_grab_pointer(_conn->get(), 0, _conn->getRoot(), 
		XCB_EVENT_MASK_BUTTON_PRESS | 
		XCB_EVENT_MASK_BUTTON_RELEASE | 
		XCB_EVENT_MASK_BUTTON_MOTION,
		XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, _conn->getRoot(), XCB_NONE, XCB_CURRENT_TIME
	);
	_conn->flush();
}

void X11Context::unbindMouseMovement() {
	xcb_ungrab_pointer(_conn->get(), XCB_CURRENT_TIME);
	_conn->flush();
	printf("UNGRAB POINTER\n");
}

void X11Context::bindKey(ModKey modKey, Key key) {
	X11KeyBinding binding;
	binding.modKey = modKey;
	binding.key = key;

	binding.keyCode = _keyboardManager->queryKeyCode(binding.key);
	binding.modKeyCode = _fromModKey(binding.modKey);
	if (binding.keyCode == 0) {
		LOG("ERROR: key %d is invalid\n", (int)binding.key);
		return;
	}

	LOG("BINDING %08X, %08X\n", binding.modKeyCode, binding.keyCode);

	_keyBindings.append(binding);
	_codeToKey.set(binding.keyCode, binding.key);

	xcb_grab_key(
		_conn->get(), 0, _conn->getScreen()->root, 
		binding.modKeyCode,
		binding.keyCode,
		XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC
	);
	_conn->flush();
}

void X11Context::unbindKey(ModKey modKey, Key key) {
	int modKeyCode = _fromModKey(modKey);
	int keyCode = _keyboardManager->queryKeyCode(key);
	if (keyCode == 0) return;
	xcb_ungrab_key(
		_conn->get(),
		keyCode,
		_conn->getScreen()->root,
		modKeyCode
	);
	_conn->flush();
}

void X11Context::bindModKey(ModKey modKey) {
	_grabWholeKeyboard();
	_grabbingWholeKeyboard = true;
	_grabbingModKey = modKey;
	_grabbingModKeyOriginal = modKey;
}

void X11Context::unbindModKey(ModKey modKey) {
	_ungrabWholeKeyboard();
	_grabbingWholeKeyboard = false;
}

Point X11Context::getMousePos() {
	xcb_query_pointer_reply_t* pointer = xcb_query_pointer_reply(_conn->get(), xcb_query_pointer(_conn->get(), _conn->getRoot()), 0);
	Point pos;
	pos.x = pointer->root_x;
	pos.y = pointer->root_y;
	free(pointer);
	return pos;
}

void X11Context::setMousePos(Point point) {
	auto conn = _conn->get();
	auto root = _conn->getRoot();
	auto cookie = xcb_query_pointer(conn, root);
	auto reply = xcb_query_pointer_reply(conn, cookie, nullptr);

	if (reply) {
		std::cout << "BEFORE: "
			<< reply->root_x << ", "
			<< reply->root_y << '\n';

		free(reply);
	}

	xcb_warp_pointer(
			conn,
			XCB_NONE,
			root,
			0, 0, 0, 0,
			point.x, point.y
			);

	xcb_flush(conn);

	usleep(10000);

	cookie = xcb_query_pointer(conn, root);
	reply = xcb_query_pointer_reply(conn, cookie, nullptr);

	if (reply) {
		std::cout << "AFTER: "
			<< reply->root_x << ", "
			<< reply->root_y << '\n';

		free(reply);
	}
}

wx::List<MonitorDef> X11Context::getConnectedMonitorDefs() {
	return _monitorManager->getConnectedMonitorDefs();
}

void X11Context::clipWindow(Window* window, Rect rect) {
	int borderWidth = window->getProp().borderWidth;
	rect.x -= borderWidth;
	rect.y -= borderWidth;
	_clipper->clip((X11WindowBackend*)window->getBackend(), rect);
}

void X11Context::unclipWindow(Window* window) {
	_clipper->unclip((X11WindowBackend*)window->getBackend());
}

void X11Context::highlightWindow(Window* window) {
}

void X11Context::unhighlightWindow(Window* window) {
}

Margin X11Context::getDockExtend(Window* window) {
	return _hint->getWindowDockExtend(((X11WindowBackend*)window->getBackend())->getClientHandle());
}

void X11Context::run(IObserver* observer) {
	_updateMonitors(observer);

	_unmanagedWindowList(observer);

	while (_running) {
		xcb_generic_event_t* ev = xcb_wait_for_event(_conn->get());
		int type = ev->response_type & ~0x80;
		switch (type) {
			case XCB_CREATE_NOTIFY: {
				LOG("[x11] create notify\n");
				break;
			}

			case XCB_MAP_REQUEST: {
				xcb_map_request_event_t* e = (xcb_map_request_event_t*) ev;
				LOG("[x11] map request %08X (%s)\n", e->window, _hint->getWindowTitle(e->window).cptr());

				Window* window = _queryWindowByClient(e->window);
				if (window == nullptr) {
					xcb_get_window_attributes_cookie_t cookie = xcb_get_window_attributes(_conn->get(), e->window);
					xcb_get_window_attributes_reply_t* attr = xcb_get_window_attributes_reply(_conn->get(), cookie, nullptr);

					if (attr == nullptr) {
						xcb_map_window(_conn->get(), e->window);
						_conn->flush();
						break;
					}
					bool unmanaged = attr->override_redirect;
					free(attr);
					if (unmanaged) {
						LOG("[x11] unmanaged map request\n");
						xcb_map_window(_conn->get(), e->window);
						_conn->flush();
						break;
					}

					X11WindowBackend* backend = new X11WindowBackend(_conn, _hint, e->window);
					window = new Window(backend);
					WindowEvent event;
					event.window = window;

					_clientWindowMap.set(e->window, window);
					_frameWindowMap.set(backend->getFrameHandle(), window);

					observer->onWindowCreate(event);
				} else {
					window->show();
				}
				window->raise();
				window->focus();
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

				Window* window = _queryWindowByClient(e->window);
				if (window) {
					X11WindowBackend* backend = (X11WindowBackend*) window->getBackend();
					backend->notifyClient();
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

				xcb_configure_window(_conn->get(), e->window, mask, values);
				_conn->flush();
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
				Window* window = _queryWindowByClient(e->window);
				if (window == nullptr) {
					break;
				}
				window->hide();
				break;
			}

			case XCB_DESTROY_NOTIFY: {
				xcb_destroy_notify_event_t* e = (xcb_destroy_notify_event_t*) ev;
				LOG("[x11] destroy notify %08X\n", e->window);
				Window* window = _queryWindowByClient(e->window);
				if (window == nullptr) {
					break;
				}
				WindowEvent event;
				event.window = window;
				observer->onWindowDestroy(event);

				_clientWindowMap.remove(e->window);
				_frameWindowMap.remove(((X11WindowBackend*)window->getBackend())->getFrameHandle());

				delete window;
				break;
			}

			case XCB_EXPOSE: {
				xcb_expose_event_t* e = (xcb_expose_event_t*) ev;
				LOG("[x11] expose %08X\n", e->window);
				Window* window = _queryWindowByFrame(e->window);
				if (window == nullptr) {
					break;
				}
				((X11WindowBackend*)window->getBackend())->draw();
				break;
			}

			case XCB_BUTTON_PRESS: {
				xcb_button_press_event_t* e = (xcb_button_press_event_t *) ev;
				LOG("[x11] button press %d, %d\n", e->root_x, e->root_y);

				MouseButtonEvent event;
				event.pos.x = e->root_x;
				event.pos.y = e->root_y;
				event.button = _toMouseButton(e->detail);
				event.modKey = _toModKey(e->state);
				event.window = _queryWindowByFrame(e->child);

				if (e->detail == 1 && e->state == 0) {
					xcb_allow_events(_conn->get(), XCB_ALLOW_REPLAY_POINTER, e->time);
					_conn->flush();
				}

				observer->onMousePress(event);
				break;
			}

			case XCB_BUTTON_RELEASE: {
				xcb_button_release_event_t* e = (xcb_button_release_event_t *) ev;
				LOG("[x11] button release %d, %d\n", e->root_x, e->root_y);

				MouseButtonEvent event;
				event.pos.x = e->root_x;
				event.pos.y = e->root_y;
				event.button = _toMouseButton(e->detail);
				event.modKey = _toModKey(e->state);
				event.window = _queryWindowByFrame(e->child);
				observer->onMouseRelease(event);
				break;
			}

			case XCB_MOTION_NOTIFY: {
				xcb_motion_notify_event_t* e = (xcb_motion_notify_event_t*) ev;
				LOG("[x11] motion notify %d, %d\n", e->root_x, e->root_y);

				MouseMovementEvent event;
				event.pos.x = e->root_x;
				event.pos.y = e->root_y;
				observer->onMouseMove(event);
				break;
			}

			case XCB_KEY_PRESS: {
				xcb_key_press_event_t* e = (xcb_key_press_event_t*) ev;
				Key* key = _codeToKey.getOrNull(e->detail);
				if (key == nullptr) {
					break;
				}
				LOG("[x11] key press %d\n", e->detail);
				KeyEvent event;
				event.modKey = _toModKey(e->state);
				event.key = *key;

				observer->onKeyPress(event);
				break;
			}

			case XCB_KEY_RELEASE: {
				xcb_key_release_event_t* e = (xcb_key_release_event_t*) ev;
				if (_grabbingWholeKeyboard) {
					if (WX_FLAGS_CONTAIN(_grabbingModKey, ModKey::Shift)) {
						if (_keyboardManager->queryKeyCode(Key::ShiftL) == e->detail ||
							_keyboardManager->queryKeyCode(Key::ShiftR) == e->detail) {
							_grabbingModKey &= ~ModKey::Shift;
						}
					}
					if (WX_FLAGS_CONTAIN(_grabbingModKey, ModKey::Ctrl)) {
						if (_keyboardManager->queryKeyCode(Key::CtrlL) == e->detail ||
							_keyboardManager->queryKeyCode(Key::CtrlR) == e->detail) {
							_grabbingModKey &= ~ModKey::Ctrl;
						}
					}
					if (WX_FLAGS_CONTAIN(_grabbingModKey, ModKey::Alt)) {
						if (_keyboardManager->queryKeyCode(Key::AltL) == e->detail ||
							_keyboardManager->queryKeyCode(Key::AltR) == e->detail) {
							_grabbingModKey &= ~ModKey::Alt;
						}
					}
					if (WX_FLAGS_CONTAIN(_grabbingModKey, ModKey::Super)) {
						if (_keyboardManager->queryKeyCode(Key::SuperL) == e->detail ||
							_keyboardManager->queryKeyCode(Key::SuperR) == e->detail) {
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
					observer->onKeyRelease(event);
					_grabbingModKey = _grabbingModKeyOriginal;
					break;
				}
				LOG("[x11] key release %d\n", e->detail);
				Key* key = _codeToKey.getOrNull(e->detail);
				if (key == nullptr) {
					break;
				}
				KeyEvent event;
				event.modKey = _toModKey(e->state);
				event.key = *key;
				observer->onKeyRelease(event);
				break;
			}

			case XCB_CLIENT_MESSAGE: {
				xcb_client_message_event_t* e = (xcb_client_message_event_t*) ev;
				LOG("[x11] client message %d\n", e->window);

				Window* window = _queryWindowByClient(e->window);
				if (window == nullptr) {
					LOG("[x11] unmanaged window id message request %d (%s)\n", e->window, _hint->atomToString(e->type).cptr());
					break;
				}

				if (_hint->isActiveWindow(e->type)) {
					// SetActiveWindow(window);
					// Context::Sync();
					break;
				}

				if (_hint->isWindowState(e->type)) {
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

				if (_hint->isMoveResizeProperty(e->type)) {
					Point cursor;
					cursor.x = e->data.data32[0];
					cursor.y = e->data.data32[1];
					int direction = e->data.data32[2];
					uint32_t button = e->data.data32[3];

					if (direction == 0) {
						WindowResizingEvent event;
						event.window = window;
						event.area = WindowResizingArea::NorthWest;
						observer->onWindowResizeRequest(event);
						break;
					}
					if (direction == 1) {
						WindowResizingEvent event;
						event.window = window;
						event.area = WindowResizingArea::North;
						observer->onWindowResizeRequest(event);
						break;
					}
					if (direction == 2) {
						WindowResizingEvent event;
						event.window = window;
						event.area = WindowResizingArea::NorthEast;
						observer->onWindowResizeRequest(event);
						break;
					}
					if (direction == 3) {
						WindowResizingEvent event;
						event.window = window;
						event.area = WindowResizingArea::East;
						observer->onWindowResizeRequest(event);
						break;
					}
					if (direction == 4) {
						WindowResizingEvent event;
						event.window = window;
						event.area = WindowResizingArea::SouthEast;
						observer->onWindowResizeRequest(event);
						break;
					}
					if (direction == 5) {
						WindowResizingEvent event;
						event.window = window;
						event.area = WindowResizingArea::South;
						observer->onWindowResizeRequest(event);
						break;
					}
					if (direction == 6) {
						WindowResizingEvent event;
						event.window = window;
						event.area = WindowResizingArea::SouthWest;
						observer->onWindowResizeRequest(event);
						break;
					}
					if (direction == 7) {
						WindowResizingEvent event;
						event.window = window;
						event.area = WindowResizingArea::West;
						observer->onWindowResizeRequest(event);
						break;
					}
					if (direction == 8) {
						WindowMovementEvent event;
						event.window = window;
						event.offset = cursor;
						observer->onWindowMoveRequest(event);
						break;
					}
				}

				LOG("uncaptured client message %d, %s\n", e->type, _hint->atomToString(e->type).cptr());
				break;
			}


			default: {
				if (type == _keyboardManager->getEventOffset()) {
					switch (ev->pad0) {
						case XCB_XKB_NEW_KEYBOARD_NOTIFY: {
							xcb_xkb_new_keyboard_notify_event_t* event = (xcb_xkb_new_keyboard_notify_event_t*) ev;
							if (event->changed & XCB_XKB_NKN_DETAIL_KEYCODES) {
								_updateKeyBindings();
							}
							break;
						}
						case XCB_XKB_MAP_NOTIFY:
							_updateKeyBindings();
							break;
					}
					break;
				}
				if (type == _monitorManager->getEventOffset() + XCB_RANDR_SCREEN_CHANGE_NOTIFY) {
					_updateMonitors(observer);
					break;
				}

				if (type == _monitorManager->getEventOffset() + XCB_RANDR_NOTIFY) {
					xcb_randr_notify_event_t* event = (xcb_randr_notify_event_t *) ev;
					if (event->subCode == XCB_RANDR_NOTIFY_OUTPUT_CHANGE) {
						xcb_randr_output_change_t oc = event->u.oc;
						if (oc.connection == XCB_RANDR_CONNECTION_CONNECTED) {
							_updateMonitors(observer);
						} else if (oc.connection == XCB_RANDR_CONNECTION_DISCONNECTED) {
							_updateMonitors(observer);
						}
					}
					if (event->subCode == XCB_RANDR_NOTIFY_CRTC_CHANGE) {
						xcb_randr_crtc_change_t cc = event->u.cc;
						_updateMonitors(observer);
					}
					break;
				}
				break;
			}
		}
		free(ev);
	}
}

void X11Context::stop() {
	_running = false;
}

void X11Context::_unmanagedWindowList(IObserver* observer) {
	xcb_query_tree_cookie_t cookie = xcb_query_tree(_conn->get(), _conn->getRoot());
	xcb_query_tree_reply_t *tree = xcb_query_tree_reply(_conn->get(), cookie, NULL);

	if (!tree)
		return;

	int n = xcb_query_tree_children_length(tree);
	xcb_window_t *children = xcb_query_tree_children(tree);

	for (int i = 0; i < n; i++) {
		xcb_window_t win = children[i];

		xcb_get_window_attributes_cookie_t ac = xcb_get_window_attributes(_conn->get(), win);
		xcb_get_window_attributes_reply_t *attr = xcb_get_window_attributes_reply(_conn->get(), ac, NULL);

		if (!attr)
			continue;

		if (attr->override_redirect) {
			free(attr);
			continue;
		}

		if (attr->map_state == XCB_MAP_STATE_VIEWABLE) {
			Window* window = _queryWindowByClient(win);
			if (!window) {
				X11WindowBackend* backend = new X11WindowBackend(_conn, _hint, win);
				window = new Window(backend);
				WindowEvent event;
				event.window = window;

				_clientWindowMap.set(win, window);
				_frameWindowMap.set(backend->getFrameHandle(), window);

				observer->onWindowCreate(event);
			}
		}

		free(attr);
	}

	free(tree);
}

Window* X11Context::_queryWindowByFrame(xcb_window_t handle) {
	if (handle == XCB_NONE) return nullptr;
	Window** window = _frameWindowMap.getOrNull(handle);
	if (window != nullptr) return *window;
	return nullptr;
}

Window* X11Context::_queryWindowByClient(xcb_window_t handle) {
	if (handle == XCB_NONE) return nullptr;
	Window** window = _clientWindowMap.getOrNull(handle);
	if (window != nullptr) return *window;
	return nullptr;
}

int X11Context::_fromModKey(ModKey modKey) {
	wx::Uint32 flags = 0;

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

ModKey X11Context::_toModKey(int modKey) {
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

int X11Context::_fromMouseButton(MouseButton button) {
	return (int)button;
}

MouseButton X11Context::_toMouseButton(int button) {
	return (MouseButton)button;
}

void X11Context::_attachKeyBindings() {
	_codeToKey.clear();

	for (int i = 0; i < _keyBindings.count(); i++) {
		X11KeyBinding& binding = _keyBindings[i];
		binding.keyCode = _keyboardManager->queryKeyCode(binding.key);
		binding.modKeyCode = _fromModKey(binding.modKey);
		if (binding.keyCode == 0) {
			LOG("ERROR: key %d is invalid\n", (int)binding.key);
			continue;
		}

		_codeToKey.set(binding.keyCode, binding.key);

		xcb_grab_key(
			_conn->get(), 0, _conn->getScreen()->root, 
			binding.modKeyCode,
			binding.keyCode,
			XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC
		);
	}
	_conn->flush();
}

void X11Context::_detachKeyBindings() {
	for (int i = 0; i < _keyBindings.count(); i++) {
		X11KeyBinding& binding = _keyBindings[i];
		if (binding.keyCode == 0) continue;

		xcb_ungrab_key(
			_conn->get(),
			binding.keyCode,
			_conn->getScreen()->root,
			binding.modKeyCode
		);
	}
	_conn->flush();
}

void X11Context::_updateKeyBindings() {
	_keyboardManager->updateKeymap();
	wx::String layout = _keyboardManager->getLayout();

	if (layout == _activeKeyboardLayout) {
		return;
	}

	_detachKeyBindings();
	_attachKeyBindings();

	LOG("changing keyboard layout to %s\n", layout.cptr());
	_activeKeyboardLayout = layout;
}

void X11Context::_grabWholeKeyboard() {
	/*
     * Now take an active keyboard grab.
     *
     * From this point until ungrab, keyboard events are directed
     * to our WM.
     */
    xcb_grab_keyboard_cookie_t cookie = xcb_grab_keyboard(
		_conn->get(),
		0,                      // owner_events
		_conn->getRoot(),
		XCB_CURRENT_TIME,
		XCB_GRAB_MODE_ASYNC,
		XCB_GRAB_MODE_ASYNC
	);

    xcb_grab_keyboard_reply_t *reply = xcb_grab_keyboard_reply(_conn->get(), cookie, NULL);

    if (!reply) {
        return;
    }

    if (reply->status != XCB_GRAB_STATUS_SUCCESS) {
    }
    free(reply);
}

void X11Context::_ungrabWholeKeyboard() {
	xcb_ungrab_keyboard(_conn->get(), XCB_CURRENT_TIME);
	_conn->flush();
}

MonitorDef* X11Context::_queryMonitorDefByPort(wx::String const& port, wx::List<MonitorDef>& defs) {
	for (int i = 0; i < defs.count(); i++) {
		if (defs[i].port == port) return &defs[i];
	}
	return nullptr;
}

void X11Context::_updateMonitors(IObserver* observer) {
	wx::List<MonitorDef> currentMonitorDefs = _monitorManager->getConnectedMonitorDefs();

	wx::List<wx::String> connectedPorts;
	wx::List<wx::String> disconnectedPorts;
	wx::List<wx::String> propertyChangedPorts;
	wx::List<wx::String> stayPorts;

	for (int i = 0; i < currentMonitorDefs.count(); i++) {
		MonitorDef const& current = currentMonitorDefs[i];
		MonitorDef* old = _queryMonitorDefByPort(current.port, _monitorDefs);
		if (old == nullptr) {
			connectedPorts.append(current.port);
		} else {
			if (old->rect.x != current.rect.x ||
				old->rect.y != current.rect.y ||
				old->rect.width != current.rect.width ||
				old->rect.height != current.rect.height) {
				propertyChangedPorts.append(current.port);
			}
			stayPorts.append(current.port);
		}
	}
	for (int i = 0; i < _monitorDefs.count(); i++) {
		MonitorDef const& old = _monitorDefs[i];
		MonitorDef* current = _queryMonitorDefByPort(old.port, currentMonitorDefs);
		if (current == nullptr) {
			disconnectedPorts.append(old.port);
		}
	}

	for (int i = 0; i < connectedPorts.count(); i++) {
		MonitorEvent event;
		event.port = connectedPorts[i];
		observer->onMonitorConnect(event);
	}

	for (int i = 0; i < disconnectedPorts.count(); i++) {
		MonitorEvent event;
		event.port = disconnectedPorts[i];
		observer->onMonitorDisconnect(event);
	}

	for (int i = 0; i < propertyChangedPorts.count(); i++) {
		MonitorEvent event;
		event.port = propertyChangedPorts[i];
		observer->onMonitorPropertyChange(event);
	}

	_monitorDefs = static_cast<wx::List<MonitorDef>&&>(currentMonitorDefs);
}

IContext* createContext() {
	return new X11Context();
}

