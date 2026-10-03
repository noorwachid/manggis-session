#include "backend/X11WindowBackend.hpp"
#include "backend/IWindowBackend.hpp"
#include "core/Logger.hpp"
#include "WindowStyle.hpp"

X11WindowBackend::X11WindowBackend(X11Connection* conn, X11Hint* hint, xcb_window_t handle): _conn{conn}, _hint{hint}, _client{handle} {
	_state = WindowState::None;
	_reparent();
	draw();
}

X11WindowBackend::~X11WindowBackend() {
	if (_frameColormap != XCB_NONE) {
		xcb_free_colormap(_conn->get(), _frameColormap);
		_frameColormap = XCB_NONE;
	}
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

	xcb_configure_window(
		_conn->get(),
		_client,
		XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y,
		values
	);

	_hint->setWindowFrame(
		_client,
		_prop.borderWidth,
		_prop.borderWidth + _prop.titlebarHeight,
		_prop.borderWidth,
		_prop.borderWidth
	);

	_conn->flush();

	draw();
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

	xcb_configure_window(
		_conn->get(),
		_frame,
		XCB_CONFIG_WINDOW_WIDTH |
		XCB_CONFIG_WINDOW_HEIGHT,
		values
	);

	LOG("[x11.window] resize to %d, %d\n", values[0], values[1]);

	uint32_t clientValues[] = {
		values[0],
		values[1] > static_cast<uint32_t>(_prop.titlebarHeight)
			? values[1] - _prop.titlebarHeight
			: 0
	};

	xcb_configure_window(
		_conn->get(),
		_client,
		XCB_CONFIG_WINDOW_WIDTH |
		XCB_CONFIG_WINDOW_HEIGHT,
		clientValues
	);

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
	_prop.buttonColor = WindowStyle::Active::buttonColor;
	draw();
}

