#pragma once

#include "wx/core/List.hpp"
#include "wx/core/String.hpp"

struct ConfigWorkspace {
	wx::String id;
	wx::List<wx::String> ports;
};

struct Config {
	wx::List<ConfigWorkspace> workspaces;
};
