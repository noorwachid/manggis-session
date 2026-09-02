#pragma once

#include "backend/X11Connection.hpp"
#include "backend/X11WindowBackend.hpp"
#include <xcb/shape.h>
#include <xcb/xproto.h>

class X11Clipper {
public:
	X11Clipper(X11Connection* conn);

	void clip(X11WindowBackend* windowBackend, Rect rect);

	void unclip(X11WindowBackend* windowBackend);

private:
	X11Connection* _conn;
};
