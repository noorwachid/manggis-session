#include "X11MonitorManager.hpp"
#include <cstdio>
#include <cstdlib>

#ifdef USE_XEPHYR
int xephyrMonitorConfiguration = 0;
#endif


X11MonitorManager::X11MonitorManager(X11Connection* conn): _conn{conn} {
}

void X11MonitorManager::initialize() {
	const xcb_query_extension_reply_t* ext = xcb_get_extension_data(_conn->get(), &xcb_randr_id);
	if (!ext || !ext->present) {
		printf("X.RandR extension is not available\n");
		return;
	}

	_eventOffset = ext->first_event;

	xcb_randr_select_input(
		_conn->get(), 
		_conn->getScreen()->root,
		XCB_RANDR_NOTIFY_MASK_OUTPUT_CHANGE | XCB_RANDR_NOTIFY_MASK_CRTC_CHANGE | XCB_RANDR_NOTIFY_MASK_SCREEN_CHANGE
	);
}

int X11MonitorManager::getEventOffset() {
	return _eventOffset;
}

wx::List<MonitorInfo> X11MonitorManager::getConnectedMonitorInfos() {
	#ifdef USE_XEPHYR
	wx::List<MonitorInfo> infos;
	bool left = false;
	bool right = false;
	if (xephyrMonitorConfiguration == 0) {
		left = true;
		right = false;
	} else if (xephyrMonitorConfiguration == 1) {
		left = true;
		right = true;
	} else if (xephyrMonitorConfiguration == 2) {
		left = false;
		right = false;
	}

	if (left) {
		MonitorInfo left;
		left.port = "VHDMI-1";
		left.rect.x = 0;
		left.rect.y = 0;
		left.rect.width = 1000;
		left.rect.height = 600;
		infos.append(left);
	}

	if (right) {
		MonitorInfo right;
		right.port = "VHDMI-2";
		right.rect.x = 500;
		right.rect.y = 0;
		right.rect.width = 500;
		right.rect.height = 600;
		infos.append(right);
	}

	return infos;
	#else

	xcb_randr_get_screen_resources_current_cookie_t resCookie = xcb_randr_get_screen_resources_current(_conn->get(), _conn->getScreen()->root);
	xcb_randr_get_screen_resources_current_reply_t* res = xcb_randr_get_screen_resources_current_reply(_conn->get(), resCookie, NULL);
	if (!res) {
		fprintf(stderr, "Failed to get screen resources.\n");
		return {};
	}

	int outputSize = xcb_randr_get_screen_resources_current_outputs_length(res);
	xcb_randr_output_t* outputs = xcb_randr_get_screen_resources_current_outputs(res);

	wx::List<MonitorInfo> infos;

	for (int i = 0; i < outputSize; i++) {
		xcb_randr_get_output_info_cookie_t outCookie = xcb_randr_get_output_info(_conn->get(), outputs[i], XCB_CURRENT_TIME);
		xcb_randr_get_output_info_reply_t* out = xcb_randr_get_output_info_reply(_conn->get(), outCookie, NULL);

		if (!out) continue;

		if (out->connection == XCB_RANDR_CONNECTION_CONNECTED) {
			char *name = (char *)xcb_randr_get_output_info_name(out);
			int nameSize = xcb_randr_get_output_info_name_length(out);

			if (out->crtc != XCB_NONE) {
				xcb_randr_get_crtc_info_cookie_t crtcCookie = xcb_randr_get_crtc_info(_conn->get(), out->crtc, XCB_CURRENT_TIME);
				xcb_randr_get_crtc_info_reply_t* crtc = xcb_randr_get_crtc_info_reply(_conn->get(), crtcCookie, NULL);

				if (crtc) {
					wx::String port(nameSize, (wx::Byte const*)name);

					MonitorInfo info = {
						.port = port,
						.rect = {
							.x = crtc->x,
							.y = crtc->y,
							.width = crtc->width,
							.height = crtc->height,
						}
					};

					infos.append(info);
					free(crtc);
				}
			}
		}
		free(out);
	}
	free(res);
	return infos;
	#endif
}

