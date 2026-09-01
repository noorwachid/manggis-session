#include "Window.hpp"

Window::Window(IWindowBackend* backend): 
	_backend{backend} {
}

Window::~Window() {
	delete _backend;
}

IWindowBackend* Window::GetBackend() {
	return _backend;
}

WindowProp Window::GetProp() {
	return _backend->GetProp();
}

WindowType Window::GetType() {
	return _backend->GetType();
}

Wx::String Window::GetTitle() {
	return _backend->GetTitle();
}

void Window::SetWorkspace(Workspace* workspace) {
	_workspace = workspace;
}

Workspace* Window::GetWorkspace() {
	return _workspace;
}

void Window::SetMonitor(Monitor* monitor) {
	_monitor = monitor;
}

Monitor* Window::GetMonitor() {
	return _monitor;
}

Rect Window::GetRect() {
	return _backend->GetRect();
}

void Window::Raise() {
	_backend->Raise();
}

void Window::Move(int x, int y) {
	_backend->Move(x, y);
}

void Window::Resize(int width, int height) {
	_backend->Resize(width, height);
}

void Window::Show() {
	_backend->Show();
}

void Window::Hide() {
	_backend->Hide();
}

void Window::Focus() {
	_backend->Focus();
}

void Window::Blur() {
	_backend->Blur();
}

void Window::Close() {
	_backend->Close();
}
