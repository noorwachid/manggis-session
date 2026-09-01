#pragma once

#include <stdio.h>
#include <stdarg.h>

#if defined(__GNUC__) || defined(__clang__)
#  define FORWARD_PRINTF(fmt, args) __attribute__((format(printf, fmt, args)))
#else
#  define FORWARD_PRINTF(fmt, args)
#endif

class Logger {
public:
	static void Init() {
		_handle = fopen("/tmp/manggis-session.log", "w");
	}

	static void Shutdown() {
		fclose(_handle);
	}

	static void Log(const char *fmt, ...) FORWARD_PRINTF(1, 2) {
		va_list args;
		va_start(args, fmt);

		vfprintf(_handle, fmt, args);

		va_end(args);

		fflush(_handle);
	}

private:
	static FILE* _handle;
};

#define LOG(...) Logger::Log(__VA_ARGS__)

