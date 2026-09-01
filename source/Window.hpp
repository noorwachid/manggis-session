#pragma once

#include "Backend/IWindowBackend.hpp"

class Monitor;
class Workspace;

class Window {
public:
	Window(IWindowBackend* backend);

	~Window();

	IWindowBackend* GetBackend();

	WindowProp GetProp();

	WindowType GetType();

	Wx::String GetTitle();

	void SetWorkspace(Workspace* workspace);

	Workspace* GetWorkspace();

	void SetMonitor(Monitor* monitor);

	Monitor* GetMonitor();

	Rect GetRect();

	void Raise();

	void Move(int x, int y);

	void Resize(int width, int height);

	void Show();

	void Hide();

	void Focus();

	void Blur();

	void Close();

private:
	IWindowBackend* _backend;
	Monitor* _monitor = nullptr;
	Workspace* _workspace = nullptr;
};
