#pragma once

#include "Common.hpp"
#include "IWindow.hpp"
#include "backend/X11Connection.hpp"
#include <cstdlib>

class X11Window: public IWindow {
public:
	X11Window(X11Connection* conn, xcb_window_t handle): _conn{conn}, _handle{handle} {
	}

	virtual Rect getRect() override {
		xcb_get_geometry_reply_t* geom = xcb_get_geometry_reply(_conn->get(), xcb_get_geometry(_conn->get(), _handle), NULL);
		Rect rect;
		rect.x = geom->x;
		rect.y = geom->y;
		rect.width = geom->width;
		rect.height = geom->height;
		free(geom);
		return rect;
	}

	virtual void raise() override {
		uint32_t value = XCB_STACK_MODE_ABOVE;
		xcb_configure_window(_conn->get(), _handle, XCB_CONFIG_WINDOW_STACK_MODE, &value);
		_conn->flush();
	}

	virtual void move(int x, int y) override {
		uint32_t values[2];
		values[0] = x;
		values[1] = y;
		xcb_configure_window(_conn->get(), _handle, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, values);
		_conn->flush();
	}

	virtual void resize(int x, int y) override {
		uint32_t values[2];
		values[0] = x > 0 ? x : 0;
		values[1] = y > 0 ? y : 0;
		xcb_configure_window(_conn->get(), _handle, XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, values);
		_conn->flush();
	}


private:
	X11Connection* _conn;
	xcb_window_t _handle;
};



