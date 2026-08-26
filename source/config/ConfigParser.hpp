#pragma once

#include "config/Config.hpp"
#include "wx/serializer/WkValue.hpp"

Config parseConfig(wx::WkValue const& value);
