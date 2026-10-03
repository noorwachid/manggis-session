#pragma once

#include "core/Unit.hpp"
#include "backend/IWindowBackend.hpp"
#include "backend/X11Connection.hpp"
#include "backend/X11Hint.hpp"
#include <xcb/xproto.h>

class X11WindowBackend: public IWindowBackend {
public:
	X11WindowBackend(X11Connection* conn, X11Hint* hint, xcb_window_t handle);

	~X11WindowBackend();

	virtual Rect getRect() override;

	virtual void setProp(WindowProp prop) override;

	virtual WindowProp getProp() override;

	virtual WindowType getType() override;

	virtual wx::String getTitle() override; 

	virtual void setState(WindowState state) override;

	virtual WindowState getState() override;

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

	bool _detectClientTransparency();

	xcb_visualid_t _findARGBVisual(uint8_t& depth) const;

	uint32_t _opaqueColor(uint32_t color) const;

	void _clearClientArea();


private:
	X11Connection* _conn;
	X11Hint* _hint;
	xcb_window_t _client;
	xcb_window_t _frame;
	xcb_gcontext_t _gc;

	WindowProp _prop;
	WindowState _state;

	// Add to X11WindowBackend class

	xcb_visualid_t _frameVisual = XCB_NONE;
	uint8_t _frameDepth = 0;
	xcb_colormap_t _frameColormap = XCB_NONE;

	// True when the client uses a 32-bit ARGB-capable visual.
	bool _clientSupportsTransparency = false;

};



