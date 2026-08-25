#pragma once

#include "wx/core/List.hpp"
#include "backend/X11Connection.hpp"
#include "MonitorInfo.hpp"
#include <xcb/randr.h>

class X11MonitorManager {
public:
	X11MonitorManager(X11Connection* conn);

	void initialize();

	int getEventOffset();

	wx::List<MonitorInfo> getConnectedMonitorInfos();


private:
	X11Connection* _conn;
	uint8_t _eventOffset;
};
