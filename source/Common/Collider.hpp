#pragma once

#include "Common/Unit.hpp"
#include "Common/Collider.hpp"

namespace Helper {
	constexpr int Min(int a, int b) {
		return a < b ? a : b;
	}
	constexpr int Max(int a, int b) {
		return a > b ? a : b;
	}
};

constexpr bool IsInsideRange(int x, int min, int max) {
	return x >= min && x <= max;
}

constexpr bool IsPointIntersectRect(Point point, Rect rect) {
	return (
		point.x >= rect.x && 
		point.x <= rect.x + rect.width && 
		point.y >= rect.y && 
		point.y <= rect.y + rect.height
	);
}

constexpr Rect IntersectRectRect(const Rect& monitor, const Rect& window) {
    const int left   = Helper::Max(monitor.x, window.x);
    const int top    = Helper::Max(monitor.y, window.y);
    const int right  = Helper::Min(monitor.x + monitor.width, window.x + window.width);
    const int bottom = Helper::Min(monitor.y + monitor.height, window.y + window.height);

    if (right <= left || bottom <= top)
        return Rect{ 0, 0, 0, 0 };

    return Rect{
        left,
        top,
        right - left,
        bottom - top
    };
}
