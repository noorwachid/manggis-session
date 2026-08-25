#include "Window.hpp"

Window::Window(IWindowBackend* backend, WindowFlags flags): 
	_backend{backend}, _flags{flags} {
}

Window::~Window() {
	delete _backend;
}

IWindowBackend* Window::getBackend() {
	return _backend;
}

WindowProp Window::getProp() {
	return _backend->getProp();
}

WindowType Window::getType() {
	return _backend->getType();
}

void Window::setWorkspace(Workspace* workspace) {
	_workspace = workspace;
}

Workspace* Window::getWorkspace() {
	return _workspace;
}

void Window::setMonitor(Monitor* monitor) {
	_monitor = monitor;
}

Monitor* Window::getMonitor() {
	return _monitor;
}

Rect Window::getRect() {
	return _backend->getRect();
}

void Window::raise() {
	_backend->raise();
}

void Window::move(int x, int y) {
	_backend->move(x, y);
}

void Window::resize(int width, int height) {
	_backend->resize(width, height);
}

void Window::show() {
	_backend->show();
}

void Window::hide() {
	_backend->hide();
}

void Window::focus() {
	_backend->focus();
}

void Window::blur() {
	_backend->blur();
}

void Window::close() {
	_backend->close();
}
