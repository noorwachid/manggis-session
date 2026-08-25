#pragma once

#include "Common.hpp"

inline bool isPointIntersectRect(Point point, Rect rect) {
	return (
		point.x >= rect.x && 
		point.x <= rect.x + rect.width && 
		point.y >= rect.y && 
		point.y <= rect.y + rect.height
	);
}

