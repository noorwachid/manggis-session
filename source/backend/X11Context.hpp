#pragma once

#include "Event.hpp"
#include "IContext.hpp"
#include "IObserver.hpp"
#include "backend/X11Connection.hpp"
#include "backend/X11Window.hpp"
#include "wx/core/HashTable.hpp"
#include <cstdlib>

class X11Context: public IContext {
public:
	X11Context() {
		_conn = new X11Connection();
	}

	virtual void bindMouseButton(MouseButton button, ModKey modKey) override {
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
				case XCB_BUTTON_PRESS: {
					xcb_button_press_event_t* e = (xcb_button_press_event_t *) ev;
					MouseButtonEvent event;
					event.pos.x = e->root_x;
					event.pos.y = e->root_y;
					event.button = _toMouseButton(e->detail);
					event.modKey = _toModKey(e->state);
					event.window = _getWindow(e->child);
					observer->onMousePress(event);
					break;
				}

				case XCB_BUTTON_RELEASE: {
					xcb_button_release_event_t* e = (xcb_button_release_event_t *) ev;
					MouseButtonEvent event;
					event.pos.x = e->root_x;
					event.pos.y = e->root_y;
					event.button = _toMouseButton(e->detail);
					event.modKey = _toModKey(e->state);
					event.window = _getWindow(e->child);
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
	X11Window* _getWindow(xcb_window_t handle) {
		if (handle == XCB_NONE) return nullptr;
		X11Window** window = _windowMap.getOrNull(handle);
		if (window != nullptr) return *window;
		X11Window* newWindow = new X11Window(_conn, handle);
		_windowMap.set(handle, newWindow);
		return newWindow;
	}

	int _fromModKey(ModKey modKey) {
		int flags = 0;
		switch (modKey) {
			case ModKey::None:
				break;
			case ModKey::Shift: 
				flags |= XCB_MOD_MASK_SHIFT;
				break;
			case ModKey::Ctrl:
				flags |= XCB_MOD_MASK_CONTROL;
				break;
			case ModKey::Alt: 
				flags |= XCB_MOD_MASK_1;
				break;
			case ModKey::Super: 
				flags |= XCB_MOD_MASK_4;
				break;
		}
		return flags;
	}

	ModKey _toModKey(int modKey) {
		ModKey flags = ModKey::None;
		switch (modKey) {
			case XCB_MOD_MASK_SHIFT: 
				flags |= ModKey::Shift;
				break;
			case XCB_MOD_MASK_CONTROL:
				flags |= ModKey::Ctrl;
				break;
			case XCB_MOD_MASK_1:
				flags |= ModKey::Alt;
				break;
			case XCB_MOD_MASK_4: 
				flags |= ModKey::Super;
				break;
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
	wx::HashTable<xcb_window_t, X11Window*> _windowMap;
};
