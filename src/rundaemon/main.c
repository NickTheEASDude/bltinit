// A very simple daemonizing program.

// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 BLT Sandwich

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.*

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#define _GNU_SOURCE // for pipe2(2)
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <syslog.h>
#include <errno.h>

int main(int argc, char *argv[]) {
	int err;
	openlog("rundaemon", LOG_PID | LOG_CONS | FD_CLOEXEC, LOG_DAEMON);
	if (argc < 2) {
		fprintf(stderr, "NOT ENOUGH ARGUMENTS\n");
		fprintf(stderr, "Usage: %s <command> [cmd_args]...\n", argv[0]);
		return 1;
	}

	// I'll explain this fully, because some people might not know how this process works.
	// This first fork is pretty normal, almost every invoked program has used this syscall
	pid_t child1 = fork();
	if (child1 < 0) { // Fail
		err = errno;
		syslog(LOG_ERR, "child1 fork failed: %s", strerror(err));
		fprintf(stderr, "rundaemon: child1 fork failed: %s\n", strerror(err));
		closelog();
		return 2;
	} else if (child1 > 0) { // Parent
		// We wait in the parent even though the first child should immediately exit, because it propagates the errors from the second child
		int status;
		waitpid(child1, &status, 0);
		if (WIFEXITED(status))
			return WEXITSTATUS(status);
		else
			return 0;
	} else { // Child
		// Now at this point we need to start detaching ourselves from the process that called this program.
		// This syscall sets a new session id (which is possible because we've already forked from any potential session leaders)
		pid_t sid = setsid();
		int pipeFD[2];
		// We set up a pipe here for exit code propagation
		if (pipe2(pipeFD, O_CLOEXEC) == -1) {
			err = errno;
			syslog(LOG_ERR, "child1 pipe failed: %s", strerror(err));
			fprintf(stderr, "rundaemon: child1 pipe failed: %s\n", strerror(err));
			_exit(3);
		}

		if (sid < 0) {
			err = errno;
			syslog(LOG_ERR, "child1 setsid failed: %s", strerror(err));
			fprintf(stderr, "rundaemon: child1 setsid failed: %s\n", strerror(err));
			closelog();
			_exit(4);
		}
		// Once the pipe is set up and we now know we are the session leader,
		// we fork again to be absolutely sure nothing from the old environment will cause issues
		pid_t child2 = fork();
		if (child2 < 0) { // Fail
			err = errno;
			syslog(LOG_ERR, "child2 fork failed: %s", strerror(err));
			fprintf(stderr, "rundaemon: child2 fork failed: %s\n", strerror(err));
			closelog();
			_exit(5);
		} else if (child2 > 0) { // Parent
			// We close the log and the write side of the pipe (since we're not using either now)
			closelog();
			close(pipeFD[1]);
			int cErr = 0;
			// Read the pipe for any potential errors
			ssize_t byteCount = read(pipeFD[0], &cErr, sizeof(cErr));
			// Since we opened it with CLOEXEC, if the exec call in the second child succeeds, the pipe will close, and no bytes will be sent
			if (byteCount == 0) {
				close(pipeFD[0]);
				_exit(0);
			} else if (byteCount > 0) { // Otherwise, something happened
				if (cErr == 0) {
					read(pipeFD[0], &cErr, sizeof(cErr));
					fprintf(stderr, "rundaemon: child2 execvp failed: %s\n", strerror(cErr));
					close(pipeFD[0]);
					_exit(7);
				} else {
					close(pipeFD[0]);
					_exit(cErr);
				}
			} else
				_exit(255);
		} else { // Child
			// Now we're fully detached from the environment we were started from
			close(pipeFD[0]);
			char *newArgv[argc];
			// Shift everything in argv to the left by 1 and discard the old argv[0]
			for (int i = 1; i < argc; i++)
				newArgv[i - 1] = argv[i];
			newArgv[argc - 1] = NULL;
			// We redirect stdio to /dev/null because I don't know a good way to redirect to a logfile or a journal...
			int fd = open("/dev/null", O_RDWR);
			if (fd != -1) {
				dup2(fd, STDIN_FILENO);
				dup2(fd, STDOUT_FILENO);
				dup2(fd, STDERR_FILENO);
				if (fd > STDERR_FILENO)
					close(fd);
			}
			syslog(LOG_INFO, "Running %s as PID %d", newArgv[0], getpid());
			execvp(newArgv[0], newArgv);
			err = errno;
			syslog(LOG_ERR, "child2 execvp failed: %s", strerror(err));
			// There's probably a better way to implement this, but this works well enough
			const int sendErr = 0;
			write(pipeFD[1], &sendErr, sizeof(sendErr));
			write(pipeFD[1], &err, sizeof(err));
			close(pipeFD[1]);
			closelog();
			_exit(6);
		}
	}
	return 0;
}
