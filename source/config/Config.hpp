#pragma once

#include "input/Key.hpp"
#include "wx/core/List.hpp"
#include "wx/core/String.hpp"
#include "wx/serializer/WkValue.hpp"

struct ConfigWorkspace {
	wx::String id;
	wx::List<wx::String> ports;
};

enum struct ConfigCommandFunc {
	None, 

	Exec,
	FocusWindow,
	CloseWindow,
	CycleNextWindow,
	CyclePreviousWindow,
	TileWindow,
	MaximizeWindow,
	RestoreWindow,

	FocusWorkspace,
	SendWindowToWorkspace,

	Quit,
};

struct ConfigCommand {
	ConfigCommandFunc func = ConfigCommandFunc::None;
	wx::WkValue arguments;
};

struct ConfigBinding {
	ModKey modKey = ModKey::None;
	Key key = Key::None;
	wx::List<ConfigCommand> commands;
};

struct Config {
	wx::List<ConfigWorkspace> workspaces;
	wx::List<ConfigBinding> bindings;

	wx::List<ConfigCommand> autostarts;
};
