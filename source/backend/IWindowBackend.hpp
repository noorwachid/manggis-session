#pragma once

#include "core/Unit.hpp"
#include "wx/core/String.hpp"
#include "wx/core/Flags.hpp"
#include <cstdint>

struct WindowProp {
	uint32_t borderColor = 0xDF1238;
	uint32_t borderWidth = 1;
	uint32_t titlebarColor = 0xFF3258; 
	uint32_t titlebarHeight = 20; 
	uint32_t buttonColor = 20; 
};

enum struct WindowType {
	Desktop,
	Dock,
	Toolbar,
	Menu,
	Utility,
	Splash,
	Dialog,
	DropdownMenu,
	PopupMenu,
	Tooltip,
	Notification,
	Combo,
	DragAndDrop,
	Normal,
};

enum struct WindowState {
	None = 0,
	Modal = 1 << 0,
	Sticky = 1 << 1,
	Maximized = 1 << 2,
	Fullscreen = 1 << 3,
	AlwaysOnTop = 1 << 4,
	AlwaysOnBottom = 1 << 5,
};

WX_FLAGS(WindowState)

class IWindowBackend {
public:
	virtual Rect getRect() = 0;

	virtual void setProp(WindowProp prop) = 0;

	virtual WindowProp getProp() = 0;

	virtual WindowType getType() = 0;

	virtual wx::String getTitle() = 0;

	virtual WindowState getState() = 0;

	virtual void setState(WindowState state) = 0;

	virtual void raise() = 0;

	virtual void move(int x, int y) = 0;

	virtual void resize(int width, int height) = 0;

	virtual void show() = 0;

	virtual void hide() = 0;

	virtual void focus() = 0;

	virtual void blur() = 0;

	virtual void close() = 0;

	virtual ~IWindowBackend() {
	}
};
