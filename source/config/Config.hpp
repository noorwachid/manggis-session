#pragma once

#include "input/Key.hpp"
#include "input/MouseButton.hpp"
#include "wx/core/List.hpp"
#include "wx/core/String.hpp"
#include "wx/serializer/WkValue.hpp"

struct ConfigWorkspace {
	wx::String id;
	wx::List<wx::String> ports;
};

enum struct CommandFunc {
	None, 

	Exec,
	FocusWindow,
	CloseWindow,
	CycleNextWindow,
	CyclePreviousWindow,
	TileWindow,
	MaximizeWindow,
	RestoreWindow,

	MoveWindow,
	ResizeWindow,

	FocusWorkspace,
	SendWindowToWorkspace,

	Quit,
};

struct Command {
	CommandFunc func = CommandFunc::None;
	wx::WkValue arguments;
};

struct KeyBinding {
	ModKey modKey = ModKey::None;
	Key key = Key::None;
	wx::List<Command> commands;
};

struct MouseBinding {
	ModKey modKey = ModKey::None;
	CommandFunc func;
};

struct Config {
	wx::List<ConfigWorkspace> workspaces;
	wx::List<KeyBinding> keyBindings;
	wx::List<MouseBinding> mouseBindings;

	wx::List<Command> autostarts;
};
