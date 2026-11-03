// Handles shutdown logic.

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

#include "scripts.h"
#include "shutdown.h"
#include "action.h"
#include "tty.h"
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>

static inline void ksleep(unsigned int seconds) {
	struct timespec req, rem;
	req.tv_sec = seconds;
	req.tv_nsec = 0;
	while (nanosleep(&req, &rem) == -1 && errno == EINTR)
		req = rem;
}
void stage2stopsys(void) {
	broadcast(": Sending SIGTERM to all processes\n");
	kill(-1, SIGTERM);
	ksleep(3);
	broadcast(": Sending SIGKILL to remaining processes\n");
	kill(-1, SIGKILL);
	for (;;) {
		pid_t reaped = waitpid(-1, NULL, 0);
		if (reaped < 0) {
			if (errno == ECHILD)
				break;
			continue;
		}
	}
retry:;
	pid_t stage2 = fork();
	if (stage2 < 0) {
		broadcast("Fork failed, retrying\n");
		goto retry;
	} else if (stage2 > 0) {
		pid_t reapedStage2;
		while ((reapedStage2 = waitpid(-1, NULL, 0)) != stage2) {
			if (reapedStage2 < 0) {
				if (errno == ECHILD)
					break;
				continue;
			}
		}
	} else {
		sigUnblock();
		int fd = open("/dev/console", O_RDWR);
		if (fd != -1) {
			dup2(fd, STDIN_FILENO);
			dup2(fd, STDOUT_FILENO);
			dup2(fd, STDERR_FILENO);
			if (fd > STDERR_FILENO) close(fd);
		}
#ifdef __linux__
		execl("/bin/sh", "/bin/sh", S2_STOPSYS, "linux", (char *) NULL);
#else
		execl("/bin/sh", "/bin/sh", S2_STOPSYS, (char *) NULL);
#endif
		int err = errno;
		broadcast("init: stage2stopsys execl failed: %s\n", strerror(err));
		_exit(1);
	}
}

__attribute__((noreturn)) void kstop(const sig_atomic_t action) {
#if defined(__linux__)
# include <sys/reboot.h>
# include <linux/reboot.h>
# define ENABLE_CAD() reboot(LINUX_REBOOT_CMD_CAD_ON)
# define HALT() reboot(LINUX_REBOOT_CMD_HALT)
# define REBOOT() reboot(LINUX_REBOOT_CMD_RESTART)
# define POWEROFF() reboot(LINUX_REBOOT_CMD_POWER_OFF)
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__DragonFly__)
# include <sys/reboot.h>
# define ENABLE_CAD() ((void) 0)
# ifdef __NetBSD__
#  define HALT() reboot(RB_HALT, NULL)
#  define REBOOT() reboot(RB_AUTOBOOT, NULL)
#  define POWEROFF() reboot(RB_POWERDOWN, NULL)
# else
#  define HALT() reboot(RB_HALT)
#  define REBOOT() reboot(RB_AUTOBOOT)
#  define POWEROFF() reboot(RB_POWEROFF)
# endif
#else
# error("Unsupported OS!")
#endif
	ENABLE_CAD();
	if (action == ACTION_REBOOT) {
		broadcast("Now rebooting system\n");
		REBOOT();
	} else if (action == ACTION_HALT) {
		broadcast("Now halting system\n");
		HALT();
	} else if (action == ACTION_POWEROFF) {
		broadcast("Now shutting down system\n");
		POWEROFF();
	}
	broadcast("Either the reboot/halt/poweroff call failed,\n");
	broadcast("or the option wasn't specified correctly.\n");
	broadcast("Either way, going into infinite loop. Treat as a halt.\n");
	HALT();
infloop:
	for(;;)
		pause();
	goto infloop;
}
