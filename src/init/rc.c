// The functions that invoke rc and rc.shutdown for service handling.

// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 BLT Sandwich

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.*

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.

// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include "scripts.h"
#include "action.h"
#include "tty.h"
#include "rc.h"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdbool.h>

bool startServices(void) {
retry:;
	// Yes I know there should be a fork attempts guard here, that's for future me to deal with
	pid_t services = fork();
	if (services < 0) {
		broadcast("Fork failed, retrying\n");
		goto retry;
	} else if (services > 0) {
		pid_t reapedServices;
		int status = -1;
		while ((reapedServices = waitpid(-1, &status, 0)) != services) {
			if (reapedServices < 0) {
				if (errno == ECHILD)
					break;
				continue;
			}
		}
		if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
			return true;
		else
			return false;
	} else {
		sigUnblock();
		// We closed stdio when init was first executed, so we need to re-open it every time we exec a shell script
		int fd = open("/dev/console", O_RDWR);
		if (fd != -1) {
			dup2(fd, STDIN_FILENO);
			dup2(fd, STDOUT_FILENO);
			dup2(fd, STDERR_FILENO);
			if (fd > STDERR_FILENO) close(fd);
		}
		execl("/bin/sh", "/bin/sh", RC, (char *) NULL);
		int err = errno;
		broadcast("init: rc execl failed: %s\n", strerror(err));
		_exit(1);
	}
}

void stopServices(const char *restrict msg) {
retry:;
	// Same deal here, I'm sure I'll come around to fixing it...
	pid_t services = fork();
	if (services < 0) {
		broadcast("Fork failed, retrying\n");
		goto retry;
	} else if (services > 0) {
		pid_t reapedServices;
		while ((reapedServices = waitpid(-1, NULL, 0)) != services) {
			if (reapedServices < 0) {
				if (errno == ECHILD)
					break;
				continue;
			}
		}
	} else {
		sigUnblock();
		// Again, same deal, stdio doesn't exist, we have to open it ourselves
		int fd = open("/dev/console", O_RDWR);
		if (fd != -1) {
			dup2(fd, STDIN_FILENO);
			dup2(fd, STDOUT_FILENO);
			dup2(fd, STDERR_FILENO);
			if (fd > STDERR_FILENO) close(fd);
		}
		execl("/bin/sh", "/bin/sh", RC_SHUT, msg, (char *) NULL);
		int err = errno;
		broadcast("init: rc.shutdown execl failed: %s\n", strerror(err));
		_exit(1);
	}
}
