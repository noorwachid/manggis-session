#include "backend/X11Clipper.hpp"
#include "core/Logger.hpp"

X11Clipper::X11Clipper(X11Connection* conn): _conn{conn} {
	const xcb_query_extension_reply_t *ext = xcb_get_extension_data(_conn->get(), &xcb_shape_id);
	if (!ext || !ext->present) {
		LOG("[X11Clipper] shape extension not available\n");
		return;
	}
}

void X11Clipper::clip(X11WindowBackend* windowBackend, Rect rect) {
	WindowProp prop = windowBackend->getProp();

	xcb_rectangle_t x11Rect;
	x11Rect.x = rect.x;
	x11Rect.y = rect.y;
	x11Rect.width = rect.width;
	x11Rect.height = rect.height;

	LOG("[x11.clipper] %d, %d, %d, %d\n", rect.x, rect.y, rect.width, rect.height);

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

void X11Clipper::unclip(X11WindowBackend* windowBackend) {
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

