#pragma once

#include "wx/core/Numeric.hpp"

namespace WindowStyle {
	constexpr int borderWidth = 1;
	constexpr int titlebarHeight = 20;

	namespace Active {
		#ifdef XEPHYR_ENABLE
		constexpr uint32_t borderColor = 0x1238DF;
		constexpr uint32_t titlebarColor = 0x3258FF;
		#else
		constexpr wx::Uint32 borderColor = 0xDF1238;
		constexpr wx::Uint32 titlebarColor = 0xFF3258;
		#endif
	}

	namespace Inactive {
		constexpr wx::Uint32 borderColor = 0x888888;
		constexpr wx::Uint32 titlebarColor = 0x888888;
	}
}

