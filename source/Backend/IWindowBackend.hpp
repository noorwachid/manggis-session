#pragma once

#include "Common/Unit.hpp"
#include "Wx/Core/String.hpp"
#include <cstdint>

struct WindowProp {
	uint32_t borderColor = 0xDF1238;
	uint32_t borderWidth = 1;
	uint32_t titlebarColor = 0xFF3258; 
	uint32_t titlebarHeight = 20; 
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
	Unknown,
	Modal,
	Sticky,
	MaximizedVertical,
	MaximizedHorizontal,
	Shaded,
	SkipTaskbar,
	SkipPager,
	Hidden,
	Fullscreen,
	Above,
	Below,
	DemandsAttention,
};

class IWindowBackend {
public:
	virtual Rect GetRect() = 0;

	virtual WindowProp GetProp() = 0;

	virtual WindowType GetType() = 0;

	virtual Wx::String GetTitle() = 0;

	virtual void Raise() = 0;

	virtual void Move(int x, int y) = 0;

	virtual void Resize(int width, int height) = 0;

	virtual void Show() = 0;

	virtual void Hide() = 0;

	virtual void Focus() = 0;

	virtual void Blur() = 0;

	virtual void Close() = 0;

	virtual ~IWindowBackend() {
	}
};
