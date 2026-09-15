#pragma once

#include "wx/core/Numeric.hpp"

namespace WindowStyle {
	constexpr int borderWidth = 1;
	constexpr int titlebarHeight = 20;
	constexpr int buttonPadding = int(titlebarHeight * 0.2);
	constexpr int buttonSize = titlebarHeight - int(titlebarHeight * 0.2 * 2);
	constexpr int buttonGap = int(buttonSize * 0.5);

	namespace Active {
		#ifdef XEPHYR_ENABLE
		constexpr uint32_t borderColor = 0x1238DF;
		constexpr uint32_t titlebarColor = 0x3258FF;
		constexpr uint32_t buttonColor = 0x022888;
		#else
		constexpr wx::Uint32 borderColor = 0xDF1238;
		constexpr wx::Uint32 titlebarColor = 0xFF3258;
		constexpr wx::Uint32 buttonColor = 0x880228;
		#endif
	}

	namespace Inactive {
		constexpr wx::Uint32 borderColor = 0x888888;
		constexpr wx::Uint32 titlebarColor = 0x888888;
		constexpr wx::Uint32 buttonColor = 0x656565;
	}
}

