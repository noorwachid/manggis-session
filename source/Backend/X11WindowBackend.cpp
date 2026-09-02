#include "Backend/X11WindowBackend.hpp"

X11WindowBackend::X11WindowBackend(X11Connection* conn, X11Hint* hint, xcb_window_t handle): _conn{conn}, _hint{hint}, _client{handle} {
	Reparent();
}

Rect X11WindowBackend::GetRect() {
	xcb_get_geometry_reply_t* geom = xcb_get_geometry_reply(_conn->Get(), xcb_get_geometry(_conn->Get(), _frame), NULL);
	Rect rect;
	rect.x = geom->x;
	rect.y = geom->y;
	rect.width = geom->width;
	rect.height = geom->height;
	free(geom);
	return rect;
}

WindowProp X11WindowBackend::GetProp() {
	return _prop;
}

WindowType X11WindowBackend::GetType() {
	return _hint->GetWindowType(_client);
}

Wx::String X11WindowBackend::GetTitle() {
	return _hint->GetWindowTitle(_client);
}

void X11WindowBackend::Raise() {
	uint32_t value = XCB_STACK_MODE_ABOVE;
	xcb_configure_window(_conn->Get(), _frame, XCB_CONFIG_WINDOW_STACK_MODE, &value);
	_conn->Flush();
}

void X11WindowBackend::Move(int x, int y) {
	uint32_t values[2];
	values[0] = x;
	values[1] = y;
	xcb_configure_window(_conn->Get(), _frame, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, values);
	_conn->Flush();
	NotifyClient();
}

void X11WindowBackend::Resize(int width, int height) {
	uint32_t values[2];
	int borders = _prop.borderWidth * 2;
	values[0] = width > borders ? width - borders : 0;
	values[1] = height > borders ? height - borders : 0;
	xcb_configure_window(_conn->Get(), _frame, XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, values);

	LOG("[x11.window] resize to %d, %d\n", values[0], values[1]);

	uint32_t clientValues[] = {
		(uint32_t)width,
		(uint32_t)height - _prop.titlebarHeight,
	};
	xcb_configure_window(_conn->Get(), _client, XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, clientValues);
	_conn->Flush();
	NotifyClient();
	Draw();
}

void X11WindowBackend::Show() {
	xcb_map_window(_conn->Get(), _frame);
	xcb_map_window(_conn->Get(), _client);
	_conn->Flush();
}

void X11WindowBackend::Hide() {
	xcb_unmap_window(_conn->Get(), _frame);
	_conn->Flush();
}

void X11WindowBackend::Focus() {
	_prop.borderColor = WindowStyle::Active::borderColor;
	_prop.titlebarColor = WindowStyle::Active::titlebarColor;
	Draw();
}

void X11WindowBackend::Blur() {
	_prop.borderColor = WindowStyle::Inactive::borderColor;
	_prop.titlebarColor = WindowStyle::Inactive::titlebarColor;
	Draw();
}

void X11WindowBackend::Close() {
	_hint->CloseWindow(_client);
	_conn->Flush();
}

xcb_window_t X11WindowBackend::GetFrameHandle() {
	return _frame;
}

xcb_window_t X11WindowBackend::GetClientHandle() {
	return _client;
}

uint32_t X11WindowBackend::GetBorderColor() {
	return _prop.borderColor;
}

uint32_t X11WindowBackend::GetBorderWidth() {
	return _prop.borderWidth;
}

uint32_t X11WindowBackend::GetTitlebarColor() {
	return _prop.titlebarColor;
}

uint32_t X11WindowBackend::GetTitlebarHeight() {
	return _prop.titlebarHeight;
}

void X11WindowBackend::Draw() {
	xcb_change_window_attributes(
		_conn->Get(),
		_frame,
		XCB_CW_BORDER_PIXEL,
		&_prop.borderColor
	);

	xcb_configure_window(
		_conn->Get(), 
		_frame, 
		XCB_CONFIG_WINDOW_BORDER_WIDTH, 
		&_prop.borderWidth
	);

	Rect geom = GetRect();
	xcb_rectangle_t rect;
	rect.x = 0;
	rect.y = 0;
	rect.width = geom.width;
	rect.height = _prop.titlebarHeight;

	xcb_change_gc(_conn->Get(), _gc, XCB_GC_FOREGROUND, &_prop.titlebarColor);
	xcb_poly_fill_rectangle(_conn->Get(), _frame, _gc, 1, &rect);
	_conn->Flush();
}

void X11WindowBackend::NotifyClient() {
	Rect rect = GetRect();
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
	xcb_send_event(_conn->Get(), 0, _client, XCB_EVENT_MASK_STRUCTURE_NOTIFY, (const char *)&event);
	_conn->Flush();
}

void X11WindowBackend::Reparent() {
	_frame = xcb_generate_id(_conn->Get());
	
	if (_hint->IsWindowDecorated(_client)) {
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

	xcb_get_geometry_reply_t* geom = xcb_get_geometry_reply(_conn->Get(), xcb_get_geometry(_conn->Get(), _client), NULL);

	int x = geom->x;
	int y = geom->y;
	int width = geom->width;
	int height = geom->height + _prop.titlebarHeight;

	free(geom);

	xcb_create_window(_conn->Get(), _conn->GetScreen()->root_depth, _frame, _conn->GetScreen()->root,
		x, 
		y,
		width,
		height,
		_prop.borderWidth, XCB_WINDOW_CLASS_INPUT_OUTPUT,
		_conn->GetScreen()->root_visual, mask, values
	);

	_gc = xcb_generate_id(_conn->Get());
	uint32_t gcValues[] = { _conn->GetScreen()->black_pixel };
	xcb_create_gc(_conn->Get(), _gc, _frame, XCB_GC_FOREGROUND, gcValues);

	int clientBorderWidth = 0;
	xcb_configure_window(_conn->Get(), _client, XCB_CONFIG_WINDOW_BORDER_WIDTH, &clientBorderWidth);
	xcb_reparent_window(_conn->Get(), _client, _frame, 0, _prop.titlebarHeight);
	xcb_map_window(_conn->Get(), _client);

	_hint->SetWindowFrame(_client, _prop.borderWidth, _prop.borderWidth + _prop.titlebarHeight, _prop.borderWidth, _prop.borderWidth);

	_conn->Flush();
}

