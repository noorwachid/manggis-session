#pragma once

#include "Common.hpp"

namespace helper {
	constexpr int min(int a, int b) {
		return a < b ? a : b;
	}
	constexpr int max(int a, int b) {
		return a > b ? a : b;
	}
};

constexpr bool isPointIntersectRect(Point point, Rect rect) {
	return (
		point.x >= rect.x && 
		point.x <= rect.x + rect.width && 
		point.y >= rect.y && 
		point.y <= rect.y + rect.height
	);
}

constexpr Rect intersectRectRect(const Rect& monitor, const Rect& window) {
    const int left   = helper::max(monitor.x, window.x);
    const int top    = helper::max(monitor.y, window.y);
    const int right  = helper::min(monitor.x + monitor.width, window.x + window.width);
    const int bottom = helper::min(monitor.y + monitor.height, window.y + window.height);

    if (right <= left || bottom <= top)
        return Rect{ 0, 0, 0, 0 };

    return Rect{
        left,
        top,
        right - left,
        bottom - top
    };
}
