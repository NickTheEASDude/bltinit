// A very simple daemonizing program, also writes a pidfile for the process it launches.

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

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
	if (argc < 3) {
		fprintf(stderr, "NOT ENOUGH ARGUMENTS\n");
		fprintf(stderr, "Usage: %s <pidfile> <command> [cmd_args]\n", argv[0]);
		return 1;
	}

	pid_t child1 = fork();
	if (child1 < 0) {
		perror("rundaemon: child1 fork failed");
		return 1;
	} else if (child1 > 0)
		waitpid(child1, NULL, 0);
	else {
		pid_t sid = setsid();
		if (sid < 0) {
			perror("rundaemon: child1 setsid failed");
			_exit(2);
		}
		pid_t child2 = fork();
		if (child2 < 0) {
			perror("rundaemon: child2 fork failed");
			_exit(3);
		} else if (child2 > 0)
			_exit(0);
		else {
			int pidFile = open(
				argv[1],
				O_RDWR |
				O_CREAT |
				O_TRUNC,
				S_IRUSR |
				S_IWUSR |
				S_IRGRP |
				S_IROTH
			);
			if (pidFile < 0) {
				perror("rundaemon: unable to create pidfile");
				_exit(4);
			}
			dprintf(pidFile,"%d\n", (int) getpid());
			close(pidFile);
			char *newArgv[argc - 1];
			for (int i = 2; i < argc; i++)
				newArgv[i - 2] = argv[i];
			newArgv[argc - 2] = NULL;
			int fd = open("/dev/null", O_RDWR);
			if (fd != -1) {
				dup2(fd, STDIN_FILENO);
				dup2(fd, STDOUT_FILENO);
				dup2(fd, STDERR_FILENO);
				if (fd > STDERR_FILENO)
					close(fd);
			}
			execvp(newArgv[0], newArgv);
			perror("rundaemon: child2 execvp failed");
			unlink(argv[1]);
			_exit(1);
		}
	}
	return 0;
}
