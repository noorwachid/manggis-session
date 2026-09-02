#pragma once

#include "core/Unit.hpp"

namespace Helper {
	constexpr int min(int a, int b) {
		return a < b ? a : b;
	}
	constexpr int max(int a, int b) {
		return a > b ? a : b;
	}
};

constexpr bool isInsideRange(int x, int min, int max) {
	return x >= min && x <= max;
}

constexpr bool isPointIntersectRect(Point point, Rect rect) {
	return (
		point.x >= rect.x && 
		point.x <= rect.x + rect.width && 
		point.y >= rect.y && 
		point.y <= rect.y + rect.height
	);
}

constexpr Rect intersectRectRect(const Rect& monitor, const Rect& window) {
    const int left   = Helper::max(monitor.x, window.x);
    const int top    = Helper::max(monitor.y, window.y);
    const int right  = Helper::min(monitor.x + monitor.width, window.x + window.width);
    const int bottom = Helper::min(monitor.y + monitor.height, window.y + window.height);

    if (right <= left || bottom <= top)
        return Rect{ 0, 0, 0, 0 };

    return Rect{
        left,
        top,
        right - left,
        bottom - top
    };
}
