#pragma once

#include "Event.hpp"
#include "IContext.hpp"
#include "IObserver.hpp"
#include "backend/X11Connection.hpp"
#include "backend/X11Window.hpp"
#include "input/Key.hpp"
#include "input/MouseButton.hpp"
#include "wx/core/HashTable.hpp"
#include <cstdio>
#include <cstdlib>
#include <xcb/xproto.h>

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

	}

	virtual ~X11Context() override {
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

	virtual void listen(IObserver* observer) override {
		while (true) {
			xcb_generic_event_t* ev = xcb_wait_for_event(_conn->get());
			switch (ev->response_type & ~0x80) {
				case XCB_CREATE_NOTIFY: {
					break;
				}

				case XCB_MAP_REQUEST: {
					xcb_map_request_event_t* e = (xcb_map_request_event_t*) ev;

					X11Window* window = _getWindowByClient(e->window);
					if (window == nullptr) {
						window = new X11Window(_conn, e->window);
						WindowEvent event;
						event.window = window;

						_clientWindowMap.set(e->window, window);
						_frameWindowMap.set(window->getFrameHandle(), window);

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

					X11Window* window = _getWindowByClient(e->window);
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
					X11Window* window = _getWindowByClient(e->window);
					if (window == nullptr) {
						break;
					}
					window->hide();
					break;
				}

				case XCB_DESTROY_NOTIFY: {
					xcb_destroy_notify_event_t* e = (xcb_destroy_notify_event_t*) ev;
					X11Window* window = _getWindowByClient(e->window);
					if (window == nullptr) {
						break;
					}
					WindowEvent event;
					event.window = window;
					observer->onWindowDestroy(event);

					// TODO: implement hashtable remove
					// _clientWindowMap.remove(e->window);
					// _frameWindowMap.remove(window->getFrameHandle());

					delete window;
					break;
				}

				case XCB_EXPOSE: {
					xcb_expose_event_t* e = (xcb_expose_event_t*) ev;
					X11Window* window = _getWindowByFrame(e->window);
					if (window == nullptr) {
						break;
					}
					window->draw();
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

			}
			free(ev);
		}
	}

private:
	X11Window* _getWindowByFrame(xcb_window_t handle) {
		if (handle == XCB_NONE) return nullptr;
		X11Window** window = _frameWindowMap.getOrNull(handle);
		if (window != nullptr) return *window;
		return nullptr;
	}

	X11Window* _getWindowByClient(xcb_window_t handle) {
		if (handle == XCB_NONE) return nullptr;
		X11Window** window = _clientWindowMap.getOrNull(handle);
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

private:
	X11Connection* _conn;
	wx::HashTable<xcb_window_t, X11Window*> _clientWindowMap;
	wx::HashTable<xcb_window_t, X11Window*> _frameWindowMap;
};
