#pragma once

#include "Common.hpp"
#include "IWindow.hpp"
#include "backend/X11Connection.hpp"
#include "wx/core/Numeric.hpp"
#include <cstdlib>

class X11Window: public IWindow {
public:
	X11Window(X11Connection* conn, xcb_window_t handle): _conn{conn}, _client{handle} {
		_reframe();
	}

	virtual Rect getRect() override {
		xcb_get_geometry_reply_t* geom = xcb_get_geometry_reply(_conn->get(), xcb_get_geometry(_conn->get(), _frame), NULL);
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
		xcb_configure_window(_conn->get(), _frame, XCB_CONFIG_WINDOW_STACK_MODE, &value);
		_conn->flush();
	}

	virtual void move(int x, int y) override {
		uint32_t values[2];
		values[0] = x;
		values[1] = y;
		xcb_configure_window(_conn->get(), _frame, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, values);
		_conn->flush();
	}

	virtual void resize(int x, int y) override {
		uint32_t values[2];
		values[0] = x > 0 ? x : 0;
		values[1] = y > 0 ? y : 0;
		xcb_configure_window(_conn->get(), _frame, XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, values);
		_conn->flush();
	}

	virtual void show() override {
		xcb_map_window(_conn->get(), _frame);
		_conn->flush();
	}

	virtual void hide() override {
		xcb_unmap_window(_conn->get(), _frame);
		_conn->flush();
	}

	xcb_window_t getFrameHandle() {
		return _frame;
	}

private:
	void _reframe() {
		_frame = xcb_generate_id(_conn->get());

		uint32_t borderColor = 0xDF1238;
		uint32_t borderWidth = 1;
		uint32_t titlebarColor = 0xFF3258; 
		uint32_t titlebarHeight = 20; 


		uint32_t mask = XCB_CW_BORDER_PIXEL | XCB_CW_EVENT_MASK;
		uint32_t values[] = {
			borderColor,
			(
				XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | 
				XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
				XCB_EVENT_MASK_EXPOSURE
			)
		};

		xcb_get_geometry_reply_t* geom = xcb_get_geometry_reply(_conn->get(), xcb_get_geometry(_conn->get(), _client), NULL);

		int x = geom->x;
		int y = geom->y;
		int width = geom->width;
		int height = geom->height + titlebarHeight;

		free(geom);

		xcb_create_window(_conn->get(), _conn->getScreen()->root_depth, _frame, _conn->getScreen()->root,
			x, 
			y,
			width,
			height,
			borderWidth, XCB_WINDOW_CLASS_INPUT_OUTPUT,
			_conn->getScreen()->root_visual, mask, values
		);

		// _gc = Context::CreateGC(_id);

		int clientBorderWidth = 0;
		xcb_configure_window(_conn->get(), _client, XCB_CONFIG_WINDOW_BORDER_WIDTH, &clientBorderWidth);
		xcb_reparent_window(_conn->get(), _client, _frame, 0, titlebarHeight);
		xcb_map_window(_conn->get(), _client);
		_conn->flush();
	}

private:
	X11Connection* _conn;
	xcb_window_t _client;
	xcb_window_t _frame;
};



