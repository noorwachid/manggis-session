#pragma once

#include "core/Unit.hpp"
#include "backend/IWindowBackend.hpp"
#include "backend/X11Connection.hpp"
#include "backend/X11Hint.hpp"
#include <xcb/xproto.h>

namespace WindowStyle {
	constexpr int borderWidth = 1;
	constexpr int titlebarHeight = 20;

	namespace Active {
		#ifdef XEPHYR_ENABLE
		constexpr uint32_t borderColor = 0x1238DF;
		constexpr uint32_t titlebarColor = 0x3258FF;
		#else
		constexpr uint32_t borderColor = 0xDF1238;
		constexpr uint32_t titlebarColor = 0xFF3258;
		#endif
	}

	namespace Inactive {
		constexpr uint32_t borderColor = 0x888888;
		constexpr uint32_t titlebarColor = 0x888888;
	}
}

class X11WindowBackend: public IWindowBackend {
public:
	X11WindowBackend(X11Connection* conn, X11Hint* hint, xcb_window_t handle);

	virtual Rect getRect() override;

	virtual WindowProp getProp() override;

	virtual WindowType getType() override;

	virtual wx::String getTitle() override; 

	virtual void raise() override;

	virtual void move(int x, int y) override;

	virtual void resize(int width, int height) override;

	virtual void show() override;

	virtual void hide() override;

	virtual void focus() override;

	virtual void blur() override;

	virtual void close() override;

	xcb_window_t getFrameHandle();

	xcb_window_t getClientHandle();

	void draw();

	void notifyClient();


private:
	void _reparent();

private:
	X11Connection* _conn;
	X11Hint* _hint;
	xcb_window_t _client;
	xcb_window_t _frame;
	xcb_gcontext_t _gc;

	WindowProp _prop;
};



