#include "backend/X11WindowBackend.hpp"
#include "backend/IWindowBackend.hpp"
#include "core/Logger.hpp"
#include "WindowStyle.hpp"

X11WindowBackend::X11WindowBackend(X11Connection* conn, X11Hint* hint, xcb_window_t handle): _conn{conn}, _hint{hint}, _client{handle} {
	_state = WindowState::None;
	_reparent();
}

Rect X11WindowBackend::getRect() {
	xcb_get_geometry_reply_t* geom = xcb_get_geometry_reply(_conn->get(), xcb_get_geometry(_conn->get(), _frame), NULL);
	Rect rect;
	rect.x = geom->x;
	rect.y = geom->y;
	rect.width = geom->width + 2 * _prop.borderWidth;
	rect.height = geom->height + 2 * _prop.borderWidth;
	free(geom);
	return rect;
}

void X11WindowBackend::setProp(WindowProp prop) {
	_prop = prop;
	if (!_hint->isWindowDecorated(_client)) {
		_prop.borderWidth = 0;
		_prop.titlebarHeight = 0;
	}

	uint32_t values[2];
	values[0] = 0;
	values[1] = _prop.titlebarHeight;
	xcb_configure_window(_conn->get(), _client, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, values);

	_hint->setWindowFrame(_client, _prop.borderWidth, _prop.borderWidth + _prop.titlebarHeight, _prop.borderWidth, _prop.borderWidth);
	_conn->flush();
}

WindowProp X11WindowBackend::getProp() {
	return _prop;
}

WindowType X11WindowBackend::getType() {
	return _hint->getWindowType(_client);
}

wx::String X11WindowBackend::getTitle() {
	return _hint->getWindowTitle(_client);
}

void X11WindowBackend::setState(WindowState state) {
	_state = state;
	_hint->setWindowState(_client, state);
	_conn->flush();
}

WindowState X11WindowBackend::getState() {
	return _state;
}

void X11WindowBackend::raise() {
	uint32_t value = XCB_STACK_MODE_ABOVE;
	xcb_configure_window(_conn->get(), _frame, XCB_CONFIG_WINDOW_STACK_MODE, &value);
	_conn->flush();
}

void X11WindowBackend::move(int x, int y) {
	uint32_t values[2];
	values[0] = x;
	values[1] = y;
	xcb_configure_window(_conn->get(), _frame, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, values);
	_conn->flush();
	notifyClient();
}

void X11WindowBackend::resize(int width, int height) {
	uint32_t values[2];
	int borders = _prop.borderWidth * 2;
	values[0] = width > borders ? width - borders : 0;
	values[1] = height > borders ? height - borders : 0;
	xcb_configure_window(_conn->get(), _frame, XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, values);

	LOG("[x11.window] resize to %d, %d\n", values[0], values[1]);

	uint32_t clientValues[] = {
		values[0],
		values[1] - _prop.titlebarHeight,
	};
	xcb_configure_window(_conn->get(), _client, XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, clientValues);
	_conn->flush();
	notifyClient();
	draw();
}

void X11WindowBackend::show() {
	xcb_map_window(_conn->get(), _frame);
	xcb_map_window(_conn->get(), _client);
	_conn->flush();
}

void X11WindowBackend::hide() {
	xcb_unmap_window(_conn->get(), _frame);
	_conn->flush();
}

void X11WindowBackend::focus() {
	_prop.borderColor = WindowStyle::Active::borderColor;
	_prop.titlebarColor = WindowStyle::Active::titlebarColor;
	draw();
}

void X11WindowBackend::blur() {
	_prop.borderColor = WindowStyle::Inactive::borderColor;
	_prop.titlebarColor = WindowStyle::Inactive::titlebarColor;
	draw();
}

void X11WindowBackend::close() {
	_hint->closeWindow(_client);
	_conn->flush();
}

xcb_window_t X11WindowBackend::getFrameHandle() {
	return _frame;
}

xcb_window_t X11WindowBackend::getClientHandle() {
	return _client;
}

void X11WindowBackend::draw() {
	xcb_change_window_attributes(
		_conn->get(),
		_frame,
		XCB_CW_BORDER_PIXEL,
		&_prop.borderColor
	);

	xcb_configure_window(
		_conn->get(), 
		_frame, 
		XCB_CONFIG_WINDOW_BORDER_WIDTH, 
		&_prop.borderWidth
	);

	Rect geom = getRect();
	xcb_rectangle_t rect;
	rect.x = 0;
	rect.y = 0;
	rect.width = geom.width;
	rect.height = _prop.titlebarHeight;

	xcb_change_gc(_conn->get(), _gc, XCB_GC_FOREGROUND, &_prop.titlebarColor);
	xcb_poly_fill_rectangle(_conn->get(), _frame, _gc, 1, &rect);
	_conn->flush();
}

void X11WindowBackend::notifyClient() {
	Rect rect = getRect();
	rect.width -= _prop.borderWidth * 2;
	rect.height -= _prop.borderWidth * 2;
	xcb_configure_notify_event_t event {};
	event.response_type = XCB_CONFIGURE_NOTIFY;
	event.window = _client;
	event.event = _client;
	event.x = rect.x;
	event.y = rect.y + _prop.titlebarHeight;
	event.width = rect.width;
	event.height = rect.height - _prop.titlebarHeight;
	event.border_width = _prop.borderWidth;
	event.above_sibling = XCB_NONE;
	event.override_redirect = 0;
	LOG("[x11.window] notify %d, %d, %d, %d\n", event.x, event.y, event.width, event.height);
	xcb_send_event(_conn->get(), 0, _client, XCB_EVENT_MASK_STRUCTURE_NOTIFY, (const char *)&event);
	_conn->flush();
}

void X11WindowBackend::_reparent() {
	_frame = xcb_generate_id(_conn->get());
	
	if (_hint->isWindowDecorated(_client)) {
		_prop.borderWidth = WindowStyle::borderWidth;
		_prop.titlebarHeight = WindowStyle::titlebarHeight;
	} else {
		_prop.borderWidth = 0;
		_prop.titlebarHeight = 0;
	}

	_prop.borderColor = WindowStyle::Active::borderColor;
	_prop.titlebarColor = WindowStyle::Active::titlebarColor;

	uint32_t mask = XCB_CW_BORDER_PIXEL | XCB_CW_OVERRIDE_REDIRECT | XCB_CW_EVENT_MASK;
	uint32_t values[] = {
		_prop.borderColor,
		1,
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
	int height = geom->height + _prop.titlebarHeight;

	free(geom);

	xcb_create_window(_conn->get(), _conn->getScreen()->root_depth, _frame, _conn->getScreen()->root,
		x, 
		y,
		width,
		height,
		_prop.borderWidth, XCB_WINDOW_CLASS_INPUT_OUTPUT,
		_conn->getScreen()->root_visual, mask, values
	);

	_gc = xcb_generate_id(_conn->get());
	uint32_t gcValues[] = { _conn->getScreen()->black_pixel };
	xcb_create_gc(_conn->get(), _gc, _frame, XCB_GC_FOREGROUND, gcValues);

	int clientBorderWidth = 0;
	xcb_configure_window(_conn->get(), _client, XCB_CONFIG_WINDOW_BORDER_WIDTH, &clientBorderWidth);
	xcb_reparent_window(_conn->get(), _client, _frame, 0, _prop.titlebarHeight);
	xcb_map_window(_conn->get(), _client);

	_hint->setWindowFrame(_client, _prop.borderWidth, _prop.borderWidth + _prop.titlebarHeight, _prop.borderWidth, _prop.borderWidth);

	_conn->flush();
}

