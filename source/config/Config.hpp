#pragma once

#include "input/Key.hpp"
#include "wx/core/List.hpp"
#include "wx/core/String.hpp"
#include "wx/serializer/WkValue.hpp"

struct ConfigWorkspace {
	wx::String id;
	wx::List<wx::String> ports;
};

enum struct ConfigAction {
	ActivateWindow,
	ActivateWorkspace,
	ActivateMonitor,

	CloseWindow,
	ForceCloseWindow,
	SendActiveWindowToWorkspace,

	ReconfigureSession, // reparse config
	ReloadSession, // full restart
	QuitSession,
};

struct ConfigBinding {
	ModKey modKey;
	Key key;
	ConfigAction action;
	wx::WkValue args;
};

struct ConfigMouse {
	bool clickToActivate = true;

	bool anyPointMove = true;
	ModKey anyPointMoveModKey = ModKey::Super;

	bool anyPointResize = true;
	ModKey anyPointResizeModKey = ModKey::Super;
};

struct Config {
	wx::List<ConfigWorkspace> workspaces;
	wx::List<ConfigBinding> bindings;

	ConfigMouse mouse;
};
