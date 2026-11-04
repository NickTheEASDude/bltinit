// Logic for console spawning and message broadcasting.

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

// NOTE: AI was used in the writing of this file and its corresponding header file.
// However, the author can assure you that this code has been thoroughly studied and tested by the author,
// and has proven to be fully functional without major bugs.
// Do note that further testing is encouraged for anybody who uses this program.

#define _POSIX_C_SOURCE 200809L
#include "tty.h"
#include "action.h"
#include <signal.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <errno.h>

console_t consoles[MAX_CONSOLES];
int nConsoles = 0;

// This is used throughout init, all it does is open /dev/console, do a formatted write to it, and then close it
void broadcast(const char *restrict format, ...) {
	va_list args;
	va_start(args, format);
	int fd = open("/dev/console", O_WRONLY);
	if (fd != -1) {
		// Yes I'm aware vdprintf isn't fully portable, but every libc I've thrown this at has it, so that's good enough for now
		vdprintf(fd, format, args);
		close(fd);
	}
	va_end(args);
}

// Some basic utility functions
static void trimLF(char *s) {
	size_t n = strlen(s);
	if (n && s[n - 1] == '\n') s[n - 1] = '\0';
}
void addConsole(console_t console) {
	consoles[nConsoles] = console;
	nConsoles++;
}

// Very strict around these parts as to what counts as a tty!
static bool ttyExists(const char *path) {
	struct stat st;
	if (stat(path, &st) != 0)
		return false;
	
	if (!S_ISCHR(st.st_mode))
		return false;
	else if (access(path, R_OK | W_OK) != 0)
		return false;
	else
		return true;
}

// This is where rc.consoles is actually parsed and loaded into the array of console_t structs
int loadConsoles(const char *path) {
	FILE *f = fopen(path, "r");
	if (!f) return -1;

	char line[1024];
	int n = 0;

	while (fgets(line, sizeof(line), f) && n < MAX_CONSOLES) {
		trimLF(line);
		char *p = line;
		while (isspace((unsigned char)*p)) p++;
		if (*p == '\0' || *p == '#') continue;

		char device[64], enabled[8], modeStr[8], prog[512];
		// Using sscanf is scary, but every variable is properly length-guarded,
		// although no checks for truncation are done (again, for future me to deal with)
		int matched = sscanf(p, "%63[^;];%7[^;];%7[^;];%511[^\n]",
			device, enabled, modeStr, prog);
		if (matched != 4) {
			// If every line is malformed, it'll treat it as empty
			broadcast("rc.consoles: malformed line %s, skipping\n", p);
			continue;
		}

		console_t *c = &consoles[n];
		memset(c, 0, sizeof(*c));
		snprintf(c->device, sizeof(c->device), "%s", device);
		// Check for valid column 2 entries
		if (
			enabled[0] == 'Y' ||
			enabled[0] == 'y' ||
			enabled[0] == 'E' ||
			enabled[0] == 'e' ||
			enabled[0] == 'N' ||
			enabled[0] == 'n'
		) snprintf(c->enabled, sizeof(c->enabled), "%s", enabled);
		else {
			broadcast("rc.consoles: line %s column 2 incorrect, skipping\n", p);
			continue;
		}
		// Check if we should even bother to register this tty
		if (c->enabled[0] == 'E' || c->enabled[0] == 'e') { // Only keep going if the tty exists
			if (!ttyExists(c->device)) continue;
		} else if (c->enabled[0] == 'N' || c->enabled[0] == 'n') // tty is disabled
			continue;
		// How is the lifecycle of the tty handled?
		c->mode = (modeStr[0] == 'R' || modeStr[0] == 'r') // Respawn if the process ever exits
			    ? SPAWN_REPEAT :
			    (modeStr[0] == 'S' || modeStr[0] == 's') // Only spawn once and never again
			    ? SPAWN_ONCE :
			    (modeStr[0] == 'A' || modeStr[0] == 'a') // Same as respawn, but wait for user to press enter before executing
			    ? SPAWN_ASK : SPAWN_ERR; // Everything else is an error
		if (c->mode == SPAWN_ERR) {
			broadcast("rc.consoles: line %s column 3 incorrect, skipping\n", p);
			continue;
		}
		// The actual program to run
		snprintf(c->prog, sizeof(c->prog), "%s", prog);
		c->pid = -1;
		n++;
	}

	fclose(f);
	nConsoles = n;
	return n;
}

// Despite the name, the program launched doesn't have to be getty, just a legacy name from when getty WAS hardcoded
bool execGetty(console_t *c) {
	int attempts = 0;
	pid_t childPID;
	// Actual miracle, I remembered to implement a fork attempts guard here!
	for (;;) {
		childPID = fork();
		if (childPID >= 0)
			break;
		attempts++;
		if (attempts >= 5) {
			broadcast("getty fork failed after 5 attempts, giving up.\n");
			return false;
		}
		// Remember how I said the libc sleep function sucks? Well clearly I didn't...
		sleep(1);
	}
	if (childPID > 0) {
		// This is basically treated as if it were a daemon, so we just record the child PID in the console struct and immediately exit
		c->pid = childPID;
		return true;
	} else {
		sigUnblock();
		// We need to completely detach from init, so that involves getting a new session id and controlling terminal
		setsid();
		char prog[513];
		strncpy(prog, c->prog, 512);
		char *args[513];
		int count = 0;
		char *tokPtr;
		for (char *tok = strtok_r(prog, " ", &tokPtr); tok != NULL; tok = strtok_r(NULL, " ", &tokPtr)) args[count++] = tok;
		args[count] = NULL;
		// We open the tty specified in the device field of the console struct
		int fd = open(c->device, O_RDWR | O_NOCTTY);
		if (fd < 0) {
			int err = errno;
			broadcast("init: console open failed: %s\n", strerror(err));
			_exit(1);
		}
		// And take over the tty (I checked, this is portable)
		if (ioctl(fd, TIOCSCTTY, 0) < 0) {
			int err = errno;
			broadcast("init: console takeover failed: %s\n", strerror(err));
			_exit(1);
		}
		// Open stdio on the console (in case the called program can't do that itself)
		dup2(fd, STDIN_FILENO);
		dup2(fd, STDOUT_FILENO);
		dup2(fd, STDERR_FILENO);
		if (fd > STDERR_FILENO) close(fd);
		// This is where that spawn ask logic comes into play
		if (c->mode == SPAWN_ASK) {
			write(STDOUT_FILENO, "Press enter to continue... ", 27);
			int userIn = getchar();
			while (userIn != '\n') {
				if (userIn < 0) {
					broadcast("\nEnd of file reached, continuing\n");
					break;
				}
				userIn = getchar();
			}
		}
		execv(args[0], args);
		int err = errno;
		broadcast("init: console execv failed: %s\n", strerror(err));
		_exit(1);
	}
}

// Spawn consoles on the consoles that were loaded in from loadConsoles
void spawnConsoles(void) {
	for (int i = 0; i < nConsoles; i++)
		execGetty(&consoles[i]);
}

// Process reaping, was offloaded here to also implement tty respawn logic
void consoleExit(pid_t reapPID) {
	for (int i = 0; i < nConsoles; i++) {
		if (consoles[i].pid != reapPID) continue;
		consoles[i].pid = -1;
		if (consoles[i].mode == SPAWN_REPEAT || consoles[i].mode == SPAWN_ASK)
			execGetty(&consoles[i]);
		return;
	}
}
