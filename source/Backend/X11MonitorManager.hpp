#pragma once

#include "Backend/X11Connection.hpp"
#include "MonitorInfo.hpp"
#include "Wx/Core/List.hpp"
#include <xcb/randr.h>

class X11MonitorManager {
public:
	X11MonitorManager(X11Connection* conn);

	void initialize();

	int getEventOffset();

	Wx::List<MonitorInfo> getConnectedMonitorInfos();


private:
	X11Connection* _conn;
	uint8_t _eventOffset;
};
