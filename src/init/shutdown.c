// Handles shutdown logic.

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
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <fcntl.h>
#include <err.h>
#include <errno.h>
#include <time.h>

static inline void ksleep(unsigned int seconds) {
	struct timespec req, rem;
	req.tv_sec = seconds;
	req.tv_nsec = 0;
	while (nanosleep(&req, &rem) == -1 && errno == EINTR) {
		req = rem;
	}
}
void stage2stopsys(void) {
	write(STDOUT_FILENO, ": Sending SIGTERM to all processes\n", 35);
	kill(-1, SIGTERM);
	ksleep(3);
	write(STDOUT_FILENO, ": Sending SIGKILL to remaining processes\n", 41);
	kill(-1, SIGKILL);
	for (;;) {
		pid_t reaped = waitpid(-1, NULL, 0);
		if (reaped < 0) {
			if (errno == ECHILD)
				break;
			continue;
		}
	}
retry:
	pid_t stage2 = fork();
	if (stage2 < 0) {
		warnx("Fork failed, retrying");
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
#ifdef __linux__
		execl("/bin/sh", "/bin/sh", S2_STOPSYS, "linux", (char *) NULL);
#else
		execl("/bin/sh", "/bin/sh", S2_STOPSYS, (char *) NULL);
#endif
		perror("init: stage2stopsys execl failed");
		_exit(1);
	}
}

[[noreturn]] void kstop(const sig_atomic_t action) {
#if defined(__linux__)
# include <sys/reboot.h>
# include <linux/reboot.h>
# define ENABLE_CAD() reboot(LINUX_REBOOT_CMD_CAD_ON)
# define HALT() reboot(LINUX_REBOOT_CMD_HALT)
# define REBOOT() reboot(LINUX_REBOOT_CMD_RESTART)
# define POWEROFF() reboot(LINUX_REBOOT_CMD_POWER_OFF)
#elif defined(__FreeBSD__) || defined(__NetBSD__)
# include <sys/reboot.h>
# define ENABLE_CAD() ((void) 0)
# ifdef __FreeBSD__
#  define HALT() reboot(RB_HALT)
#  define REBOOT() reboot(RB_AUTOBOOT)
#  define POWEROFF() reboot(RB_POWEROFF)
# else
#  define HALT() reboot(RB_HALT, NULL)
#  define REBOOT() reboot(RB_AUTOBOOT, NULL)
#  define POWEROFF() reboot(RB_POWERDOWN, NULL)
# endif
#else
# error("Unsupported OS!")
#endif
	ENABLE_CAD();
	if (action == ACTION_REBOOT) {
		write(STDOUT_FILENO, "Now rebooting system\n", 21);
		REBOOT();
	} else if (action == ACTION_HALT) {
		write(STDOUT_FILENO, "Now halting system\n", 19);
		HALT();
	} else if (action == ACTION_POWEROFF) {
		write(STDOUT_FILENO, "Now shutting down system\n", 25);
		POWEROFF();
	}
	write(STDERR_FILENO, "Either the reboot/halt/poweroff call failed,\n", 45);
	write(STDERR_FILENO, "or the option wasn't specified correctly.\n", 42);
	write(STDERR_FILENO, "Either way, going into infinite loop. Treat as a halt.\n", 55);
	HALT();
infloop:
	for(;;)
		pause();
	goto infloop;
}
