#pragma once

#include "Event.hpp"
#include "IContext.hpp"
#include "IObserver.hpp"
#include "MonitorInfo.hpp"
#include "Window.hpp"
#include "backend/X11Connection.hpp"
#include "backend/X11KeyboardManager.hpp"
#include "backend/X11MonitorManager.hpp"
#include "backend/X11WindowBackend.hpp"
#include "input/Key.hpp"
#include "input/MouseButton.hpp"
#include "wx/core/List.hpp"
#include "wx/core/HashTable.hpp"
#include "wx/core/String.hpp"
#include <cstdio>
#include <cstdlib>
#include <xcb/randr.h>
#include <xcb/xproto.h>

struct X11KeyBinding {
	ModKey modKey;
	Key key;
	xcb_keycode_t keyCode = 0;
	int modKeyCode = 0;
};

class X11Context: public IContext {
public:
	X11Context() {
		_conn = new X11Connection();

		int mask = (
			XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT |
			XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
			XCB_EVENT_MASK_PROPERTY_CHANGE |
			XCB_EVENT_MASK_BUTTON_PRESS
		);
		xcb_change_window_attributes(_conn->get(), _conn->getRoot(), XCB_CW_EVENT_MASK, &mask);

		_keyboardManager = new X11KeyboardManager(_conn);
		_keyboardManager->initialize();

		_monitorManager = new X11MonitorManager(_conn);
		_monitorManager->initialize();

		// NOTE: click to focus
		xcb_grab_button(
			_conn->get(), 0, _conn->getScreen()->root, 
			XCB_EVENT_MASK_BUTTON_PRESS,
			XCB_GRAB_MODE_SYNC, XCB_GRAB_MODE_ASYNC, 
			XCB_NONE, XCB_NONE, 1, XCB_NONE
		);
	}

	virtual ~X11Context() override {
		delete _keyboardManager;
		delete _conn;
	}

