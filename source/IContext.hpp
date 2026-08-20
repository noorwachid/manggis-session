#pragma once

#include "Common.hpp"
#include "IObserver.hpp"
#include "input/Key.hpp"
#include "input/MouseButton.hpp"

class IContext {
public:
	virtual void bindMouseButton(MouseButton button, ModKey modKey) = 0;

	virtual void bindMouseMovement() = 0;

	virtual void unbindMouseMovement() = 0;

	virtual Point getMousePos() = 0;

	virtual void listen(IObserver* observer) = 0;
};

IContext* createContext();
