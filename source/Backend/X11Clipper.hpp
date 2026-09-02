#pragma once

#include "Backend/X11Connection.hpp"
#include "Backend/X11WindowBackend.hpp"
#include <xcb/shape.h>
#include <xcb/xproto.h>

class X11Clipper {
public:
	X11Clipper(X11Connection* conn);

	void Clip(X11WindowBackend* windowBackend, Rect rect);

	void Unclip(X11WindowBackend* windowBackend);

private:
	X11Connection* _conn;
};
