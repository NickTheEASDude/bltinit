// The main logic, contains the process reaping loop and signal handlers.

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

#include "tty.h"
#include "rc.h"
#include "action.h"
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <linux/reboot.h>
#include <sys/reboot.h>

volatile sig_atomic_t reaped = ACTION_NORMAL;
volatile sig_atomic_t stopsys_condition = ACTION_NORMAL;

void handleSignals(int sig) {
	if (sig == SIGINT)
		stopsys_condition = ACTION_REBOOT;
	else if (sig == SIGUSR1)
		stopsys_condition = ACTION_HALT;
	else if (sig == SIGTERM)
		stopsys_condition = ACTION_POWEROFF;
	else if (sig == SIGCHLD)
		reaped = ACTION_REAP;
}

int main(int argc, char *argv[]) {
	if (getpid() != 1) {
		write(STDERR_FILENO, "This program must be run as PID 1 (init)\n", 41);
		return 1;
	}
	reboot(LINUX_REBOOT_CMD_CAD_OFF);
	struct sigaction sa;
	sa.sa_handler = handleSignals;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGUSR1, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGCHLD, &sa, NULL);
	
	if (startServices() == false) {
		broadcast("FATAL: /etc/rc exited abnormally, launching /bin/sh on primary console\n");
		loadConsoles("/usr/lib/rc/rc.fallback");
		goto multiSkip;
	}
	broadcast("Spawning consoles\n");
	if (loadConsoles("/etc/rc.consoles") <= 0) {
		broadcast("ERROR: rc.consoles empty, launching /bin/sh on primary console\n");
		
		loadConsoles("/usr/lib/rc/rc.fallback");
	}
multiSkip:
	spawnConsoles();
	for (;;) {
		pause();
		if (reaped == ACTION_REAP) {
			reaped = ACTION_NORMAL;
			pid_t reapPID;
			while ((reapPID = waitpid(-1, NULL, WNOHANG)) > 0)
				consoleExit(reapPID);
		}
		if (stopsys_condition != ACTION_NORMAL) {
			if (stopsys_condition == ACTION_REBOOT) {
				broadcast("Now rebooting system\n");
				stopServices();
				execl("/usr/libexec/stage2stopsys", "/usr/libexec/stage2stopsys", "reboot", (char *) NULL);
			} else if (stopsys_condition == ACTION_HALT) {
				broadcast("Now halting system\n");
				stopServices();
				execl("/usr/libexec/stage2stopsys", "/usr/libexec/stage2stopsys", "halt", (char *) NULL);
			} else if (stopsys_condition == ACTION_POWEROFF) {
				broadcast("Now shutting down system\n");
				stopServices();
				execl("/usr/libexec/stage2stopsys", "/usr/libexec/stage2stopsys", "shutdown", (char *) NULL);
			}
		}
	}
	return 1;
}
