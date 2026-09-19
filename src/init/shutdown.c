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

#include "shutdown.h"
#include "action.h"
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <err.h>
#include <errno.h>

void stage2stopsys(void) {
retry:
	pid_t stage2 = fork();
	if (stage2 < 0) {
		warnx("Fork failed, retrying");
		goto retry;
	} else if (stage2 > 0) {
		pid_t reapedStage2;
		while ((reapedStage2 = waitpid(-1, NULL, 0)) != stage2) {
			if (stage2 < 0) {
				if (errno == ECHILD)
					break;
				continue;
			}
		}
	} else {
		execl("/usr/libexec/stage2stopsys", "/usr/libexec/stage2stopsys", (char *) NULL);
		perror("init: stage2stopsys execl failed");
		_exit(1);
	}
}

[[noreturn]] void kstop(const sig_atomic_t action) {
#if defined(__linux__)
# include <sys/reboot.h>
# include <linux/reboot.h>
# define ENABLE_CAD reboot(LINUX_REBOOT_CMD_CAD_ON)
# define HALT reboot(LINUX_REBOOT_CMD_HALT)
# define REBOOT reboot(LINUX_REBOOT_CMD_RESTART)
# define POWEROFF reboot(LINUX_REBOOT_CMD_POWER_OFF)
#endif
	ENABLE_CAD;
	if (action == ACTION_REBOOT) {
		write(STDOUT_FILENO, "Now rebooting system\n", 21);
		REBOOT;
	} else if (action == ACTION_HALT) {
		write(STDOUT_FILENO, "Now halting system\n", 19);
		HALT;
	} else if (action == ACTION_POWEROFF) {
		write(STDOUT_FILENO, "Now shutting down system\n", 25);
		POWEROFF;
	}
	write(STDERR_FILENO, "Either the reboot/halt/poweroff call failed,\n", 45);
	write(STDERR_FILENO, "or the option wasn't specified correctly.\n", 42);
	write(STDERR_FILENO, "Either way, going into infinite loop. Treat as a halt.\n", 55);
	HALT;
infloop:
	for(;;)
		pause();
	goto infloop;
}
