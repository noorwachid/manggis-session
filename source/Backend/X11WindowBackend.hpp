#pragma once

#include "Common/Unit.hpp"
#include "Common/Logger.hpp"
#include "Backend/IWindowBackend.hpp"
#include "Backend/X11Connection.hpp"
#include "Backend/X11Hint.hpp"
#include <cstdlib>
#include <xcb/xproto.h>

namespace WindowStyle {
	constexpr int borderWidth = 5;
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

	virtual Rect GetRect() override;

	virtual WindowProp GetProp() override;

	virtual WindowType GetType() override;

	virtual Wx::String GetTitle() override; 

	virtual void Raise() override;

	virtual void Move(int x, int y) override;

	virtual void Resize(int width, int height) override;

	virtual void Show() override;

	virtual void Hide() override;

	virtual void Focus() override;

	virtual void Blur() override;

	virtual void Close() override;

	xcb_window_t GetFrameHandle();

	xcb_window_t GetClientHandle();

	uint32_t GetBorderColor();

	uint32_t GetBorderWidth();

	uint32_t GetTitlebarColor();

	uint32_t GetTitlebarHeight();

	void Draw();

	void NotifyClient();


private:
	void Reparent();

private:
	X11Connection* _conn;
	X11Hint* _hint;
	xcb_window_t _client;
	xcb_window_t _frame;
	xcb_gcontext_t _gc;

	WindowProp _prop;
};



