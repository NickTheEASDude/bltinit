// The functions that invoke rc and rc.shutdown for service handling.

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

#include "rc.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <err.h>
#include <errno.h>
#include <stdbool.h>

bool startServices() {
retry:
	pid_t services = fork();
	if (services < 0) {
		warnx("Fork failed, retrying");
		goto retry;
	} else if (services > 0) {
		pid_t reapedServices;
		int status;
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
		execl("/etc/rc", "/etc/rc", (char *) NULL);
		perror("init: rc execl failed");
		_exit(1);
	}
}

void stopServices() {
retry:
	pid_t services = fork();
	if (services < 0) {
		warnx("Fork failed, retrying");
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
		execl("/etc/rc.shutdown", "/etc/rc.shutdown", (char *) NULL);
		perror("init: rc.shutdown execl failed");
		_exit(1);
	}
}
