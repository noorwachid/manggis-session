#pragma once

#include "Common.hpp"

class IWindow {
public:
	virtual Rect getRect() = 0;

	virtual void raise() = 0;

	virtual void move(int x, int y) = 0;

	virtual void resize(int width, int height) = 0;

	virtual void show() = 0;

	virtual void hide() = 0;

	virtual ~IWindow() {
	}
};
