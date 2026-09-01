#pragma once

#include "Input/Key.hpp"
#include "Wx/Core/List.hpp"
#include "Wx/Core/String.hpp"
#include "Wx/Serializer/WkValue.hpp"

struct ConfigWorkspace {
	Wx::String id;
	Wx::List<Wx::String> ports;
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
	Wx::WkValue arguments;

};

struct ConfigBinding {
	ModKey modKey = ModKey::None;
	Key key = Key::None;
	Wx::List<ConfigCommand> commands;
};

struct Config {
	Wx::List<ConfigWorkspace> workspaces;
	Wx::List<ConfigBinding> bindings;

	Wx::List<ConfigCommand> autostarts;
};