void X11WindowBackend::blur() {
	_prop.borderColor = WindowStyle::Inactive::borderColor;
	_prop.titlebarColor = WindowStyle::Inactive::titlebarColor;
	_prop.buttonColor = WindowStyle::Inactive::buttonColor;
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

static void createCircleImage(
	wx::List<wx::Uint32>& pixels,
	int size,
	uint32_t color,
	uint32_t background,
	int samples = 4
) {
	const float radius = size / 2.0f;
	const float cx = radius;
	const float cy = radius;

	const float invSamples = 1.0f / samples;
	const int totalSamples = samples * samples;

	const uint8_t srcR = (color >> 16) & 0xff;
	const uint8_t srcG = (color >> 8) & 0xff;
	const uint8_t srcB = color & 0xff;

	const uint8_t bgR = (background >> 16) & 0xff;
	const uint8_t bgG = (background >> 8) & 0xff;
	const uint8_t bgB = background & 0xff;

	for (int y = 0; y < size; ++y) {
		for (int x = 0; x < size; ++x) {

			int inside = 0;

			for (int sy = 0; sy < samples; ++sy) {
				for (int sx = 0; sx < samples; ++sx) {

					float px =
						x + (sx + 0.5f) * invSamples;

					float py =
						y + (sy + 0.5f) * invSamples;

					float dx = px - cx;
					float dy = py - cy;

					if (dx * dx + dy * dy <= radius * radius) {
						++inside;
					}
				}
			}

			float coverage =
				static_cast<float>(inside) / totalSamples;

			uint8_t r = static_cast<uint8_t>(
				srcR * coverage +
				bgR * (1.0f - coverage)
			);

			uint8_t g = static_cast<uint8_t>(
				srcG * coverage +
				bgG * (1.0f - coverage)
			);

			uint8_t b = static_cast<uint8_t>(
				srcB * coverage +
				bgB * (1.0f - coverage)
			);

			/*
			 * Buttons are opaque. The anti-aliased edge is
			 * achieved by RGB blending against the opaque
			 * titlebar rather than alpha blending.
			 */
			pixels[y * size + x] =
				0xff000000u |
				(static_cast<uint32_t>(r) << 16) |
				(static_cast<uint32_t>(g) << 8) |
				static_cast<uint32_t>(b);
		}
	}
}

void X11WindowBackend::draw() {
	uint32_t borderColor = _opaqueColor(_prop.borderColor);
	uint32_t titlebarColor = _opaqueColor(_prop.titlebarColor);
	uint32_t buttonColor = _opaqueColor(_prop.buttonColor);

	xcb_change_window_attributes(_conn->get(), _frame, XCB_CW_BORDER_PIXEL, &borderColor);
	xcb_configure_window(_conn->get(), _frame, XCB_CONFIG_WINDOW_BORDER_WIDTH, &_prop.borderWidth);

	_clearClientArea();

	Rect geom = getRect();

	if (_prop.titlebarHeight > 0) {
		xcb_rectangle_t titlebarRect {};
		titlebarRect.x = 0;
		titlebarRect.y = 0;
		titlebarRect.width = geom.width;
		titlebarRect.height = _prop.titlebarHeight;

		xcb_change_gc(_conn->get(), _gc, XCB_GC_FOREGROUND, &titlebarColor);
		xcb_poly_fill_rectangle(_conn->get(), _frame, _gc, 1, &titlebarRect);

		int x = WindowStyle::buttonPadding;
		int size = WindowStyle::buttonSize;

		wx::List<wx::Uint32> pixels;
		pixels.reserve(size * size);

		for (int i = 0; i < size * size; ++i) {
			pixels.append(0);
		}

		createCircleImage(pixels, size, buttonColor, titlebarColor);

		xcb_change_gc(_conn->get(), _gc, XCB_GC_FOREGROUND, &titlebarColor);

		xcb_put_image(
			_conn->get(),
			XCB_IMAGE_FORMAT_Z_PIXMAP,
			_frame,
			_gc,
			size,
			size,
			x,
			_prop.borderWidth + WindowStyle::buttonPadding,
			0,
			_frameDepth,
			pixels.count() * sizeof(wx::Uint32),
			reinterpret_cast<const uint8_t*>(pixels.ptr())
		);

		x += size + WindowStyle::buttonGap;

		xcb_put_image(
			_conn->get(),
			XCB_IMAGE_FORMAT_Z_PIXMAP,
			_frame,
			_gc,
			size,
			size,
			x,
			_prop.borderWidth + WindowStyle::buttonPadding,
			0,
			_frameDepth,
			pixels.count() * sizeof(wx::Uint32),
			reinterpret_cast<const uint8_t*>(pixels.ptr())
		);

		x += size + WindowStyle::buttonGap;

		xcb_put_image(
			_conn->get(),
			XCB_IMAGE_FORMAT_Z_PIXMAP,
			_frame,
			_gc,
			size,
			size,
			x,
			_prop.borderWidth + WindowStyle::buttonPadding,
			0,
			_frameDepth,
			pixels.count() * sizeof(wx::Uint32),
			reinterpret_cast<const uint8_t*>(pixels.ptr())
		);
	}

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

	_clientSupportsTransparency = _detectClientTransparency();

	xcb_screen_t* screen = _conn->getScreen();

	if (_clientSupportsTransparency) {
		_frameVisual = _findARGBVisual(_frameDepth);
		_frameColormap = xcb_generate_id(_conn->get());

		xcb_create_colormap(
			_conn->get(),
			XCB_COLORMAP_ALLOC_NONE,
			_frameColormap,
			screen->root,
			_frameVisual
		);
	} else {
		_frameVisual = screen->root_visual;
		_frameDepth = screen->root_depth;
		_frameColormap = screen->default_colormap;
	}

	uint32_t borderColor = _opaqueColor(_prop.borderColor);

	uint32_t mask =
		XCB_CW_BORDER_PIXEL |
		XCB_CW_OVERRIDE_REDIRECT |
		XCB_CW_EVENT_MASK;

	uint32_t values[] = {
		borderColor,
		1,
		(
			XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT |
			XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
			XCB_EVENT_MASK_EXPOSURE
		)
	};

	/*
	 * A custom visual needs the matching colormap.
	 */
	if (_clientSupportsTransparency) {
		mask |= XCB_CW_COLORMAP;
	}

	xcb_get_geometry_reply_t* geom =
		xcb_get_geometry_reply(
			_conn->get(),
			xcb_get_geometry(_conn->get(), _client),
			nullptr
		);

	if (!geom) {
		return;
	}

	int x = geom->x;
	int y = geom->y;
	int width = geom->width;
	int height = geom->height + _prop.titlebarHeight;

	free(geom);

	if (_clientSupportsTransparency) {
		/*
		 * XCB_CW_COLORMAP must be appended to the value list
		 * in exactly the same order as the mask bits.
		 */
		uint32_t transparentValues[] = {
			borderColor,
			1,
			(
				XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT |
				XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
				XCB_EVENT_MASK_EXPOSURE
			),
			_frameColormap
		};

		xcb_create_window(
			_conn->get(),
			_frameDepth,
			_frame,
			screen->root,
			x,
			y,
			width,
			height,
			_prop.borderWidth,
			XCB_WINDOW_CLASS_INPUT_OUTPUT,
			_frameVisual,
			mask,
			transparentValues
		);
	} else {
		xcb_create_window(
			_conn->get(),
			_frameDepth,
			_frame,
			screen->root,
			x,
			y,
			width,
			height,
			_prop.borderWidth,
			XCB_WINDOW_CLASS_INPUT_OUTPUT,
			_frameVisual,
			mask,
			values
		);
	}

	_gc = xcb_generate_id(_conn->get());

	uint32_t gcValues[] = {
		_clientSupportsTransparency
			? 0x00000000
			: screen->black_pixel
	};

	xcb_create_gc(
		_conn->get(),
		_gc,
		_frame,
		XCB_GC_FOREGROUND,
		gcValues
	);

	int clientBorderWidth = 0;

	xcb_configure_window(
		_conn->get(),
		_client,
		XCB_CONFIG_WINDOW_BORDER_WIDTH,
		&clientBorderWidth
	);

	/*
	 * Put the client below the opaque titlebar.
	 *
	 * The transparent pixels of the ARGB frame are behind the
	 * client, allowing the compositor to see through transparent
	 * parts of the client.
	 */
	xcb_reparent_window(
		_conn->get(),
		_client,
		_frame,
		0,
		_prop.titlebarHeight
	);

	xcb_map_window(_conn->get(), _client);

	_hint->setWindowFrame(
		_client,
		_prop.borderWidth,
		_prop.borderWidth + _prop.titlebarHeight,
		_prop.borderWidth,
		_prop.borderWidth
	);

	_clearClientArea();
	_conn->flush();
}


bool X11WindowBackend::_detectClientTransparency() {
	xcb_get_geometry_reply_t* geom =
		xcb_get_geometry_reply(
			_conn->get(),
			xcb_get_geometry(_conn->get(), _client),
			nullptr
		);

	if (!geom) {
		return false;
	}

	const uint8_t clientDepth = geom->depth;

	free(geom);

	if (clientDepth != 32) {
		return false;
	}

	xcb_get_window_attributes_reply_t* attr =
		xcb_get_window_attributes_reply(
			_conn->get(),
			xcb_get_window_attributes(_conn->get(), _client),
			nullptr
		);

	if (!attr) {
		return false;
	}

	const xcb_visualid_t clientVisual = attr->visual;

	free(attr);

	xcb_screen_t* screen = _conn->getScreen();

	for (xcb_depth_iterator_t depthIt =
			xcb_screen_allowed_depths_iterator(screen);
		 depthIt.rem;
		 xcb_depth_next(&depthIt)) {

		if (depthIt.data->depth != 32) {
			continue;
		}

		for (xcb_visualtype_iterator_t visualIt =
				xcb_depth_visuals_iterator(depthIt.data);
			 visualIt.rem;
			 xcb_visualtype_next(&visualIt)) {

			const xcb_visualtype_t* visual = visualIt.data;

			if (visual->visual_id != clientVisual) {
				continue;
			}

			if (visual->_class == XCB_VISUAL_CLASS_TRUE_COLOR &&
				visual->red_mask == 0x00ff0000 &&
				visual->green_mask == 0x0000ff00 &&
				visual->blue_mask == 0x000000ff) {

				return true;
			}

			return false;
		}
	}

	return false;
}


xcb_visualid_t X11WindowBackend::_findARGBVisual(uint8_t& depth) const {
	xcb_screen_t* screen = _conn->getScreen();

	for (xcb_depth_iterator_t depthIt =
				xcb_screen_allowed_depths_iterator(screen);
		 depthIt.rem;
		 xcb_depth_next(&depthIt)) {

		if (depthIt.data->depth != 32) {
			continue;
		}

		for (xcb_visualtype_iterator_t visualIt =
					xcb_depth_visuals_iterator(depthIt.data);
			 visualIt.rem;
			 xcb_visualtype_next(&visualIt)) {

			const xcb_visualtype_t* visual = visualIt.data;

			if (visual->_class != XCB_VISUAL_CLASS_TRUE_COLOR) {
				continue;
			}

			if (visual->red_mask == 0x00ff0000 &&
				visual->green_mask == 0x0000ff00 &&
				visual->blue_mask == 0x000000ff) {

				depth = 32;
				return visual->visual_id;
			}
		}
	}

	depth = screen->root_depth;
	return screen->root_visual;
}

uint32_t X11WindowBackend::_opaqueColor(uint32_t color) const {
	if (!_clientSupportsTransparency) {
		return color;
	}

	return 0xff000000u | (color & 0x00ffffffu);
}

void X11WindowBackend::_clearClientArea() {
	if (!_clientSupportsTransparency) {
		return;
	}

	Rect geom = getRect();

	xcb_rectangle_t rect {};
	rect.x = _prop.borderWidth;
	rect.y = _prop.borderWidth + _prop.titlebarHeight;
	rect.width = geom.width - (_prop.borderWidth * 2);
	rect.height = geom.height -
		(_prop.borderWidth * 2) -
		_prop.titlebarHeight;

	if (rect.width <= 0 || rect.height <= 0) {
		return;
	}

	uint32_t transparent = 0x00000000;

	xcb_change_gc(
		_conn->get(),
		_gc,
		XCB_GC_FOREGROUND,
		&transparent
	);

	xcb_poly_fill_rectangle(
		_conn->get(),
		_frame,
		_gc,
		1,
		&rect
	);
}

