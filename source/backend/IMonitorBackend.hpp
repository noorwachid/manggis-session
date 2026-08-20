#pragma once

#include "Common.hpp"
#include "wx/core/String.hpp"

class IMonitorBackend {
public:
	virtual Rect getRect() = 0;

	virtual wx::String const& getPort() const = 0;
};
