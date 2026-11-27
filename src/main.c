// The main logic, contains the process reaping loop and signal handlers.

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
#include "tty.h"
#include "rc.h"
#include "action.h"
#include "shutdown.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>
#include <syslog.h>
#include <string.h>

#ifdef __linux__
# include <sys/reboot.h>
# include <sys/mount.h>
# include <linux/reboot.h>
# include <errno.h>
#endif

volatile sig_atomic_t reaped = ACTION_NORMAL;
volatile sig_atomic_t stopsys_condition = ACTION_NORMAL;
volatile sig_atomic_t alarmTriggered = ACTION_NORMAL;

// We only set the sig atomics in the handler to be thread safe
void handleSignals(int sig) {
	if (sig == SIGINT)
		stopsys_condition = ACTION_REBOOT;
	else if (sig == SIGUSR1)
		stopsys_condition = ACTION_POWEROFF;
	else if (sig == SIGUSR2)
		stopsys_condition = ACTION_HALT;
	else if (sig == SIGCHLD)
		reaped = ACTION_REAP;
	else if (sig == SIGALRM)
		alarmTriggered = ACTION_ALARM;
}

int main(int argc, char *argv[]) {
	// There's no reason for a user to invoke this program, so we guard against that, since it could break things
	// (had to learn that the hard way...)
	if (getpid() != 1) {
		write(STDERR_FILENO, "This program must be run as PID 1 (init)\n", 41);
		return 1;
	}
	// Redirect stdio to null, needed if we want init messages to follow the virtual terminal
	int fd = open("/dev/null", O_RDWR);
	if (fd != -1) {
		dup2(fd, STDIN_FILENO);
		dup2(fd, STDOUT_FILENO);
		dup2(fd, STDERR_FILENO);
		if (fd > STDERR_FILENO) close(fd);
	}
	// Set up the main signal handler
	struct sigaction sa;
	sa.sa_handler = handleSignals;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGUSR1, &sa, NULL);
	sigaction(SIGUSR2, &sa, NULL);
	sigaction(SIGCHLD, &sa, NULL);
	sigaction(SIGALRM, &sa, NULL);
	if (argc > 1 && strcmp(argv[1], "stopsys") == 0) {
		// This only executes during poweroff/reboot/halt,
		// see where the re-exec occurs for the explanation as to why this needs to be done
		sa.sa_handler = SIG_IGN;
		sigaction(SIGINT, &sa, NULL);
		sigaction(SIGUSR1, &sa, NULL);
		sigaction(SIGUSR2, &sa, NULL);
		sigaction(SIGALRM, &sa, NULL);
		if (argc > 2) {
			if (strcmp(argv[2], "reboot") == 0)
				stopsys_condition = ACTION_REBOOT;
			else if (strcmp(argv[2], "poweroff") == 0)
				stopsys_condition = ACTION_POWEROFF;
			else if (strcmp(argv[2], "halt") == 0)
				stopsys_condition = ACTION_HALT;
		}
		goto stopsys;
	}
	// The BSDs do this for us, only linux needs to set this
#ifdef __linux__
	reboot(LINUX_REBOOT_CMD_CAD_OFF);
#endif
// This invokes /etc/rc using the system /bin/sh
	if (startServices() == false) {
		// If it fails, assume the worst has happened and only /dev/console exists as a tty
		broadcast("FATAL: rc exited abnormally, launching /bin/sh on primary console\n");
		loadConsoles(RC_FALL);
		goto multiSkip;
	}
	broadcast("Spawning consoles\n");
	if (loadConsoles(RC_CONS) <= 0) {
		// Same here
		broadcast("ERROR: rc.consoles empty, launching /bin/sh on primary console\n");
		
		loadConsoles(RC_FALL);
	}
multiSkip:
	spawnConsoles();
	// Set up signal sets for sigsuspend
	sigset_t blockMask, waitMask;
	sigemptyset(&blockMask);
	sigaddset(&blockMask, SIGINT);
	sigaddset(&blockMask, SIGUSR1);
	sigaddset(&blockMask, SIGUSR2);
	sigaddset(&blockMask, SIGCHLD);
	sigaddset(&blockMask, SIGALRM);
	sigemptyset(&waitMask);
	sigprocmask(SIG_BLOCK, &blockMask, NULL);

	// The main reaping loop
	for (;;) {
		// This puts init into a blocking state until a caught signal changes either of the sigatomic vars
		while (reaped == ACTION_NORMAL && stopsys_condition == ACTION_NORMAL && alarmTriggered == ACTION_NORMAL)
			sigsuspend(&waitMask);
		
		// SIGALRM, handle a failed getty fork
		if (alarmTriggered == ACTION_ALARM) {
			alarmTriggered = ACTION_NORMAL;
			retryConsoles();
		}

		// SIGCHLD, reap a dead process
		if (reaped == ACTION_REAP) {
			reaped = ACTION_NORMAL;
			pid_t reapPID;
			while ((reapPID = waitpid(-1, NULL, WNOHANG)) > 0)
				// If the dead process is a console from rc.consoles, handle it
				consoleExit(reapPID);
		}

		if (stopsys_condition != ACTION_NORMAL) {
			// Now we begin to get the system ready for poweroff/reboot/halt
			alarm(0);
			char *stopType = "";
			openlog("init", LOG_PID | LOG_CONS, LOG_DAEMON);
			if (stopsys_condition == ACTION_REBOOT) {
				broadcast("Now rebooting system\n");
				syslog(LOG_ALERT, "ALERT! System going down for reboot.");
				stopType = "reboot";
			} else if (stopsys_condition == ACTION_HALT) {
				broadcast("Now halting system\n");
				syslog(LOG_ALERT, "ALERT! System going down for halt.");
				stopType = "halt";
			} else if (stopsys_condition == ACTION_POWEROFF) {
				broadcast("Now shutting down system\n");
				syslog(LOG_ALERT, "ALERT! System going down for poweroff.");
				stopType = "poweroff";
			}
			closelog();
			// Run /etc/rc.shutdown using the system /bin/sh
			stopServices(stopType);
			// Empty all of the signals so the re-exec'd init has a sane signal state
			sigprocmask(SIG_SETMASK, &waitMask, NULL);
			// This is needed to clear any random open FDs or references to things that would stop / from remounting RO
			// (again, had to learn that the hard way...)
			alarm(0);
			execl(argv[0], argv[0], "stopsys", stopType, NULL);
			
		stopsys:
			// Run /usr/libexec/stage2stopsys using the system /bin/sh
			int err = stage2stopsys();
			if (err != 0) { // we handle the error here so we have access to argv
				broadcast("ERROR: Forking for %s failed: %s\n", S2_STOPSYS, strerror(err));
				broadcast("Falling back to emergency measures, system will %s in at least 2 seconds\n", argv[2]);
				sync();
				ksleep(1);
				sync();
				ksleep(1);
#ifdef __linux__ // BSDs don't require / to be remounted read-only (and remember, this is emergency code)
				if (mount(NULL, "/", NULL, MS_REMOUNT | MS_RDONLY, NULL) != 0) {
					err = errno;
					broadcast("Remounting / as read-only failed: %s\n", strerror(err));
				}
				sync();
				ksleep(1);
#endif
			}
			// Actually execute the reboot syscall with the given stopsys condition
			kstop(stopsys_condition);
		}
	}
	// This should never be reached
	return 1;
}
