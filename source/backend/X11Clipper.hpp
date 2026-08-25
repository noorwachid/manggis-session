#pragma once

#include "backend/X11Connection.hpp"
#include "backend/X11WindowBackend.hpp"
#include <cstdio>
#include <xcb/shape.h>
#include <xcb/xproto.h>

class X11Clipper {
public:
	X11Clipper(X11Connection* conn): _conn{conn} {
	}

	void initialize() {
		const xcb_query_extension_reply_t *ext = xcb_get_extension_data(_conn->get(), &xcb_shape_id);
		if (!ext || !ext->present) {
			printf("[X11Clipper] shape extension not available\n");
			return;
		}
	}

	void clip(X11WindowBackend* windowBackend, Rect rect) {
		WindowProp prop = windowBackend->getProp();

		xcb_rectangle_t x11Rect;
		x11Rect.x = rect.x - prop.borderWidth;
		x11Rect.y = rect.y - prop.borderWidth;
		x11Rect.width = rect.width + prop.borderWidth * 2;
		x11Rect.height = rect.height + prop.borderWidth * 2;

		xcb_shape_rectangles(
			_conn->get(),
			XCB_SHAPE_SO_SET,
			XCB_SHAPE_SK_BOUNDING,
			0,
			windowBackend->getFrameHandle(),
			0, 
			0, 
			1,
			&x11Rect
		);
		_conn->flush();
	}

	void unclip(X11WindowBackend* windowBackend) {
		xcb_shape_mask(
			_conn->get(),
			XCB_SHAPE_SO_SET,
			XCB_SHAPE_SK_BOUNDING,
			windowBackend->getFrameHandle(),
			0,
			0,
			XCB_NONE
		);
		_conn->flush();
	}

private:
	X11Connection* _conn;
};
