// Logic for console spawning and message broadcasting.

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

#include "tty.h"
#include <signal.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/ioctl.h>

console_t consoles[MAX_CONSOLES];
int nConsoles = 0;

void broadcast(const char *restrict format, ...) {
	va_list args;
	va_start(args, format);
	int tty0 = open("/dev/tty0", O_RDWR);
	if (tty0 < 0) tty0 = STDOUT_FILENO;
	vdprintf(tty0, format, args);
	va_end(args);
	if (tty0 != STDOUT_FILENO) close(tty0);
}

static void trimLF(char *s) {
	size_t n = strlen(s);
	if (n && s[n - 1] == '\n') s[n - 1] = '\0';
}

int loadConsoles(const char *path) {
	FILE *f = fopen(path, "r");
	if (!f) return -1;

	char line[512];
	int n = 0;

	while (fgets(line, sizeof(line), f) && n < MAX_CONSOLES) {
		trimLF(line);
		char *p = line;
		while (isspace((unsigned char)*p)) p++;
		if (*p == '\0' || *p == '#') continue;

		char device[64], baud[16], modeStr[8], prog[256];

		int matched = sscanf(p, "%63s %15s %7s %255[^\n]",
				      device, baud, modeStr, prog);
		if (matched != 4) {
			broadcast("rc.consoles: malformed line: %s\n", p);
			continue;
		}

		console_t *c = &consoles[n];
		memset(c, 0, sizeof(*c));
		snprintf(c->device, sizeof(c->device), "%s", device);
		snprintf(c->baud, sizeof(c->baud), "%s", baud);
		c->mode = (modeStr[0] == 'R' || modeStr[0] == 'r')
			    ? SPAWN_REPEAT : SPAWN_ONCE;
		snprintf(c->prog, sizeof(c->prog), "%s", prog);
		c->pid = -1;
		n++;
	}

	fclose(f);
	nConsoles = n;
	return n;
}

bool execGetty(console_t *c) {
	int attempts = 0;
	pid_t childPID;
	for (;;) {
		childPID = fork();
		if (childPID >= 0)
			break;
		attempts++;
		if (attempts >= 5) {
			broadcast("getty fork failed after 5 attempts, giving up.\n");
			return false;
		}
		sleep(1);
	}
	if (childPID > 0) {
		c->pid = childPID;
		return true;
	} else {
		char *args[7];
		args[0] = "/sbin/getty";
		int nextArg = 1;
		if (strcmp(c->prog, "default") != 0) {
			args[nextArg++] = "-n";
			args[nextArg++] = "-l";
			args[nextArg++] = c->prog;
		}
		args[nextArg++] = c->baud;
		args[nextArg++] = c->device;
		args[nextArg] = (char *) NULL;
		
		sigset_t signals;
		sigfillset(&signals);
		sigprocmask(SIG_UNBLOCK, &signals, NULL);

		struct sigaction sa;
		sa.sa_handler = SIG_DFL;
		sigemptyset(&sa.sa_mask);
		sa.sa_flags = 0;
		sigaction(SIGHUP, &sa, NULL);
		sigaction(SIGINT, &sa, NULL);
		sigaction(SIGQUIT, &sa, NULL);
		sigaction(SIGTERM, &sa, NULL);
		sigaction(SIGCHLD, &sa, NULL);
		
		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);
		
		execv("/sbin/getty", args);
		_exit(1);
	}
}

void spawnConsoles(void) {
	for (int i = 0; i < nConsoles; i++)
		execGetty(&consoles[i]);
}

void consoleExit(pid_t reapPID) {
	for (int i = 0; i < nConsoles; i++) {
		if (consoles[i].pid != reapPID) continue;
		consoles[i].pid = -1;
		if (consoles[i].mode == SPAWN_REPEAT)
			execGetty(&consoles[i]);
		return;
	}
}
