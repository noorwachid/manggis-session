#include "Exec.hpp"
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

bool Exec(Wx::List<Wx::String> const& args) {
	Wx::List<char const*> argv;
	argv.Reserve(args.Count() + 1);
	for (int i = 0; i < args.Count(); i++) {
		argv.Append(args[i].CPtr());
	}
	argv.Append(nullptr);

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
        execvp(argv[0], const_cast<char* const*>(argv.Ptr()));

        // execvp() failed.
        exit(127);
    }

    // parent process.
    return true;
}

