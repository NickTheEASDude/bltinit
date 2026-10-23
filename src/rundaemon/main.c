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

	pid_t child1 = fork();
	if (child1 < 0) {
		err = errno;
		syslog(LOG_ERR, "child1 fork failed: %s", strerror(err));
		fprintf(stderr, "rundaemon: child1 fork failed: %s\n", strerror(err));
		closelog();
		return 2;
	} else if (child1 > 0) {
		int status;
		waitpid(child1, &status, 0);
		if (WIFEXITED(status))
			return WEXITSTATUS(status);
		else
			return 0;
	} else {
		pid_t sid = setsid();
		int pipeFD[2];
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
		pid_t child2 = fork();
		if (child2 < 0) {
			err = errno;
			syslog(LOG_ERR, "child2 fork failed: %s", strerror(err));
			fprintf(stderr, "rundaemon: child2 fork failed: %s\n", strerror(err));
			closelog();
			_exit(5);
		} else if (child2 > 0) {
			closelog();
			close(pipeFD[1]);
			int cErr = 0;
			ssize_t byteCount = read(pipeFD[0], &cErr, sizeof(cErr));
			if (byteCount == 0) {
				close(pipeFD[0]);
				_exit(0);
			} else if (byteCount > 0) {
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
		} else {
			close(pipeFD[0]);
			char *newArgv[argc];
			for (int i = 1; i < argc; i++)
				newArgv[i - 1] = argv[i];
			newArgv[argc - 1] = NULL;
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
