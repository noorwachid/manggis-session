#pragma once

// NOTE: 
// this is used to abstract the wm observer
// like to register the correct x11 hints foreach action without clutering the main logic

class IWMObserver {
public:
	virtual void onMonitorActive();

	virtual void onWorkspaceActive();

	virtual void onWindowActive();
};
