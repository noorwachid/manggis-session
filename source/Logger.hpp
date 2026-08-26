#pragma once

#include <stdio.h>
#include <stdarg.h>

class Logger {
public:
	static void init() {
		_handle = fopen("/tmp/manggis-session.log", "w");
	}

	static void shutdown() {
		fclose(_handle);
	}

	static void log(const char *fmt, ...) {
		va_list args;
		va_start(args, fmt);

		vfprintf(_handle, fmt, args);

		va_end(args);

		fflush(_handle);
	}

private:
	static FILE* _handle;
};

#define LOG(...) Logger::log(__VA_ARGS__)

