#pragma once

#include "wx/core/Flags.hpp"

enum struct ModKey {
	None  = 0,
	Shift = 1 << 0,
	Ctrl  = 1 << 1,
	Alt   = 1 << 2,
	Super = 1 << 3,
};

WX_FLAGS(ModKey);

enum struct Key {
};
