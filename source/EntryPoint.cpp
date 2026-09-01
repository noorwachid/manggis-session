#include "Common/Logger.hpp"
#include "Config/Config.hpp"
#include "Config/ConfigParser.hpp"
#include "Session.hpp"
#include "Wx/Core/String.hpp"
#include "Wx/Serializer/WkParser.hpp"
#include "Wx/Serializer/WkValue.hpp"
#include <cstdio>
#include <cstdlib>
#include <unistd.h>

Wx::String ReadFile(Wx::String const& path) {
	FILE* file = fopen(path.CPtr(), "r");
	if (!file) return "";

	fseek(file, 0, SEEK_END);
	int size = ftell(file);
	fseek(file, 0, SEEK_SET);
	Wx::String content;
	content.Reserve(size);
	for (int i = 0; i < size; i++) {
		Wx::Byte nullByte = '\0';
		content.Append(1, &nullByte);
	}
	fread(content.Ptr(), 1, size, file);
	fclose(file);
	return content;
}


int main(int argc, char **argv) {
	Logger::Init();

	LOG("hello world\n");

	Config config;

	char* home = getenv("HOME");
	Wx::String configPath = home;
	configPath.Append("/.config/manggis-session/config.wk");

	#ifdef XEPHYR_ENABLE
	configPath = "runtime/config.wk";
	#endif

	if (access(configPath.CPtr(), F_OK) == 0) {
		Wx::WkValue wkValue = Wx::ParseWk(ReadFile(configPath));
		config = ParseConfig(wkValue);
	}

	Session session;
	session.Run(config);

	Logger::Shutdown();

	return 0;
}
