#pragma once

#include "MonitorDef.hpp"
#include "backend/X11Connection.hpp"
#include "wx/core/List.hpp"
#include <xcb/randr.h>

class X11MonitorManager {
public:
	X11MonitorManager(X11Connection* conn);

	int getEventOffset();

	wx::List<MonitorDef> getConnectedMonitorDefs();


private:
	X11Connection* _conn;
	uint8_t _eventOffset;
};
