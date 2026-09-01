#include "X11MonitorManager.hpp"
#include <cstdio>
#include <cstdlib>

#ifdef XEPHYR_ENABLE
int xephyrMonitorConfiguration = 0;
#endif


X11MonitorManager::X11MonitorManager(X11Connection* conn): _conn{conn} {
}

void X11MonitorManager::Init() {
	const xcb_query_extension_reply_t* ext = xcb_get_extension_data(_conn->Get(), &xcb_randr_id);
	if (!ext || !ext->present) {
		printf("X.RandR extension is not available\n");
		return;
	}

	_eventOffset = ext->first_event;

	xcb_randr_select_input(
		_conn->Get(), 
		_conn->GetScreen()->root,
		XCB_RANDR_NOTIFY_MASK_OUTPUT_CHANGE | XCB_RANDR_NOTIFY_MASK_CRTC_CHANGE | XCB_RANDR_NOTIFY_MASK_SCREEN_CHANGE
	);
}

int X11MonitorManager::GetEventOffset() {
	return _eventOffset;
}

Wx::List<MonitorDef> X11MonitorManager::GetConnectedMonitorDefs() {
	#ifdef XEPHYR_ENABLE
		Wx::List<MonitorDef> defs;

		#ifdef XEPHYR_EMULATE_TWO_MONITOR
		bool left = false;
		bool right = false;
		if (xephyrMonitorConfiguration % 3 == 0) {
			left = true;
			right = false;
		} else if (xephyrMonitorConfiguration % 3 == 1) {
			left = true;
			right = true;
		} else if (xephyrMonitorConfiguration % 3 == 2) {
			left = false;
			right = false;
		}

		if (left) {
			MonitorDef left;
			left.port = "HDMI-1";
			left.rect.x = 0;
			left.rect.y = 0;
			left.rect.width = XEPHYR_WIDTH / 2;
			left.rect.height = XEPHYR_HEIGHT;
			defs.Append(left);
		}

		if (right) {
			MonitorDef right;
			right.port = "HDMI-2";
			right.rect.x = XEPHYR_WIDTH / 2;
			right.rect.y = 0;
			right.rect.width = XEPHYR_WIDTH / 2;
			right.rect.height = XEPHYR_HEIGHT;
			defs.Append(right);
		}

		return defs;
		#else
		MonitorDef right;
		right.port = "HDMI-1";
		right.rect.x = 0;
		right.rect.y = 0;
		right.rect.width = XEPHYR_WIDTH;
		right.rect.height = XEPHYR_HEIGHT;
		defs.Append(right);

		return defs;
		#endif
	#else

	xcb_randr_get_screen_resources_current_cookie_t resCookie = xcb_randr_get_screen_resources_current(_conn->Get(), _conn->GetScreen()->root);
	xcb_randr_get_screen_resources_current_reply_t* res = xcb_randr_get_screen_resources_current_reply(_conn->Get(), resCookie, NULL);
	if (!res) {
		fprintf(stderr, "Failed to get screen resources.\n");
		return {};
	}

	int outputSize = xcb_randr_get_screen_resources_current_outputs_length(res);
	xcb_randr_output_t* outputs = xcb_randr_get_screen_resources_current_outputs(res);

	Wx::List<MonitorDef> defs;

	for (int i = 0; i < outputSize; i++) {
		xcb_randr_get_output_def_cookie_t outCookie = xcb_randr_get_output_def(_conn->Get(), outputs[i], XCB_CURRENT_TIME);
		xcb_randr_get_output_def_reply_t* out = xcb_randr_get_output_def_reply(_conn->Get(), outCookie, NULL);

		if (!out) continue;

		if (out->connection == XCB_RANDR_CONNECTION_CONNECTED) {
			char *name = (char *)xcb_randr_get_output_def_name(out);
			int nameSize = xcb_randr_get_output_def_name_length(out);

			if (out->crtc != XCB_NONE) {
				xcb_randr_get_crtc_def_cookie_t crtcCookie = xcb_randr_get_crtc_def(_conn->Get(), out->crtc, XCB_CURRENT_TIME);
				xcb_randr_get_crtc_def_reply_t* crtc = xcb_randr_get_crtc_def_reply(_conn->Get(), crtcCookie, NULL);

				if (crtc) {
					Wx::String port(nameSize, (Wx::Byte const*)name);

					MonitorDef def = {
						.port = port,
						.rect = {
							.x = crtc->x,
							.y = crtc->y,
							.width = crtc->width,
							.height = crtc->height,
						}
					};

					defs.Append(def);
					free(crtc);
				}
			}
		}
		free(out);
	}
	free(res);
	return defs;
	#endif
}

