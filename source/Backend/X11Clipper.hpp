#pragma once

#include "Common/Logger.hpp"
#include "Backend/X11Connection.hpp"
#include "Backend/X11WindowBackend.hpp"
#include <xcb/shape.h>
#include <xcb/xproto.h>

class X11Clipper {
public:
	X11Clipper(X11Connection* conn): _conn{conn} {
	}

	void Init() {
		const xcb_query_extension_reply_t *ext = xcb_get_extension_data(_conn->Get(), &xcb_shape_id);
		if (!ext || !ext->present) {
			LOG("[X11Clipper] shape extension not available\n");
			return;
		}
	}

	void Clip(X11WindowBackend* windowBackend, Rect rect) {
		WindowProp prop = windowBackend->GetProp();

		xcb_rectangle_t x11Rect;
		x11Rect.x = rect.x;
		x11Rect.y = rect.y;
		x11Rect.width = rect.width;
		x11Rect.height = rect.height;

		xcb_shape_rectangles(
			_conn->Get(),
			XCB_SHAPE_SO_SET,
			XCB_SHAPE_SK_BOUNDING,
			0,
			windowBackend->getFrameHandle(),
			0, 
			0, 
			1,
			&x11Rect
		);
		_conn->Flush();
	}

	void Unclip(X11WindowBackend* windowBackend) {
		xcb_shape_mask(
			_conn->Get(),
			XCB_SHAPE_SO_SET,
			XCB_SHAPE_SK_BOUNDING,
			windowBackend->getFrameHandle(),
			0,
			0,
			XCB_NONE
		);
		_conn->Flush();
	}

private:
	X11Connection* _conn;
};
