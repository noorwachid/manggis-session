#include "Exec.hpp"
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

bool exec(wx::List<wx::String> const& args) {
	wx::List<char const*> argv;
	argv.reserve(args.count() + 1);
	for (int i = 0; i < args.count(); i++) {
		argv.append(args[i].cptr());
	}
	argv.append(nullptr);

	pid_t pid = fork();
    if (pid < 0) {
        // fork() failed.
        return false;
    }

    if (pid == 0) {
        // Child process.
        //
        // execvp() replaces this process with the requested application.
        // If execvp() succeeds, this code is never reached.
        execvp(argv[0], const_cast<char* const*>(argv.ptr()));

        // execvp() failed.
        exit(127);
    }

    // parent process.
    return true;
}

