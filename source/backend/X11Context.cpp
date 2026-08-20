#include "IContext.hpp"
#include "backend/X11Context.hpp"

IContext* createContext() {
	return new X11Context();
}

