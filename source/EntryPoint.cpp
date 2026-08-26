#include "Logger.hpp"
#include "Session.hpp"
#include "config/Config.hpp"
#include "config/ConfigParser.hpp"
#include "wx/core/String.hpp"
#include "wx/serializer/WkParser.hpp"
#include "wx/serializer/WkValue.hpp"
#include <cstdio>
#include <cstdlib>
#include <unistd.h>

wx::String readFile(wx::String const& path) {
	FILE* file = fopen(path.cptr(), "r");
	if (!file) return "";

	fseek(file, 0, SEEK_END);
	int size = ftell(file);
	fseek(file, 0, SEEK_SET);
	wx::String content;
	content.reserve(size);
	for (int i = 0; i < size; i++) {
		wx::Byte nullByte = '\0';
		content.append(1, &nullByte);
	}
	fread(content.ptr(), 1, size, file);
	fclose(file);
	return content;
}


int main(int argc, char **argv) {
	Logger::init();

	LOG("hello world\n");

	Config config;

	char* home = getenv("HOME");
	wx::String configPath = home;
	configPath.append("/.config/manggis-session/config.wk");

	if (access(configPath.cptr(), F_OK) == 0) {
		wx::WkValue wkValue = wx::parseWk(readFile(configPath));
		config = parseConfig(wkValue);
	}

	Session session;
	session.run(config);

	Logger::shutdown();

	return 0;
}