	virtual void bindMouseButton(ModKey modKey, MouseButton button) override {
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

	virtual void unbindMouseButton(ModKey modKey, MouseButton button) override {
		xcb_ungrab_button(_conn->get(), _fromMouseButton(button), _conn->getRoot(), _fromModKey(modKey));
		_conn->flush();
	}

	virtual void bindMouseMovement() override {
		xcb_grab_pointer(_conn->get(), 0, _conn->getRoot(), 
			XCB_EVENT_MASK_BUTTON_PRESS | 
			XCB_EVENT_MASK_BUTTON_RELEASE | 
			XCB_EVENT_MASK_BUTTON_MOTION,
			XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, _conn->getRoot(), XCB_NONE, XCB_CURRENT_TIME
		);
		_conn->flush();
	}

	virtual void unbindMouseMovement() override {
		xcb_ungrab_pointer(_conn->get(), XCB_CURRENT_TIME);
		_conn->flush();
	}

	virtual void bindKey(ModKey modKey, Key key) override {
		X11KeyBinding binding;
		binding.modKey = modKey;
		binding.key = key;
		_keyBindings.append(binding);

		
	}

	virtual void unbindKey(ModKey modKey, Key key) override {
	}

	virtual Point getMousePos() override {
		xcb_query_pointer_reply_t* pointer = xcb_query_pointer_reply(_conn->get(), xcb_query_pointer(_conn->get(), _conn->getRoot()), 0);
		Point pos;
		pos.x = pointer->root_x;
		pos.y = pointer->root_y;
		free(pointer);
		return pos;
	}

	virtual wx::List<MonitorInfo> getConnectedMonitorInfos() override {
		return _monitorManager->getConnectedMonitorInfos();
	}

	virtual void listen(IObserver* observer) override {
		_attachKeyBindings();
		_updateMonitors(observer);

		while (true) {
			xcb_generic_event_t* ev = xcb_wait_for_event(_conn->get());
			int type = ev->response_type & ~0x80;
			switch (type) {
				case XCB_CREATE_NOTIFY: {
					break;
				}

				case XCB_MAP_REQUEST: {
					xcb_map_request_event_t* e = (xcb_map_request_event_t*) ev;

					Window* window = _getWindowByClient(e->window);
					if (window == nullptr) {
						X11WindowBackend* backend = new X11WindowBackend(_conn, e->window);
						window = new Window(backend, WindowFlags::None);
						WindowEvent event;
						event.window = window;

						_clientWindowMap.set(e->window, window);
						_frameWindowMap.set(backend->getFrameHandle(), window);

						observer->onWindowCreate(event);
					}
					window->show();
					break;
				}

				case XCB_MAP_NOTIFY: {
					break;
				}

				case XCB_CONFIGURE_REQUEST: {
					xcb_configure_request_event_t* e = (xcb_configure_request_event_t*) ev;
					uint16_t mask = 0;
					uint32_t values[7];
					uint32_t i = 0;

					Window* window = _getWindowByClient(e->window);
					if (window != nullptr) {
						bool moved = false;
						if (e->value_mask & XCB_CONFIG_WINDOW_X) {
							moved = true;
						}
						if (e->value_mask & XCB_CONFIG_WINDOW_Y) {
							moved = true;
						}
						if (moved) {
							window->move(e->x, e->y);
						}
						_conn->flush();
						break;
					}

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
					break;
				}

				case XCB_UNMAP_NOTIFY: {
					xcb_unmap_notify_event_t* e = (xcb_unmap_notify_event_t*) ev;
					Window* window = _getWindowByClient(e->window);
					if (window == nullptr) {
						break;
					}
					window->hide();
					break;
				}

				case XCB_DESTROY_NOTIFY: {
					xcb_destroy_notify_event_t* e = (xcb_destroy_notify_event_t*) ev;
					Window* window = _getWindowByClient(e->window);
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
					Window* window = _getWindowByFrame(e->window);
					if (window == nullptr) {
						break;
					}
					((X11WindowBackend*)window->getBackend())->draw();
					break;
				}

				case XCB_BUTTON_PRESS: {
					printf("mouse press\n");

					xcb_button_press_event_t* e = (xcb_button_press_event_t *) ev;
					MouseButtonEvent event;
					event.pos.x = e->root_x;
					event.pos.y = e->root_y;
					event.button = _toMouseButton(e->detail);
					event.modKey = _toModKey(e->state);
					event.window = _getWindowByFrame(e->child);

					xcb_allow_events(_conn->get(), XCB_ALLOW_REPLAY_POINTER, e->time);
					_conn->flush();

					observer->onMousePress(event);
					break;
				}

				case XCB_BUTTON_RELEASE: {
					printf("mouse release\n");

					xcb_button_release_event_t* e = (xcb_button_release_event_t *) ev;
					MouseButtonEvent event;
					event.pos.x = e->root_x;
					event.pos.y = e->root_y;
					event.button = _toMouseButton(e->detail);
					event.modKey = _toModKey(e->state);
					event.window = _getWindowByFrame(e->child);
					observer->onMouseRelease(event);
					break;
				}

				case XCB_MOTION_NOTIFY: {
					xcb_motion_notify_event_t* e = (xcb_motion_notify_event_t*) ev;
					MouseMovementEvent event;
					event.pos.x = e->root_x;
					event.pos.y = e->root_y;
					observer->onMouseMove(event);
					break;
				}

				case XCB_KEY_PRESS: {
					xcb_key_press_event_t* e = (xcb_key_press_event_t*) ev;
					Key* key = _keyCodeMap.getOrNull(e->detail);
					if (key == nullptr) {
						break;
					}
					KeyEvent event;
					event.modKey = _toModKey(e->state);
					event.key = *key;
					observer->onKeyPress(event);
					break;
				}

				case XCB_KEY_RELEASE: {
					xcb_key_release_event_t* e = (xcb_key_release_event_t*) ev;
					Key* key = _keyCodeMap.getOrNull(e->detail);
					if (key == nullptr) {
						break;
					}
					KeyEvent event;
					event.modKey = _toModKey(e->state);
					event.key = *key;
					observer->onKeyRelease(event);
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

private:
	Window* _getWindowByFrame(xcb_window_t handle) {
		if (handle == XCB_NONE) return nullptr;
		Window** window = _frameWindowMap.getOrNull(handle);
		if (window != nullptr) return *window;
		return nullptr;
	}

	Window* _getWindowByClient(xcb_window_t handle) {
		if (handle == XCB_NONE) return nullptr;
		Window** window = _clientWindowMap.getOrNull(handle);
		if (window != nullptr) return *window;
		return nullptr;
	}

	int _fromModKey(ModKey modKey) {
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

	ModKey _toModKey(int modKey) {
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

	int _fromMouseButton(MouseButton button) {
		return (int)button;
	}

	MouseButton _toMouseButton(int button) {
		return (MouseButton)button;
	}

	void _attachKeyBindings() {
		_keyCodeMap.clear();

		for (int i = 0; i < _keyBindings.count(); i++) {
			X11KeyBinding& binding = _keyBindings[i];
			binding.keyCode = _keyboardManager->queryKeyCode(binding.key);
			binding.modKeyCode = _fromModKey(binding.modKey);
			if (binding.keyCode == 0) {
				printf("ERROR: key %d is invalid\n", (int)binding.key);
				continue;
			}

			_keyCodeMap.set(binding.keyCode, binding.key);

			xcb_grab_key(
				_conn->get(), 0, _conn->getScreen()->root, 
				binding.modKeyCode,
				binding.keyCode,
				XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC
			);
		}
		_conn->flush();
	}

	void _detachKeyBindings() {
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

	void _updateKeyBindings() {
		_keyboardManager->updateKeymap();
		wx::String layout = _keyboardManager->getLayout();

		if (layout == _activeKeyboardLayout) {
			return;
		}

		_detachKeyBindings();
		_attachKeyBindings();

		printf("changing keyboard layout to %s\n", layout.cptr());
		_activeKeyboardLayout = layout;
	}

	MonitorInfo* _getMonitorInfoByPort(wx::String const& port, wx::List<MonitorInfo>& infos) {
		for (int i = 0; i < infos.count(); i++) {
			if (infos[i].port == port) return &infos[i];
		}
		return nullptr;
	}

	void _updateMonitors(IObserver* observer) {
		wx::List<MonitorInfo> currentMonitorInfos = _monitorManager->getConnectedMonitorInfos();

		wx::List<wx::String> connectedPorts;
		wx::List<wx::String> disconnectedPorts;
		wx::List<wx::String> propertyChangedPorts;
		wx::List<wx::String> stayPorts;

		for (int i = 0; i < currentMonitorInfos.count(); i++) {
			MonitorInfo const& current = currentMonitorInfos[i];
			MonitorInfo* old = _getMonitorInfoByPort(current.port, _monitorInfos);
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
		for (int i = 0; i < _monitorInfos.count(); i++) {
			MonitorInfo const& old = _monitorInfos[i];
			MonitorInfo* current = _getMonitorInfoByPort(old.port, currentMonitorInfos);
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

		_monitorInfos = static_cast<wx::List<MonitorInfo>&&>(currentMonitorInfos);
	}

private:
	X11Connection* _conn;
	X11KeyboardManager* _keyboardManager;
	wx::HashTable<xcb_window_t, Window*> _clientWindowMap;
	wx::HashTable<xcb_window_t, Window*> _frameWindowMap;

	wx::List<X11KeyBinding> _keyBindings;
	wx::String _activeKeyboardLayout;
	wx::HashTable<xcb_keycode_t, Key> _keyCodeMap;

	X11MonitorManager* _monitorManager;
	wx::List<MonitorInfo> _monitorInfos;
};
