#pragma once

#include "Common.hpp"
#include <cstdint>

struct WindowProp {
	uint32_t borderColor = 0xDF1238;
	uint32_t borderWidth = 1;
	uint32_t titlebarColor = 0xFF3258; 
	uint32_t titlebarHeight = 20; 
};

class IWindowBackend {
public:
	virtual Rect getRect() = 0;

	virtual WindowProp getProp() = 0;

	virtual void raise() = 0;

	virtual void move(int x, int y) = 0;

	virtual void resize(int width, int height) = 0;

	virtual void show() = 0;

	virtual void hide() = 0;

	virtual void activate() = 0;

	virtual void deactivate() = 0;

	virtual ~IWindowBackend() {
	}
};
